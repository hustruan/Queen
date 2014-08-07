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
#define FLOAT_EQUAL_PRECISION 0.001f

namespace {

inline bool FCMP(float a, float b, float epsilon = FLOAT_EQUAL_PRECISION)
{
	if ( fabsf( a - b ) < epsilon )
	{
		return true;
	}
	return false;
}


inline int32_t iround(float x)
{
	return _mm_cvt_ss2si( _mm_load_ps( &x ) ); 
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
	: RenderStage(device), mCurrFrameBuffer(nullptr), mNumTileX(0), mNumTileY(0), mCurrVSOutputCount(0)
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
	mThreadPackage.resize(nunWorkThreads);
}

Rasterizer::~Rasterizer(void)
{
}

void Rasterizer::OnBindFrameBuffer( const shared_ptr<FrameBuffer>& fb )
{
	if (mCurrFrameBuffer != fb)
	{
		uint32_t width = fb->mWidth;
		uint32_t height = fb->mHeight;

		if ( !mCurrFrameBuffer || (width != mCurrFrameBuffer->mWidth && height != mCurrFrameBuffer->mHeight) )
		{
			// init render target tiles
			uint32_t extraPixelsX = width % TileSize;
			uint32_t numTileX = extraPixelsX ? (width / TileSize + 1) : (width / TileSize);
			uint32_t extraPixelsY = height % TileSize;
			uint32_t numTileY = extraPixelsY ? (height / TileSize + 1) : (height / TileSize);

			const uint32_t NumWorkThreads = GetNumWorkThreads();

			mTiles.resize(numTileX*numTileY);
			mTilesQueue.resize(numTileX * numTileY);

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

		const Viewport& viewport = mCurrFrameBuffer->GetViewport();
		MinClipX = (float)viewport.Left;
		MaxClipX = (float)viewport.Left + viewport.Width;
		MinClipY = (float)viewport.Top;
		MaxClipY = (float)viewport.Top + viewport.Height;
	}
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

	// viewport transform
	vertex->Position = vertex->Position * mDevice.mCurrentFrameBuffer->GetViewportMatrix();	

	/** 
	 * After Projection transform, W is eaqual Z in view space.
	 * divide shader output registers by w; this way we can interpolate them linearly while rasterizing ...
	 */
	vertex->Position.W() = invW;
	
	VS_Output_ProjectAttrib(vertex, invW, mCurrVSOutputCount);
}

bool Rasterizer::BackFaceCulling( const VS_Output& v0, const VS_Output& v1, const VS_Output& v2, bool* oriented)
{
	const float signedArea = (v1.Position.X()- v0.Position.X()) * (v2.Position.Y() - v0.Position.Y()) -
		(v1.Position.Y() - v0.Position.Y()) * (v2.Position.X()- v0.Position.X());
	
	bool ccw = (signedArea <= 0.0f);

	if (oriented)	*oriented = ccw;

	// Do backface-culling ----------------------------------------------------
	if( mDevice.RasterizerState.PolygonCullMode == CM_None )
		return false;

	if (ccw)
	{
		// polygon is CCW
		if (mDevice.RasterizerState.FrontCounterClockwise)
			return (mDevice.RasterizerState.PolygonCullMode == CM_Front);
		else
			return (mDevice.RasterizerState.PolygonCullMode == CM_Back);
	}
	else
	{
		// polygon is CW
		if (mDevice.RasterizerState.FrontCounterClockwise)
			return (mDevice.RasterizerState.PolygonCullMode == CM_Back);
		else
			return (mDevice.RasterizerState.PolygonCullMode == CM_Front);
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

void Rasterizer::Draw( PrimitiveType primitiveType, uint32_t primitiveCount )
{
	pool& theadPool = GlobalThreadPool();
	uint32_t numWorkThreads = GetNumWorkThreads();

	// after frustum clip, one triangle can generate maximum 5 vertices
	mClippedVertices.resize(primitiveCount * 5);
	mClippedFaces.resize(primitiveCount);

	std::atomic<uint32_t> workingPackage(0);

	//mProfiler.StartTimer("Vertex Process");
	
	//input assembly, vertex shading, culling/clipping
	for (size_t i = 0; i < numWorkThreads - 1; ++i)
	{
		theadPool.schedule(std::bind(&Rasterizer::SetupGeometry, this, std::ref(mClippedVertices), std::ref(mClippedFaces), 
			std::ref(workingPackage), primitiveCount));
	}
	// schedule current thread
	SetupGeometry(std::ref(mClippedVertices), std::ref(mClippedFaces), std::ref(workingPackage), primitiveCount);
	theadPool.wait();
	
	//mProfiler.EndTimer("Vertex Process");
	//auto elapsedTime = mProfiler.GetElapsedTime("Vertex Process");

	//mProfiler.StartTimer("Rasterizer Process");

	workingPackage = 0;
	for (size_t i = 0; i < numWorkThreads - 1; ++i)
	{
		theadPool.schedule(std::bind(&Rasterizer::RasterizeFaces, this, std::ref(mClippedFaces), std::ref(workingPackage), mClippedFaces.size()));
	}
	// schedule current thread
	RasterizeFaces(std::ref(mClippedFaces), std::ref(workingPackage), mClippedFaces.size());
	theadPool.wait();

	// 不能简单的用多线程光栅化，因为要写backbuffer的Fragment，就会有同步问题，这里先简单的使用单线程
	/*for (uint32_t i = 0; i < mClippedFaces.size(); ++i)
	{
		for (uint32_t iFace = 0; iFace < mClippedFaces[i].TriCount; ++iFace)
		{
			const VS_Output& v0 = mClippedVertices[mClippedFaces[i].Indices[iFace * 3 + 0]];
			const VS_Output& v1 = mClippedVertices[mClippedFaces[i].Indices[iFace * 3 + 1]];
			const VS_Output& v2 = mClippedVertices[mClippedFaces[i].Indices[iFace * 3 + 2]];
			RasterizeTriangle(v0, v1, v2);
		}		
	}*/

	//mProfiler.EndTimer("Rasterizer Process");
	//auto elapsedTimeR = mProfiler.GetElapsedTime("Rasterizer Process");
}

void Rasterizer::SetupGeometry( std::vector<VS_Output>& outVertices, std::vector<RasterFace>& outFaces, std::atomic<uint32_t>& workingPackage, uint32_t primitiveCount )
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

void Rasterizer::RasterizeFaces( std::vector<RasterFace>& faces, std::atomic<uint32_t>& workingPackage, uint32_t faceCount )
{
	uint32_t numPackages = (faceCount + SetupGeometryPackageSize - 1) / SetupGeometryPackageSize;
	uint32_t localWorkingPackage  = workingPackage ++;

	while (localWorkingPackage < numPackages)
	{
		const uint32_t start = localWorkingPackage * SetupGeometryPackageSize;
		const uint32_t end = (std::min)(faceCount, start + SetupGeometryPackageSize);
		
		for (uint32_t i = start; i < end; ++i)
		{
			for (uint32_t iFace = 0; iFace < mClippedFaces[i].TriCount; ++iFace)
			{
				const VS_Output& v0 = mClippedVertices[mClippedFaces[i].Indices[iFace * 3 + 0]];
				const VS_Output& v1 = mClippedVertices[mClippedFaces[i].Indices[iFace * 3 + 1]];
				const VS_Output& v2 = mClippedVertices[mClippedFaces[i].Indices[iFace * 3 + 2]];
				RasterizeTriangle(v0, v1, v2);
			}		
		}

		localWorkingPackage = workingPackage++;

	};
}

void Rasterizer::RasterizeTriangle( const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2 )
{
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

	if (vC.Y() < MinClipY || vA.Y() > MaxClipY ||
		(vA.X() < MinClipX && vB.X() < MinClipX && vB.X() < MinClipX ) ||
		(vA.X() > MaxClipX && vB.X() > MaxClipX && vB.X() > MaxClipX ) )
	{
		return;
	}

	// store base vertex
	const VS_Output* pBaseVertex = pVertices[0];

	VS_Output vsOutput01, vsOutput02;
	VS_Output_Sub(&vsOutput01, pVertices[1], pVertices[0], mCurrVSOutputCount);
	VS_Output_Sub(&vsOutput02, pVertices[2], pVertices[0], mCurrVSOutputCount);

	const float area = vsOutput01.Position.X() * vsOutput02.Position.Y() - vsOutput02.Position.X() * vsOutput01.Position.Y();
	const float invArea = 1.0f / area;

	VS_Output ddxAttrib, ddyAttrib;
	VS_Output_Difference(&ddxAttrib, &ddyAttrib, &vsOutput01, &vsOutput02, invArea, mCurrVSOutputCount);

	// Calculate slopes for stepping
	const float fStepX[3] = {
		( vB.Y() - vA.Y() > 0.0f ) ? ( vB.X() - vA.X() ) / ( vB.Y() - vA.Y() ) : 0.0f,
		( vC.Y() - vA.Y() > 0.0f ) ? ( vC.X() - vA.X() ) / ( vC.Y() - vA.Y() ) : 0.0f,
		( vC.Y() - vB.Y() > 0.0f ) ? ( vC.X() - vB.X() ) / ( vC.Y() - vB.Y() ) : 0.0f };

	// Begin rasterization
	float fX[2] = { vA.X(), vA.X() };

	for(int32_t iPart = 0; iPart < 2; ++iPart)
	{
		int32_t iY[2];
		float fDeltaX[2];

		switch (iPart)
		{
		case 0: // 平底三角形
			{
				// D3D 9, Pixel Center (0, 0)
				//iY[0] = (int32_t)(std::max)(MinClipY, ceilf(vA.Y()));
				//iY[1] = (int32_t)(std::min)(MaxClipY, ceilf(vB.Y()));

				// D3D10,OpenGL, Pixel Center(0.5, 0.5)
				iY[0] = (int32_t)(std::max)(MinClipY, ceilf(vA.Y() - 0.5f));
				iY[1] = (int32_t)(std::min)(MaxClipY, ceilf(vB.Y() - 0.5f));

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

				// D3D 9, Pixel Center (0, 0)
				//const float fPreStepY = (float)iY[0] - vA.Y();

				// D3D10,OpenGL, Pixel Center(0.5, 0.5)
				const float fPreStepY = (float)iY[0] - vA.Y() + 0.5f;
				fX[0] += fDeltaX[0] * fPreStepY;
				fX[1] += fDeltaX[1] * fPreStepY;
			}
			break;

		case 1:
			{	
				// D3D 9, Pixel Center (0, 0)
				//iY[1] = (int32_t)(std::min)(MaxClipY, ceilf(vC.Y()));
				//iY[1] = (int32_t)(std::min)(MaxClipY, ceilf(vC.Y()));

				// D3D10,OpenGL, Pixel Center(0.5, 0.5)
				iY[1] = (int32_t)(std::min)(MaxClipY, ceilf(vC.Y() - 0.5f));
				const float fPreStepY = (float)iY[0] - vB.Y() + 0.5f;

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
			// D3D 9, Pixel Center (0, 0)
			/*int32_t iX[2] = { (int32_t)( ceilf( fX[0]) ), (int32_t)( ceilf( fX[1]) ) };
			const float fOffsetX = ( (float)iX[0] - pBaseVertex->Position.X() );
			const float fOffsetY = ( (float)iY[0] - pBaseVertex->Position.Y() );*/
			
			// D3D10,OpenGL, Pixel Center(0.5, 0.5)
			int32_t iX[2] = { (int32_t)( ceilf( fX[0] - 0.5f ) ), (int32_t)( ceilf( fX[1] - 0.5f ) ) };
			const float fOffsetX = ( (float)iX[0] - pBaseVertex->Position.X() + 0.5f);
			const float fOffsetY = ( (float)iY[0] - pBaseVertex->Position.Y() + 0.5f);

			VS_Output vsOutput;
			VS_Output_BaryCentric(&vsOutput, pBaseVertex, &ddxAttrib, &ddyAttrib, fOffsetX, fOffsetY, mCurrVSOutputCount);

			// 水平裁剪
			if (iX[0] < MinClipX)
			{
				iX[0] = (int32_t)MinClipX;
				if (iX[1] < MinClipX)
					continue;		
			}

			if (iX[1] > MaxClipX)
			{
				iX[1] = (int32_t)MaxClipX;
				if (iX[0] > MaxClipX)
					continue;
			}

			RasterizeScanline(iX[0], iX[1], iY[0], &vsOutput, &ddxAttrib);	

			// Next scan line
			fX[0] += fDeltaX[0], fX[1] += fDeltaX[1];
		}
	}
}

//void Rasterizer::RasterizeTriangle( const VS_Output& vsOut0, const VS_Output& vsOut1, const VS_Output& vsOut2 )
//{
//	// Sort vertices by y-coordinate 
//	const VS_Output* pVertices[3] = { &vsOut0, &vsOut1, &vsOut2 };
//
//	if( pVertices[1]->Position.Y() < pVertices[0]->Position.Y() ) 
//		std::swap(pVertices[0], pVertices[1]);
//
//	if( pVertices[2]->Position.Y() < pVertices[0]->Position.Y() ) 
//		std::swap(pVertices[0], pVertices[2]); 
//
//	if( pVertices[2]->Position.Y() < pVertices[1]->Position.Y() ) 
//		std::swap(pVertices[1], pVertices[2]); 
//
//	// Screenspace-position references 
//	const float4& vA = pVertices[0]->Position;
//	const float4& vB = pVertices[1]->Position;
//	const float4& vC = pVertices[2]->Position;
//
//	const float X1 = vA.X();
//	const float X2 = vB.X();
//	const float X3 = vC.X();
//
//	const float Y1 = vA.Y();
//	const float Y2 = vB.Y();
//	const float Y3 = vC.Y();
//
//	if (Y3 < MinClipY || Y1 > MaxClipY ||
//		(X1 < MinClipX && X2 < MinClipX && X3 < MinClipX ) ||
//		(X1 > MaxClipX && X2 > MaxClipX && X3 > MaxClipX ) )
//	{
//		return;
//	}
//
//	/** 
//	 * Compute difference of attributes.
//	 *
//	 * 根据BaryCentric插值attribute，这里计算ddx, ddy，后面只要根据offsetX, offsetY,
//	 * 乘上相应的ddx, ddy，就可以计算插值后的attribute。
//	 */
//
//	// store base vertex
//	const VS_Output* pBaseVertex = pVertices[0];
//
//	VS_Output vsOutput01, vsOutput02;
//	VS_Output_Sub(&vsOutput01, pVertices[1], pVertices[0], mCurrVSOutputCount);
//	VS_Output_Sub(&vsOutput02, pVertices[2], pVertices[0], mCurrVSOutputCount);
//
//	const float area = vsOutput01.Position.X() * vsOutput02.Position.Y() - vsOutput02.Position.X() * vsOutput01.Position.Y();
//	const float invArea = 1.0f / area;
//
//	VS_Output ddxAttrib, ddyAttrib;
//	VS_Output_Difference(&ddxAttrib, &ddyAttrib, &vsOutput01, &vsOutput02, invArea, mCurrVSOutputCount);
//
//
//	if (FCMP(Y1, Y2))
//	{
//		RasterizeTriangle_Top(X1, Y1, X2, Y2, X3, Y3, *pBaseVertex, ddxAttrib, ddyAttrib);
//	}
//	else if (FCMP(Y3, Y2))
//	{
//		RasterizeTriangle_Bottom(X1, Y1, X2, Y2, X3, Y3, *pBaseVertex, ddxAttrib, ddyAttrib);
//	}
//	else
//	{
//		float newX = X1 + (Y2 - Y1) * (X3 - X1) / (Y3 - Y1);
//		RasterizeTriangle_Bottom(X1, Y1, newX, Y2, X2, Y2, *pBaseVertex, ddxAttrib, ddyAttrib);
//		RasterizeTriangle_Top(newX, Y2, X2, Y2, X3, Y3, *pBaseVertex, ddxAttrib, ddyAttrib);
//	}
//}

void Rasterizer::RasterizeTriangle_Bottom( float X1, float Y1, float X2, float Y2, float X3, float Y3,
											const VS_Output& pBase, const VS_Output& ddxAttrib, const VS_Output& ddyAttrib )
{
	float dxLeft, dxRight;
	float xStart, xEnd;
	int32_t iYStart, iYEnd;
	int32_t iXStart, iXEnd;

	if (X3 < X2)	std::swap(X3, X2);

	float height = Y2 - Y1;
	dxLeft = (X2 - X1) / height;
	dxRight = (X3 - X1) / height;

	xStart = X1;
	xEnd = X1;

	// vertical clip
	if (Y1 < MinClipY)
	{
		xStart += (MinClipY - Y1) * dxLeft;
		xEnd += (MinClipY - Y1) * dxRight;

		iYStart = (int32_t)MinClipY; 
	}
	else
	{
		iYStart = (int32_t)ceilf(Y1 );

		xStart += (iYStart - Y1) * dxLeft;
		xEnd += (iYStart - Y1) * dxRight;
	}

	if (xEnd > 262.0f)
	{
		int a = 0;
	}

	if (Y3 > MaxClipY)
	{
		// Top-Left fill rule
		iYEnd = (int32_t)MaxClipY - 1; 
	}
	else
	{
		iYEnd = (int32_t)ceilf(Y3 );
	}

	for (int32_t iY = iYStart; iY < iYEnd; ++iY, xStart += dxLeft, xEnd += dxRight)
	{
		iXStart = (int32_t)ceilf(xStart);
		iXEnd = (int32_t)ceilf(xEnd);

		//水平裁剪
		if (iXStart < MinClipX)
		{
			iXStart = (int32_t)MinClipX;
			if (iXEnd < MinClipX)
				continue;		
		}

		if (iXEnd > MaxClipX)
		{
			iXEnd = (int32_t)MaxClipX;
			if (iXStart > MaxClipX)
				continue;
		}

		float fOffsetX = iXStart - pBase.Position.X();
		float fOffsetY = iY - pBase.Position.Y();

		VS_Output vsOutput;
		VS_Output_BaryCentric(&vsOutput, &pBase, &ddxAttrib, &ddyAttrib, fOffsetX, fOffsetY, mCurrVSOutputCount);
		RasterizeScanline(iXStart, iXEnd, iY, &vsOutput, &ddxAttrib);
	}
}

void Rasterizer::RasterizeTriangle_Top( float X1, float Y1, float X2, float Y2, float X3, float Y3,
									   const VS_Output& pBase, const VS_Output& ddxAttrib, const VS_Output& ddyAttrib )
{
	float dxLeft, dxRight;
	float xStart, xEnd;
	int32_t iYStart, iYEnd;
	int32_t iXStart, iXEnd;

	if (X2 < X1)	std::swap(X1, X2);

	float height = Y3 - Y1;
	dxLeft = (X3 - X1) / height;
	dxRight = (X3 - X2) / height;

	xStart = X1;
	xEnd = X2;

	// vertical clip
	if (Y1 < MinClipY)
	{
		xStart += (MinClipY - Y1) * dxLeft;
		xEnd += (MinClipY - Y1) * dxRight;

		iYStart = (int32_t)MinClipY; 
	}
	else
	{
		iYStart = (int32_t)ceilf(Y1);

		xStart += (iYStart - Y1) * dxLeft;
		xEnd += (iYStart - Y1) * dxRight;
	}

	if (Y3 > MaxClipY)
	{
		// Top-Left fill rule
		iYEnd = (int32_t)MaxClipY; 
	}
	else
	{
		iYEnd = (int32_t)ceilf(Y3);
	}

	if (xEnd > 262.0f && iYStart < iYEnd)
	{
		int a = 0;
	}

	for (int32_t iY = iYStart; iY < iYEnd; ++iY, xStart += dxLeft, xEnd += dxRight)
	{
		iXStart = (int32_t)ceilf(xStart);
		iXEnd = (int32_t)ceil(xEnd);

		//水平裁剪
		if (iXStart < MinClipX)
		{
			iXStart = (int32_t)MinClipX;
			if (iXEnd < MinClipX)
				continue;		
		}

		if (iXEnd > MaxClipX)
		{
			iXEnd = (int32_t)MaxClipX;
			if (iXStart > MaxClipX)
				continue;
		}

		float fOffsetX = iXStart - pBase.Position.X();
		float fOffsetY = iY - pBase.Position.Y();

		VS_Output vsOutput;
		VS_Output_BaryCentric(&vsOutput, &pBase, &ddxAttrib, &ddyAttrib, fOffsetX, fOffsetY, mCurrVSOutputCount);
		RasterizeScanline(iXStart, iXEnd, iY, &vsOutput, &ddxAttrib);
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
		case CF_Less: DEPTH_TEST(srcDepth < destDepth);
		case CF_LessEqual: DEPTH_TEST(srcDepth <= destDepth);
		case CF_GreaterEqual: DEPTH_TEST(srcDepth >= destDepth);
		case CF_Greater: DEPTH_TEST(srcDepth > destDepth);
		case CF_AlwaysPass: break;
		}

		mDevice.mCurrentFrameBuffer->WritePixel(X, Y, &PSOutput,
			mDevice.DepthStencilState.DepthWriteMask ? &srcDepth : NULL);
	}

#undef DEPTH_TEST
}

void Rasterizer::PreDraw()
{
	// Set vertex varing count
	mCurrVSOutputCount = mDevice.mVertexShaderStage->VSOutputCount;

	mCurrFrameBuffer = mDevice.GetCurrentFrameBuffer();

	mTilesQueueSize = 0;

	for (auto& threadVertexSize : mNumVerticesThreads)
		threadVertexSize = 0;

	// reset each thread's vertex cache
	for (auto& threadVertexCache : mVertexCaches)
	{
		for (auto& cacheElement : threadVertexCache)
		{
			cacheElement.Index = UINT_MAX;
		}
	}
}

void Rasterizer::PostDraw()
{

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

	const VS_Output& V1 = vertices[0];
	const VS_Output& V2 = vertices[1];
	const VS_Output& V3 = vertices[2];

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
	bool ccw = false;
	if( BackFaceCulling( vertices[clipVertices[srcStage][0]], vertices[clipVertices[srcStage][1]], vertices[clipVertices[srcStage][2]], &ccw))
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
		if (!ccw)
			Binning(vertices[clipVertices[srcStage][0]], vertices[clipVertices[srcStage][i]], vertices[clipVertices[srcStage][i-1]], threadIdx);
		else
			Binning(vertices[clipVertices[srcStage][0]], vertices[clipVertices[srcStage][i-1]], vertices[clipVertices[srcStage][i]], threadIdx);
	}
}

void Rasterizer::SetupGeometryTiled( std::vector<VS_Output>& outVertices, std::vector<RasterFaceTiled>& outFaces, uint32_t theadIdx, ThreadPackage package )
{
	VS_Output clippedVertices[7]; // 7 enough ???

	for (uint32_t iPrim = package.Start; iPrim < package.End; ++iPrim)
	{
		const uint32_t baseVertex = iPrim * 4;
		const uint32_t baseFace = iPrim;

		// fetch vertices
		for (uint32_t iVertex = 0; iVertex < 3; ++ iVertex)
		{		
			const uint32_t index = mDevice.FetchIndex(iPrim * 3 + iVertex); 
			const VS_Output& v = FetchVertex(index, theadIdx);
			memcpy(&clippedVertices[iVertex], &v, sizeof(VS_Output));
			//VS_Output_Copy(&clippedVertices[iVertex], &v, mCurrVSOutputCount);
		}

		ClipTriangleTiled(clippedVertices, theadIdx);		
	}
}

void Rasterizer::Binning( const VS_Output& V1, const VS_Output& V2, const VS_Output& V3, uint32_t threadIdx )
{
	uint32_t baseIdx = mNumVerticesThreads[threadIdx];
	uint32_t faceIdx = baseIdx / 3;

	RasterFaceTiled& face = mFacesThreads[threadIdx][faceIdx];

	// 28.4 fixed-point coordinates
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

	// Correct for fill convention
	if(DY12 < 0 || (DY12 == 0 && DX12 > 0)) C1++;
	if(DY23 < 0 || (DY23 == 0 && DX23 > 0)) C2++;
	if(DY31 < 0 || (DY31 == 0 && DX31 > 0)) C3++;

	// keep half-space constant, so when in block text, don't need calculate it again
	face.C1 = C1; face.C2 = C2; face.C3 = C3;

	face.X[0] = X1; face.X[1] = X2; face.X[2] = X3;
	face.Y[0] = Y1; face.Y[1] = Y2; face.Y[2] = Y3;

	// Compute bounding box, fixed point
	face.MinX = (Min(X1, Min(X2, X3)));
	face.MaxX = (Max(X1, Max(X2, X3)));
	face.MinY = (Min(Y1, Min(Y2, Y3)));
	face.MaxY = (Max(Y1, Max(Y2, Y3)));

	// Compute tile bounding box
	const int32_t minTileX = Max(face.MinX >> ( 4 + TileSizeShift), 0);
	const int32_t minTileY = Max(face.MinY >> ( 4 + TileSizeShift), 0);
	const int32_t maxTileX = Min(face.MaxX >> ( 4 + TileSizeShift), mNumTileX-1);
	const int32_t maxTileY = Min(face.MaxY >> ( 4 + TileSizeShift), mNumTileY-1);

	if ((maxTileX == minTileX) && (maxTileY == minTileY))
	{
		// Small primitive
		Tile& tile = mTiles[minTileY * mNumTileX + minTileX];
		tile.TriQueue[threadIdx][tile.TriQueueSize[threadIdx]++] = faceIdx << 1;
	}
	else
	{
		for (int32_t y = minTileY; y <= maxTileY; ++y)
		{
			for (int32_t x = minTileX; x <= maxTileX; ++x)
			{
				// Corners of block, fixed point
				int32_t x0 = (x << TileSizeShift) << 4;
				int32_t x1 = (((x + 1) << TileSizeShift) - 1) << 4;
				int32_t y0 = (y << TileSizeShift) << 4;
				int32_t y1 = (((y + 1) << TileSizeShift) - 1) << 4;

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
				if(a == 0x0 || b == 0x0 || c == 0x0) 
					continue;

				// Test if we can trivially accept the entire tile
				uint32_t accept = ( a != 0xF || b != 0xF || c != 0xF ) ? 0 : 1;
	
				Tile& tile = mTiles[y * mNumTileX + x];
				tile.TriQueue[threadIdx][tile.TriQueueSize[threadIdx]++] = (faceIdx << 1) | accept;  // least is a flag for partial or accept bit
			}
		}
	}

	VS_Output* vsOut0 = &mVerticesThreads[threadIdx][baseIdx];
	VS_Output* vsOut1 = &mVerticesThreads[threadIdx][baseIdx+1];
	VS_Output* vsOut2 = &mVerticesThreads[threadIdx][baseIdx+2];

	VS_Output_Copy(vsOut0, &V1, mCurrVSOutputCount);
	VS_Output_Copy(vsOut1, &V2, mCurrVSOutputCount);
	VS_Output_Copy(vsOut2, &V3, mCurrVSOutputCount);

	face.V[0] = vsOut0; face.V[1] = vsOut1; face.V[2] = vsOut2;
	mNumVerticesThreads[threadIdx] += 3;

	/** 
	 * Compute difference of attributes and store in face. When rasterize tiles,
	 * we can directly use it. Calculate once, used several times in different tiles.
	 */
	VS_Output vsOutput01, vsOutput02;
	VS_Output_Sub(&vsOutput01, vsOut1, vsOut0, mCurrVSOutputCount);
	VS_Output_Sub(&vsOutput02, vsOut2, vsOut0, mCurrVSOutputCount);

	const float area = vsOutput01.Position.X() * vsOutput02.Position.Y() - vsOutput02.Position.X() * vsOutput01.Position.Y();
	const float invArea = 1.0f / area;

	VS_Output ddxAttrib, ddyAttrib;
	VS_Output_Difference(&face.ddxVarying, &face.ddyVarying, &vsOutput01, &vsOutput02, invArea, mCurrVSOutputCount);	
}

void Rasterizer::DrawTiled( PrimitiveType primitiveType, uint32_t primitiveCount )
{
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

	if (idx == 0)
	{
		mThreadPackage[idx].Start = 0;
		mThreadPackage[idx].End = primitivesPerThread;
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
		mNumVerticesThreads[idx] = 0;

		const uint32_t primCount = (mThreadPackage[idx].End - mThreadPackage[idx].Start);

		// after frustum clip, one triangle can generate maximum 5 vertices, maximum 3 triangle faces
		mVerticesThreads[idx].resize(primCount * 5);
		mFacesThreads[idx].resize(primCount * 3);
	}

	// profiler
	mProfiler.StartTimer("Vertex Process + Binning");
	
	for (idx = 0; idx < numWorkThreads - 1; ++idx)
	{
		//SetupGeometryTiled(mVerticesThreads[idx], mFacesThreads[idx], idx, mThreadPackage[idx]);
		theadPool.schedule(std::bind(&Rasterizer::SetupGeometryTiled, this, std::ref(mVerticesThreads[idx]), 
			std::ref(mFacesThreads[idx]), idx, mThreadPackage[idx]));
	}
	SetupGeometryTiled(mVerticesThreads[idx], mFacesThreads[idx], idx, mThreadPackage[idx]);
	theadPool.wait();  // Synchronization
	
	mProfiler.EndTimer("Vertex Process + Binning");
	auto elapsedTimeVB = mProfiler.GetElapsedTime("Vertex Process + Binning");


	mProfiler.StartTimer("Build Tiles Job Queue");
	// build non-empty tile job queue
	for (int32_t y = 0; y < mNumTileY; ++y)
	{
		for (int32_t x = 0; x < mNumTileX; ++x)
		{
			const int32_t tileIdx = y * mNumTileX + x;
			for (uint32_t i = 0; i < numWorkThreads; ++i)
			{
				if (mTiles[tileIdx].TriQueueSize[i] > 0)
				{
					mTilesQueue[mTilesQueueSize++] = tileIdx;
					break;
				}
			}	
		}
	}
	mProfiler.EndTimer("Build Tiles Job Queue");
	auto elapsedTimeBT = mProfiler.GetElapsedTime("Build Tiles Job Queue");

	mProfiler.StartTimer("RasterizeTiles");

	// Rasterize each tile in tile job queue
	std::atomic<uint32_t> workingPackage(0);
	for (size_t i = 0; i < numWorkThreads - 1; ++i)
	{
		theadPool.schedule(std::bind(&Rasterizer::RasterizeTiles, this, std::ref(mTilesQueue), std::ref(workingPackage), mTilesQueueSize));
	}
	RasterizeTiles(mTilesQueue, workingPackage, mTilesQueueSize);
	theadPool.wait();  // Synchronization

	mProfiler.EndTimer("RasterizeTiles");
	auto elapsedTimeRT = mProfiler.GetElapsedTime("RasterizeTiles");
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

			int32_t tileX = tile.X << 4; // fixed point
			int32_t tileY = tile.Y << 4; // fixed point
			int32_t tileWidth = tile.Width << 4; // fixed point
			int32_t tileHeight = tile.Height << 4; // fixed point

			for (uint32_t iThread = 0; iThread < numWorkThreads; ++iThread)
			{
				auto tris = tile.TriQueueSize[iThread];
				for (uint32_t iTri = 0; iTri < tile.TriQueueSize[iThread]; ++iTri)
				{
					uint32_t faceIdx = tile.TriQueue[iThread][iTri];
					uint32_t accept = faceIdx & 0x1;
					faceIdx = faceIdx >> 1; 

					RasterFaceTiled& face = mFacesThreads[iThread][faceIdx];
					
					if (accept)
					{
						DrawPixels(face, tile.X, tile.Y, tile.X + tile.Width, tile.Y + tile.Height);
					}
					else
					{
						DrawPartialTile(face, tileX, tileY, tileWidth, tileHeight);
					}
				}

				tile.TriQueueSize[iThread] = 0;
			}

		}

		localWorkingPackage  = workingPackage ++;
	}
}

void Rasterizer::DrawPartialTile(const RasterFaceTiled& face, int32_t tileX, int32_t tileY, int32_t tileWidth, int32_t tileHeight)
{
	const VS_Output* V1 = face.V[0];
	const VS_Output* V2 = face.V[1];
	const VS_Output* V3 = face.V[2];

	// 28.4 fixed-point coordinates
	const int32_t X1 = face.X[0];
	const int32_t X2 = face.X[1];
	const int32_t X3 = face.X[2];

	const int32_t Y1 = face.Y[0];
	const int32_t Y2 = face.Y[1];
	const int32_t Y3 = face.Y[2];

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

#ifdef USE_SIMD
	const __m128i OffsetDY12 = _mm_set_epi32(FDY12 * 3, FDY12 * 2, FDY12 * 1, 0);
	const __m128i OffsetDY23 = _mm_set_epi32(FDY12 * 3, FDY12 * 2, FDY12 * 1, 0);
	const __m128i OffsetDY31 = _mm_set_epi32(FDY31 * 3, FDY31 * 2, FDY31 * 1, 0);
#endif

	// Half-edge constants
	const int32_t C1 = face.C1;
	const int32_t C2 = face.C2;
	const int32_t C3 = face.C3;

	// Compute bounding box
	int32_t minX = (Max(face.MinX, tileX) + 0xF) >> 4;
	int32_t maxX = (Min(face.MaxX, tileX + tileWidth) + 0xF) >> 4;
	int32_t minY = (Max(face.MinY, tileY) + 0xF) >> 4;
	int32_t maxY = (Min(face.MaxY, tileY + tileHeight) + 0xF) >> 4;

	// Block size, standard 8x8 (must be power of two)
	const int BlockSize = 8;

	// Start in corner of 8x8 block
	minX &= ~(BlockSize - 1);
	minY &= ~(BlockSize - 1);

    const VS_Output* pBaseVertex = face.V[0];

	for (int32_t y = minY; y < maxY; y += BlockSize)
	{
		for (int32_t x = minX; x < maxX; x += BlockSize)
		{
			// Corners of block
			int32_t x0 = x << 4;
			int32_t x1 = (x + BlockSize - 1) << 4;
			int32_t y0 = y << 4;
			int32_t y1 = (y + BlockSize - 1) << 4;

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
			
			// Accept whole block when totally covered
			if( a == 0xF && b == 0xF && c == 0xF )
			{
				// draw whole block
				DrawPixels(face, x, y, Min(x+BlockSize, maxX), Min(y+BlockSize, maxY));
			}
			else
			{
				int32_t CY1 = C1 + DX12 * y0 - DY12 * x0;
				int32_t CY2 = C2 + DX23 * y0 - DY23 * x0;
				int32_t CY3 = C3 + DX31 * y0 - DY31 * x0;

				for(int32_t iy = y; iy < Min(y + BlockSize, maxY); iy++)
				{
#ifdef USE_SIMD
					// 前面4个像素
					__m128i CX1 = _mm_sub_epi32(_mm_set1_epi32(CY1), OffsetDY12);
					__m128i CX2 = _mm_sub_epi32(_mm_set1_epi32(CY2), OffsetDY12);
					__m128i CX3 = _mm_sub_epi32(_mm_set1_epi32(CY3), OffsetDY12);

					__m128i CX1Mask = _mm_cmpgt_epi32(CX1, _mm_setzero_si128());
					__m128i CX2Mask = _mm_cmpgt_epi32(CX2, _mm_setzero_si128());
					__m128i CX3Mask = _mm_cmpgt_epi32(CX3, _mm_setzero_si128());

					__m128i CXMaskComp = _mm_and_si128( CX1Mask, _mm_and_si128( CX2Mask, CX3Mask ) );

					// Generate a 4-bit mask from the composite 128-bit mask 
					int32_t mask = _mm_movemask_ps(_mm_castsi128_ps(CXMaskComp));				
					
					if (mask)
					{
						DrawMaskedPixels(face, mask, x, Min(x+4, maxX), iy);
					}


					// 后面4个像素
					CX1 = _mm_sub_epi32(CX1, OffsetDY12);
					CX2 = _mm_sub_epi32(CX2, OffsetDY12);
					CX3 = _mm_sub_epi32(CX3, OffsetDY12);

					CX1Mask = _mm_cmpgt_epi32(CX1, _mm_setzero_si128());
					CX2Mask = _mm_cmpgt_epi32(CX2, _mm_setzero_si128());
					CX3Mask = _mm_cmpgt_epi32(CX3, _mm_setzero_si128());

					CXMaskComp = _mm_and_si128( CX1Mask, _mm_and_si128( CX2Mask, CX3Mask ) );

					mask = _mm_movemask_ps(_mm_castsi128_ps(CXMaskComp));		
					if (mask)
					{
						x+=4;
						DrawMaskedPixels(face, mask, x, Min(x+4, maxX), iy);
					}
#else
					int32_t CX1 = CY1;
					int32_t CX2 = CY2;
					int32_t CX3 = CY3;

					for(int32_t ix = x; ix < Min(x + BlockSize, maxX); ix++)
					{
						if(CX1 > 0 && CX2 > 0 && CX3 > 0)
						{
							VS_Output vsOutput;
							float fOffsetX = ix - pBaseVertex->Position.X();
							float fOffsetY = iy - pBaseVertex->Position.Y();
							VS_Output_BaryCentric(&vsOutput, pBaseVertex, &face.ddxVarying, &face.ddyVarying, fOffsetX, fOffsetY, mCurrVSOutputCount);
						
							DrawPixel(ix, iy, vsOutput);
						}

						CX1 -= FDY12;
						CX2 -= FDY23;
						CX3 -= FDY31;
					}
#endif 

					CY1 += FDX12;
					CY2 += FDX23;
					CY3 += FDX31;
				}
			}
		}
	}
}

void Rasterizer::DrawPixels(const RasterFaceTiled& face, int32_t xStart, int32_t yStart, int32_t xEnd, int32_t yEnd)
{
	const VS_Output* pBaseVertex = face.V[0];
	
	for (int32_t iY = yStart; iY < yEnd; ++iY)
	{
		for (int32_t iX = xStart; iX < xEnd; ++iX)
		{
			float fOffsetX = (float)iX - pBaseVertex->Position.X();
			float fOffsetY = (float)iY - pBaseVertex->Position.Y();

			VS_Output VSOutput;
			VS_Output_BaryCentric(&VSOutput, pBaseVertex, &face.ddxVarying, &face.ddyVarying, fOffsetX, fOffsetY, mCurrVSOutputCount);

			DrawPixel(iX, iY, VSOutput);
		}
	}
}

void Rasterizer::DrawMaskedPixels( const RasterFaceTiled& face, int32_t mask, int32_t xStart, int32_t xEnd, int32_t iY )
{
	const VS_Output* pBaseVertex = face.V[0];

	VS_Output vsOutput;
	float fOffsetX, fOffsetY = iY - pBaseVertex->Position.Y();

	static const int32_t bits[4] = {0x1, 0x2, 0x4, 0x8};

	for (int32_t iX= xStart; iX < xEnd; ++iX)
	{
		if (mask & bits[iX]) 
		{
			fOffsetX = iX - pBaseVertex->Position.X();

			VS_Output_BaryCentric(&vsOutput, pBaseVertex, &face.ddxVarying, &face.ddyVarying, fOffsetX, fOffsetY, mCurrVSOutputCount);
			DrawPixel(iX, iY, vsOutput);
		}
	}
}

void Rasterizer::DrawPixel( uint32_t iX, uint32_t iY, const VS_Output& vsOutput )
{
#define DEPTH_TEST(condition) if((condition)) break; else return;

	float srcDepth, destDepth;

	// read back buffer pixel
	mDevice.mCurrentFrameBuffer->ReadPixel(iX, iY, NULL, &destDepth);

	// Get depth of current pixel
	srcDepth = vsOutput.Position.Z();

	VS_Output PSInput;
	float curPixelInvW = 1.0f / vsOutput.Position.W();
	VS_Output_Mul( &PSInput, &vsOutput, curPixelInvW, mCurrVSOutputCount );

	// Execute the pixel shader
	//m_TriangleInfo.iCurPixelX = i_iX;
	PS_Output PSOutput;
	if( !mDevice.mPixelShaderStage->GetPixelShader()->Execute(&PSInput, &PSOutput, &srcDepth ))
	{
		// kill this pixel
		return;
	}

	// Perform depth-test
	switch( mDevice.DepthStencilState.DepthFunc )
	{
	case CF_AlwaysFail: return;
	case CF_Equal: DEPTH_TEST(fabsf( srcDepth - destDepth ) < FLT_EPSILON);
	case CF_NotEqual: DEPTH_TEST(fabsf( srcDepth - destDepth ) >= FLT_EPSILON);
	case CF_Less: DEPTH_TEST(srcDepth < destDepth);
	case CF_LessEqual: DEPTH_TEST(srcDepth <= destDepth);
	case CF_GreaterEqual: DEPTH_TEST(srcDepth >= destDepth);
	case CF_Greater: DEPTH_TEST(srcDepth > destDepth);
	case CF_AlwaysPass: break;
	}

	mDevice.mCurrentFrameBuffer->WritePixel(iX, iY, &PSOutput,
		mDevice.DepthStencilState.DepthWriteMask ? &srcDepth : NULL);

#undef DEPTH_TEST
}




