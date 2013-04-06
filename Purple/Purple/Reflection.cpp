#include "Reflection.h"
#include "MentoCarlo.h"

namespace Purple {

using namespace RxLib;

ColorRGB BxDF::Sample_f( const float3& wo, float3* wi, float u1, float u2, float* pdf ) const
{
	*wi = CosineSampleHemisphere(u1, u2);
	if (wo.Z() < 0.) wi->Z() *= -1.f;
	*pdf = Pdf(wo, *wi);
	return f(wo, *wi);
}

ColorRGB BxDF::rho( const float3& wo, int32_t numSamples, const float* samples ) const
{
	ColorRGB r(0.0f, 0.0f, 0.0f);
	for (int32_t i = 0; i < numSamples; ++i )
	{
		float3 wi; 
		float pdf = 0.0f;
		ColorRGB f = Sample_f(wo, &wi, samples[2 * i], samples[2 * i + 1], &pdf);
		
		if (pdf > 0.0f)
		{
			r += AbsCosTheta(wi) * f / pdf;
		}
	}
	return r / float(numSamples);
}

ColorRGB BxDF::rho( int32_t numSamples, const float* samples1, const float* samples2 ) const
{
	ColorRGB r(0.0f, 0.0f, 0.0f);
	for (int32_t i = 0; i < numSamples; ++i)
	{
		float3 wo = UniformSampleHemisphere(samples1[2 * i], samples2[2 * i + 1]);

		float pdf_i, pdf_o = UniformHemispherePdf();
		float3 wi; 
		ColorRGB f = Sample_f(wo, &wi, samples2[2 * i], samples2[2 * i + 1], &pdf_i);
		if (pdf_i > 0.0f)
			r += f * AbsCosTheta(wi) * AbsCosTheta(wo) / (pdf_i * pdf_o);
	}

	return r / (Mathf::PI*numSamples);
}

float BxDF::Pdf( const float3& wo, const float3& wi ) const
{
	return CosineHemispherePdf(AbsCosTheta(wi));
}

ColorRGB SpecularRelection::Sample_f( const float3& wo, float3* wi, float u1, float u2, float* pdf ) const
{
	// Compute reflect vector
	*wi = float3(-wo.X(), -wo.Y(), wo.Z());
	*pdf = 1.0f;

	return mFresnel->Evaluate(CosTheta(wo)) * mR / AbsCosTheta(*wi);
}


ColorRGB OrenNayar::f( const float3& wo, const float3& wi ) const
{
	float sinthetai = SinTheta(wi);
	float sinthetao = SinTheta(wo);

	float maxcos = 0.0f;
	if (sinthetai > 1e-4 && sinthetao > 1e-4) 
	{
		float sinphii = SinPhi(wi), cosphii = CosPhi(wi);
		float sinphio = SinPhi(wo), cosphio = CosPhi(wo);
		float dcos = cosphii * cosphio + sinphii * sinphio;
		maxcos = (std::max)(0.0f, dcos);
	}

	float sinalpha, tanbeta;
	if (AbsCosTheta(wi) > AbsCosTheta(wo))
	{
		sinalpha = sinthetao;
		tanbeta = sinthetai / AbsCosTheta(wi);
	}
	else
	{
		sinalpha = sinthetai;
		tanbeta = sinthetao / AbsCosTheta(wo);
	}

	return mR * Mathf::INV_PI * (A + B * maxcos * sinalpha * tanbeta);
}


float TorranceSparrow::G( const float3& wo, const float3& wi, const float3& wh ) const
{
	float NdotWh = AbsCosTheta(wh);
	float NdotWo = AbsCosTheta(wo);
	float NdotWi = AbsCosTheta(wi);
	float OdotWh = fabsf(Dot(wo, wh));
	
	return std::min(1.0f, std::min(2.0f * NdotWh * NdotWo / OdotWh, 2.0f * NdotWh * NdotWi / OdotWh));
}

ColorRGB TorranceSparrow::f( const float3& wo, const float3& wi ) const
{
	float cosThetaO = AbsCosTheta(wo);
	float cosThetaI = AbsCosTheta(wi);

	if (cosThetaO == 0.0f || cosThetaI == 0.0f)
		return ColorRGB(0.0f, 0.0f, 0.0f);

	float3 wh = Normalize(wo + wi);

	// Compute distribution term
	float cosThetaH = Dot(wi, wh);

	return mR* mFresnel->Evaluate(cosThetaH) * mD->D(wh) * G(wo, wi, wh) / 
		          (4.0f * cosThetaI * cosThetaO);
}

ColorRGB TorranceSparrow::Sample_f( const float3& wo, float3* wi, float u1, float u2, float* pdf ) const
{
	mD->Sample_f(wo, wi, u1, u2, pdf);
	
	if (!SameHemisphere(wo, *wi)) 
		return ColorRGB(0.0f, 0.0f, 0.0f);

	return f(wo, *wi);
}

float TorranceSparrow::Pdf( const float3& wo, const float3& wi ) const
{
	if (!SameHemisphere(wo, wi))
		return 0.0f;

	return mD->Pdf(wo, wi);
}

void Blin::Sample_f( const float3& wo, float3* wi, float u1, float u2, float* pdf ) const
{
	float cosTheta = powf(u1, 1.0f / (mExponent + 1.0f));
	float sinTheta = sqrtf(std::max(0.0f, 1- cosTheta * cosTheta));
	float phi = Mathf::TWO_PI * u2;

	float3 wh = SphericalDirection(cosTheta, sinTheta, phi);
	if (!SameHemisphere(wh, wo))
		wh = -wh;

	*wi = 2.0f * Dot(wo, wh) * wh - wo;

	float blinPdf = (mExponent + 1.0f) * powf(AbsCosTheta(wh), mExponent) / 
		(Mathf::TWO_PI * 4.0f * Dot(wo, wh));

	if (Dot(wo, wh) <= 0.f) 
		*pdf = 0.f;

	*pdf = blinPdf;
}

float Blin::Pdf( const float3& wo, const float3& wi ) const
{
	float3 wh = Normalize(wo + wi);

	if (Dot(wo, wh) <= 0.f) 
		return 0.0f;

	return (mExponent + 1.0f) * powf(AbsCosTheta(wh), mExponent) / 
		(Mathf::TWO_PI * 4.0f * Dot(wo, wh));
}

}

