#ifndef SampleState_h__
#define SampleState_h__

#include <ColorRGBA.hpp>
#include <functional>
#include "GraphicCommon.h"


struct SamplerState
{
public:
	typedef std::function<RxLib::ColorRGBA(int32_t, int32_t)> TexelFunc;
	typedef RxLib::ColorRGBA (*SampleFunc)( TexelFunc texel, float u, float v, int32_t width, int32_t height );

public:
	SamplerState()
		: AddressU(TAM_Wrap), AddressV(TAM_Wrap), AddressW(TAM_Wrap), 
		Filter(TF_Min_Mag_Mip_Point), BorderColor(0, 0, 0, 0),
		MipMapLODBias(0),MinLOD(FLT_MIN), MaxLOD( FLT_MAX),
		MaxAnisotropy(16), ComparisonFunc(CF_AlwaysFail), BindStage(ST_Pixel)
	{

	}


	RxLib::ColorRGBA Sample( float u, float v, int32_t width, int32_t height, TexelFunc texel ) const;

public:
	ShaderType				   BindStage;

	TextureFilter              Filter;
	TextureAddressMode		   AddressU;
	TextureAddressMode		   AddressV;
	TextureAddressMode		   AddressW;
	float                      MipMapLODBias;
	uint8_t                    MaxAnisotropy;
	CompareFunction            ComparisonFunc;
	float                      MinLOD;
	float                      MaxLOD;

	RxLib::ColorRGBA				   BorderColor;	
};



#endif // SampleState_h__
