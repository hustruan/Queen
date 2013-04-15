#include "Integrator.h"
#include "Reflection.h"
#include "Scene.h"
#include "Light.h"
#include "Sampler.h"
#include "Renderer.h"
#include "DifferentialGeometry.h"

namespace Purple {

using namespace RxLib;

ColorRGB SpecularReflect( const RayDifferential& ray, BSDF* bsdf, Random& rng, const DifferentialGeometry& isect, const Renderer* renderer, const Scene* scene, const Sample* sample, MemoryArena& arena )
{
	float3 wi, wo = ray.Direction;

	const float3& p = bsdf->dgShading.Point;
	const float3& n = bsdf->dgShading.Normal;

	float pdf;
	ColorRGB f = bsdf->Sample(wo, &wi, BSDFSample(rng), &pdf, BSDF_Reflection | BSDF_Specular);

	ColorRGB L = ColorRGB::Black;
	if (f != ColorRGB::Black && AbsDot(wo, wi) != 0.0f && pdf > 0.0f)
	{
		RayDifferential rd(p, wi, ray, 1e-4f, Mathf::INFINITY);
		if (ray.hasDifferentials)
		{
			rd.hasDifferentials = true;
			rd.rxOrigin = p + isect.dpdx;
			rd.ryOrigin = p + isect.dpdy;

			// Compute differential reflected directions
			float3 dndx = bsdf->dgShading.dndu * bsdf->dgShading.dudx + bsdf->dgShading.dndv * bsdf->dgShading.dvdx;
			float3 dndy = bsdf->dgShading.dndu * bsdf->dgShading.dudy + bsdf->dgShading.dndv * bsdf->dgShading.dvdy;
			float3 dwodx = -ray.rxDirection - wo, dwody = -ray.ryDirection - wo;
			float dDNdx = Dot(dwodx, n) + Dot(wo, dndx);
			float dDNdy = Dot(dwody, n) + Dot(wo, dndy);
			rd.rxDirection = wi - dwodx + 2.0f * float3(Dot(wo, n) * dndx + dDNdx * n);
			rd.ryDirection = wi - dwody + 2.0f * float3(Dot(wo, n) * dndy + dDNdy * n);
		}

		ColorRGB Li = renderer->Li(scene, rd, sample, rng, arena);
		L = f * Li * AbsDot(wi, n) / pdf;
	}
	
	return L;
}

ColorRGB SpecularTransmit( const RayDifferential& ray, BSDF* bsdf, Random& rng, const DifferentialGeometry& isect, const Renderer* renderer, const Scene* scene, const Sample* sample, MemoryArena& arena )
{
	float3 wi, wo = ray.Direction;

	const float3& p = bsdf->dgShading.Point;
	const float3& n = bsdf->dgShading.Normal;

	float pdf;
	ColorRGB f = bsdf->Sample(wo, &wi, BSDFSample(rng), &pdf, BSDF_Transmission | BSDF_Specular);

	ColorRGB L = ColorRGB::Black;
	if (f != ColorRGB::Black && AbsDot(wo, wi) != 0.0f && pdf > 0.0f)
	{
		RayDifferential rd(p, wi, ray, 1e-4f, Mathf::INFINITY);
		if (ray.hasDifferentials)
		{
			rd.hasDifferentials = true;
			rd.rxOrigin = p + isect.dpdx;
			rd.ryOrigin = p + isect.dpdy;

			float eta = bsdf->eta;
			float3 w = -wo;
			if (Dot(wo, n) < 0) eta = 1.f / eta;

			float3 dndx = bsdf->dgShading.dndu * bsdf->dgShading.dudx + bsdf->dgShading.dndv * bsdf->dgShading.dvdx;
			float3 dndy = bsdf->dgShading.dndu * bsdf->dgShading.dudy + bsdf->dgShading.dndv * bsdf->dgShading.dvdy;

			float3 dwodx = -ray.rxDirection - wo, dwody = -ray.ryDirection - wo;
			float dDNdx = Dot(dwodx, n) + Dot(wo, dndx);
			float dDNdy = Dot(dwody, n) + Dot(wo, dndy);

			float mu = eta * Dot(w, n) - Dot(wi, n);
			float dmudx = (eta - (eta*eta*Dot(w,n))/Dot(wi, n)) * dDNdx;
			float dmudy = (eta - (eta*eta*Dot(w,n))/Dot(wi, n)) * dDNdy;

			rd.rxDirection = wi + eta * dwodx - float3(mu * dndx + dmudx * n);
			rd.ryDirection = wi + eta * dwody - float3(mu * dndy + dmudy * n);
		}

		ColorRGB Li = renderer->Li(scene, rd, sample, rng, arena);
		L = f * Li * AbsDot(wi, n) / pdf;
	}

	return L;
}


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
		auto cos = fabsf(Dot(wi, n));
		auto c = f * Li * fabsf(Dot(wi, n));
		auto b = visibility.Unoccluded(scene);

		if (f != ColorRGB::Black && visibility.Unoccluded(scene))
			L += f * Li * fabsf(Dot(wi, n)) /** visibility.Transmittance(scene, renderer, sample, rng, arena)*/ /*/ pdf*/;
	}
	
	if (ray.Depth + 1 < mMaxDepth) 
	{
		//// Trace rays for specular reflection and refraction
		L += SpecularReflect(ray, bsdf, rng, isect, renderer, scene, sample, arena);
		L += SpecularTransmit(ray, bsdf, rng, isect, renderer, scene, sample, arena);
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