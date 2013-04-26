#ifndef MipMap_h__
#define MipMap_h__

#include "Prerequisites.h"
#include <FloatCast.hpp>

namespace Purple {

enum MIPFilterType
{
	/// No filtering (i.e. nearest neighbor lookups)
	MFT_Nearest = 0,
	/// Basic trilinear filtering
	MFT_Trilinear = 1,
	/// Elliptically weighted average
	MFT_EWA = 2,
};

enum TextureAddressMode
{
	TAM_Wrap = 0,
	TAM_Mirror,
	TAM_Clamp,	
	TAM_Zero,
	TAM_One,
	TAM_Count,
};


struct SampleState
{
	MIPFilterType              Filter;
	TextureAddressMode		   AddressU;
	TextureAddressMode		   AddressV;
	TextureAddressMode		   AddressW;
	float                      MaxAnisotropy;
};


template<typename T>
class TMipMap
{
public:
	typedef std::vector< std::vector<T> > Array2DType;

public:
	TMipMap(int width, int height, int levels, const T* data);
	~TMipMap(void);

	inline int GetWidth(int level) const  { return mWidths[level]; }
	inline int GetHeight(int level) const { return mHeights[level]; }
	inline int GetLevels() const          { return mMipMaps; }

	inline T Eval(const SampleState& sampleState, const float2& uv, const float2& d0, const float2& d1) const
	{
		//if (sampleState.Filter == MFT_Nearest)
			return EvalBox(sampleState, 0, uv);

		// eval first 
		// todo add mipmap
		//return EvalBilinear(sampleState, 0, uv);
	}


private:
	inline T EvalBox(const SampleState& sampleState, int level, const float2& uv) const 
	{
		int levelWidth = mWidths[level];
		int levelHeight = mHeights[level];
		return Texel(sampleState, level, Floor2Int(uv.X()*levelWidth), Floor2Int(uv.Y()*levelHeight));
	}

	inline T EvalBilinear(const SampleState& sampleState, int level, const float2& uv) const 
	{
		assert(!uv.HasNaNs() && "evalBilinear(): encountered a NaN!" );

		if (level >= mMipMaps)
		{
			/* The lookup is larger than the entire texture */
			return EvalBox(sampleState, mMipMaps-1, uv);
		}

		/* Convert to fractional pixel coordinates on the specified level */
		int levelWidth = mWidths[level];
		int levelHeight = mHeights[level];

		float u = uv.X() * levelWidth - 0.5f, v = uv.Y() * levelHeight - 0.5f;

		int xPos = Floor2Int(u), yPos = Floor2Int(v);
		float dx1 = u - xPos, dx2 = 1.0f - dx1,
			dy1 = v - yPos, dy2 = 1.0f - dy1;

		return Texel(sampleState, level, xPos, yPos) * dx2 * dy2
			+ Texel(sampleState, level, xPos, yPos + 1) * dx2 * dy1
			+ Texel(sampleState, level, xPos + 1, yPos) * dx1 * dy2
			+ Texel(sampleState, level, xPos + 1, yPos + 1) * dx1 * dy1;
	}

	T Texel(const SampleState& sampleState, int level, int x, int y) const;

private:
	Array2DType mPyramid;
	std::vector<int> mWidths, mHeights;
	int mMipMaps;
};

}

#endif // MipMap_h__

