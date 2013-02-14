#include "Texture.h"
#include <exception>

namespace {

template<uint32_t format>
struct PixelUpdater
{
	static ColorRGBA ReadPixel(int32_t x, int32_t y, void* pData, uint32_t pitch);
	static void WritePixel(int32_t x, int32_t y, const ColorRGBA& pixel, void* pData, uint32_t pitch);
};

template<>
struct PixelUpdater<PF_A32B32G32R32F>
{
	static void ReadPixel(int32_t x, int32_t y, ColorRGBA& pixel, void* pData, uint32_t pitch)
	{
		float* pColor = (float*)((uint8_t*)pData + y * pitch + x * 16);

		pixel.R = pColor[0];
		pixel.G = pColor[1];
		pixel.B = pColor[2];
		pixel.A = pColor[3];
	}

	static void WritePixel(int32_t x, int32_t y, const ColorRGBA& pixel, void* pData, uint32_t pitch)
	{
		float* pColor = (float*)((uint8_t*)pData + y * pitch + x * 16);

		pColor[0] = pixel.R;
		pColor[1] = pixel.G;
		pColor[2] = pixel.B;
		pColor[3] = pixel.A;	
	}
};

template<>
struct PixelUpdater<PF_Depth32>
{
	static void ReadPixel(int32_t x, int32_t y, ColorRGBA& pixel, void* pData, uint32_t pitch)
	{
		float* pDepth = (float*)((uint8_t*)pData + y * pitch + x * 4);
		pixel.R = pDepth[0];
	}

	static void WritePixel(int32_t x, int32_t y, const ColorRGBA& pixel, void* pData, uint32_t pitch)
	{
		float* pDepth = (float*)((uint8_t*)pData + y * pitch + x * 4);
		pDepth[0] = pixel.R;
	}
};

}

Texture::Texture( TextureType type, PixelFormat format, uint32_t numMipMaps, uint32_t sampleCount, uint32_t sampleQuality, uint32_t accessHint )
	: mType(type), mFormat(format), mMipMaps(numMipMaps), mSampleCount(sampleCount), mSampleQuality(sampleQuality), mAccessHint(accessHint)
{
	ASSERT(sampleCount <= 1);
}


Texture::~Texture(void)
{
}

uint32_t Texture::GetWidth( uint32_t level ) const
{
	throw std::exception("Shouldn't be here");
}

uint32_t Texture::GetHeight( uint32_t level ) const
{
	throw std::exception("Shouldn't be here");
}

uint32_t Texture::GetDepth( uint32_t level ) const
{
	throw std::exception("Shouldn't be here");
}


void Texture::Map1D( uint32_t level, TextureMapAccess tma, uint32_t xOffset, uint32_t width, void*& data )
{
	throw std::exception("Shouldn't be here");
}

void Texture::Map2D(  uint32_t level, TextureMapAccess tma, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, void*& data, uint32_t& rowPitch )
{
	throw std::exception("Shouldn't be here");
}

void Texture::Map3D(  uint32_t level, TextureMapAccess tma, uint32_t xOffset, uint32_t yOffset, uint32_t zOffset, uint32_t width, uint32_t height, uint32_t depth, void*& data, uint32_t& rowPitch, uint32_t& slicePitch )
{
	throw std::exception("Shouldn't be here");
}

void Texture::MapCube(  CubeMapFace face, uint32_t level, TextureMapAccess tma, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, void*& data, uint32_t& rowPitch )
{
	throw std::exception("Shouldn't be here");
}

void Texture::Unmap1D( uint32_t level )
{
	throw std::exception("Shouldn't be here");
}

void Texture::Unmap2D( uint32_t level )
{
	throw std::exception("Shouldn't be here");
}

void Texture::Unmap3D( uint32_t level )
{
	throw std::exception("Shouldn't be here");
}

void Texture::UnmapCube(CubeMapFace face, uint32_t level )
{
	throw std::exception("Shouldn't be here");
}

//--------------------------------------------------------------------------------------------

Texture2D::Texture2D(PixelFormat format, uint32_t width, uint32_t height, uint32_t numMipMaps, uint32_t sampleCount, uint32_t sampleQuality, uint32_t accessHint )
	: Texture(TT_Texture2D, format, numMipMaps, sampleCount, sampleQuality, accessHint)
{
	if( numMipMaps == 0 )
	{
		mMipMaps = 1;
	}
	else
	{
		mMipMaps = numMipMaps;
	}

	mWidths.resize(mMipMaps);
	mHeights.resize(mMipMaps);
	{
		// store every level width and height
		uint32_t w = width;
		uint32_t h = height;
		for(uint32_t level = 0; level < mMipMaps; level++)
		{
			mWidths[level] = w;
			mHeights[level] = h;
			w = std::max<uint32_t>(1U, w / 2);
			h = std::max<uint32_t>(1U, h / 2);
		}
	} 

	uint32_t texelSize = PixelFormatUtils::GetNumElemBytes(mFormat);
	mTextureData.resize(mMipMaps);

	for (uint32_t level = 0; level < mMipMaps; ++ level)
	{
		uint32_t levelWidth = mWidths[level];
		uint32_t levelHeight = mHeights[level];

		if (PixelFormatUtils::IsCompressed(mFormat))
		{
			int blockSize = (mFormat == PF_DXT1) ? 8 : 16; 
			uint32_t imageSize = ((levelWidth+3)/4)*((levelHeight+3)/4)*blockSize; 

			// resize texture data for copy
			mTextureData[mMipMaps + level].resize(imageSize);
		}
		else
		{
			uint32_t imageSize = levelWidth * levelHeight * texelSize;
			// resize texture data for copy
			mTextureData[level].resize(imageSize);
		}
	}
}

uint32_t Texture2D::GetWidth( uint32_t level ) const
{
	return mWidths[level];
}

uint32_t Texture2D::GetHeight( uint32_t level ) const
{
	return mHeights[level];
}


void Texture2D::Map2D(uint32_t level, TextureMapAccess tma, uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height, void*& data, uint32_t& rowPitch )
{
	// store 
	mTextureMapAccess = tma;

	uint32_t texelSize = PixelFormatUtils::GetNumElemBytes(mFormat);
	rowPitch = mWidths[level] * texelSize; 
	uint8_t* p = &mTextureData[level][0];

	int blockSize = 0;
	if (PixelFormatUtils::IsCompressed(mFormat))
	{
		blockSize = (mFormat == PF_DXT1) ? 8 : 16; 
		data = p + (yOffset / 4) * rowPitch + (xOffset / 4 * blockSize);
	}
	else
	{
		data = p + (yOffset * mWidths[level] + xOffset) * texelSize;
	}	
}

void Texture2D::Unmap2D( uint32_t level )
{

}

Texture2D::~Texture2D()
{

}


//----------------------------------------------------------------------------------------
TextureFetch::ReadPixelFunc TextureFetch::ReadPixelFuncs[PF_Count]; 
TextureFetch::WritePixelFunc TextureFetch::WritePixelFuncs[PF_Count]; 

void TextureFetch::Init()
{
	ReadPixelFuncs[PF_Depth32] = &PixelUpdater<PF_Depth32>::ReadPixel;
	WritePixelFuncs[PF_Depth32] = &PixelUpdater<PF_Depth32>::WritePixel;

	ReadPixelFuncs[PF_A32B32G32R32F] = &PixelUpdater<PF_A32B32G32R32F>::ReadPixel;
	WritePixelFuncs[PF_A32B32G32R32F] = &PixelUpdater<PF_A32B32G32R32F>::WritePixel;
}
