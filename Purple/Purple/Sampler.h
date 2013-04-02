#ifndef Sampler_h__
#define Sampler_h__

#include "Prerequisites.h"

namespace Purple {

struct CameraSample 
{
	float2 ImageSample;
	float2 LensSample;
	float TimeSample;
};


class Sampler
{
public:
	Sampler(int32_t xStart, int32_t xEnd, int32_t yStart, int32_t yEnd);
	virtual ~Sampler(void);

	// Return the number of configured pixel samples 
	virtual uint32_t GetSampleCount() const = 0;

	virtual uint32_t GetMoreSamples(CameraSample* samples, Random& rng) = 0;

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

	uint32_t GetMoreSamples(CameraSample* samples, Random& rng);

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
