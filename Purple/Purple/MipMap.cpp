#include "MipMap.h"

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
		memcpy(&mPyramid[0], data+levelDataOffset, imageSize);
		levelDataOffset+= imageSize;
	}
}


template<typename T>
TMipMap<T>::~TMipMap( void )
{

}


}
