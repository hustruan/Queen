#include "Light.h"
#include "Scene.h"
#include <MathUtil.hpp>
#include <BoundingSphere.hpp>

namespace Purple {

using namespace RxLib;

PointLight::PointLight( const float44& light2world, const ColorRGB& intensity )
	: Light(light2world), mIntensity(intensity)
{
	mLightPosW = Transform(float3(0.0f, 0.0f, 0.0f), light2world);
}

PointLight::~PointLight()
{
}

ColorRGB PointLight::Sample_f( const float3& pt, float3* wi, float* pdf, VisibilityTester* vis )
{
	*wi = Normalize(mLightPosW - pt);
	*pdf = 1.0f;

	return mIntensity / LengthSquared(mLightPosW - pt);
}

ColorRGB PointLight::Power( const Scene& scene ) const
{
	return 4.0f * Mathf::PI * mIntensity;
}


SpotLight::SpotLight( const float44& light2world, const ColorRGB& intensity, float inner, float outer, float falloff )
	: Light(light2world), mIntensity(intensity), mCosSpotInner(cosf(inner)),
	  mCosSpotOuter(cosf(outer)), mSpotFalloff(falloff)
{
	mLightPosW = Transform(float3(0.0f, 0.0f, 0.0f), light2world);
	// Transform into world light direction
	mLightDirectionW = Normalize(TransformDirection(float3(0.0f, 0.0f, 1.0f), light2world));
}


SpotLight::~SpotLight()
{

}

ColorRGB SpotLight::Sample_f( const float3& pt, float3* wi, float* pdf, VisibilityTester* vis )
{
	*wi = Normalize(mLightPosW - pt);
	*pdf = 1.0f;

	return mIntensity * FallOff(*wi) / LengthSquared(mLightPosW - pt);
}

float SpotLight::FallOff( const float3& wi )
{
	float costheta = Dot(wi, mLightDirectionW);
	if(costheta < mCosSpotOuter) return 0.0f;
	if(costheta > mCosSpotInner) return 1.0f;
	return powf((costheta - mCosSpotOuter) / (mCosSpotInner - mCosSpotOuter), mSpotFalloff);
}

ColorRGB SpotLight::Power( const Scene& scene ) const
{
	return mIntensity * 2.f * Mathf::PI * (1.0f - 0.5f * (mCosSpotInner + mCosSpotOuter));
}



DirectionalLight::DirectionalLight( const float44& light2world, const float3& dir, const ColorRGB& radiance )
	:Light(light2world), mRadiance(radiance)
{
	mLightDirectionW = Normalize(TransformDirection(dir, light2world));
}

DirectionalLight::~DirectionalLight()
{

}

ColorRGB DirectionalLight::Sample_f( const float3& pt, float3* wi, float* pdf, VisibilityTester* vis )
{
	*wi = -mLightDirectionW;
	*pdf = 1.0f;

	return mRadiance;
}

ColorRGB DirectionalLight::Power( const Scene& scene ) const
{
	const BoundingBoxf& bbox = scene.GetBoundingBox();
	BoundingSpheref sphere = FromBox(bbox);

	return mRadiance * Mathf::PI * sphere.Radius * sphere.Radius;

}


ColorRGB AreaLight::Sample_f( const float3& pt, float3* wi, float* pdf, VisibilityTester* vis )
{
	return ColorRGB(0, 0, 0);
}

AreaLight::AreaLight( const float44& light2world, const ColorRGB& intensity, const shared_ptr<Shape>& shape, int32_t numSamples )
	: Light(light2world), mIntensity(intensity)
{
	 
}

AreaLight::~AreaLight()
{

}

}