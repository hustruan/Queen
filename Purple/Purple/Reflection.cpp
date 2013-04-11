#include "Reflection.h"
#include "MentoCarlo.h"
#include "Sampler.h"

namespace Purple {

using namespace RxLib;

ColorRGB BxDF::Sample( const float3& wo, float3* wi, float u1, float u2, float* pdf ) const
{
	*wi = CosineSampleHemisphere(u1, u2);
	if (wo.Z() < 0.) wi->Z() *= -1.f;
	*pdf = Pdf(wo, *wi);
	return Eval(wo, *wi);
}

ColorRGB BxDF::Rho( const float3& wo, int32_t numSamples, const float* samples ) const
{
	ColorRGB r(0.0f, 0.0f, 0.0f);
	for (int32_t i = 0; i < numSamples; ++i )
	{
		float3 wi; 
		float pdf = 0.0f;
		ColorRGB f = Sample(wo, &wi, samples[2 * i], samples[2 * i + 1], &pdf);
		
		if (pdf > 0.0f)
		{
			r += AbsCosTheta(wi) * f / pdf;
		}
	}
	return r / float(numSamples);
}

ColorRGB BxDF::Rho( int32_t numSamples, const float* samples1, const float* samples2 ) const
{
	ColorRGB r(0.0f, 0.0f, 0.0f);
	for (int32_t i = 0; i < numSamples; ++i)
	{
		float3 wo = UniformSampleHemisphere(samples1[2 * i], samples2[2 * i + 1]);

		float pdf_i, pdf_o = UniformHemispherePdf();
		float3 wi; 
		ColorRGB f = Sample(wo, &wi, samples2[2 * i], samples2[2 * i + 1], &pdf_i);
		if (pdf_i > 0.0f)
			r += f * AbsCosTheta(wi) * AbsCosTheta(wo) / (pdf_i * pdf_o);
	}

	return r / (Mathf::PI*numSamples);
}

float BxDF::Pdf( const float3& wo, const float3& wi ) const
{
	return CosineHemispherePdf(AbsCosTheta(wi));
}

ColorRGB SpecularRelection::Sample( const float3& wo, float3* wi, float u1, float u2, float* pdf ) const
{
	// Compute reflect vector
	*wi = float3(-wo.X(), -wo.Y(), wo.Z());
	*pdf = 1.0f;

	return mFresnel->Evaluate(CosTheta(wo)) * mR / AbsCosTheta(*wi);
}


ColorRGB OrenNayar::Eval( const float3& wo, const float3& wi ) const
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

ColorRGB TorranceSparrow::Eval( const float3& wo, const float3& wi ) const
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

ColorRGB TorranceSparrow::Sample( const float3& wo, float3* wi, float u1, float u2, float* pdf ) const
{
	mD->Sample_f(wo, wi, u1, u2, pdf);
	
	if (!SameHemisphere(wo, *wi)) 
		return ColorRGB(0.0f, 0.0f, 0.0f);

	return Eval(wo, *wi);
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


BSDF::BSDF( const DifferentialGeometry &dgs, const float3& ngeom, float e /*= 1.0f*/ )
	: dgShading(dgs), mGeoNormal(ngeom), eta(e)
{
	mNormal = dgShading.Normal;
	mBinormal = Normalize(dgShading.dpdu);
	mTangent = Cross(mNormal, mBinormal);
	mNumBxDFs = 0;
}

ColorRGB BSDF::Eval( const float3& woW, const float3& wiW, BSDFType flags /*= BSDF_All*/ ) const
{
	float3 wi = WorldToLocal(wiW), wo = WorldToLocal(woW);

	if (Dot(wiW, mGeoNormal) * Dot(woW, mGeoNormal) > 0) // ignore BTDFs
		flags = BSDFType(flags & ~BSDF_Transmission);
	else // ignore BRDFs
		flags = BSDFType(flags & ~BSDF_Reflection);

	ColorRGB retVal = ColorRGB::Black;
	
	for (int i = 0; i < mNumBxDFs; ++i)
		if (mBxDFs[i]->MatchFlags(flags))
			retVal += mBxDFs[i]->Eval(wo, wi);

	return retVal;
}

ColorRGB BSDF::Sample( const float3& woW, float3* wiW, const BSDFSample& bsdfSample, float *pdf, uint32_t bsdfFlags /*= BSDF_All*/, uint32_t* sampledType /*= NULL*/ ) const
{
	ColorRGB retVal;

	int matchingComps = NumComponents(bsdfFlags);

	if (matchingComps == 0)
	{
		*pdf = 0.0f;
		if(sampledType) *sampledType = (0);
		return ColorRGB::Black;
	}

	int which = std::min(Floor2Int(matchingComps * bsdfSample.uComponent), matchingComps-1);

	BxDF* bxdf = NULL;

	for (int i = 0; i < mNumBxDFs; ++i)
	{
		if (mBxDFs[i]->MatchFlags(bsdfFlags) && which-- == 0) 
		{
			bxdf = mBxDFs[i];
			break;
		}
	}
	assert(bxdf);

	float3 wo = WorldToLocal(woW);
	float3 wi;
	*pdf = 0.f;
	retVal = bxdf->Sample(wo, &wi, bsdfSample.uDir[0], bsdfSample.uDir[1], pdf);

	if (*pdf == 0.f)
	{
		if(sampledType) *sampledType = BSDFType(0);
		return ColorRGB::Black;
	}

	if (sampledType) *sampledType = bxdf->BxDFType;
	*wiW = LocalToWorld(wi);


	if ( !(bsdfFlags & BSDF_Specular) && matchingComps > 1)
	{
		for (int i = 0; i < mNumBxDFs; ++i)
		{
			if (mBxDFs[i] != bxdf && mBxDFs[i]->MatchFlags(bsdfFlags)) 
				*pdf += mBxDFs[i]->Pdf(wo, wi);
		}
	}

	if (matchingComps > 1)
		*pdf /= float(matchingComps);


	if ( !(bsdfFlags & BSDF_Specular) )
	{
		retVal = ColorRGB::Black;
		if (Dot(*wiW, mGeoNormal) * Dot(woW, mGeoNormal) > 0) // ignore BTDFs
			bsdfFlags = (bsdfFlags & ~BSDF_Transmission);
		else // ignore BRDFs
			bsdfFlags = (bsdfFlags & ~BSDF_Reflection);

		for (int i = 0; i < mNumBxDFs; ++i)
		{
			if (mBxDFs[i]->MatchFlags(bsdfFlags))
				retVal += mBxDFs[i]->Eval(wo, wi);
		}

	}

	return retVal;
}

float BSDF::Pdf( const float3& woW, const float3& wiW, BSDFType flags /*= BSDF_All*/ ) const
{
	float3 wi = WorldToLocal(wiW), wo = WorldToLocal(woW);
	float pdf = 0.f;
	int matchingComps = 0;
	for (int i = 0; i < mNumBxDFs; ++i)
	{
		if (mBxDFs[i]->MatchFlags(flags)) 
		{
			++matchingComps;
			pdf += mBxDFs[i]->Pdf(wo, wi);
		}
	}
	return matchingComps > 0 ? pdf / matchingComps : 0.f;
}

ColorRGB BSDF::Rho( Random& rng, BSDFType flags /*= BSDF_All*/, int sqrtSamples /*= 6*/ ) const
{
	int nSamples = sqrtSamples * sqrtSamples;
	float *s1 = (float*)alloca(2 * nSamples);
	StratifiedSample2D(s1, sqrtSamples, sqrtSamples, rng);
	float *s2 = (float*)alloca(2 * nSamples);
	StratifiedSample2D(s2, sqrtSamples, sqrtSamples, rng);

	ColorRGB retVal = ColorRGB::Black;
	for (int i = 0; i < mNumBxDFs; ++i)
	{
		if (mBxDFs[i]->MatchFlags(flags)) 
		{
			retVal += mBxDFs[i]->Rho(nSamples, s1, s2);
		}
	}
	
	return retVal;
}

ColorRGB BSDF::Rho( const float3& wo, Random& rng, BSDFType flags /*= BSDF_All*/, int sqrtSamples /*= 6*/ ) const
{
	int nSamples = sqrtSamples * sqrtSamples;
	float *s1 = (float*)alloca(2 * nSamples);
	StratifiedSample2D(s1, sqrtSamples, sqrtSamples, rng);

	ColorRGB retVal = ColorRGB::Black;
	for (int i = 0; i < mNumBxDFs; ++i)
	{
		if (mBxDFs[i]->MatchFlags(flags)) 
		{
			retVal += mBxDFs[i]->Rho(wo, nSamples, s1);
		}
	}

	return retVal;
}

int BSDF::NumComponents( uint32_t bsdfFlags ) const
{
	int num = 0;
	for (int i = 0; i < mNumBxDFs; ++i)
	{
		if (mBxDFs[i]->MatchFlags(bsdfFlags))
			++num;
	}

	return num;
}


BSDFSampleOffsets::BSDFSampleOffsets( int count, Sample* sample )
	: nSamples(count)
{
	componentOffset = sample->Add1D(nSamples);
	dirOffset = sample->Add2D(nSamples);
}


BSDFSample::BSDFSample( Sample* sample, const BSDFSampleOffsets& offset, uint32_t n )
{
	uDir[0] = sample->TwoD[offset.dirOffset][2*n];
	uDir[1] = sample->TwoD[offset.dirOffset][2*n+1];
	uComponent = sample->OneD[offset.componentOffset][n];
}

}

