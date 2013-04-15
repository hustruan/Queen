#ifndef Film_h__
#define Film_h__

#include "Prerequisites.h"
#include <BlockedArray.h>

namespace Purple {

class Filter;

class Film 
{
public:
	Film(int xRes, int yRes);

	virtual ~Film();

	virtual void AddSample(const Sample& sample, const ColorRGB& L) = 0;

	virtual void WriteImage(const char* filename) = 0;

	const int xResolution, yResolution;
};


class ImageFilm : public Film
{
public:
	ImageFilm(int xRes, int yRes, Filter* filter);
	~ImageFilm();

	void AddSample(const Sample& sample, const ColorRGB& L);

	void WriteImage(const char* filename);

	//void RefreshColor(Sampler* tileSampler);

private:
	Filter* mFilter;
	float* mFilterTable;
	int xPixelStart, yPixelStart, xPixelCount, yPixelCount;

	struct Pixel 
	{
		Pixel() {
			for (int i = 0; i < 3; ++i) Lrgb[i] = splatRGB[i] = 0.f;
			weightSum = 0.f;
		}

		float Lrgb[3];
		float weightSum;
		float splatRGB[3];
		float pad;
	};

	RxLib::BlockedArray<Pixel, 2>* pixels;


	//std::vector<float> mColorBuffer;
};


}


#endif // Film_h__
