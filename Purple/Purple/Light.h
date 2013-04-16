#ifndef Light_h__
#define Light_h__

#include "Prerequisites.h"
#include "Ray.h"
#include "Random.h"

namespace Purple {

struct LightSampleOffsets
{
	LightSampleOffsets() { }
	LightSampleOffsets(int count, Sample* sample);
	int nSamples, componentOffset, posOffset;
};

struct LightSample
{
	LightSample() { }

	LightSample(Random& rng)
	{
		uPos[0] = rng.RandomFloat();
		uPos[1] = rng.RandomFloat();
		uComponent = rng.RandomFloat();
	}

	LightSample(float u1, float u2, float uCom)
	{
		uPos[0] = u1;
		uPos[1] = u2;
		uComponent = uCom;
	}

	LightSample(const Sample* sample, const LightSampleOffsets& offset, uint32_t num);

	float uPos[2], uComponent;
};

struct VisibilityTester
{
public:
	void SetSegment(const float3&  p1, float eps1, const float3& p2, float eps2, float time);
	void SetRay(const float3& p, float eps, const float3& w, float time);
	bool Unoccluded(const Scene *scene) const;

private:
	Ray mRay;
};

class Light
{
public:
	Light(const float44& light2world, int32_t numSamples = 1)
		: mLightToWorld(light2world), NumSamples(numSamples) {}
	
	virtual ~Light(void) {}

	/** 
	 * 
	 */
	virtual ColorRGB Sample(const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis) const = 0;

	virtual float Pdf(const float3& pt, const float3& wi) const = 0;

	virtual ColorRGB Power(const Scene& scene) const = 0;
	
	virtual bool DeltaLight() const = 0;

	virtual ColorRGB Le(const RayDifferential &r) const { return ColorRGB::Black; }

public:
	int32_t NumSamples;  // used for mento carlo integration

protected:
	float44 mLightToWorld;
};

class PointLight : public Light
{
public:
	PointLight(const float44& light2world, const ColorRGB& intensity);
	~PointLight();

	ColorRGB Sample(const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis) const;

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

	ColorRGB Sample(const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis) const;

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

	ColorRGB Sample(const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis) const;

	ColorRGB Power(const Scene& scene) const;

	float Pdf(const float3& pt, const float3& wi) const { return 0.0f; }

	bool DeltaLight() const  { return true; }

private:
	float FallOff(const float3& wi) const;

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

	ColorRGB Sample(const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis) const;

	bool DeltaLight() const  { return false; } 

	float Pdf(const float3& pt, const float3& wi) const;

	ColorRGB Power(const Scene& scene) const;

	ColorRGB L(const float3& p, const float3& n, const float3& w) const;

private:
	float mArea;
	ColorRGB mIntensity;

	shared_ptr<Shape> mShape;
};



}

#endif // Light_h__


