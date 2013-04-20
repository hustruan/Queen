#include "Film.h"
#include "Sampler.h"
#include "Filter.h"
#include "pfm.h"
#include <FloatCast.hpp>

#define FILTER_TABLE_SIZE  16

namespace Purple {

using namespace RxLib;

//-------------------------------------------------------------------------------
Film::Film( int xRes, int yRes, Filter* filter )
	: xResolution(xRes), yResolution(yRes), mFilter(filter)
{
	// Allocate film image storage
	pixels = new BlockedArray<Pixel, 2>(xResolution, yResolution);

	mFilterTable = new float[FILTER_TABLE_SIZE * FILTER_TABLE_SIZE];
	float *ftp = mFilterTable;
	for (int y = 0; y < FILTER_TABLE_SIZE; ++y)
	{
		float fy = ((float)y + 0.5f) * filter->yWidth / FILTER_TABLE_SIZE;
		for (int x = 0; x < FILTER_TABLE_SIZE; ++x) 
		{
			float fx = ((float)x + 0.5f) * filter->xWidth / FILTER_TABLE_SIZE;
			*ftp++ = filter->Evaluate(fx, fy);
		}
	}
}

Film::~Film()
{
	delete mFilter;
}

void Film::AddSample( const Sample& sample, const ColorRGB& L )
{
	// Compute sample's raster extent
	float dimageX = sample.ImageSample.X() - 0.5f;
	float dimageY = sample.ImageSample.Y() - 0.5f;
	int x0 = Ceil2Int (dimageX - mFilter->xWidth);
	int x1 = Floor2Int(dimageX + mFilter->xWidth);
	int y0 = Ceil2Int (dimageY - mFilter->yWidth);
	int y1 = Floor2Int(dimageY + mFilter->yWidth);
	
	x0 = std::max(x0, 0);
	x1 = std::min(x1, xResolution - 1);
	y0 = std::max(y0, 0);
	y1 = std::min(y1, yResolution - 1);

	if ((x1-x0) < 0 || (y1-y0) < 0)
	{
		return;
	}

	// Loop over filter support and add sample to pixel arrays

	// Precompute x and y filter table offsets
	int* ifx = (int* )_alloca(sizeof(int) * (x1 - x0 + 1) ); 
	for (int x = x0; x <= x1; ++x) 
	{
		float fx = fabsf((x - dimageX) * mFilter->invXWidth * FILTER_TABLE_SIZE);
		ifx[x-x0] = std::min(Floor2Int(fx), FILTER_TABLE_SIZE-1);
	}

	int *ify = (int* )_alloca(sizeof(int) * (y1 - y0 + 1) ); 
	for (int y = y0; y <= y1; ++y) 
	{
		float fy = fabsf((y - dimageY) * mFilter->invYWidth * FILTER_TABLE_SIZE);
		ify[y-y0] = std::min(Floor2Int(fy), FILTER_TABLE_SIZE-1);
	}


	bool syncNeeded = (mFilter->xWidth > 0.5f || mFilter->yWidth > 0.5f);
	for (int y = y0; y <= y1; ++y)
	{
		for (int x = x0; x <= x1; ++x)
		{
			// Evaluate filter value at(x,y) pixel
			int offset = ify[y-y0]*FILTER_TABLE_SIZE + ifx[x-x0];
			float filterWt = mFilterTable[offset];
	
			//// Update pixel values with filtered sample contribution
			Pixel& pixel = (*pixels)(x, y);
			
			pixel.Lrgb[0] += filterWt * L[0];
			pixel.Lrgb[1] += filterWt * L[1];
			pixel.Lrgb[2] += filterWt * L[2];
			pixel.weightSum += filterWt;

			//if (!syncNeeded) 
			//{
			//	pixel.Lrgb[0] += filterWt * L[0];
			//	pixel.Lrgb[1] += filterWt * L[1];
			//	pixel.Lrgb[2] += filterWt * L[2];
			//	pixel.weightSum += filterWt;
			//}
			//else
			//{
			//	// Safely update _Lxyz_ and _weightSum_ even with concurrency
			//	AtomicAdd(&pixel.Lrgb[0], filterWt * L[0]);
			//	AtomicAdd(&pixel.Lrgb[1], filterWt * L[1]);
			//	AtomicAdd(&pixel.Lrgb[2], filterWt * L[2]);
			//	AtomicAdd(&pixel.weightSum, filterWt);
			//}
		}
	}
}

void Film::WriteImage( const char* filename )
{
	int nPix = xResolution * yResolution;
	float *rgb = new float[3*nPix];

	for (int y = 0; y < yResolution; ++y)
	{
		for (int x = 0; x < xResolution; ++x)
		{
			float* src = (*pixels)(x, y).Lrgb;

			int tmp = 3*((yResolution - y - 1) * xResolution + x);

			rgb[tmp  ] = std::max(0.f, src[0]);
			rgb[tmp+1] = std::max(0.f, src[1]);
			rgb[tmp+2] = std::max(0.f, src[2]);

			float weightSum = (*pixels)(x, y).weightSum;
			if (weightSum != 0.f) 
			{
				float invWt = 1.f / weightSum;
				rgb[tmp  ] = std::max(0.0f, rgb[tmp  ] * invWt);
				rgb[tmp+1] = std::max(0.0f, rgb[tmp+1] * invWt);
				rgb[tmp+2] = std::max(0.0f, rgb[tmp+2] * invWt);
			}
		}
	}

	WritePfm(filename, xResolution, yResolution, 3, rgb);

	delete[] rgb;
}

FilmBlock::FilmBlock( const int2& size, Filter* filter )
	: mSize(size), mOffset(0, 0), mFilterRadius(filter->Radius)
{
	mBorderSize = (int)ceilf(mFilterRadius - 0.5f);

	mFilterTable = new float[FILTER_TABLE_SIZE + 1];

	for (int i=0; i < FILTER_TABLE_SIZE; ++i)
	{
		float pos = ((float)i + 0.5f) * mFilterRadius / FILTER_TABLE_SIZE;
		mFilterTable[i] = filter->Eval(pos);
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
	int x1 = std::min(mSize.X() - 1, Floor2Int(dimageX + mFilterRadius));
	int y1 = std::min(mSize.Y() - 1, Floor2Int(dimageY + mFilterRadius));

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
			pixel.Color += L * weight;
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

}