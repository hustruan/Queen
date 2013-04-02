#include "Reflection.h"
#include "MentoCarlo.h"

namespace {

using namespace RxLib;

inline float CosTheta(const float3& w) 
{
	return w.Z();
}

inline float AbsCosTheta(const float3& w) 
{
	return fabsf(w.Z());
}

inline float SinTheta2(const float3& w) 
{
	return (std::max)(0.0f, 1.0f - w.Z() * w.Z());
}

inline float SinTheta(const float3& w) 
{
	return sqrtf(SinTheta2(w));
}

inline float SinPhi(const float3& w) 
{
	float sintheta = SinTheta(w);
	if (sintheta == 0.f) return 1.f;
	return Clamp(w.Y() / sintheta, -1.f, 1.f);
}

inline float CosPhi(const float3& w) 
{
	float sintheta = SinTheta(w);
	if (sintheta == 0.f) return 0.f;
	return Clamp(w.X() / sintheta, -1.f, 1.f);
}

}

namespace Purple {

using namespace RxLib;

ColorRGB BxDF::Sample_f( const float3& wo, float3* wi, float u1, float u2, float* pdf ) const
{
	*wi = CosineSampleHemisphere(u1, u2);
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

}

