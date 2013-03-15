#include "SampleState.h"

#include <Math.hpp>
#include <Vector.hpp>

using namespace RxLib;

namespace {

template<uint32_t AddressModeType>
struct AddressModeMapper
{
	static float MapCoord(float coord) { return 0.0f; } ;
};
	
template<>
struct AddressModeMapper<TAM_Wrap>
{
	static float MapCoord(float coord)
	{
		return coord - floorf(coord);
	}
};

template<>
struct AddressModeMapper<TAM_Mirror>
{
	static float MapCoord(float coord)
	{
		float select = floorf(coord);
		return ((int)select & 1) ? (1.0f + select - coord) : (coord - select);
	}
};

template<>
struct AddressModeMapper<TAM_Clamp>
{
	static float MapCoord(float coord)
	{
		return RxLib::Clamp(coord, 0.0f, 1.0f);
	}
};


typedef float (*AddressFunc)(float coord);

AddressFunc gAddressFuncs[TAM_Count] =
{ 
	&AddressModeMapper<TAM_Wrap>::MapCoord, 
	&AddressModeMapper<TAM_Mirror>::MapCoord,
	&AddressModeMapper<TAM_Clamp>::MapCoord,
};


class PointSampler
{
public:

	template<typename Texel>
	static ColorRGBA Sample(const Texel& texel, int32_t width, int32_t height, float U, uint32_t addressU, float V, uint32_t addressV)
	{
		uint32_t texelX = gAddressFuncs[addressU](U, width) * (size - 1);
		uint32_t texelY = gAddressFuncs[addressV](V, height) * (size - 1);

		return texel(texelX, texelY); 
	}

};

class LinearSampler
{
public:
	template<typename Texel>
	static ColorRGBA Sample(const Texel& texel, int32_t width, int32_t height, float U, uint32_t addressU, float V, uint32_t addressV)
	{
		int32_t xLeft, xRight, yBottom, yTop;
		float uT, vT;

		TexelCoordCalculator(U, width, addressU, xLeft, xRight, uT);
		TexelCoordCalculator(V, height, addressV, yBottom, yTop, vT);

		auto c00 = texel(xLeft, yBottom);
		auto c01 = texel(xRight, yBottom);
		auto c10 = texel(xLeft, yTop);
		auto c11 = texel(xRight, yTop);
	
		return (1.0f - uT) * (1.0f - vT) * texel(xLeft, yBottom) + uT * (1.0f - vT) * texel(xRight, yBottom) +
		(1.0f - uT) * vT * texel(xLeft, yTop) + uT * vT * texel(xRight, yTop);
	}

private:

	static void TexelCoordCalculator(float value, int32_t size, uint32_t addressMode, int32_t& low, int32_t& up, float& decimal)
	{
		value = gAddressFuncs[addressMode](value)* size;
		
		low = (int32_t)floor(value);
		up = low + 1;
		decimal = value - low;
	}
};

}

ColorRGBA SamplerState::Sample( float u, float v, int32_t width, int32_t height, TexelFunc texel ) const
{
	return LinearSampler::Sample(texel, width, height, u, AddressU, v, AddressV);
}
