#include "FrameBuffer.h"
#include "RenderState.h"
#include "RenderDevice.h"
#include "Texture.h"
#include "Context.h"
#include "Shader.h"
#include "threadpool.h"
#include <MathUtil.hpp>

namespace {

inline void BlendOp(ColorRGBA* pOut, const ColorRGBA* pSrc, const ColorRGBA* pDest, const ColorRGBA* pCurrFactor, const BlendState::RenderTargetBlend& state)
{
	float srcAlpha, destAlpha;
	ColorRGBA srcColor, destColor;

	switch (state.SrcBlendAlpha)
	{
	case ABF_Src_Alpha: srcAlpha =  pSrc->A; break;
	case ABF_Dst_Alpha: srcAlpha = pDest->A; break;
	case ABF_Inv_Src_Alpha: srcAlpha = 1.0f - pSrc->A; break;
	case ABF_Inv_Dst_Alpha: srcAlpha = 1.0f - pDest->A; break;
	case ABF_Src_Alpha_Sat: srcAlpha = (std::min)(1.0f, pSrc->A);
	}

	switch (state.DestBlendAlpha)
	{
	case ABF_Src_Alpha: destAlpha = pSrc->A; break;
	case ABF_Dst_Alpha: destAlpha =  pDest->A; break;
	case ABF_Inv_Src_Alpha: destAlpha =  1.0f - pSrc->A; break;
	case ABF_Inv_Dst_Alpha: destAlpha =  1.0f - pDest->A; break;
	case ABF_Src_Alpha_Sat: destAlpha =  (std::min)(1.0f, pSrc->A); break;
	}

	switch (state.SrcBlend)
	{
	case ABF_Zero: srcColor = ColorRGBA(0, 0, 0, 0); break;
	case ABF_One: srcColor = ColorRGBA(1, 1, 1, 1); break;
	case ABF_Src_Color: srcColor = *pSrc; break;
	case ABF_Dst_Color: srcColor = *pDest; break;
	case ABF_Blend_Factor: srcColor = *pCurrFactor; break;
	case ABF_Inv_Src_Color:
		srcColor.R = 1.0f - pSrc->R;
		srcColor.G = 1.0f - pSrc->G;
		srcColor.B = 1.0f - pSrc->B;
		break;
	case ABF_Inv_Dst_Color:
		srcColor.R = 1.0f - pDest->R;
		srcColor.G = 1.0f - pDest->G;
		srcColor.B = 1.0f - pDest->B;
		break;
	case ABF_Inv_Blend_Factor:
		srcColor.R = 1.0f - pCurrFactor->R;
		srcColor.G = 1.0f - pCurrFactor->G;
		srcColor.B = 1.0f - pCurrFactor->B;
		break;
	}

	switch (state.DestBlend)
	{
	case ABF_Zero: destColor = ColorRGBA(0, 0, 0, 0); break;
	case ABF_One: destColor = ColorRGBA(1, 1, 1, 1); break;
	case ABF_Src_Color: destColor = *pSrc; break;
	case ABF_Dst_Color: destColor = *pDest; break;
	case ABF_Blend_Factor: destColor = *pCurrFactor; break;
	case ABF_Inv_Src_Color:
		destColor.R = 1.0f - pSrc->R;
		destColor.G = 1.0f - pSrc->G;
		destColor.B = 1.0f - pSrc->B;
		break;
	case ABF_Inv_Dst_Color:
		destColor.R = 1.0f - pDest->R;
		destColor.G = 1.0f - pDest->G;
		destColor.B = 1.0f - pDest->B;
		break;
	case ABF_Inv_Blend_Factor:
		destColor.R = 1.0f - pCurrFactor->R;
		destColor.G = 1.0f - pCurrFactor->G;
		destColor.B = 1.0f - pCurrFactor->B;
		break;
	}

	switch (state.BlendOp)
	{
	case BOP_Add:
		*pOut = srcColor + destColor;
		break;
	case BOP_Sub:
		*pOut = srcColor - destColor;
		break;
	case BOP_Rev_Sub:
		*pOut = destColor - srcColor;
		break;
	case BOP_Min:
		break;
	case BOP_Max:
		break;
	}

	switch (state.BlendOpAlpha)
	{
	case BOP_Add:
		pOut->A = srcAlpha + destAlpha;
		break;
	case BOP_Sub:
		pOut->A = srcAlpha - destAlpha;
		break;
	case BOP_Rev_Sub:
		pOut->A = destAlpha - srcAlpha;
		break;
	case BOP_Min:
		pOut->A  = (std::min)(destAlpha, srcAlpha);
		break;
	case BOP_Max:
		pOut->A  = (std::max)(destAlpha, srcAlpha);
		break;
	}
}


}

FrameBuffer::FrameBuffer( int32_t width, int32_t height )
	:mDepthStencilTarget(0), mActice(false), mDirty(false)
{
	mViewport.Left = 0;
	mViewport.Width = width;
	mViewport.Top = 0;
	mViewport.Height = height;

	mViewportMatrix = RxLib::CreateViewportMatrixD3D(0.0f, 0.0f, (float)width, (float)height);


	// init render target tiles
	uint32_t extraPixelsX = width % TileSize;
	uint32_t numTileX = extraPixelsX ? (width / TileSize + 1) : (width / TileSize);

	uint32_t extraPixelsY = height % TileSize;
	uint32_t numTileY = extraPixelsX ? (height / TileSize + 1) : (height / TileSize);

	// set up each tile info 
	mTiles.resize(numTileX * numTileY);
	for (uint32_t y = 0; y < numTileY; ++y)
	{
		bool extraY = ((y == numTileY-1) && extraPixelsY);

		for (uint32_t x = 0; x < numTileX; ++x)
		{
			bool extraX = ((x == numTileX-1) && extraPixelsX);

			uint32_t index = y * numTileX + x;
			
			mTiles[index].X = x * TileSize; 
			mTiles[index].Y = y * TileSize;
			
			mTiles[index].Width = extraX ? extraPixelsX : TileSize;
			mTiles[index].Height = extraY ? extraPixelsY : TileSize;
		}
	}
}


FrameBuffer::~FrameBuffer(void)
{
}

void FrameBuffer::Attach( Attachment att, const shared_ptr<Texture2D>& renderTarget )
{
	switch(att)
	{
	case ATT_DepthStencil:
		{
			if(mDepthStencilTarget)
			{
				Detach(ATT_DepthStencil);
			}
			mIsDepthBuffered = true;
			mDepthStencilTarget = renderTarget;
			PixelFormatUtils::GetNumDepthStencilBits(mDepthStencilTarget->GetTextureFormat(), mDepthBits, mStencilBits);
		}
		break;
	default:
		{
			uint32_t index = att - ATT_Color0;

			// if it already has an render target attach it, detach if first. 
			if(index < mRenderTargets.size() && mRenderTargets[index])
			{
				Detach(att);
			}

			if(mRenderTargets.size() < index + 1 )
			{
				mRenderTargets.resize(index + 1);
			}

			mRenderTargets[index] = renderTarget;

			// Frame buffer中所有的render target的大小都一样，以最小的一个作为参考
			size_t minColorView = index;
			for(size_t i = 0; i < mRenderTargets.size(); ++i)
			{
				if(mRenderTargets[i])
				{
					minColorView = i;
				}
			}

			if(minColorView == index)
			{
				mWidth = renderTarget->GetWidth(0);
				mHeight = renderTarget->GetHeight(0);
				mColorFormat = renderTarget->GetTextureFormat();
			}
		}
	}

	mActice = true;
	mDirty = true;
}

void FrameBuffer::Detach( Attachment att )
{
	switch(att)
	{
	case ATT_DepthStencil:
		mDepthStencilTarget = nullptr;
		mIsDepthBuffered = false;
		mDepthBits = 0;
		mStencilBits = 0;
		break;
	default:
		uint32_t index = att - ATT_Color0;
		if(mRenderTargets.size() < index + 1 )
		{
			mRenderTargets.resize(index + 1);
		}

		if(mRenderTargets[index])
		{
			mRenderTargets[index] = nullptr;
		}
	}

	mDirty = true;
}

void FrameBuffer::DetachAll()
{
	mRenderTargets.clear();
	mDepthStencilTarget = nullptr;
	mIsDepthBuffered = false;
	mDepthBits = 0;
	mStencilBits = 0;

	mDirty = true;
}

void FrameBuffer::OnBind()
{
	for (size_t i = 0; i < mRenderTargets.size(); ++i)
	{
		if(mRenderTargets[i])
		{
			mRenderTargets[i]->Map2D(0, TMA_Read_Write, 0, 0, 0, 0, mRTBuffer[ATT_Color0+i], mRTBufferPitch[ATT_Color0+i]);
		}
	}

	if (mDepthStencilTarget)
	{
		mDepthStencilTarget->Map2D(0, TMA_Read_Write, 0, 0, 0, 0, mRTBuffer[ATT_DepthStencil], mRTBufferPitch[ATT_DepthStencil]);
	}

	mDirty = false;
	mActice = true;
}

void FrameBuffer::OnUnbind()
{
	mActice = false;
}

void FrameBuffer::ReadPixel( int32_t x, int32_t y, PS_Output* oPixel, float* oDepth )
{
	if (oPixel)
	{
		for (size_t i = 0; i < mRenderTargets.size(); ++i)
		{
			if (mRenderTargets[i])
			{
				const PixelFormat& fmt = mRenderTargets[i]->GetTextureFormat();
				TextureFetch::ReadPixelFuncs[fmt](x, y, oPixel->Color[i], mRTBuffer[ATT_Color0+i], mRTBufferPitch[ATT_Color0+i]);
			}
		}
	}

	if (oDepth)
	{
		ColorRGBA depth;
		const PixelFormat& fmt = mDepthStencilTarget->GetTextureFormat();
		TextureFetch::ReadPixelFuncs[fmt](x, y, depth, mRTBuffer[ATT_DepthStencil], mRTBufferPitch[ATT_DepthStencil]);
		*oDepth = depth.R;
	}
}

void FrameBuffer::WritePixel( int32_t x, int32_t y, const PS_Output* psOutput, float* pDepth )
{
	RenderDevice& device = Context::GetSingleton().GetRenderDevice();

	const BlendState& blendState = device.BlendState;
	ColorRGBA dest;

	

	for (size_t i = 0; i < mRenderTargets.size(); ++i)
	{
		if (mRenderTargets[i])
		{
			if (blendState.RenderTarget[i].ColorWriteMask)
			{
				const ColorRGBA& src = psOutput->Color[i];	
				const PixelFormat& fmt = mRenderTargets[i]->GetTextureFormat();
		
				// do alpha blend
				if (blendState.RenderTarget[i].BlendEnable)
				{
					// read dest pixel
					TextureFetch::ReadPixelFuncs[fmt](x, y, dest, mRTBuffer[ATT_Color0+i], mRTBufferPitch[ATT_Color0+i]);
					BlendOp(&dest, &src, &dest, &device.CurrentBlendFactor, blendState.RenderTarget[ATT_Color0+i]);

					// upadte pixel
					TextureFetch::WritePixelFuncs[fmt](x, y, dest, mRTBuffer[ATT_Color0+i], mRTBufferPitch[ATT_Color0+i]);
				}
				else
				{
					// upadte pixel
					TextureFetch::WritePixelFuncs[fmt](x, y, src, mRTBuffer[ATT_Color0+i], mRTBufferPitch[ATT_Color0+i]);
				}
			}
		}
	}

	if (pDepth)
	{
		ColorRGBA depth(*pDepth, 0, 0, 0);
		const PixelFormat& fmt = mDepthStencilTarget->GetTextureFormat();
		TextureFetch::WritePixelFuncs[fmt](x, y, depth, mRTBuffer[ATT_DepthStencil], mRTBufferPitch[ATT_DepthStencil]);
	}
}

void FrameBuffer::Clear( uint32_t flags, const ColorRGBA& clr, float depth, uint32_t stencil )
{
	// only clear color0 and depth
	const uint32_t width = mRenderTargets[0]->GetWidth(0);
	const uint32_t height = mRenderTargets[0]->GetHeight(0);

	std::atomic<uint32_t> workingPackage(0);

	// Clear color0
	ScheduleAndJoin(GlobalThreadPool(), std::bind(&FrameBuffer::ClearColor, this, ATT_Color0, 
		mRenderTargets[0]->GetTextureFormat(), clr, std::ref(workingPackage), width, height));

	// clear depth
	if (mDepthStencilTarget)
	{
		workingPackage = 0;
		ColorRGBA depthColor(depth, 0, 0, 0);
		ScheduleAndJoin(GlobalThreadPool(), std::bind(&FrameBuffer::ClearColor, this, ATT_DepthStencil, 
			mDepthStencilTarget->GetTextureFormat(), depthColor, std::ref(workingPackage), width, height));
	}
}

void FrameBuffer::ClearColor(uint32_t index, PixelFormat fmt, const ColorRGBA& clr, std::atomic<uint32_t>& workingPackage, uint32_t width, uint32_t height)
{
#define NumClearRowPerPackage 64

	uint32_t num_packages = (height + NumClearRowPerPackage - 1) / NumClearRowPerPackage;
	uint32_t localWorkingPackage  = workingPackage ++;

	while (localWorkingPackage < num_packages)
	{
		const uint32_t start = localWorkingPackage * NumClearRowPerPackage;
		const uint32_t end = (std::min)(height, start + NumClearRowPerPackage);

		for (uint32_t y = start; y < end; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				TextureFetch::WritePixelFuncs[fmt](x, y, clr, mRTBuffer[index], mRTBufferPitch[index]);
			}
		}

		localWorkingPackage = workingPackage++;
	}
}

const shared_ptr<Texture2D>& FrameBuffer::GetRenderTarget( Attachment att ) const
{
	if (att == ATT_DepthStencil)
	{
		return mDepthStencilTarget;
	}else
	{
		return mRenderTargets[att - ATT_Color0];
	}
}

