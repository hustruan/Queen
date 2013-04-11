#include "Integrator.h"
#include "Reflection.h"
#include "Scene.h"
#include "Light.h"
#include "Sampler.h"
#include "DifferentialGeometry.h"

namespace Purple {

ColorRGB WhittedIntegrator::Li( const Scene* scene, const Renderer* renderer, const RayDifferential& ray, const DifferentialGeometry& isect, const Sample* sample, Random& rng, MemoryArena& arena ) const
{
	ColorRGB L = ColorRGB::Black;

	// Compute emitted and reflected light at ray intersection point

	// Evaluate BSDF at hit point
	BSDF* bsdf = isect.GetBSDF(ray, arena);

	// Initialize common variables for Whitted integrator
	const float3& p = bsdf->dgShading.Point;
	const float3& n = bsdf->dgShading.Normal;
	
	float3 wo = -ray.Direction;

	// Compute emitted light if ray hit an area light source
	L += isect.Le(wo);

	// Add contribution of each light source
	for (size_t i = 0; i < scene->Lights.size(); ++i)
	{
		float3 wi;
		float pdf;

		VisibilityTester visibility;
		ColorRGB Li = scene->Lights[i]->Sample(p, LightSample(rng), ray.Time, &wi, &pdf, &visibility);
		if (Li == ColorRGB::Black || pdf == 0.f) 
			continue;
		
		ColorRGB f = bsdf->Eval(wo, wi);

		/*if (!f.IsBlack() && visibility.Unoccluded(scene))*/
			L += f * Li * fabsf(Dot(wi, n)) /** visibility.Transmittance(scene, renderer, sample, rng, arena)*/ / pdf;
	}
	
	if (ray.Depth + 1 < mMaxDepth) 
	{
		//// Trace rays for specular reflection and refraction
		//L += SpecularReflect(ray, bsdf, rng, isect, renderer, scene, sample,
		//	arena);
		//L += SpecularTransmit(ray, bsdf, rng, isect, renderer, scene, sample,
		//	arena);
	}
	return L;
}

DirectLightingIntegrator::DirectLightingIntegrator( LightStrategy ls /*= LS_Sample_All_Uniform*/, int md /*= 5*/ ) 
	: mLightStrategy(ls), mMaxDepth(md), mLightSampleOffsets(NULL), mBSDFSampleOffsets(NULL)
{

}

DirectLightingIntegrator::~DirectLightingIntegrator()
{
	if(mLightSampleOffsets)
		delete[] mLightSampleOffsets;

	if (mBSDFSampleOffsets)
		delete[] mBSDFSampleOffsets;
}

ColorRGB DirectLightingIntegrator::Li( const Scene* scene, const Renderer* renderer, const RayDifferential& ray, const DifferentialGeometry& isect, const Sample* sample, Random& rng, MemoryArena& arena ) const
{
	return ColorRGB::Black;
}

void DirectLightingIntegrator::RequestSamples( Sampler* sampler, Sample* sample, const Scene* scene )
{
	if (mLightStrategy == LS_Sample_All_Uniform)
	{
		mLightNumOffset = -1;

		size_t nLights = scene->Lights.size();

		mLightSampleOffsets = new LightSampleOffsets[nLights];
		mBSDFSampleOffsets = new BSDFSampleOffsets[nLights];

		for (size_t i = 0; i < nLights; ++i)
		{
			int nSamples = scene->Lights[i]->NumSamples;
			if (sampler) nSamples = sampler->RoundSize(nSamples);

			mLightSampleOffsets[i] = LightSampleOffsets(nSamples, sample);
			mBSDFSampleOffsets[i] = BSDFSampleOffsets(nSamples, sample);
		}
	}
	else
	{
		mLightNumOffset = sample->Add1D(1);

		mLightSampleOffsets = new LightSampleOffsets[1];
		mBSDFSampleOffsets = new BSDFSampleOffsets[1];

		mLightSampleOffsets[0] = LightSampleOffsets(1, sample);
		mBSDFSampleOffsets[0] = BSDFSampleOffsets(1, sample);
	}
}



}