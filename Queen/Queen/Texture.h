#ifndef Texture_h__
#define Texture_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"
#include "PixelFormat.h"
#include <ColorRGBA.hpp>

using RxLib::ColorRGBA;

class Texture
{
public:
	Texture(TextureType type, PixelFormat format, uint32_t numMipMaps, uint32_t sampleCount, uint32_t sampleQuality, uint32_t accessHint);
	virtual ~Texture(void);
	
	uint32_t GetSampleCount() const			{ return mSampleCount; }
	uint32_t GetSampleQuality() const		{ return mSampleQuality; }
	PixelFormat GetTextureFormat() const		{ return mFormat; }
	TextureType GetTextureType() const          { return mType; }

	virtual uint32_t GetWidth(uint32_t level) const;
	virtual uint32_t GetHeight(uint32_t level) const;
	virtual uint32_t GetDepth(uint32_t level) const;

	virtual void Map1D(uint32_t level, TextureMapAccess tma,
		uint32_t xOffset, uint32_t width, void*& data);

	virtual void Map2D(uint32_t level, TextureMapAccess tma,
		uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height,
		void*& data, uint32_t& rowPitch);

	virtual void Map3D(uint32_t level, TextureMapAccess tma,
		uint32_t xOffset, uint32_t yOffset, uint32_t zOffset,
		uint32_t width, uint32_t height, uint32_t depth,
		void*& data, uint32_t& rowPitch, uint32_t& slicePitch);

	virtual void MapCube(CubeMapFace face, uint32_t level, TextureMapAccess tma,
		uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height,
		void*& data, uint32_t& rowPitch);

	virtual void Unmap1D(uint32_t level);
	virtual void Unmap2D(uint32_t level);
	virtual void Unmap3D(uint32_t level);
	virtual void UnmapCube(CubeMapFace face, uint32_t level);

protected:
	std::vector<std::vector<uint8_t> > mTextureData;

	uint32_t mSampleCount, mSampleQuality;
	uint32_t mMipMaps;
	uint32_t mAccessHint;
	PixelFormat mFormat;
	TextureType mType;

	TextureMapAccess mTextureMapAccess;
};


class Texture2D : public Texture
{
public:
	Texture2D(PixelFormat format, uint32_t width, uint32_t height, 
		uint32_t numMipMaps, uint32_t sampleCount, uint32_t sampleQuality, uint32_t accessHint);
	~Texture2D();

	virtual uint32_t GetWidth(uint32_t level) const;
	virtual uint32_t GetHeight(uint32_t level) const;

	virtual void Map2D(uint32_t level, TextureMapAccess tma,
		uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height,
		void*& data, uint32_t& rowPitch);

	virtual void Unmap2D(uint32_t level);

private:
	std::vector<uint32_t> mWidths;
	std::vector<uint32_t> mHeights;

};


class TextureFetch
{
public:
	typedef void (*ReadPixelFunc)(int32_t x, int32_t y, ColorRGBA& pixel, void* pData, uint32_t pitch);
	typedef void (*WritePixelFunc)(int32_t x, int32_t y, const ColorRGBA& pixel, void* pData, uint32_t pitch);
	
	static void Init();

	static ReadPixelFunc ReadPixelFuncs[PF_Count]; 
	static WritePixelFunc WritePixelFuncs[PF_Count]; 
};


#endif // Texture_h__
