#include "Light.h"
#include "Scene.h"
#include "Sampler.h"
#include "Shape.h"
#include <MathUtil.hpp>
#include <BoundingSphere.hpp>

namespace Purple {

using namespace RxLib;

void VisibilityTester::SetRay( const float3& p, float eps, const float3& w, float time )
{
	mRay = Ray(p, w, eps, Mathf::INFINITY, time);
}

bool VisibilityTester::Unoccluded( const Scene *scene ) const
{
	return !scene->IntersectP(mRay);
}

//-----------------------------------------------------------------------------------
PointLight::PointLight( const float44& light2world, const ColorRGB& intensity )
	: Light(light2world), mIntensity(intensity)
{
	mLightPosW = Transform(float3(0.0f, 0.0f, 0.0f), light2world);
}

PointLight::~PointLight()
{
}

ColorRGB PointLight::Sample( const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis )
{
	*wi = Normalize(mLightPosW - pt);
	*pdf = 1.0f;

	if (vis)
		vis->SetRay(pt, 1e-4f, *wi, time);

	return mIntensity /*/ LengthSquared(mLightPosW - pt)*/;
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

ColorRGB SpotLight::Sample( const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis )
{
	*wi = Normalize(mLightPosW - pt);
	*pdf = 1.0f;

	if (vis)
		vis->SetRay(pt, 1e-3f, *wi, time);

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

ColorRGB DirectionalLight::Sample( const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis )
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

AreaLight::AreaLight( const float44& light2world, const ColorRGB& intensity, const shared_ptr<Shape>& shape, int32_t numSamples )
	: Light(light2world, numSamples), mIntensity(intensity), mShape(shape)
{
	 
}

AreaLight::~AreaLight()
{

}


ColorRGB AreaLight::Sample( const float3& pt, const LightSample& lightSample, float time, float3* wi, float* pdf, VisibilityTester* vis )
{
	float3 ns;
	float3 ps = mShape->Sample(pt, lightSample.uPos[0], lightSample.uPos[1], lightSample.uComponent, &ns);

	*wi = Normalize(ps - pt);
	*pdf = mShape->Pdf(pt, *wi);

	if (vis)
		vis->SetRay(pt, 1e-3f, *wi, time);

	ColorRGB Ls = L(ps, ns, -*wi);

	return Ls;
}

float AreaLight::Pdf( const float3& pt, const float3& wi ) const
{
	return mShape->Pdf(pt, wi);
}

RxLib::ColorRGB AreaLight::Power( const Scene& scene ) const
{
	return mIntensity * mShape->Area() * Mathf::PI;
}


LightSampleOffsets::LightSampleOffsets( int count, Sample* sample )
	:nSamples(count)
{
	componentOffset = sample->Add1D(nSamples);
	posOffset =  sample->Add2D(nSamples);
}


LightSample::LightSample( Sample* sample, const LightSampleOffsets& offset, uint32_t n )
{
	uPos[0] = sample->TwoD[offset.posOffset][2*n];
	uPos[1] = sample->TwoD[offset.posOffset][2*n+1];
	uComponent = sample->OneD[offset.componentOffset][n];
}


ColorRGB Light::Le( const RayDifferential &r ) const
{
	return ColorRGB::Black;
}




}