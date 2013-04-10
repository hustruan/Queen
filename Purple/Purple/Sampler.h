#ifndef Sampler_h__
#define Sampler_h__

#include "Prerequisites.h"

namespace Purple {


struct Sample
{
	float2 ImageSample;
	float2 LensSample;
	float TimeSample;


	uint32_t Add1D(uint32_t nSamples)
	{
		mSamplesRecord1D.push_back(nSamples);
		return mSamplesRecord1D.size() -1;
	}

	uint32_t Add2D(uint32_t nSamples)
	{
		mSamplesRecord2D.push_back(nSamples);
		return mSamplesRecord2D.size() -1;
	}

	Sample* Duplicate(int count) const;


	~Sample();

	std::vector<uint32_t> mSamplesRecord1D, mSamplesRecord2D;
	float **oneD, **twoD;

private:

    void AllocateSampleMemory();
};

/**
 * A Sampler is responsible for doing following things
 * 1. Generate image samples for generating camera ray
 * 2. Generate samples which will used by Integrator to evaluate light transport equation
 */
class Sampler
{
public:
	Sampler(int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd, int32_t samplerPerPixel);
	virtual ~Sampler(void);

	// Return the number of configured pixel samples 
	virtual uint32_t GetSampleCount() const = 0;

	virtual uint32_t GetMoreSamples(Sample* samples, Random& rng) = 0;

	/**
	 * Divide image into tiles and multi-thread can get a sampler to execute.
	 * @param count, total number of subsamplers
	 * @param num, subsampler index
	 */
	virtual Sampler* GetSubSampler(int32_t num, int32_t count) = 0;

protected:

	/**
	 * An utility function for divide window into rectangle tiles.
	 */
	void ComputeSubWindow(int32_t num, int32_t count, int32_t* newXStart, int32_t* newXEnd, int32_t* newYStart, int32_t* newYEnd);


public:
	const int32_t SamplesPerPixel;

protected:
	int32_t mPixelStartX, mPixelStartY, mPixelEndX, mPixelEndY;

};


/** 
 * Stratified jitter sampler
 */
class StratifiedSampler : public Sampler
{
public:
	StratifiedSampler(int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd, int32_t xNumSamples, int32_t yNunSamples);
	~StratifiedSampler();

	uint32_t GetSampleCount() const { return mPixelSamplesX * mPixelSamplesY; }
	
	Sampler* GetSubSampler(int32_t num, int32_t count);

	uint32_t GetMoreSamples(Sample* samples, Random& rng);

private:

	// sample count in a single pixel, X and Y direction
	int32_t mPixelSamplesX, mPixelSamplesY;

	// current sample pixel
	int32_t mCurrPixelX, mCurrPixelY;

	// stratified samples buffer
	float* mSamplesBuffer;
};

}



#endif // Sampler_h__
