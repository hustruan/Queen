#include "MipMap.h"

namespace {

// Some helpful functions

inline int Modulo(int a, int b)
{
	int n = int(a/b);
	a -= n*b;
	if (a < 0) a += b;
	return a;
}

}

namespace Purple {

template TMipMap<ColorRGB>;

template<typename T>
TMipMap<T>::TMipMap( int width, int height, int levels, const T* data )
{
	mMipMaps = (std::max)(1, levels);

	mWidths.resize(mMipMaps);
	mHeights.resize(mMipMaps);
	{
		// store every level width and height
		int w = width;
		int h = height;
		for(int level = 0; level < mMipMaps; level++)
		{
			mWidths[level] = w;
			mHeights[level] = h;
			w = std::max(1, w / 2);
			h = std::max(1, h / 2);
		}
	} 

	mPyramid.resize(mMipMaps);

	int levelDataOffset = 0;
	for (int level = 0; level < mMipMaps; ++ level)
	{
		int levelWidth = mWidths[level];
		int levelHeight = mHeights[level];
		int imageSize = levelWidth * levelHeight;

		mPyramid[level].resize(imageSize);
		memcpy(&mPyramid[0][0], data+levelDataOffset, imageSize*sizeof(T));
		levelDataOffset+= imageSize;
	}
}


template<typename T>
TMipMap<T>::~TMipMap( void )
{

}


template<typename T>
T TMipMap<T>::Texel( const SampleState& sampleState, int level, int x, int y ) const
{
	int levelWidth = mWidths[level];
	int levelHeight = mHeights[level];

	if (x < 0 || x >= levelWidth) 
	{
		/* Encountered an out of bounds access -- determine what to do */
		switch (sampleState.AddressU)
		{
		case TAM_Wrap:
			// Assume that the input repeats in a periodic fashion
			x = Modulo(x, levelWidth);
			break;
		case TAM_Clamp:
			// Clamp to the outermost sample position
			x = RxLib::Clamp(x, 0, levelWidth - 1);
			break;
		case TAM_Mirror:
			// Assume that the input is mirrored along the boundary
			x = Modulo(x, 2*levelWidth);
			if (x >= levelWidth)
				x = 2*levelWidth - x - 1;
			break;
		case TAM_Zero:
			// Assume that the input function is zero
			// outside of the defined domain
			return T(0.0f);
		case TAM_One:
			// Assume that the input function is equal
			// to one outside of the defined domain
			return T(1.0f);
		}
	}

	if (y < 0 || y >= levelHeight) 
	{
		/* Encountered an out of bounds access -- determine what to do */
		switch (sampleState.AddressU)
		{
		case TAM_Wrap:
			// Assume that the input repeats in a periodic fashion
			y = Modulo(y, levelHeight);
			break;
		case TAM_Clamp:
			// Clamp to the outermost sample position
			y = RxLib::Clamp(y, 0, levelHeight - 1);
			break;
		case TAM_Mirror:
			// Assume that the input is mirrored along the boundary
			y = Modulo(y, 2*levelHeight);
			if (y >= levelHeight)
				y = 2*levelHeight - y - 1;
			break;
		case TAM_Zero:
			// Assume that the input function is zero
			// outside of the defined domain
			return T(0.0f);
		case TAM_One:
			// Assume that the input function is equal
			// to one outside of the defined domain
			return T(1.0f);
		}
	}

	return mPyramid[level][y*levelWidth+x];
}



}
