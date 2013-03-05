#include "Rasterizer.h"
#include "RenderDevice.h"
#include "FrameBuffer.h"
#include "Cache.hpp"
#include "threadpool.h"
#include "stack_pool.h"

using namespace RxLib;

#define InRange(v, a, b) ((a) <= (v) && (v) <= (b))

namespace {

inline bool FCMP(float a, float b)
{
	return true;
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

Rasterizer::Rasterizer( RenderDevice& device )
	: RenderStage(device)
{
	// Near and Far plane
	mClipPlanes[0] = float4(0, 0, 1, 0);
	mClipPlanes[1] = float4(0, 0, -1, 1);
}

Rasterizer::~Rasterizer(void)
{
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

	if( mDevice.RasterizerState.FrontCounterClockwise )  
	{
		if( signedArea >= 0.0f )
			return true;
	}
	else	
	{
		if( signedArea <= 0.0f )
			return true;
	}

	return false;
}

uint32_t Rasterizer::Clip( VS_Output* clipped, const VS_Output& v0, const VS_Output& v1, const VS_Output& v2 )
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
	
	for (uint32_t i = 0; i < resultNumVertices; ++i)
	{
		memcpy(&clipped[i], pClipVertices[srcStage][i], sizeof(VS_Output));
	}

	return resultNumVertices;
}

void Rasterizer::Draw( PrimitiveType primitiveType, uint32_t primitiveCount )
{
	ASSERT(primitiveType == PT_Triangle_List); // Only support triangle list 

	// after frustum clip, one triangle can generate maximum 4 vertices
	mClippedVertices.resize(primitiveCount * 4);
	mClippedFaces.resize(primitiveCount);

	std::atomic<uint32_t> workingPackage(0);

	//Vertex Processing
	ScheduleAndJoin( GlobalThreadPool(), 
		std::bind(&Rasterizer::SetupGeometry, this, std::ref(mClippedVertices), std::ref(mClippedFaces), 
		std::ref(workingPackage), primitiveCount));


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
	uint32_t num_packages = (primitiveCount + NumPrimitivePerPackage - 1) / NumPrimitivePerPackage;
	uint32_t localWorkingPackage  = workingPackage ++;

	//LRUCache<uint32_t, VS_Output, VertexCacheSize> vertexCache(std::bind(&RenderDevice::FetchVertex, &mDevice, std::placeholders::_1));
	
	DirectMapCache<uint32_t, VS_Output, VertexCacheSize> vertexCache(std::bind(&RenderDevice::FetchVertex, &mDevice, std::placeholders::_1));

	while (localWorkingPackage < num_packages)
	{
		const uint32_t start = localWorkingPackage * NumPrimitivePerPackage;
		const uint32_t end = (std::min)(primitiveCount, start + NumPrimitivePerPackage);

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
			uint32_t numCliped = Clip(&outVertices[baseVertex], *pVSOutputs[0], *pVSOutputs[1], *pVSOutputs[2]);
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

					/*if (outFaces[baseFace].FrontFace)
					{
					outFaces[baseFace].Indices[iTri * 3 + 1] = baseVertex + iTri * 3 + 0;
					outFaces[baseFace].Indices[iTri * 3 + 2] = baseVertex + iTri * 3 + 1;
					}
					else
					{
					outFaces[baseFace].Indices[iTri * 3 + 1] = baseVertex + iTri * 3 + 1;
					outFaces[baseFace].Indices[iTri * 3 + 2] = baseVertex + iTri * 3 + 0;
					}*/
			}
		}

		localWorkingPackage = workingPackage++;
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


	// Screenspace-position references ----------------------------------------
	const float4& vA = pVertices[0]->Position;
	const float4& vB = pVertices[1]->Position;
	const float4& vC = pVertices[2]->Position;

	// Calculate slopes for stepping ------------------------------------------
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

	// Begin rasterization ----------------------------------------------------
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

void Rasterizer::RasterizeTriangle_Top( const VS_Output& vsOut1, const VS_Output& vsOut2, const VS_Output& vsOut3 )
{
	//     1___2
	//     |  /
	//     | /
	// 	   |/
	// 	   3

	float dxRight,    // the dx/dy ratio of the right edge of line
		dxLeft,     // the dx/dy ratio of the left edge of line
		xStart,xEnd,       // the starting and ending points of the edges
		height,      // the height of the triangle
		right,       // used by clipping
		left;

	int32_t iY1,iY3,loopY; // integers for y loops

	float x1 = vsOut1.Position.X();
	float x2 = vsOut2.Position.X();
	float x3 = vsOut3.Position.X();

	float y1 = vsOut1.Position.Y();
	float y2 = vsOut2.Position.Y();
	float y3 = vsOut3.Position.Y();

	const Viewport& vp = mDevice.mCurrentFrameBuffer->GetViewport();
	const int32_t MinClipY = vp.Top;
	const int32_t MaxClipY = vp.Top + vp.Height;
	const int32_t MinClipX = vp.Left;
	const int32_t MaxClipX = vp.Left + vp.Width;

	const float fMinClipY = (float)MinClipY;
	const float fMaxClipY = (float)MaxClipY;
	const float fMinClipX = (float)MinClipX;
	const float fMaxClipX = (float)MaxClipX;

	// compute delta's
	height = y3-y1;

	dxLeft  = (x3-x1)/height;
	dxRight = (x3-x2)/height;

	// set starting points
	xStart = x1;
	xEnd = x2;

	// perform y clipping
	if (y1 < fMinClipY)
	{
		iY1 = MinClipY;

		xStart = xStart + dxLeft * (iY1 - y1);
		xEnd = xEnd + dxRight * (iY1 - y1);
	}
	else
	{
		// make sure top left fill convention is observed
		iY1 = (int32_t)ceil(y1);

		xStart = xStart + dxLeft * (iY1 - y1);
		xEnd = xEnd + dxRight * (iY1 - y1);

	} 

	if (y3 > fMaxClipY)
	{
		iY3 = MaxClipY;
	} 
	else
	{
		iY3 = (int32_t)ceil(y3);
	} 

	if (InRange(x1, fMinClipX, fMaxClipX) && InRange(x2, fMinClipX, fMaxClipX)&& InRange(x3, fMinClipX, fMaxClipX))
	{
		// draw the triangle
		for (loopY = iY1; loopY < iY3; loopY++)
		{

			// adjust starting point and ending point
			xStart += dxLeft;
			xEnd += dxRight;
		}
	} 
	else
	{
		// draw the triangle
		for (loopY = iY1; loopY < iY3; loopY++)
		{
			// do x clip
			left  = xStart;
			right = xEnd;

			// adjust starting point and ending point
			xStart += dxLeft;
			xEnd += dxRight;

			// clip line
			if (left < fMinClipX)
			{
				left = fMinClipX;

				if (right < fMinClipX)
					continue;
			}

			if (right > fMaxClipX)
			{
				right = fMaxClipX;

				if (left > fMaxClipX)
					continue;
			}	
		} 
	} 
}

void Rasterizer::RasterizeTriangle_Bottom( const VS_Output& vsOut1, const VS_Output& vsOut2, const VS_Output& vsOut3 )
{
	// 	     1  
	// 	    /|  
	// 	   / |  
	// 2  /  |3  
	//	 ----    

	float dxRight,    // the dx/dy ratio of the right edge of line
		dxLeft,     // the dx/dy ratio of the left edge of line
		xStart,xEnd,       // the starting and ending points of the edges
		height,      // the height of the triangle
		right,       // used by clipping
		left;

	int32_t iY1,iY3,loopY; // integers for y loops

	float x1 = vsOut1.Position.X();
	float x2 = vsOut2.Position.X();
	float x3 = vsOut3.Position.X();

	float y1 = vsOut1.Position.Y();
	float y2 = vsOut2.Position.Y();
	float y3 = vsOut3.Position.Y();

	const Viewport& vp = mDevice.mCurrentFrameBuffer->GetViewport();
	const int32_t MinClipY = vp.Top;
	const int32_t MaxClipY = vp.Top + vp.Height;
	const int32_t MinClipX = vp.Left;
	const int32_t MaxClipX = vp.Left + vp.Width;

	const float fMinClipY = (float)MinClipY;
	const float fMaxClipY = (float)MaxClipY;
	const float fMinClipX = (float)MinClipX;
	const float fMaxClipX = (float)MaxClipX;

	// compute delta's
	height = y3-y1;

	dxLeft  = (x2-x1)/height;
	dxRight = (x3-x1)/height;

	// set starting points
	xStart = x1;
	xEnd = x1;

	// perform y clipping
	if (y1 < fMinClipY)
	{
		iY1 = MinClipY;

		xStart = xStart + dxLeft * (iY1 - y1);
		xEnd = xEnd + dxRight * (iY1 - y1);
	}
	else
	{
		// make sure top left fill convention is observed
		iY1 = (int32_t)ceil(y1);

		xStart = xStart + dxLeft * (iY1 - y1);
		xEnd = xEnd + dxRight * (iY1 - y1);
	} 

	if (y3 > fMaxClipY)
	{
		iY3 = MaxClipY;
	} 
	else
	{
		iY3 = (int32_t)ceil(y3);
	} 

	if (InRange(x1, fMinClipX, fMaxClipX) && InRange(x2, fMinClipX, fMaxClipX)&& InRange(x3, fMinClipX, fMaxClipX))
	{
		// draw the triangle
		for (loopY = iY1; loopY < iY1; loopY++)
		{

			// adjust starting point and ending point
			xStart += dxLeft;
			xEnd += dxRight;
		}
	} 
	else
	{
		// draw the triangle
		for (loopY = iY1; loopY < iY3; loopY++)
		{
			// do x clip
			left  = xStart;
			right = xEnd;

			// adjust starting point and ending point
			xStart += dxLeft;
			xEnd += dxRight;

			// clip line
			if (left < fMinClipX)
			{
				left = fMinClipX;

				if (right < fMinClipX)
					continue;
			}

			if (right > fMaxClipX)
			{
				right = fMaxClipX;

				if (left > fMaxClipX)
					continue;
			}	
		} 
	} 
}

void Rasterizer::PreDraw()
{
	mCurrVSOutputCount = mDevice.mVertexShaderStage->VSOutputCount;
}

void Rasterizer::PostDraw()
{

}



