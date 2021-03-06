#ifndef Texture_h__
#define Texture_h__

#include "Prerequisites.h"
#include "MipMap.h"

namespace Purple {

template <typename T> 
class Texture 
{
public:
	// Texture Interface
	virtual T Evaluate(const DifferentialGeometry &) const = 0;
	virtual ~Texture() { }
};

template <typename T> 
class ConstantTexture : public Texture<T>
{
public:
	ConstantTexture(const T &v) { value = v; }
	T Evaluate(const DifferentialGeometry &) const { return value;}

private:
	T value;
};

class RGBImageTexture : public Texture<ColorRGB>
{
public:
	RGBImageTexture(const std::string& filename, TextureAddressMode warpU, TextureAddressMode warpV,
		MIPFilterType mipFilter = MFT_Nearest,  float gamma = 0.0f, float maxAniso = 1.0f);

	ColorRGB Evaluate(const DifferentialGeometry& dg) const;

public:
	static void ClearCache();

private:
	static TMipMap<ColorRGB>* CreateOrReuseMipMap(const std::string& filename);

private:
	TMipMap<ColorRGB>* mMipMap;

	SampleState mSampleState;

	float2 mUVOffset;
	float2 mUVScale;

	float mGamma, mMaxAnisotropy;	

private:

	static std::map< std::string, TMipMap<ColorRGB>* > msTextures;
};




}



#endif // Texture_h__

