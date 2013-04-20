#ifndef Film_h__
#define Film_h__

#include "Prerequisites.h"
#include <BlockedArray.h>

namespace Purple {

using RxLib::int2;

class Filter;

class Film 
{
public:
	Film(int xRes, int yRes, Filter* filter);
	~Film();

	void AddSample(const Sample& sample, const ColorRGB& L);
	void WriteImage(const char* filename);

	const int xResolution, yResolution;

private:

	Filter* mFilter;
	float* mFilterTable;

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
};

class FilmBlock 
{
public:
	FilmBlock(const int2& size, Filter* filter);
	~FilmBlock();

	inline void SetOffset(const int2& offset) { mOffset = offset; }
	inline const int2& GetOffset() const      { return mOffset; }

	inline void SetSize(const int2& size)   { mSize = size; }
	inline const int2& GetSize() const      { return mSize; }

	/// Return the border region used by the reconstruction filter
	inline int getBorderSize() const { return mBorderSize; }

	void AddSample(const Sample& sample, const ColorRGB& L);
	void AddBlock(const FilmBlock& block);

	/// Accumulate another image block into this one
	void put(const FilmBlock* block);

private:
	int2 mSize;
	int2 mOffset;
	int mBorderSize;

	/// Reconstructor filer
	float mLookupFactor;
	float mFilterRadius;
	float* mFilterTable;
	float* mWeightsX;
	float* mWeightsY;

	struct Pixel 
	{
		Pixel() : Color(0.0f, 0.0f, 0.0f), WeightSum(0.0f) { }
		Pixel& operator += (const Pixel& rhs) { Color += rhs.Color; WeightSum += rhs.WeightSum; }

		ColorRGB Color;
		float WeightSum;
	};

	RxLib::BlockedArray<Pixel, 2>* mPixels;
};

}


#endif // Film_h__
