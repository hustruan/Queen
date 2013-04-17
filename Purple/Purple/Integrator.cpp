#include "Integrator.h"
#include "Reflection.h"
#include "Scene.h"
#include "Light.h"
#include "Sampler.h"
#include "Renderer.h"
#include "Shape.h"
#include "MentoCarlo.h"
#include "DifferentialGeometry.h"
#include <FloatCast.hpp>

namespace Purple {

using namespace RxLib;

ColorRGB EstimateDirect( const Scene* scene, const Renderer* renderer, MemoryArena& arena, const Light* light, const float3& p, const float3& n, const float3& wo, float time, const BSDF* bsdf, Random& rng, const LightSample& lightSample, const BSDFSample& bsdfSample, uint32_t flags )
{
	ColorRGB Ld(0.);
	
	// Sample light source with multiple importance sampling
	float3 wi;
	float lightPdf, bsdfPdf;
	VisibilityTester visibility;

	ColorRGB Li = light->Sample(p, lightSample, time, &wi, &lightPdf, &visibility);

	if (lightPdf > 0.0f && Li != ColorRGB::Black)
	{
		ColorRGB f = bsdf->Eval(wo, wi, BSDFType(flags));
		if (f != ColorRGB::Black && visibility.Unoccluded(scene))
		{
			// Add light's contribution to reflected radiance
			//Li *= visibility.Transmittance(scene, renderer, NULL, rng, arena);
			if (light->DeltaLight())
				Ld += f * Li * (AbsDot(wi, n) / lightPdf);
			else 
			{
				bsdfPdf = bsdf->Pdf(wo, wi, BSDFType(flags));
				float weight = PowerHeuristic(1, lightPdf, 1, bsdfPdf);
				Ld += f * Li * (AbsDot(wi, n) * weight / lightPdf);
			}
		}
	}

	// Sample BSDF with multiple importance sampling
	if (!light->DeltaLight())
	{
		uint32_t sampledType;
		ColorRGB f = bsdf->Sample(wo, &wi, bsdfSample, &bsdfPdf, flags, &sampledType);
		if (f != ColorRGB::Black && bsdfPdf > 0.0f)
		{
			float weight = 1.f;
			if (!(sampledType & BSDF_Specular))
			{
				lightPdf = light->Pdf(p, wi);
				if (lightPdf == 0.)
					return Ld;
				weight = PowerHeuristic(1, bsdfPdf, 1, lightPdf);
			}

			// Add light contribution from BSDF sampling
			DifferentialGeometry lightIsect;
			ColorRGB Li(0.f);
			RayDifferential ray(p, wi, 1e-3f, Mathf::INFINITY, time);
			if (scene->Intersect(ray, &lightIsect))
			{
				if (lightIsect.Instance->GetAreaLight() == light)
					Li = lightIsect.Le(-wi);
			}
			else
				Li = light->Le(ray);

			if (Li != ColorRGB::Black) 
			{
				Li *= renderer->Transmittance(scene, ray, NULL, rng, arena);
				Ld += f * Li * AbsDot(wi, n) * weight / bsdfPdf;
			}
		}
	}
	return Ld;
}

ColorRGB SpecularReflect( const RayDifferential& ray, BSDF* bsdf, Random& rng, const DifferentialGeometry& isect, const Renderer* renderer, const Scene* scene, const Sample* sample, MemoryArena& arena )
{
	float3 wi, wo = ray.Direction;

	const float3& p = bsdf->dgShading.Point;
	const float3& n = bsdf->dgShading.Normal;

	float pdf;
	ColorRGB f = bsdf->Sample(wo, &wi, BSDFSample(rng), &pdf, BSDF_Reflection | BSDF_Specular);

	ColorRGB L = ColorRGB::Black;
	if (f != ColorRGB::Black && AbsDot(n, wi) != 0.0f && pdf > 0.0f)
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
	if (f != ColorRGB::Black && AbsDot(wo, n) != 0.0f && pdf > 0.0f)
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
			L += f * Li * fabsf(Dot(wi, n)) / pdf;
	}
	
	if (ray.Depth + 1 < mMaxDepth) 
	{
		//// Trace rays for specular reflection and refraction
		L += SpecularReflect(ray, bsdf, rng, isect, renderer, scene, sample, arena);
		//L += SpecularReflect(ray, bsdf, rng, isect, renderer, scene, sample, arena);
		L += SpecularTransmit(ray, bsdf, rng, isect, renderer, scene, sample, arena);
	}
	return L;
}

//-----------------------------------------------------------------------------------------------------------------
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

ColorRGB DirectLightingIntegrator::Li( const Scene* scene, const Renderer* renderer, const RayDifferential& ray, const DifferentialGeometry& isect, const Sample* sample, Random& rng, MemoryArena& arena ) const
{
	ColorRGB L = ColorRGB::Black;

	// Evaluate BSDF at hit point
	BSDF* bsdf = isect.GetBSDF(ray, arena);

	// Initialize common variables for Whitted integrator
	const float3& p = bsdf->dgShading.Point;
	const float3& n = bsdf->dgShading.Normal;

	float3 wo = -ray.Direction;

	// Compute emitted light if ray hit an area light source
	L += isect.Le(wo);

	if (scene->Lights.size() > 0) 
	{
		// Apply direct lighting strategy
		switch (mLightStrategy) 
		{
		case LS_Sample_All_Uniform:
			{
				for (size_t i = 0; i < scene->Lights.size(); ++i) 
				{	
					Light* light = scene->Lights[i];
					int nSamples = mLightSampleOffsets ? mLightSampleOffsets[i].nSamples : 1;
				
					ColorRGB Ld(0.0f);
					for (int j = 0; j < nSamples; ++j) 
					{
						// Find light and BSDF sample values for direct lighting estimate
						LightSample lightSample;
						BSDFSample bsdfSample;
						if (mLightSampleOffsets != NULL && mBSDFSampleOffsets != NULL)
						{
							lightSample = LightSample(sample, mLightSampleOffsets[i], j);
							bsdfSample = BSDFSample(sample, mBSDFSampleOffsets[i], j);
						}
						else 
						{
							lightSample = LightSample(rng);
							bsdfSample = BSDFSample(rng);
						}
						Ld += EstimateDirect(scene, renderer, arena, light, p, n, wo, 0.0f, bsdf, rng, lightSample, bsdfSample,
							BSDFType(BSDF_All & ~BSDF_Specular));
					}
					L += Ld / float(nSamples);
				}
			}
			break;
		case LS_Sample_One_Uniform:
			{
				// Randomly choose a single light to sample
				size_t nLights = scene->Lights.size();
				size_t lightNum;			 
				if (mLightNumOffset != -1)
					lightNum = Floor2Int(sample->OneD[mLightNumOffset][0] * nLights);
				else
					lightNum = Floor2Int(rng.RandomFloat() * nLights);

				lightNum = std::min(lightNum, nLights-1);
				Light *light = scene->Lights[lightNum];

				LightSample lightSample;
				BSDFSample bsdfSample;
				if (mLightSampleOffsets != NULL && mBSDFSampleOffsets != NULL)
				{
					lightSample = LightSample(sample, mLightSampleOffsets[0], 0);
					bsdfSample = BSDFSample(sample, mBSDFSampleOffsets[0], 0);
				}
				else 
				{
					lightSample = LightSample(rng);
					bsdfSample = BSDFSample(rng);
				}

				L += nLights * EstimateDirect(scene, renderer, arena, light, p, n, wo, 0.0f, bsdf, rng, lightSample, bsdfSample,
					BSDFType(BSDF_All & ~BSDF_Specular));
			}
			break;
		}
	}

	if (ray.Depth + 1 < mMaxDepth) 
	{
		//// Trace rays for specular reflection and refraction
		L += SpecularReflect(ray, bsdf, rng, isect, renderer, scene, sample, arena);
		//L += SpecularReflect(ray, bsdf, rng, isect, renderer, scene, sample, arena);
		L += SpecularTransmit(ray, bsdf, rng, isect, renderer, scene, sample, arena);
	}
	return L;
}

}