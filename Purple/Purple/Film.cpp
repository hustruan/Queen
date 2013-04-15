#include "Film.h"
#include "Sampler.h"
#include "Filter.h"
#include "pfm.h"
#include <FloatCast.hpp>

#define FILTER_TABLE_SIZE  16

namespace Purple {

using namespace RxLib;

Film::Film( int xRes, int yRes ) : xResolution(xRes), yResolution(yRes)
{

}

Film::~Film()
{

}

//-------------------------------------------------------------------------------
ImageFilm::ImageFilm( int xRes, int yRes, Filter* filter )
	: Film(xRes, yRes), mFilter(filter)
{
	// Compute film image extent
	xPixelStart = 0;
	xPixelCount = xRes;
	yPixelStart = 0;
	yPixelCount = yRes;

	// Allocate film image storage
	pixels = new BlockedArray<Pixel, 2>(xPixelCount, yPixelCount);

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

	//mColorBuffer.resize(xPixelCount*yPixelCount*3);
}

ImageFilm::~ImageFilm()
{
	delete mFilter;
}

void ImageFilm::AddSample( const Sample& sample, const ColorRGB& L )
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
			//printf("(%d, %d)\n", x - xPixelStart, y - yPixelStart);
			Pixel& pixel = (*pixels)(x - xPixelStart, y - yPixelStart);
			
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

void ImageFilm::WriteImage( const char* filename )
{
	int nPix = xPixelCount * yPixelCount;
	float *rgb = new float[3*nPix];

	for (int y = 0; y < yPixelCount; ++y)
	{
		for (int x = 0; x < xPixelCount; ++x)
		{
			float* src = (*pixels)(x, y).Lrgb;

			int tmp = 3*((yPixelCount - y - 1) * xPixelCount + x);

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

	WritePfm(filename, xPixelCount, yPixelCount, 3, rgb);

	delete[] rgb;
}

//void ImageFilm::RefreshColor( Sampler* tileSampler )
//{
//	for (int y = tileSampler->PixelStartY; y < tileSampler->PixelEndY; ++y)
//	{
//		float* rgb = &mColorBuffer[tileSampler->PixelStartY * xPixelCount * 3];
//
//		for (int x = tileSampler->PixelStartX; x < tileSampler->PixelEndX; ++x)
//		{
//			float* src = (*pixels)(x, y).Lrgb;
//
//			int tmp = 3*((yPixelCount - y - 1) * xPixelCount + x);
//
//			rgb[tmp  ] = std::max(0.f, src[0]);
//			rgb[tmp+1] = std::max(0.f, src[1]);
//			rgb[tmp+2] = std::max(0.f, src[2]);
//
//			float weightSum = (*pixels)(x, y).weightSum;
//			if (weightSum != 0.f) 
//			{
//				float invWt = 1.f / weightSum;
//				rgb[tmp  ] = std::max(0.0f, rgb[tmp  ] * invWt);
//				rgb[tmp+1] = std::max(0.0f, rgb[tmp+1] * invWt);
//				rgb[tmp+2] = std::max(0.0f, rgb[tmp+2] * invWt);
//			}
//		}
//	}
//}



}