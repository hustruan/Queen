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
		Num1D.push_back(nSamples);
		return Num1D.size() -1;
	}

	uint32_t Add2D(uint32_t nSamples)
	{
		Num2D.push_back(nSamples);
		return Num2D.size() -1;
	}

	Sample* Duplicate(int count) const;


	Sample(Sampler* sampler, SurfaceIntegrator* si, const Scene* scene);
	~Sample();

	std::vector<uint32_t> Num1D, Num2D;
	float **OneD, **TwoD;

private:
	Sample() { OneD = TwoD = NULL; }
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

	virtual Sampler* Clone(int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd) const = 0;

	// Return the number of configured pixel samples 
	virtual uint32_t GetSampleCount() const = 0;

	virtual uint32_t GetMoreSamples(Sample* samples, Random& rng) = 0;

	virtual int32_t RoundSize(int32_t size) const = 0;

	/**
	 * Divide image into tiles and multi-thread can get a sampler to execute.
	 * @param count, total number of subsamplers
	 * @param num, subsampler index
	 */
	//virtual Sampler* GetSubSampler(int32_t num, int32_t count) = 0;

protected:

	/**
	 * An utility function for divide window into rectangle tiles.
	 */
	void ComputeSubWindow(int32_t num, int32_t count, int32_t* newXStart, int32_t* newXEnd, int32_t* newYStart, int32_t* newYEnd);


public:
	const int32_t SamplesPerPixel;
	int32_t PixelStartX, PixelStartY, PixelEndX, PixelEndY;

};


/** 
 * Stratified jitter sampler
 */
class StratifiedSampler : public Sampler
{
public:
	StratifiedSampler(int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd, int32_t xNumSamples, int32_t yNunSamples);
	~StratifiedSampler();

	Sampler* Clone(int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd) const;

	uint32_t GetSampleCount() const { return mPixelSamplesX * mPixelSamplesY; }

	uint32_t GetMoreSamples(Sample* samples, Random& rng);

	int32_t RoundSize(int32_t size) const { return size; }

	//Sampler* GetSubSampler(int32_t num, int32_t count);

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
