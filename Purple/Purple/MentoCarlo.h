#ifndef MentoCarlo_h__
#define MentoCarlo_h__

#include "Prerequisites.h"
#include <Vector.hpp>
#include <FloatCast.hpp>
#include "Random.h"

namespace Purple {

inline float3 UniformSampleHemisphere(float u1, float u2)
{
	float sintheta = sqrtf((std::max)(0.0f, 1 - u1*u1));
	float phi = RxLib::Mathf::TWO_PI * u2;
	return float3(sintheta*cosf(phi), sintheta*sinf(phi), u1);
}

inline float UniformHemispherePdf()
{
	return RxLib::Mathf::INV_TWO_PI;
}

inline float3 UniformSampleSphere(float u1, float u2)
{
	float z = 1.0f - 2.0f * u1;
	float sintheta = sqrtf(std::max(0.0f, 1 - z*z));
	float phi = RxLib::Mathf::TWO_PI * u2;
	return float3(sintheta*cosf(phi), sintheta*sinf(phi), z);
}

inline float UniformSpherePdf()
{
	return RxLib::Mathf::INV_FOUR_PI;
}

inline void UniformSampleDisk(float u1, float u2, float* x, float* y)
{
	float r = sqrtf(u1);
	float theta = RxLib::Mathf::TWO_PI * u2;
	*x = r * cosf(theta);
	*y = r * sinf(theta);
}

void ConcentricSampleDisk(float u1, float u2, float *dx, float *dy);

inline float3 CosineSampleHemisphere(float u1, float u2)
{
	float3 retVal;
	//UniformSampleDisk(u1, u2, &retVal[0], &retVal[1]);
	ConcentricSampleDisk(u1, u2, &retVal[0], &retVal[1]);
	retVal.Z() = sqrtf(std::max(0.0f, 1.0f - retVal.X()*retVal.X() - retVal.Y()*retVal.Y()));

	///* Guard against numerical imprecisions */
	//if (retVal.Z() == 0.0f)
	//{
	//	printf("Z===0\n");
	//	retVal.Z() = 1e-10f;
	//}

	return retVal;
}

inline float CosineHemispherePdf(float costheta) 
{
	return RxLib::Mathf::INV_PI * costheta;
}

inline float3 UniformSampleCone(float u1, float u2, float costhetamax) 
{
	float costheta = (1.f - u1) + u1 * costhetamax;
	float sintheta = sqrtf(1.f - costheta*costheta);
	float phi = u2 * 2.f * RxLib::Mathf::PI;
	return float3(cosf(phi) * sintheta, sinf(phi) * sintheta, costheta);
}

inline float3 UniformSampleCone(float u1, float u2, float costhetamax, const float3& x, const float3& y, const float3& z)
{
	float costheta = (1.f - u1) + u1 * costhetamax;
	float sintheta = sqrtf(1.f - costheta*costheta);
	float phi = u2 * 2.f * RxLib::Mathf::PI;
	return x * cosf(phi) * sintheta + y * sinf(phi) * sintheta + z *costheta;
}

inline float UniformConePdf(float cosThetaMax) 
{
	return 1.0f / (2.f * RxLib::Mathf::PI * (1.0f - cosThetaMax));
}

inline void UniformSampleTriangle(float u1, float u2, float* u, float* v)
{
	float tmp = sqrtf(u1);	
	*u = 1.0f - tmp;
	*v = u2 * tmp;
}

inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	float f = nf * fPdf, g = ng * gPdf;
	return (f*f) / (f*f + g*g);
}

/**
 * Generate 1D and 2D samples using stratified method
 * @param jitter add jitter offset 
 */
void StratifiedSample1D(float* samples, int32_t numSamples, Random& rng, bool jitter = true);
void StratifiedSample2D(float* samples, int32_t xNumSamples, int32_t yNumSamples, Random& rng, bool jitter = true);

void LatinHypercube(float* samples, int32_t numSamples, int32_t dim, Random& rng);

/**
 * Random Shuffle 
 */
template<typename T>
void Shuffle(T* samples, int32_t numSamples, int32_t dims, Random& rng)
{
	for (int32_t i = numSamples - 1; i > 0; --i)
	{
		int32_t other = (rng.RandomUInt() % (i+1));
		for (int32_t j = 0; j < dims; ++j)
			std::swap(samples[dims*i + j], samples[dims*other + j]);
	}
}

/**
 * This data structure can be used to transform uniformly distributed
 * samples to a stored discrete probability distribution.
 */
struct Distribution1D
{
	/**
	 * @param f 
	 *   unnormalized pdf sequence
	 */
	Distribution1D(const float* f, int n);
	~Distribution1D();

	float SampleContinuous(float u, float *pdf, int* off = nullptr) const
	{
		// Find surrounding CDF segments and offset
		float *ptr = std::upper_bound(cdf, cdf+Count+1, u);
		int offset = std::max((ptrdiff_t)0, ptr-cdf-1);
		if (off) *off = offset;

		assert(offset < Count);
		assert(u >= cdf[offset] && u < cdf[offset+1]);

		// Compute offset along CDF segment
		float du = (u - cdf[offset]) / (cdf[offset+1] - cdf[offset]);
		assert(!isnan(du));

		// Compute PDF for sampled offset
		if (pdf) *pdf = pdf[offset] / Integral;

		return (offset + du) / Count;
	}

	int SampleDiscrete(float u, float* pdf ) const 
	{
		float *ptr = std::upper_bound(cdf, cdf+Count+1, u);
		int offset = std::max((ptrdiff_t)0, ptr-cdf-1);

		assert(offset < Count);
		assert(u >= cdf[offset] && u < cdf[offset+1]);
		
		if (pdf) 
			*pdf = pdf[offset] / (Integral * Count);
		
		return offset;
	}

private:
	float* pdf;		//概率密度，并没有归一化到1,需要除以Integral
	float* cdf;	
	float Integral;  // 没有归一化的pdf在domain上的积分
	int Count;

	friend struct Distribution2D;
};

struct Distribution2D
{
	Distribution2D(const float* f, int nu, int nv);
	~Distribution2D();

	void SampleContinuous(float u0, float u1, float uv[2], float *pdf)
	{
		float pdfs[2];

		int v;
		uv[1] = mMarginal->SampleContinuous(u1, &pdfs[1], &v);
		uv[0] = mConditionalV[v]->SampleContinuous(u0, &pdf[0]);

		if(pdf)
			*pdf = pdfs[0] * pdfs[1];
	}

	float Pdf(float u, float v)
	{
		int iu = RxLib::Clamp(Float2Int(u * mConditionalV[0]->Count), 0, mConditionalV[0]->Count - 1);
		int iv = RxLib::Clamp(Float2Int(v * mMarginal->Count), 0, mMarginal->Count - 1);

		return (mConditionalV[iv]->pdf[iu] / mConditionalV[iv]->Integral) *
			(mMarginal->pdf[iv] / mMarginal->Integral);
	}

private:
	Distribution1D* mMarginal;
	std::vector<Distribution1D*> mConditionalV;
};

}

#endif // MentoCarlo_h__