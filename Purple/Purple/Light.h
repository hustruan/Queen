#ifndef Light_h__
#define Light_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

struct VisibilityTester
{


private:
	Ray Ray;
};

class Light
{
public:
	Light(const float44& light2world, int32_t numSamples = 1)
		: mLightToWorld(light2world), mNumSamples(numSamples) {}
	
	virtual ~Light(void) {}

	/** 
	 * 
	 */
	virtual ColorRGB Sample_f(const float3& pt, float3* wi, float* pdf, VisibilityTester* vis) = 0;

	virtual float Pdf(const float3& pt, const float3& wi) const = 0;

	virtual ColorRGB Power(const Scene& scene) const = 0;
	
	virtual bool DeltaLight() const = 0;

protected:
	float44 mLightToWorld;
	int32_t mNumSamples;  // used for mento carlo integration
};

class PointLight : public Light
{
public:
	PointLight(const float44& light2world, const ColorRGB& intensity);
	~PointLight();

	ColorRGB Sample_f(const float3& pt, float3* wi, float* pdf, VisibilityTester* vis);

	ColorRGB Power(const Scene& scene) const;

	float Pdf(const float3& pt, const float3& wi) const { return 0.0f; }

	bool DeltaLight() const  { return true; }

private:
	ColorRGB mIntensity;
	float3 mLightPosW;	
};


class DirectionalLight : public Light
{
public:
	/**
	 * You can directly set the dir in world space, and leave the light2world identity
	 */
	DirectionalLight(const float44& light2world, const float3& dir, const ColorRGB& radiance);
	~DirectionalLight();

	ColorRGB Sample_f(const float3& pt, float3* wi, float* pdf, VisibilityTester* vis);

	ColorRGB Power(const Scene& scene) const;

	float Pdf(const float3& pt, const float3& wi) const { return 0.0f; }

	bool DeltaLight() const  { return true; }

private:
	float3 mLightDirectionW;
	ColorRGB mRadiance;

};

class SpotLight : public Light
{
public:
	SpotLight(const float44& light2world, const ColorRGB& intensity, float inner, float outer, float falloff);
	~SpotLight();

	ColorRGB Sample_f(const float3& pt, float3* wi, float* pdf, VisibilityTester* vis);

	ColorRGB Power(const Scene& scene) const;

	float Pdf(const float3& pt, const float3& wi) const { return 0.0f; }

	bool DeltaLight() const  { return true; }

private:
	float FallOff(const float3& wi);

private:
	ColorRGB mIntensity;

	float3 mLightPosW;
	float3 mLightDirectionW;

	float mCosSpotOuter;
	float mCosSpotInner;
	float mSpotFalloff;

};

class AreaLight : public Light
{
public:
	AreaLight(const float44& light2world, const ColorRGB& intensity, const shared_ptr<Shape>& shape, int32_t numSamples);
	~AreaLight();

	ColorRGB Sample_f(const float3& pt, float3* wi, float* pdf, VisibilityTester* vis);

	bool DeltaLight() const  { return false; } 

protected:
	ColorRGB L(const float3& p, const float3& n, const float3& w) const
	{
		return RxLib::Dot(n, w) > 0.0f ? mIntensity :  ColorRGB::Black;
	}

private:
	float mArea;
	ColorRGB mIntensity;

	shared_ptr<Shape> mShape;
};



}

#endif // Light_h__


