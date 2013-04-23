#ifndef MipMap_h__
#define MipMap_h__

#include "Prerequisites.h"

namespace Purple {

enum MIPFilterType
{
	/// No filtering (i.e. nearest neighbor lookups)
	MFT_ENearest = 0,
	/// Basic trilinear filtering
	MFT_ETrilinear = 1,
	/// Elliptically weighted average
	MFT_EEWA = 2,
};


template<typename T>
class TMipMap
{
public:
	typedef std::vector< std::vector<T> > Array2DType;

public:
	TMipMap(int width, int height, int levels, const T* data);
	~TMipMap(void);

private:
	Array2DType mPyramid;
	std::vector<int> mWidths, mHeights;
	int mMipMaps;
};

}

#endif // MipMap_h__

