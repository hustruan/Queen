#ifndef Reflection_h__
#define Reflection_h__

#include "Prerequisites.h"
#include "DifferentialGeometry.h"
#include "Random.h"
#include <Math.hpp>

namespace Purple {

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
	return RxLib::Clamp(w.Y() / sintheta, -1.f, 1.f);
}

inline float CosPhi(const float3& w) 
{
	float sintheta = SinTheta(w);
	if (sintheta == 0.f) return 0.f;
	return RxLib::Clamp(w.X() / sintheta, -1.f, 1.f);
}

inline bool SameHemisphere(const float3& w, const float3& wp) 
{
	return w.Z() * wp.Z() > 0.f;
}

inline float3 ReflectDirection(const float3& w)
{
	return float3(-w.X(), -w.Y(), w.Z());
}

inline float3 SphericalDirection(float cosTheta, float sinTheta, float phi)
{
	return float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
}

//-------------------------------------------------------------------------------
enum BSDFType
{
	BSDF_Reflection        = 1U << 0,
	BSDF_Transmission      = 1U << 1,
	BSDF_Diffuse           = 1U << 2,
	BSDF_Glossy            = 1U << 3,
	BSDF_Specular          = 1U << 4,
	BSDF_All_Type          = BSDF_Diffuse | BSDF_Glossy | BSDF_Specular,
	BSDF_All_Reflection    = BSDF_Reflection | BSDF_All_Type,
	BSDF_All_Transmission  = BSDF_Transmission | BSDF_All_Type,
	BSDF_All               = BSDF_All_Reflection | BSDF_All_Transmission
};

struct BSDFSampleOffsets
{
	BSDFSampleOffsets() { }
	BSDFSampleOffsets(int count, Sample* sample);
	int nSamples, componentOffset, dirOffset;
};

struct BSDFSample
{
	BSDFSample(Random& rng)
	{
		uDir[0] = rng.RandomFloat();
		uDir[1] = rng.RandomFloat();
		uComponent = rng.RandomFloat();
	}

	BSDFSample(float u1, float u2, float uCom)
	{
		uDir[0] = u1;
		uDir[1] = u2;
		uComponent = uCom;
	}

	BSDFSample(Sample* sample, const BSDFSampleOffsets& offset, uint32_t num);

	float uDir[2], uComponent;
};

class BxDF;

class BSDF
{
public:
	BSDF(const DifferentialGeometry &dgs, const float3& ngeom, float eta = 1.0f);

	inline void Add(BxDF* bxdf)
	{
		assert(mNumBxDFs < MAX_BxDFS);
		mBxDFs[mNumBxDFs++] = bxdf;
	}
	
	int NumComponents() const { return mNumBxDFs; }
	
	int NumComponents(uint32_t bsdfFlags) const;

	ColorRGB Sample(const float3& woW, float3* wiW, const BSDFSample& bsdfSample,
		float *pdf, uint32_t bsdfFlags = BSDF_All, uint32_t* sampledType = NULL) const;

	float Pdf(const float3& woW, const float3& wiW, BSDFType flags = BSDF_All) const;

	ColorRGB Eval(const float3& woW, const float3& wiW, BSDFType flags = BSDF_All) const;

	ColorRGB Rho(Random& rng, BSDFType flags = BSDF_All, int sqrtSamples = 6) const;

	ColorRGB Rho(const float3& wo, Random& rng, BSDFType flags = BSDF_All, int sqrtSamples = 6) const;


	float3 WorldToLocal(const float3& v) const
	{
		return float3(Dot(v, mBinormal), Dot(v, mTangent), Dot(v, mNormal));
	}

	float3 LocalToWorld(const float3& v) const
	{
		return float3(mBinormal.X() * v.X() + mTangent.X() * v.Y() + mNormal.X() * v.Z(),
			mBinormal.Y() * v.X() + mTangent.Y() * v.Y() + mNormal.Y() * v.Z(),
			mBinormal.Z() * v.X() + mTangent.Z() * v.Y() + mNormal.Z() * v.Z());
	}	


public:
	// BSDF Public Data
	const DifferentialGeometry dgShading;
	const float eta;

private:

	// BSDF Private Data
	float3 mNormal, mGeoNormal;
	float3 mTangent, mBinormal;

	int mNumBxDFs;

	static const int MAX_BxDFS = 8;

	BxDF* mBxDFs[MAX_BxDFS];
};

#define BSDF_ALLOC(arena, Type) new (arena.Alloc(sizeof(Type))) Type

class BxDF 
{
public:

	BxDF(uint32_t bxdfType) : BxDFType(bxdfType) { }
	virtual ~BxDF() { }

	bool MatchFlags(uint32_t bxdfFlags) const { return (BxDFType & bxdfFlags) == BxDFType; }

	/**
	 * @breif Evaluate the BRDF for a pair of given directions.
	 */
	virtual ColorRGB Eval(const float3& wo, const float3& wi) const = 0;

	/**
	 * Sample the BRDF and return the sampled BRDF value. Also return the sampled indicent 
	 */
	virtual ColorRGB Sample(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;

	virtual float Pdf(const float3& wo, const float3& wi) const;

	/**
	 * 计算给定方向的光在半球空间反射的积分，rho <= 1 能量守恒。如果不能通过解析方法积分。就根据
	 * Samples 用Mento Carlo积分的方法计算。
	 */
	virtual ColorRGB Rho(const float3& wo, int32_t numSamples, const float* samples) const;

	/**
	 * 
	 */
	virtual ColorRGB Rho(int32_t numSamples, const float* samples1, const float* samples2) const;
	

public:
	const uint32_t BxDFType;
};

//------------------------------------------------------------------------
struct Fresnel
{
	virtual ~Fresnel() { };
	virtual ColorRGB Evaluate(float cosi) const = 0;
};

/**
 * Calculates the unpolarized Fresnel reflection coefficient
 * at a planar interface between two conductors.
 */
struct FresnelConductor : public Fresnel
{
public:
	/**
	 * Cosine of the angle between the normal and the incident ray
	 */
	ColorRGB Evaluate(float cosi) const;

	/**
	 * @param eta
	 *     Relative refractive index
	 * @param k
	 *     Absorption coefficient
	 */
	FresnelConductor(const ColorRGB& feta, const ColorRGB& fk)
		: eta(feta), k(fk) {}

private:
	ColorRGB eta, k;
};

/**
 * Calculates the unpolarized Fresnel reflection coefficient
 * at a planar interface between two dielectrics.
 */
struct FresnelDielectric : public Fresnel
{
public:
	 /**
	  * @param cosThetaI
	  *		Cosine of the angle between the normal and the incident ray (may be negative)
	  */
	ColorRGB Evaluate(float cosi) const;

	FresnelDielectric(float ei, float et) : eta_i(ei), eta_t(et) { }

private:
	float eta_i, eta_t;
};

/**
  * Perfect specular reflection
  */
class SpecularReflection : public BxDF
{
public:
	SpecularReflection(const ColorRGB& reflectance, Fresnel* fresnel)
		: BxDF(BSDF_Reflection | BSDF_Specular), mR(reflectance), mFresnel(fresnel) {}

	ColorRGB Eval(const float3& wo, const float3& wi) const  { return ColorRGB::Black; }

	float Pdf(const float3& wo, const float3& wi) const { return 0.0f; }

	ColorRGB Sample(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;

private:
	ColorRGB mR;			// reflect color
	Fresnel* mFresnel;	    
};

class SpecularTransmission : public BxDF
{
public:
	SpecularTransmission(const ColorRGB& t, float ei, float et)
		: BxDF(BSDF_Transmission | BSDF_Specular), mT(t), eta_i(et), eta_t(et), mFresnel(ei, et)
	{ }

	ColorRGB Eval(const float3& wo, const float3& wi) const  { return ColorRGB::Black; }

	float Pdf(const float3& wo, const float3& wi) const { return 0.0f; }

	ColorRGB Sample(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;

private:
	ColorRGB mT;			// reflect color
	FresnelDielectric mFresnel;	   
	float eta_i, eta_t;
};

class Lambertian : public BxDF
{
public:
	Lambertian(const ColorRGB& reflectance) 
		: BxDF(BSDF_Reflection | BSDF_Diffuse), mR(reflectance) { }

	ColorRGB Eval(const float3& wo, const float3& wi) const	{ return mR * RxLib::Mathf::INV_PI; }

	ColorRGB Rho(const float3& wo, int32_t numSamples, const float* samples) const { return mR; }

	ColorRGB Rho(int32_t numSamples, const float* samples1, const float* samples2) const { return mR; }

private:
	ColorRGB mR;			// reflect color
};


class OrenNayar : public BxDF
{
public:
	OrenNayar(const ColorRGB& reflectance, float sigma)
		: BxDF(BSDF_Reflection | BSDF_Diffuse), mR(reflectance)
	{
		sigma = RxLib::ToRadian(sigma);
		float sigma2 = sigma*sigma; 
		
		A = 1.0f - sigma2 / (2.0f * sigma2 + 0.33f);
		B = 0.45f * sigma2 / (sigma2 + 0.09f);
	}

	ColorRGB Eval(const float3& wo, const float3& wi) const;

private:
	float A, B;             // OrenNayar BRDF contants
	ColorRGB mR;			// reflect color
};


struct MicrofacetDistribution
{
	virtual ~MicrofacetDistribution() {}
	virtual float D(const float3& wh) const = 0;
	virtual void Sample_f(const float3& wo, float3* wi, float u1, float u2, float* pdf) const = 0;
	virtual float Pdf(const float3& wo, const float3& wi) const = 0;
};

class TorranceSparrow : public BxDF
{
public:
	TorranceSparrow(const ColorRGB& reflectance, Fresnel* fresnel)
		: BxDF(BSDF_Reflection | BSDF_Glossy), mR(reflectance), mFresnel(fresnel) {}

	ColorRGB Eval(const float3& wo, const float3& wi) const;

	float Pdf(const float3& wo, const float3& wi) const;

	ColorRGB Sample(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;

private:

	/**
	 * Geometric attenuation term
	 */
	float G(const float3& wo, const float3& wi, const float3& wh) const;

private:

	ColorRGB mR;
	Fresnel* mFresnel;	 
	MicrofacetDistribution* mD;
};

struct Blinn : MicrofacetDistribution
{
	Blinn(float e) : mExponent(e) {}
	
	float D(const float3& wh) const
	{
		return (mExponent + 2.0f) * RxLib::Mathf::INV_TWO_PI * powf(AbsCosTheta(wh), mExponent);
	}

	void Sample_f(const float3& wo, float3* wi, float u1, float u2, float* pdf) const;
	float Pdf(const float3& wo, const float3& wi) const;

private:
	float mExponent;
};


}


#endif // Reflection_h__
