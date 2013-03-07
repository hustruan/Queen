#include "Rasterizer.h"
#include "RenderDevice.h"
#include "FrameBuffer.h"
#include "Cache.hpp"
#include "threadpool.h"
#include "stack_pool.h"

using namespace RxLib;

#define InRange(v, a, b) ((a) <= (v) && (v) <= (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))

namespace {

inline int32_t iround(float x)
{
	return _mm_cvt_ss2si( _mm_load_ps( &x ) ); 
}

inline bool FCMP(float a, float b)
{
	return true;
}

inline void VS_Output_Copy(VS_Output* dest, const VS_Output* src, uint32_t numAttri)
{
	dest->Position =  src->Position;
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		dest->ShaderOutputs[i] = src->ShaderOutputs[i];
	}
}

inline void VS_Output_Sub(VS_Output* out, const VS_Output* a, const VS_Output* b, uint32_t numAttri)
{
	out->Position = a->Position - b->Position;
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		out->ShaderOutputs[i] = a->ShaderOutputs[i] - b->ShaderOutputs[i];
	}
}

inline void VS_Output_Add(VS_Output* out, const VS_Output* a, const VS_Output* b, uint32_t numAttri)
{
	out->Position = a->Position + b->Position;
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		out->ShaderOutputs[i] = a->ShaderOutputs[i] + b->ShaderOutputs[i];
	}
}

inline void VS_Output_Mul(VS_Output* out, const VS_Output* in, float val, uint32_t numAttri)
{
	out->Position = in->Position * val;
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		out->ShaderOutputs[i]  = in->ShaderOutputs[i] * val;
	}
}

inline void VS_Output_Difference(VS_Output* ddx, VS_Output* ddy, const VS_Output* v01, const VS_Output* v02, float invArea, uint32_t numAttri)
{
	const float v01XInvArea = v01->Position.X() * invArea;
	const float v02XInvArea = v02->Position.X() * invArea;
	const float v01YInvArea = v01->Position.Y() * invArea;
	const float v02YInvArea = v02->Position.Y() * invArea;

	ddx->Position = v01->Position * v02YInvArea - v02->Position * v01YInvArea;
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		ddx->ShaderOutputs[i] = v01->ShaderOutputs[i] * v02YInvArea - v02->ShaderOutputs[i] * v01YInvArea;
	}

	ddy->Position = v02->Position * v01XInvArea - v01->Position * v02XInvArea;
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		ddy->ShaderOutputs[i] = v02->ShaderOutputs[i] * v01XInvArea - v01->ShaderOutputs[i] * v02XInvArea;
	}
}

inline void VS_Output_BaryCentric(VS_Output* out, const VS_Output* base, const VS_Output* ddx, const VS_Output* ddy,
								 float offsetX, float offsetY, uint32_t numAttri)
{
	out->Position = base->Position + ddx->Position * offsetX + ddy->Position * offsetY;
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		out->ShaderOutputs[i]  = base->ShaderOutputs[i] + ddx->ShaderOutputs[i] * offsetX + ddy->ShaderOutputs[i] * offsetY;
	}
}

inline void VS_Output_ProjectAttrib(VS_Output* out, float val, uint32_t numAttri)
{
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		out->ShaderOutputs[i] = out->ShaderOutputs[i] * val;
	}
}

void VS_Output_Interpolate(VS_Output* out, const VS_Output* a, const VS_Output* b, float t, uint32_t numAttri)
{
	out->Position = Lerp(a->Position, b->Position, t);
	for (uint32_t i = 0; i < numAttri; ++i)
	{
		out->ShaderOutputs[i] = Lerp(a->ShaderOutputs[i], b->ShaderOutputs[i], t);
	}
}

}

//--------------------------------------------------------------------------------------------
Rasterizer::Rasterizer( RenderDevice& device )
	: RenderStage(device)
{
	// Near and Far plane
	mClipPlanes[0] = float4(0, 0, 1, 0);
	mClipPlanes[1] = float4(0, 0, -1, 1);

	const uint32_t nunWorkThreads = GetNumWorkThreads();

	mThreadPackage.resize(nunWorkThreads);
	mVertexCaches.resize(nunWorkThreads);
	mFacesThreads.resize(nunWorkThreads);
	mVerticesThreads.resize(nunWorkThreads);
	mNumVerticesThreads.resize(nunWorkThreads);
}

Rasterizer::~Rasterizer(void)
{
}

const VS_Output& Rasterizer::FetchVertex( uint32_t index, uint32_t threadIdx )
{
	VertexCacheElement& cacheItem = mVertexCaches[threadIdx][index & (VertexCacheSize-1)];

	if( cacheItem.Index != index )
	{		
		cacheItem.Index = index;
		cacheItem.Vertex = mDevice.FetchVertex(index);
	} 

	return cacheItem.Vertex;
}

void Rasterizer::ProjectVertex( VS_Output* vertex )
{
	if (vertex->Position.W() < std::numeric_limits<float>::epsilon())
		return;

	// perspective divide
	const float invW = 1.0f / vertex->Position.W();
	vertex->Position *= invW;

	auto mat = mDevice.mCurrentFrameBuffer->GetViewportMatrix();
	// viewport transform
	vertex->Position = vertex->Position * mDevice.mCurrentFrameBuffer->GetViewportMatrix();	

	// divide shader output registers by w; this way we can interpolate them linearly while rasterizing ...
	vertex->Position.W() = invW;
	
	VS_Output_ProjectAttrib(vertex, invW, mCurrVSOutputCount);
}

bool Rasterizer::BackFaceCulling( const VS_Output& v0, const VS_Output& v1, const VS_Output& v2 )
{
	// Do backface-culling ----------------------------------------------------
	if( mDevice.RasterizerState.PolygonCullMode == CM_None )
		return false;

	const float signedArea = (v1.Position.X()- v0.Position.X()) * (v2.Position.Y() - v0.Position.Y()) -
		(v1.Position.Y() - v0.Position.Y()) * (v2.Position.X()- v0.Position.X());


	if (signedArea >= 0.0f)
	{
		// polygon is CCW
		if (mDevice.RasterizerState.FrontCounterClockwise)
			return (mDevice.RasterizerState.PolygonCullMode == CM_Back);
		else
			return (mDevice.RasterizerState.PolygonCullMode == CM_Front);
	}
	else
	{
		// polygon is CW
		if (mDevice.RasterizerState.FrontCounterClockwise)
			return (mDevice.RasterizerState.PolygonCullMode == CM_Front);
		else
			return (mDevice.RasterizerState.PolygonCullMode == CM_Back);
	}
	

	return false;
}

uint32_t Rasterizer::ClipTriangle( VS_Output* clipped, const VS_Output& v0, const VS_Output& v1, const VS_Output& v2 )
{
	size_t srcStage = 0;
	size_t destStage = 1;

	stack_pool<VS_Output, 6> pool;
	uint32_t numClippedVertices[2];

	const VS_Output* pClipVertices[2][5];
	pClipVertices[0][0] = &v0;
	pClipVertices[0][1] = &v1;
	pClipVertices[0][2] = &v2;
	numClippedVertices[srcStage] = 3;

	for (size_t iPlane = 0; iPlane < mClipPlanes.size(); ++iPlane)
	{
		numClippedVertices[destStage] = 0;

		for( uint32_t i = 0, j = 1; i < numClippedVertices[srcStage]; ++i, ++j )
		{
			if( j == numClippedVertices[srcStage] ) // wrap over
				j = 0;

			float di, dj;
			di = Dot(mClipPlanes[iPlane], pClipVertices[srcStage][i]->Position);
			dj = Dot(mClipPlanes[iPlane], pClipVertices[srcStage][j]->Position);

			if (di >= 0.0f)
			{
				pClipVertices[destStage][numClippedVertices[destStage]++] = pClipVertices[srcStage][i];

				if (dj < 0.0f)
				{
					// InterpolateVertexShaderOutput
					VS_Output* newVSOutput = pool.malloc();
					VS_Output_Interpolate(newVSOutput, pClipVertices[srcStage][i], pClipVertices[srcStage][j], 
						di / (di - dj), mCurrVSOutputCount);
					
					pClipVertices[destStage][numClippedVertices[destStage]++] =newVSOutput;	
				}
			}
			else
			{
				if (dj >= 0.0f)
				{
					// InterpolateVertexShaderOutput
					VS_Output* newVSOutput = pool.malloc();
					VS_Output_Interpolate(newVSOutput, pClipVertices[srcStage][j], pClipVertices[srcStage][i], 
						dj / (dj - di), mCurrVSOutputCount);

					pClipVertices[destStage][numClippedVertices[destStage]++] = newVSOutput;
				}
			}
		}

		// cull out
		if (numClippedVertices[destStage] < 3)
		{
			return numClippedVertices[destStage];
		}

		//swap src and dest stage
		srcStage = (srcStage +1) & 1;
		destStage = (destStage +1) & 1;
	}

	const uint32_t resultNumVertices = numClippedVertices[srcStage];
	ASSERT(resultNumVertices <= 5);

	if (resultNumVertices == 5)
	{
		ASSERT(false);
	}
	
	for (uint32_t i = 0; i < resultNumVertices; ++i)
	{
		memcpy(&clipped[i], pClipVertices[srcStage][i], sizeof(VS_Output));
	}

	return resultNumVertices;
}

void Rasterizer::ClipTriangleTiled(  VS_Output* vertices, uint32_t threadIdx )
{
	size_t srcStage = 0;
	size_t destStage = 1;

	uint8_t clipVertices[2][5];
	clipVertices[srcStage][0] = 0;
	clipVertices[srcStage][1] = 1; 
	clipVertices[srcStage][2] = 2;

	uint8_t numClippedVertices[2];
	numClippedVertices[srcStage] = 3;

	uint8_t nunVert = numClippedVertices[srcStage];

	for (size_t iPlane = 0; iPlane < mClipPlanes.size(); ++iPlane)
	{
		numClippedVertices[destStage] = 0;
		
		uint8_t idxPrev = clipVertices[srcStage][0];
		float dpPrev = Dot(mClipPlanes[iPlane], vertices[idxPrev].Position);

		// wrap over
		clipVertices[srcStage][numClippedVertices[srcStage]] = clipVertices[srcStage][0];
		for (uint8_t i = 1; i <= numClippedVertices[srcStage]; ++i)
		{
			uint8_t idxCurr = clipVertices[srcStage][i];
			float dpCurr = Dot(mClipPlanes[iPlane], vertices[idxCurr].Position);

			if (dpPrev >= 0.0f)	
			{
				clipVertices[destStage][numClippedVertices[destStage]++] = idxPrev;

				if (dpCurr < 0.0f)
				{
					VS_Output_Interpolate(&vertices[nunVert], &vertices[idxPrev], &vertices[idxCurr], 
						dpPrev / (dpPrev - dpCurr), mCurrVSOutputCount);

					clipVertices[destStage][numClippedVertices[destStage]++] = nunVert++;	
				}
			}
			else
			{
				if (dpCurr >= 0.0f)
				{
					VS_Output_Interpolate(&vertices[nunVert], &vertices[idxCurr], &vertices[idxPrev], 
						dpCurr / (dpCurr - dpPrev), mCurrVSOutputCount);

					clipVertices[destStage][numClippedVertices[destStage]++] = nunVert++;	
				}
			}

			idxPrev = idxCurr;
			dpPrev = dpCurr;
		}

		// cull out
		if (numClippedVertices[destStage] < 3)
		{
			return;
		}

		//swap src and dest stage
		srcStage = (srcStage +1) & 1;
		destStage = (destStage +1) & 1;
	}

	const uint32_t resultNumVertices = numClippedVertices[srcStage];
	ASSERT(resultNumVertices <= 5);

	
	// Project the first three vertices for culling
	ProjectVertex( &vertices[clipVertices[srcStage][0]] );
	ProjectVertex( &vertices[clipVertices[srcStage][1]] );
	ProjectVertex( &vertices[clipVertices[srcStage][2]] );


	// We do not have to check for culling for each sub-polygon of the triangle, as they
	// are all in the same plane. If the first polygon is culled then all other polygons
	// would be culled, too.
	if( BackFaceCulling( vertices[clipVertices[srcStage][0]], vertices[clipVertices[srcStage][1]], vertices[clipVertices[srcStage][2]] ) )
	{
		// back face culled
		return;
	}

	// Project the remaining vertices
	for(uint32_t i = 3; i < resultNumVertices; ++i )
		ProjectVertex( &vertices[clipVertices[srcStage][i]] );


	// binning
	for( uint32_t i = 2; i < resultNumVertices; i++ )
	{
		Bin(vertices[clipVertices[srcStage][0]], vertices[clipVertices[srcStage][i-1]], vertices[clipVertices[srcStage][i]], threadIdx);
	}
}

void Rasterizer::Draw( PrimitiveType primitiveType, uint32_t primitiveCount )
{
	ASSERT(primitiveType == PT_Triangle_List); // Only support triangle list 

	pool& theadPool = GlobalThreadPool();
	uint32_t numWorkThreads = GetNumWorkThreads();

	// after frustum clip, one triangle can generate maximum 4 vertices
	mClippedVertices.resize(primitiveCount * 4);
	mClippedFaces.resize(primitiveCount);

	std::atomic<uint32_t> workingPackage(0);

	//input assembly, vertex shading, culling/clipping
	for (size_t i = 0; i < numWorkThreads - 1; ++i)
	{
		theadPool.schedule(std::bind(&Rasterizer::SetupGeometry, this, std::ref(mClippedVertices), std::ref(mClippedFaces), 
			std::ref(workingPackage), primitiveCount));
	}
	// schedule current thread
	SetupGeometry(std::ref(mClippedVertices), std::ref(mClippedFaces), std::ref(workingPackage), primitiveCount);
	theadPool.wait();


	// binning, dispatch primitive to tiles
	//std::vector<FrameBuffer::Tile>& tiles = mDevice.GetCurrentFrameBuffer()->mTiles;

	//workingPackage = 0;
	//for (size_t i = 0; i < theadPool.size(); ++i)
	//{
	//	theadPool.schedule(std::bind(&Rasterizer::Binning, this/*, std::ref(mClippedVertices), std::ref(mClippedFaces)*/, 
	//		std::ref(workingPackage), primitiveCount));
	//}
	
	// rasterize tiles
	for(size_t i = 0; i < mClippedFaces.size(); ++i)
	{
		if(mClippedFaces[i].TriCount == 1)
		{
			const VS_Output& v0 = mClippedVertices[mClippedFaces[i].Indices[0]];
			const VS_Output& v1 = mClippedVertices[mClippedFaces[i].Indices[1]];
			const VS_Output& v2 = mClippedVertices[mClippedFaces[i].Indices[2]];

			RasterizeTriangle(v0, v1, v2);
		}

	}
}

void Rasterizer::SetupGeometry( std::vector<VS_Output>& outVertices, std::vector<RasterFaceInfo>& outFaces, std::atomic<uint32_t>& workingPackage, uint32_t primitiveCount )
{
	uint32_t numPackages = (primitiveCount + SetupGeometryPackageSize - 1) / SetupGeometryPackageSize;
	uint32_t localWorkingPackage  = workingPackage ++;

	//LRUCache<uint32_t, VS_Output, VertexCacheSize> vertexCache(std::bind(&RenderDevice::FetchVertex, &mDevice, std::placeholders::_1));
	
	DirectMapCache<uint32_t, VS_Output, VertexCacheSize> vertexCache(std::bind(&RenderDevice::FetchVertex, &mDevice, std::placeholders::_1));

	while (localWorkingPackage < numPackages)
	{
		const uint32_t start = localWorkingPackage * SetupGeometryPackageSize;
		const uint32_t end = (std::min)(primitiveCount, start + SetupGeometryPackageSize);

		for (uint32_t iPrim = start; iPrim < end; ++iPrim)
		{
			const uint32_t baseVertex = iPrim * 4;
			const uint32_t baseFace = iPrim;

			// fetch vertices
			const VS_Output* pVSOutputs[3];
			uint32_t iVertex;
			for (iVertex = 0; iVertex < 3; ++ iVertex)
			{		
				const uint32_t index = mDevice.FetchIndex(iPrim * 3 + iVertex); 
				const VS_Output& v = vertexCache(index);
				pVSOutputs[iVertex] = &v;
			}

			// frustum cull and clip
			uint32_t numCliped = ClipTriangle(&outVertices[baseVertex], *pVSOutputs[0], *pVSOutputs[1], *pVSOutputs[2]);
			ASSERT(numCliped <= 5);

			if (numCliped < 3)
			{
				// culled, no triangle
				outFaces[baseFace].TriCount = 0;
				return;
			}
			
			// Project the first three vertices for culling
			for (iVertex = 0; iVertex < 3; ++ iVertex)
				ProjectVertex( &outVertices[baseVertex + iVertex] );

			// We do not have to check for culling for each sub-polygon of the triangle, as they
			// are all in the same plane. If the first polygon is culled then all other polygons
			// would be culled, too.
			if( BackFaceCulling( outVertices[baseVertex+0], outVertices[baseVertex+1], outVertices[baseVertex+2] ) )
			{
				// back face culled
				outFaces[baseFace].TriCount = 0;
				continue;
			}

			// Project the remaining vertices
			for( iVertex = 3; iVertex < numCliped; ++iVertex )
				ProjectVertex( &outVertices[baseVertex + iVertex] );

			// if out clip vertices is less than 3, no triangle, or generate  (numCliped - 2) triangles
			const uint32_t triCount = (numCliped < 3) ? 0 : (numCliped - 2);
			outFaces[baseFace].TriCount = triCount;

			for(uint32_t iTri = 0; iTri < triCount; ++iTri)
			{
				outFaces[baseFace].Indices[iTri * 3 + 0] = baseVertex + iTri * 3 + 0;
				outFaces[baseFace].Indices[iTri * 3 + 1] = baseVertex + iTri * 3 + 1;
				outFaces[baseFace].Indices[iTri * 3 + 2] = baseVertex + iTri * 3 + 2;
			}
		}

		localWorkingPackage = workingPackage++;
	}
}

void Rasterizer::SetupGeometryTiled( std::vector<VS_Output>& outVertices, std::vector<RasterFace>& outFaces, uint32_t theadIdx, ThreadPackage package )
{
	VS_Output clippedVertices[7]; // 7 enough ???

	for (uint32_t iPrim = package.Start; iPrim < package.End; ++iPrim)
	{
		const uint32_t baseVertex = iPrim * 4;
		const uint32_t baseFace = iPrim;

		// fetch vertices
		uint32_t iVertex;
		for (iVertex = 0; iVertex < 3; ++ iVertex)
		{		
			const uint32_t index = mDevice.FetchIndex(iPrim * 3 + iVertex); 
			const VS_Output& v = FetchVertex(index, theadIdx);
			memcpy(&clippedVertices[iVertex], &v, sizeof(VS_Output));
			//VS_Output_Copy(&clippedVertices[iVertex], &v, mCurrVSOutputCount);
		}

		ClipTriangleTiled(clippedVertices, theadIdx);		
	}
}

void Rasterizer::Binning(std::atomic<uint32_t>& workPackage , uint32_t primitiveCount )
{
	uint32_t numPackages = (primitiveCount + BinningPackageSize - 1) / BinningPackageSize;
	uint32_t localWorkingPackage  = numPackages ++;

	while (localWorkingPackage < numPackages)
	{
		const uint32_t start = localWorkingPackage * BinningPackageSize;
		const uint32_t end = (std::min)(primitiveCount, start + BinningPackageSize);
	
		for (uint32_t iPrim = start; iPrim < end; ++iPrim)
		{
		}
	}

}

void Rasterizer::RasterizeTriangle( const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2 )
{
	const VS_Output* pBaseVertex;

	/** 
	 * Compute difference of attributes.
	 *
	 * 根据BaryCentric插值attribute，这里计算ddx, ddy，后面只要根据offsetX, offsetY,
	 * 乘上相应的ddx, ddy，就可以计算插值后的attribute。
	 */
	VS_Output vsOutput01, vsOutput02;
	VS_Output_Sub(&vsOutput01, &vsOut1, &vsOut0, mCurrVSOutputCount);
	VS_Output_Sub(&vsOutput02, &vsOut2, &vsOut0, mCurrVSOutputCount);

	const float area = vsOutput01.Position.X() * vsOutput02.Position.Y() - vsOutput02.Position.X() * vsOutput01.Position.Y();
	const float invArea = 1.0f / area;

	// store base vertex
	pBaseVertex = &vsOut0;

	VS_Output ddxAttrib, ddyAttrib;
	VS_Output_Difference(&ddxAttrib, &ddyAttrib, &vsOutput01, &vsOutput02, invArea, mCurrVSOutputCount);

	// Sort vertices by y-coordinate 
	const VS_Output* pVertices[3] = { &vsOut0, &vsOut1, &vsOut2 };
	
	if( pVertices[1]->Position.Y() < pVertices[0]->Position.Y() ) 
		std::swap(pVertices[0], pVertices[1]);

	if( pVertices[2]->Position.Y() < pVertices[0]->Position.Y() ) 
		std::swap(pVertices[0], pVertices[2]); 
	
	if( pVertices[2]->Position.Y() < pVertices[1]->Position.Y() ) 
		std::swap(pVertices[1], pVertices[2]); 


	// Screenspace-position references 
	const float4& vA = pVertices[0]->Position;
	const float4& vB = pVertices[1]->Position;
	const float4& vC = pVertices[2]->Position;

	// Calculate slopes for stepping
	const float fStepX[3] = {
		( vB.Y() - vA.Y() > 0.0f ) ? ( vB.X() - vA.X() ) / ( vB.Y() - vA.Y() ) : 0.0f,
		( vC.Y() - vA.Y() > 0.0f ) ? ( vC.X() - vA.X() ) / ( vC.Y() - vA.Y() ) : 0.0f,
		( vC.Y() - vB.Y() > 0.0f ) ? ( vC.X() - vB.X() ) / ( vC.Y() - vB.Y() ) : 0.0f };

	const Viewport& vp = mDevice.mCurrentFrameBuffer->GetViewport();
	const int32_t MinClipY = vp.Top;
	const int32_t MaxClipY = vp.Top + vp.Height;
	const int32_t MinClipX = vp.Left;
	const int32_t MaxClipX = vp.Left + vp.Width;

	const float fMinClipY = (float)MinClipY;
	const float fMaxClipY = (float)MaxClipY;
	const float fMinClipX = (float)MinClipX;
	const float fMaxClipX = (float)MaxClipX;

	// Begin rasterization
	float fX[2] = { vA.X(), vA.X() };

	for(int32_t iPart = 0; iPart < 2; ++iPart)
	{
		int32_t iY[2];
		float fDeltaX[2];

		switch (iPart)
		{
		case 0:
			{
				// 平底三角形
				iY[0] = (std::max)(MinClipY, (int32_t)ceilf(vA.Y()));
				iY[1] = (std::min)(MaxClipY, (int32_t)ceilf(vB.Y()));

				if( fStepX[0] > fStepX[1] ) // left <-> right ?
				{
					fDeltaX[0] = fStepX[1];
					fDeltaX[1] = fStepX[0];
				}
				else
				{
					fDeltaX[0] = fStepX[0];
					fDeltaX[1] = fStepX[1];
				}

				const float fPreStepY = (float)iY[0] - vA.Y();
				fX[0] += fDeltaX[0] * fPreStepY;
				fX[1] += fDeltaX[1] * fPreStepY;
			}
			break;

		case 1:
			{
				iY[1] = (std::min)(MaxClipY, (int32_t)ceilf(vC.Y()));

				const float fPreStepY = (float)iY[0] - vB.Y();

				if( fStepX[1] > fStepX[2] ) // left <-> right ?
				{
					fDeltaX[0] = fStepX[1];
					fDeltaX[1] = fStepX[2];
					fX[1] = vB.X() + fDeltaX[1] * fPreStepY;
				}
				else
				{
					fDeltaX[0] = fStepX[2];
					fDeltaX[1] = fStepX[1];
					fX[0] = vB.X() + fDeltaX[0] * fPreStepY;
				}
			}
			break;
		}

		for ( ; iY[0] < iY[1]; ++iY[0])
		{
			int32_t iX[2] = { (int32_t)( ceilf( fX[0] ) ), (int32_t)( ceilf( fX[1] ) ) };

			const float fOffsetX = ( (float)iX[0] - pBaseVertex->Position.X() );
			const float fOffsetY = ( (float)iY[0] - pBaseVertex->Position.Y() );

			/*float beta = fOffsetX * vsOutput02.Position.Y() - fOffsetY * vsOutput02.Position.X();
			float gama = fOffsetY * vsOutput01.Position.X() - fOffsetX * vsOutput01.Position.Y();

			beta *= invArea;
			gama *= invArea;

			float4 pos = pBaseVertex->Position + vsOutput01.Position * beta + vsOutput02.Position * gama;*/

			VS_Output VSOutput;
			VS_Output_BaryCentric(&VSOutput, pBaseVertex, &ddxAttrib, &ddyAttrib, fOffsetX, fOffsetY, mCurrVSOutputCount);

			// 水平裁剪
			if (iX[0] < MinClipX)
			{
				iX[0] = MinClipX;
				if (iX[1] < MinClipX)
					continue;		
			}

			if (iX[1] > MaxClipX)
			{
				iX[1] = MaxClipX;
				if (iX[0] > MaxClipX)
					continue;
			}

			RasterizeScanline(iX[0], iX[1], iY[0], &VSOutput, &ddxAttrib);	

			// Next scan line
			fX[0] += fDeltaX[0], fX[1] += fDeltaX[1];
		}
	}
}

void Rasterizer::RasterizeScanline(int32_t xStart, int32_t xEnd, int32_t Y, VS_Output* pBaseVertex, const VS_Output* pDdx)
{
#define DEPTH_TEST(condition) if((condition)) break; else continue;

	float destDepth, srcDepth;

	for (int32_t X = xStart; X < xEnd; ++X, VS_Output_Add(pBaseVertex, pBaseVertex, pDdx, mCurrVSOutputCount))
	{
		// read back buffer pixel
		mDevice.mCurrentFrameBuffer->ReadPixel(X, Y, NULL, &destDepth);

		// Get depth of current pixel
		srcDepth = pBaseVertex->Position.Z();

		VS_Output PSInput;
		float curPixelInvW = 1.0f / pBaseVertex->Position.W();
		VS_Output_Mul( &PSInput, pBaseVertex, curPixelInvW, mCurrVSOutputCount );

		// Execute the pixel shader
		//m_TriangleInfo.iCurPixelX = i_iX;
		PS_Output PSOutput;
		if( !mDevice.mPixelShaderStage->GetPixelShader()->Execute(&PSInput, &PSOutput, &srcDepth ))
		{
			// kill this pixel
			continue;
		}

		// Perform depth-test
		switch( mDevice.DepthStencilState.DepthFunc )
		{
		case CF_AlwaysFail: continue;
		case CF_Equal: DEPTH_TEST(fabsf( srcDepth - destDepth ) < FLT_EPSILON);
		case CF_NotEqual: DEPTH_TEST(fabsf( srcDepth - destDepth ) >= FLT_EPSILON);
		case CF_Less: /*DEPTH_TEST(srcDepth < destDepth);*/
			{
				if (srcDepth < destDepth)
				{
					break;
				}
				else
				{
					continue;
				}
			}
		case CF_LessEqual: DEPTH_TEST(srcDepth <= destDepth);
		case CF_GreaterEqual: DEPTH_TEST(srcDepth >= destDepth);
		case CF_Greater: DEPTH_TEST(srcDepth > destDepth);
		case CF_AlwaysPass: break;
		}

		mDevice.mCurrentFrameBuffer->WritePixel(X, Y, &PSOutput,
			mDevice.DepthStencilState.DepthWriteMask ? &srcDepth : NULL);
	}
}

void Rasterizer::PreDraw()
{
	// Set vertex varing count
	mCurrVSOutputCount = mDevice.mVertexShaderStage->VSOutputCount;

	mCurrFrameBuffer = mDevice.GetCurrentFrameBuffer();

	// reset each thread's vertex cache
	for (auto iIter = mVertexCaches.begin(); iIter != mVertexCaches.end(); ++iIter)
	{
		for (auto jIter = iIter->begin(); jIter != iIter->end(); ++jIter)
		{
			jIter->Index = UINT_MAX;
		}
	}

	
}

void Rasterizer::PostDraw()
{

}

void Rasterizer::Bin( const VS_Output& V1, const VS_Output& V2, const VS_Output& V3, uint32_t threadIdx )
{
	uint32_t baseIdx = mNumVerticesThreads[threadIdx];
	uint32_t faceIdx = baseIdx / 3;

	RasterFace& face = mFacesThreads[threadIdx][faceIdx];

	const int32_t X1 = iround(16.0f * V1.Position.X());
	const int32_t X2 = iround(16.0f * V2.Position.X());
	const int32_t X3 = iround(16.0f * V3.Position.X());

	const int32_t Y1 = iround(16.0f * V1.Position.Y());
	const int32_t Y2 = iround(16.0f * V2.Position.Y());
	const int32_t Y3 = iround(16.0f * V3.Position.Y());

	// Deltas
	const int32_t DX12 = X1 - X2;
	const int32_t DX23 = X2 - X3;
	const int32_t DX31 = X3 - X1;

	const int32_t DY12 = Y1 - Y2;
	const int32_t DY23 = Y2 - Y3;
	const int32_t DY31 = Y3 - Y1;

	// Fixed-point deltas
	const int32_t FDX12 = DX12 << 4;
	const int32_t FDX23 = DX23 << 4;
	const int32_t FDX31 = DX31 << 4;
	const int32_t FDY12 = DY12 << 4;
	const int32_t FDY23 = DY23 << 4;
	const int32_t FDY31 = DY31 << 4;

	// Half-edge constants
	int32_t C1 = DY12 * X1 - DX12 * Y1;
	int32_t C2 = DY23 * X2 - DX23 * Y2;
	int32_t C3 = DY31 * X3 - DX31 * Y3;

	// Compute the fixed point bounding box
	const int32_t minX = Min(X1, Min(X2, X3));
	const int32_t maxX = Max(X1, Max(X2, X3));
	const int32_t minY = Min(Y1, Min(Y2, Y3));
	const int32_t maxY = Max(Y1, Max(Y2, Y3));

	// Compute tile bounding box
	const int32_t minTileX = Max((minX >> 4) >> TileSizeShift, 0);
	const int32_t minTileY = Max((minY >> 4) >> TileSizeShift, 0);
	const int32_t maxTileX = Min((maxX >> 4) >> TileSizeShift, mNumTileX);
	const int32_t maxTileY = Min((maxY >> 4) >> TileSizeShift, mNumTileY);

	if ((minTileX + 1 == maxTileX) && (minTileY + 1 == maxTileY))
	{
		// Small primitive
		Tile& tile = mTiles[minTileY * mNumTileX + minTileX];
		tile.TriQueue[threadIdx][tile.TriQueueSize[threadIdx]++] = faceIdx;
	}
	else
	{
		for (int32_t y = minTileY; y < maxTileY; ++y)
		{
			for (int32_t x = minTileX; x < maxTileX; ++x)
			{
				// Corners of block
				int32_t x0 = x << (TileSizeShift + 4);
				int32_t x1 = ((x + 1) << (TileSizeShift + 4)) - 1;
				int32_t y0 = y << (TileSizeShift + 4);
				int32_t y1 = ((y + 1) << (TileSizeShift + 4)) - 1;

				// Evaluate half-space functions
				bool a00 = C1 + DX12 * y0 - DY12 * x0 > 0;
				bool a10 = C1 + DX12 * y0 - DY12 * x1 > 0;
				bool a01 = C1 + DX12 * y1 - DY12 * x0 > 0;
				bool a11 = C1 + DX12 * y1 - DY12 * x1 > 0;
				int32_t a = (a00 << 0) | (a10 << 1) | (a01 << 2) | (a11 << 3);

				bool b00 = C2 + DX23 * y0 - DY23 * x0 > 0;
				bool b10 = C2 + DX23 * y0 - DY23 * x1 > 0;
				bool b01 = C2 + DX23 * y1 - DY23 * x0 > 0;
				bool b11 = C2 + DX23 * y1 - DY23 * x1 > 0;
				int32_t b = (b00 << 0) | (b10 << 1) | (b01 << 2) | (b11 << 3);

				bool c00 = C3 + DX31 * y0 - DY31 * x0 > 0;
				bool c10 = C3 + DX31 * y0 - DY31 * x1 > 0;
				bool c01 = C3 + DX31 * y1 - DY31 * x0 > 0;
				bool c11 = C3 + DX31 * y1 - DY31 * x1 > 0;
				int32_t c = (c00 << 0) | (c10 << 1) | (c01 << 2) | (c11 << 3);

				// Skip block when outside an edge
				if(a == 0x0 || b == 0x0 || c == 0x0) continue;

				Tile& tile = mTiles[minTileY * mNumTileX + minTileX];
				tile.TriQueue[threadIdx][tile.TriQueueSize[threadIdx]++] = faceIdx;

				// Test if we can trivially accept the entire tile
				//tile.frag_tiles[ thread_id ][ face_idx ] = ( a != 0xF || b != 0xF || c != 0xF ) ? 0 : 1;
			}
		}
	}

	VS_Output_Copy(&mVerticesThreads[threadIdx][baseIdx],   &V1, mCurrVSOutputCount);
	VS_Output_Copy(&mVerticesThreads[threadIdx][baseIdx+1], &V2, mCurrVSOutputCount);
	VS_Output_Copy(&mVerticesThreads[threadIdx][baseIdx+2], &V3, mCurrVSOutputCount);

	face.V[0] = &mVerticesThreads[threadIdx][baseIdx];
	face.V[1] = &mVerticesThreads[threadIdx][baseIdx+1];
	face.V[2] = &mVerticesThreads[threadIdx][baseIdx+2];

	mNumVerticesThreads[threadIdx] += 3;
}

void Rasterizer::DrawTiled( PrimitiveType primitiveType, uint32_t primitiveCount )
{
	ASSERT(primitiveType == PT_Triangle_List); // Only support triangle list 

	pool& theadPool = GlobalThreadPool();
	uint32_t numWorkThreads = GetNumWorkThreads();

	// calculate package size for each thread
	uint32_t primitivesPerThread = primitiveCount / numWorkThreads;
	uint32_t extraPrimitives = primitiveCount % numWorkThreads;

	uint32_t idx = 0;
	while (idx < numWorkThreads && extraPrimitives)
	{
		// add one primitives more to thread until extra primitives all dispatched
		mThreadPackage[idx].Start = idx * (primitivesPerThread + 1);
		mThreadPackage[idx].End = mThreadPackage[idx].Start + primitivesPerThread + 1;
		extraPrimitives--;
		idx++;
	}

	while (idx < numWorkThreads)
	{
		// set up remain thread package
		mThreadPackage[idx].Start = mThreadPackage[idx-1].End;
		mThreadPackage[idx].End = mThreadPackage[idx].Start + primitivesPerThread;
		idx++;
	}

	// allocate each thread's vertex and face buffer
	for (idx = 0; idx < numWorkThreads; ++idx)
	{
		const uint32_t primCount = (mThreadPackage[idx].End - mThreadPackage[idx].Start);

		// after frustum clip, one triangle can generate maximum 5 vertices, maximum 3 triangle faces
		mVerticesThreads[idx].resize( primCount * 5 );
		mFacesThreads[idx].resize(primCount * 3);
	}

	for (size_t i = 0; i < numWorkThreads - 1; ++i)
	{
		theadPool.schedule(std::bind(&Rasterizer::SetupGeometryTiled, this, std::ref(mVerticesThreads[idx]), std::ref(mFacesThreads[idx]), idx, mThreadPackage[idx]));
	}
	SetupGeometryTiled(mVerticesThreads[idx], mFacesThreads[idx], numWorkThreads - 1, mThreadPackage[idx]);
	theadPool.wait();  // Synchronization

	// build non-empty tile job queue
	std::vector<uint32_t> tilesQueue;
	for (int32_t y = 0; y < mNumTileY; ++y)
	{
		for (int32_t x = 0; x < mNumTileX; ++x)
		{
			const int32_t tileIdx = y * mNumTileX + x;
			for (uint32_t i = 0; i < numWorkThreads; ++i)
			{
				if (mTiles[tileIdx].TriQueueSize[i] > 0)
				{
					tilesQueue.push_back(tileIdx);
					break;
				}
			}	
		}
	}

	// Rasterize each tile in tile job queue
	std::atomic<uint32_t> workingPackage(0);
	for (size_t i = 0; i < numWorkThreads - 1; ++i)
	{
		theadPool.schedule(std::bind(&Rasterizer::RasterizeTiles, this, std::ref(tilesQueue), std::ref(workingPackage), tilesQueue.size()));
	}
	RasterizeTiles(tilesQueue, workingPackage, tilesQueue.size());
	theadPool.wait();  // Synchronization


}

void Rasterizer::RasterizeTiles(std::vector<uint32_t>& tilesQueue, std::atomic<uint32_t>& workingPackage, uint32_t numTiles)
{
	uint32_t numPackages = (numTiles + RasterizeTilePackageSize - 1) / RasterizeTilePackageSize;
	uint32_t localWorkingPackage  = workingPackage ++;

	const uint32_t numWorkThreads = GetNumWorkThreads();

	while (localWorkingPackage < numPackages)
	{
		const uint32_t start = localWorkingPackage * SetupGeometryPackageSize;
		const uint32_t end = (std::min)(numTiles, start + SetupGeometryPackageSize);
		for (uint32_t iTile = start; iTile < end; ++iTile)
		{
			Tile& tile = mTiles[tilesQueue[iTile]];
			for (uint32_t iThread = 0; iThread < numWorkThreads; ++iThread)
			{
				for (uint32_t iTri = 0; iTri < tile.TriQueueSize[iThread]; ++iThread)
				{
					uint32_t faceIdx = tile.TriQueue[iThread][iTri];

					if (1)
					{
						DrawWholeTile(iThread, faceIdx, iTile);
					}
					else
					{

					}
				}

				tile.TriQueueSize[iThread] = 0;
			}

		}

		localWorkingPackage  = workingPackage ++;
	}
}

void Rasterizer::OnBindFrameBuffer( const shared_ptr<FrameBuffer>& fb )
{
	if (mCurrFrameBuffer != fb)
	{
		uint32_t width = fb->mWidth;
		uint32_t height = fb->mHeight;

		if (width != mCurrFrameBuffer->mWidth && height != mCurrFrameBuffer->mHeight)
		{
			// init render target tiles
			uint32_t extraPixelsX = width % TileSize;
			uint32_t numTileX = extraPixelsX ? (width / TileSize + 1) : (width / TileSize);
			uint32_t extraPixelsY = height % TileSize;
			uint32_t numTileY = extraPixelsY ? (height / TileSize + 1) : (height / TileSize);

			const uint32_t NumWorkThreads = GetNumWorkThreads();

			mTiles.resize(numTileX*numTileY);
			for (uint32_t y = 0; y < numTileY; ++y)
			{
				bool extraY = ((y == numTileY-1) && extraPixelsY);
				for (uint32_t x = 0; x < numTileX; ++x)
				{
					bool extraX = ((y == numTileX-1) && extraPixelsY);

					uint32_t index = y*numTileX + x;
					mTiles[index].X = x * TileSize;
					mTiles[index].Y = y * TileSize;
					mTiles[index].Width = extraX ? extraPixelsX : TileSize;
					mTiles[index].Height = extraY ? extraPixelsY : TileSize;

					mTiles[index].TriQueue.resize( NumWorkThreads );
					mTiles[index].TriQueueSize.resize(NumWorkThreads);
					for (uint32_t i = 0; i < NumWorkThreads; ++i)
					{
						mTiles[index].TriQueueSize[i]  = 0;
						mTiles[index].TriQueue[i].resize(MaxBinQueueSize / NumWorkThreads);
					}
				}
			}

			mNumTileX = numTileX;
			mNumTileY = numTileY;
		}

		mCurrFrameBuffer = fb;		
	}
}

void Rasterizer::DrawWholeTile( uint32_t threadIdx, uint32_t faceIdx, uint32_t tileIdx )
{

}



