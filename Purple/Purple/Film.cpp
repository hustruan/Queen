#include "Film.h"
#include "Sampler.h"
#include "Filter.h"
#include "pfm.h"
#include <FloatCast.hpp>

#define FILTER_TABLE_SIZE  16

namespace Purple {

using namespace RxLib;


Film::Film( const int2& size, Filter* filter )
	: mSize(size), mFilter(filter)
{
	mMainBlock = new FilmBlock(size, filter);
}

Film::~Film()
{
	delete mFilter;
	delete mMainBlock;
}

void Film::AddBlock( const FilmBlock& block )
{
	mMainBlock->AddBlock(block);
}

void Film::Clear()
{
	mMainBlock->Clear();
}

void Film::WriteImage( const char* filename )
{
	const int2& size = mMainBlock->GetSize();

	int nPix = size.Y() * size.X();
	float *rgb = new float[3*nPix];

	for (int y = 0; y < size.Y(); ++y)
	{
		for (int x = 0; x < size.X(); ++x)
		{
			int filmY = y + mMainBlock->getBorderSize();
			int filmX = x + mMainBlock->getBorderSize();

			const ColorRGB& L = (*(mMainBlock->mPixels))(filmX, filmY).L;
			float weightSum = (*(mMainBlock->mPixels))(filmX, filmY).WeightSum;

			int tmp = 3 * ((size.Y() - y - 1) * size.X() + x);

			rgb[tmp  ] = std::max(0.f, L[0]);
			rgb[tmp+1] = std::max(0.f, L[1]);
			rgb[tmp+2] = std::max(0.f, L[2]);
	
			if (weightSum != 0.f) 
			{
				float invWt = 1.f / weightSum;
				rgb[tmp  ] *= invWt;
				rgb[tmp+1] *= invWt;
				rgb[tmp+2] *= invWt;
			}
		}
	}

	WritePfm(filename, size.X(), size.Y(), 3, rgb);

	delete[] rgb;
}

//-----------------------------------------------------------------------------
FilmBlock::FilmBlock( const int2& size, Filter* filter )
	: mSize(size), mOffset(0, 0), mFilterRadius(filter->Radius)
{
	mBorderSize = (int)ceilf(mFilterRadius - 0.5f);

	mFilterTable = new float[FILTER_TABLE_SIZE + 1];

	for (int i=0; i < FILTER_TABLE_SIZE; ++i)
	{
		float pos = ((float)i + 0.5f) * mFilterRadius / FILTER_TABLE_SIZE;
		mFilterTable[i] = filter->Eval(pos);
		//printf("%f\n", mFilterTable[i]);
	}
	mFilterTable[FILTER_TABLE_SIZE] = 0.0f;
	mLookupFactor = FILTER_TABLE_SIZE / mFilterRadius;

	mWeightsX = new float[(int) std::ceil(2*mFilterRadius) + 1];
	mWeightsY = new float[(int) std::ceil(2*mFilterRadius) + 1];

	mPixels = new BlockedArray<Pixel, 2>(mSize.X() + 2*mBorderSize, mSize.Y() + 2*mBorderSize);

}

FilmBlock::~FilmBlock()
{
	delete[] mFilterTable;
	delete[] mWeightsX;
	delete[] mWeightsY;
	delete mPixels;
}

void FilmBlock::AddSample( const Sample& sample, const ColorRGB& L )
{
	/* Convert to pixel coordinates within the film block */
	float dimageX = sample.ImageSample.X() - 0.5f - (mOffset.X() - mBorderSize);
	float dimageY = sample.ImageSample.Y() - 0.5f - (mOffset.Y() - mBorderSize);
	
	int x0 = std::max(0, Ceil2Int (dimageX - mFilterRadius));
	int y0 = std::max(0, Ceil2Int (dimageY - mFilterRadius));
	int x1 = std::min(mSize.X() + 2*mBorderSize - 1, Floor2Int(dimageX + mFilterRadius));
	int y1 = std::min(mSize.Y() + 2*mBorderSize - 1, Floor2Int(dimageY + mFilterRadius));

	for (int x = x0, idx = 0; x<= x1; ++x)
		mWeightsX[idx++] = mFilterTable[int(fabsf(x-dimageX) * mLookupFactor)];
	for (int y= y0, idx = 0; y<= y1; ++y)
		mWeightsY[idx++] = mFilterTable[int(fabsf(y-dimageY) * mLookupFactor)];

	float weight = 0.0f;
	for (int y= y0, yr = 0; y<= y1; ++y, ++yr)
	{
		for (int x = x0, xr = 0; x<= x1; ++x, ++xr)
		{
			weight = mWeightsX[xr] * mWeightsY[yr];

			Pixel& pixel = (*mPixels)(x, y);		
			pixel.L += L * weight;
			pixel.WeightSum += weight;
		}
	}
}

void FilmBlock::AddBlock( const FilmBlock& block )
{
	int2 offset = (block.GetOffset() - mOffset)/* - int2(block.getBorderSize() - mBorderSize)*/;
	int2 size = block.GetSize() + int2(2*block.getBorderSize());
	
	for (int y= 0; y < size.Y(); ++y)
	{
		for (int x = 0; x < size.X(); ++x)
		{
			(*mPixels)(x+offset.X(), y+offset.Y()) += (*block.mPixels)(x, y);
		}
	}
}

void FilmBlock::Clear()
{
	int32_t xSize = mPixels->uSize();
	int32_t ySize = mPixels->vSize();

	for (int32_t y= 0; y < ySize; ++y)
	{
		for (int32_t x = 0; x < xSize; ++x)
		{
			(*mPixels)(x, y) = Pixel();
		}
	}
}

inline float ToSRGB(float value)
{
	if (value < 0.0031308f)
		return 12.92f * value;

	return 1.055f * powf(value, 0.41666f) - 0.055f;
}

void FilmBlock::FillBuffer( ColorRGB* buffer ) const
{
	for (int y = 0; y < mSize.Y(); ++y)
	{
		for (int x = 0; x < mSize.X(); ++x)
		{
			int filmY = y + mBorderSize;
			int filmX = x + mBorderSize;

			const ColorRGB& L = (*mPixels)(filmX, filmY).L;
			float weightSum = (*mPixels)(filmX, filmY).WeightSum;

			int tmp =(/*(mSize.Y() - y - 1)*/ y * mSize.X() + x); 

			buffer[tmp] = L;

			if (weightSum != 0.f) 
			{
				float invWt = 1.f / weightSum;
				buffer[tmp] *= invWt;
			}

			buffer[tmp] =  ColorRGB(ToSRGB(buffer[tmp].R), ToSRGB(buffer[tmp].G), ToSRGB(buffer[tmp].B));
		}
	}
}

//---------------------------------------------------------------------------------------------------------
BlockGenerator::BlockGenerator( const int2& size, int blockSize ) : mSize(size), mBockSize(blockSize)
{
	mNumBlocks = int2(
		(int)std::ceil(size.X() / (float) blockSize),
		(int)std::ceil(size.Y() / (float) blockSize));

	mBlocksLeft = mNumBlocks.X() * mNumBlocks.Y();

	mDirection = Right;

	mBlock = mNumBlocks / 2;

	mStepsLeft = 1;
	mNumSteps = 1;

	mStartTime = clock();
}

bool BlockGenerator::Next( FilmBlock& block )
{
	mMutex.lock();

	if (mBlocksLeft == 0)
	{
		mMutex.unlock();
		return false;
	}

	int2 pos = mBlock * mBockSize;
	block.SetOffset(pos);

	int2 left = mSize - pos;
	block.SetSize(int2(std::min(left.X(), mBockSize), std::min(left.Y(), mBockSize)));

	if (--mBlocksLeft == 0)
	{
		double elapsed = (double)(clock() - mStartTime) / CLOCKS_PER_SEC / 60.0;
		std::cout << "Rendering finished (took " << elapsed << " minutes)\n";
		mMutex.unlock();
		return true;
	}

	do 
	{
		switch (mDirection) 
		{
		case Right: ++mBlock.X(); break;
		case Down:  ++mBlock.Y(); break;
		case Left:  --mBlock.X(); break;
		case Up:    --mBlock.Y(); break;
		}

		if (--mStepsLeft == 0)
		{
			mDirection = (mDirection + 1) % 4;
			if (mDirection == Left || mDirection == Right) 
				++mNumSteps;
			mStepsLeft = mNumSteps;
		}

	} while (mBlock.X() < 0 || mBlock.Y() < 0 ||
		mBlock.X() >= mNumBlocks.X() || mBlock.Y() >= mNumBlocks.Y());

	printf("Total: %f%%\n", (1.0f - float(mBlocksLeft) / (mNumBlocks.X() * mNumBlocks.Y())) * 100);

	mMutex.unlock();
	return true;
}


}