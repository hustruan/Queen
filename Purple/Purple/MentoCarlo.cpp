#include "MentoCarlo.h"
#include "Random.h"

static const float OneMinusEpsilon=0.9999999403953552f;

namespace Purple {

using namespace RxLib;

Distribution1D::Distribution1D( const float* f, int n ) : Count(n)
{
	pdf = new float[n];
	memcpy(pdf, f, n*sizeof(float));

	cdf = new float[n+1];
	cdf[0] = 0.0f;
	for (int i = 1; i <= n; ++i)
		cdf[i] = cdf[i-1] + pdf[i-1] / n;

	Integral = cdf[n];
	if (Integral == 0.0f)
	{
		for (int i = 1; i <= n; ++i)
			cdf[i] = float(i) / float(n);
	}
	else
	{
		for (int i = 1; i <= n; ++i)
			cdf[i] = cdf[i] / Integral;
	}
}

Distribution1D::~Distribution1D()
{
	delete[] cdf;
	delete[] pdf;
}

Distribution2D::Distribution2D( const float* f, int nu, int nv )
{
	mConditionalV.reserve(nv);
	for (int v = 0; v < nv; ++v)
		mConditionalV.push_back(new Distribution1D(f + v * nu, nu));

	std::vector<float> integrals;
	integrals.reserve(nv);
	for (int v = 0; v < nv; ++v)
		integrals.push_back(mConditionalV[v]->Integral);

	mMarginal = new Distribution1D(&integrals[0], nv);
}

Distribution2D::~Distribution2D()
{
	delete mMarginal;
	for (auto iter = mConditionalV.begin(); iter != mConditionalV.end(); ++iter)
		delete (*iter);
}

void StratifiedSample1D( float* samples, int32_t numSamples, Random& rng, bool jitter /*= true*/ )
{
	float invStrata = 1.0f / numSamples;

	for (int32_t i = 0; i < numSamples; ++i)
	{
		float delta = jitter ? rng.RandomFloat() : 0.5f;
		samples[i] = std::min((i + delta) * invStrata, OneMinusEpsilon);
	}
}

void StratifiedSample2D( float* samples, int32_t xNumSamples, int32_t yNumSamples, Random& rng, bool jitter /*= true*/ )
{
	float invStrataX = 1.0f / xNumSamples;
	float invStrataY = 1.0f / yNumSamples;

	float deltaX, deltaY;
	for (int32_t y = 0; y < yNumSamples; ++y)
	{
		for (int32_t x = 0; x < xNumSamples; ++x)
		{		
			if (jitter)
			{
				deltaX = rng.RandomFloat();
				deltaY = rng.RandomFloat();
			}
			else
			{
				deltaX = deltaY = 0.5f;
			}

			*samples++ = std::min((x + deltaX) * invStrataX, OneMinusEpsilon);
			*samples++ = std::min((y + deltaY) * invStrataY, OneMinusEpsilon);
		}
	}
}



}


