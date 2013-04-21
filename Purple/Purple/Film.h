#ifndef Film_h__
#define Film_h__

#include "Prerequisites.h"
#include <BlockedArray.h>

namespace Purple {

using RxLib::int2;

class Filter;
class FilmBlock;

class Film
{
public:
	Film(const int2& size, Filter* filter);
	~Film();

	inline void SetSize(const int2& size)   { mSize = size; }
	inline const int2& GetSize() const      { return mSize; }

		/// Return reconstruction filter
	inline Filter* GetFilter() const { return mFilter; }

	inline FilmBlock* GetFilmBlock() const { return mMainBlock; }

	void AddBlock(const FilmBlock& block);
	void Clear();

	void WriteImage(const char* name);

protected:
	FilmBlock* mMainBlock;
	int2 mSize;
	Filter* mFilter;
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

	inline void Lock() { mMutex.lock(); }
	inline void Unlock() { mMutex.unlock(); }

	void AddSample(const Sample& sample, const ColorRGB& L);

	/// Accumulate another image block into this one
	void AddBlock(const FilmBlock& block);

	void FillBuffer(ColorRGB* buffer) const;
	void Clear();

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
		Pixel() : L(0.0f, 0.0f, 0.0f), WeightSum(0.0f) { }
		Pixel& operator += (const Pixel& rhs) { L += rhs.L; WeightSum += rhs.WeightSum; return *this; }

		ColorRGB L;
		float WeightSum;
	};

	std::mutex mMutex;

	RxLib::BlockedArray<Pixel, 2>* mPixels;

	// To do remove friend
	friend class Film;
};

/**
 *  This class can be used to chop up an image into many small
  * rectangular blocks suitable for parallel rendering. The blocks
 *  are ordered in spiraling pattern so that the center is
 *  rendered first.
 */
class BlockGenerator 
{
public:
	BlockGenerator(const int2& size, int blockSize);

	bool Next(FilmBlock& block);

protected:

	enum Direction { Right = 0, Down, Left, Up };

	int2 mBlock;
	int2 mNumBlocks;
	int2 mSize;
	int mBockSize;
	int mNumSteps;
	int mBlocksLeft;
	int mStepsLeft;
	int mDirection;
	
	clock_t mStartTime;

	std::mutex mMutex;
};



}


#endif // Film_h__
