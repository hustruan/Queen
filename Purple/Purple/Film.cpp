#include "Film.h"
#include "Sampler.h"
#include "Filter.h"
#include <FloatCast.hpp>

namespace Purple {

using namespace RxLib;

ImageFilm::ImageFilm( int xRes, int yRes, Filter* filter )
	: Film(xRes, yRes, filter)
{

}

ImageFilm::~ImageFilm()
{

}

void ImageFilm::AddSample( const Sample& sample, const ColorRGB& L )
{
	// Compute sample's raster extent
	float dimageX = sample.ImageSample.X() - 0.5f;
	float dimageY = sample.ImageSample.Y() - 0.5f;
	int x0 = Ceil2Int (dimageX - filter->xWidth);
	int x1 = Floor2Int(dimageX + filter->xWidth);
	int y0 = Ceil2Int (dimageY - filter->yWidth);
	int y1 = Floor2Int(dimageY + filter->yWidth);
	
	x0 = std::max(x0, 0);
	x1 = std::min(x1, xResolution - 1);
	y0 = std::max(y0, 0);
	y1 = std::min(y1, yResolution - 1);

	if ((x1-x0) < 0 || (y1-y0) < 0)
	{
		return;
	}

	// Loop over filter support and add sample to pixel arrays
	float xyz[3];
	L.ToXYZ(xyz);

	// Precompute $x$ and $y$ filter table offsets
	int *ifx = ALLOCA(int, x1 - x0 + 1);
	for (int x = x0; x <= x1; ++x) {
		float fx = fabsf((x - dimageX) *
			filter->invXWidth * FILTER_TABLE_SIZE);
		ifx[x-x0] = min(Floor2Int(fx), FILTER_TABLE_SIZE-1);
	}
	int *ify = ALLOCA(int, y1 - y0 + 1);
	for (int y = y0; y <= y1; ++y) {
		float fy = fabsf((y - dimageY) *
			filter->invYWidth * FILTER_TABLE_SIZE);
		ify[y-y0] = min(Floor2Int(fy), FILTER_TABLE_SIZE-1);
	}
	bool syncNeeded = (filter->xWidth > 0.5f || filter->yWidth > 0.5f);
	for (int y = y0; y <= y1; ++y) {
		for (int x = x0; x <= x1; ++x) {
			// Evaluate filter value at $(x,y)$ pixel
			int offset = ify[y-y0]*FILTER_TABLE_SIZE + ifx[x-x0];
			float filterWt = filterTable[offset];

			// Update pixel values with filtered sample contribution
			Pixel &pixel = (*pixels)(x - xPixelStart, y - yPixelStart);
			if (!syncNeeded) {
				pixel.Lxyz[0] += filterWt * xyz[0];
				pixel.Lxyz[1] += filterWt * xyz[1];
				pixel.Lxyz[2] += filterWt * xyz[2];
				pixel.weightSum += filterWt;
			}
			else {
				// Safely update _Lxyz_ and _weightSum_ even with concurrency
				AtomicAdd(&pixel.Lxyz[0], filterWt * xyz[0]);
				AtomicAdd(&pixel.Lxyz[1], filterWt * xyz[1]);
				AtomicAdd(&pixel.Lxyz[2], filterWt * xyz[2]);
				AtomicAdd(&pixel.weightSum, filterWt);
			}
		}
	}
}



}