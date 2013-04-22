#include "Sampler.h"
#include "MentoCarlo.h"
#include "Integrator.h"
#include <aligned_allocator.h>
#include <assert.h>
#include <FloatCast.hpp>
#include <Math.hpp>

namespace Purple {

using namespace RxLib;

Sampler::Sampler( int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd, int32_t samplerPerPixel )
	: PixelStartX(xStart), PixelStartY(yStart), PixelEndY(yEnd), PixelEndX(xEnd), SamplesPerPixel(samplerPerPixel)
{

}

Sampler::~Sampler(void)
{
}


StratifiedSampler::StratifiedSampler( int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd, int32_t xNumSamples, int32_t yNumSamples )
	: Sampler(xStart, xEnd, yStart, yEnd, xNumSamples * yNumSamples), mPixelSamplesX(xNumSamples), mPixelSamplesY(yNumSamples)
{
	mCurrPixelX = xStart;
	mCurrPixelY = yStart;

	mSamplesBuffer = new float[mPixelSamplesX * mPixelSamplesY * 5];
}

StratifiedSampler::~StratifiedSampler()
{
	delete[] mSamplesBuffer;
}

Sampler* StratifiedSampler::Clone( int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd ) const
{
	return new StratifiedSampler(xStart, xEnd, yStart, yEnd, mPixelSamplesX, mPixelSamplesY);
}

uint32_t StratifiedSampler::GetMoreSamples( Sample* samples, Random& rng )
{
	if (mCurrPixelY == PixelEndY)
		return 0;

	int32_t numSamples = mPixelSamplesX * mPixelSamplesY;

	float* imageSamplesBuffer = mSamplesBuffer;
	float* lensSamplesBuffer = mSamplesBuffer + numSamples * 2;
	float* timeSamplesBuffer = mSamplesBuffer + numSamples * 4;

	StratifiedSample2D(imageSamplesBuffer, mPixelSamplesX, mPixelSamplesY, rng);
	StratifiedSample2D(lensSamplesBuffer, mPixelSamplesX, mPixelSamplesY, rng);
	StratifiedSample1D(timeSamplesBuffer, numSamples, rng);

	// add pixel offset
	for (int32_t i = 0; i < 2 * numSamples; i += 2)
	{
		imageSamplesBuffer[i] += mCurrPixelX;
		imageSamplesBuffer[i+1] += mCurrPixelY;
	}

	// shuffle lens and time samples
	Shuffle(lensSamplesBuffer, numSamples, 2, rng);
	Shuffle(timeSamplesBuffer, numSamples, 1, rng);

	for (int32_t i = 0; i < numSamples; ++i)
	{
		samples[i].ImageSample = float2(imageSamplesBuffer[i*2], imageSamplesBuffer[i*2+1]);
		samples[i].LensSample = float2(lensSamplesBuffer[i*2], lensSamplesBuffer[i*2+1]);
		samples[i].TimeSample = timeSamplesBuffer[i];/*Lerp(timeSamplesBuffer[i], mShutterOpen, shutterClose)*/; 

		// Generate stratified samples for integrators
		for (uint32_t j = 0; j < samples[i].Num1D.size(); ++j)
			LatinHypercube(samples[i].OneD[j], samples[i].Num1D[j], 1, rng);
		for (uint32_t j = 0; j < samples[i].Num2D.size(); ++j)
			LatinHypercube(samples[i].TwoD[j], samples[i].Num2D[j], 2, rng);
	}

	// Advance to next pixel for stratified sampling
	if ( ++mCurrPixelX == PixelEndX )
	{
		mCurrPixelX = PixelStartX;
		++mCurrPixelY;
	}

	return numSamples;
}

//--------------------------------------------------------------------------------------------
Sample::Sample( Sampler* sampler, SurfaceIntegrator* si, const Scene* scene )
{
	if(si) si->RequestSamples(sampler, this, scene);
	AllocateSampleMemory();
}

Sample::~Sample()
{
	if (OneD)
	{
		aligned_free(OneD[0]);
		aligned_free(OneD);
	}
}

void Sample::AllocateSampleMemory()
{
	// Allocate storage for sample pointers
	int nPtrs = Num1D.size() + Num2D.size();

	if (!nPtrs) {
		OneD = TwoD = NULL;
		return;
	}

	OneD = (float**)aligned_malloc(sizeof(float*) * nPtrs, L1_CACHE_LINE_SIZE);
	TwoD = OneD + Num1D.size();

	// Compute total number of sample values needed
	int totSamples = 0;
	for (uint32_t i = 0; i < Num1D.size(); ++i)
		totSamples += Num1D[i];
	for (uint32_t i = 0; i < Num2D.size(); ++i)
		totSamples += 2 * Num2D[i];

	// Allocate storage for sample values
	float *mem = (float*)aligned_malloc(sizeof(float) * totSamples, L1_CACHE_LINE_SIZE);

	for (uint32_t i = 0; i < Num1D.size(); ++i)
	{
		OneD[i] = mem;
		mem += Num1D[i];
	}

	for (uint32_t i = 0; i < Num2D.size(); ++i)
	{
		TwoD[i] = mem;
		mem += 2 * Num2D[i];
	}
}

Sample* Sample::Duplicate( int count ) const
{
	Sample *ret = new Sample[count];
	for (int i = 0; i < count; ++i)
	{
		ret[i].Num1D = Num1D;
		ret[i].Num2D = Num2D;
		ret[i].AllocateSampleMemory();
	}
	return ret;
}



}



