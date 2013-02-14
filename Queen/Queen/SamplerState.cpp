#include <Graphics/SamplerState.h>

namespace RcEngine {

SamplerStateDesc::SamplerStateDesc()
	: AddressU(TAM_Wrap), AddressV(TAM_Wrap), AddressW(TAM_Wrap), 
	Filter(TF_Min_Mag_Mip_Point), BorderColor(0, 0, 0, 0),
	MipMapLODBias(0),MinLOD(FLT_MIN), MaxLOD( FLT_MAX),
	MaxAnisotropy(16), ComparisonFunc(CF_AlwaysFail), BindStage(ST_Pixel)

{

}

bool operator< (const SamplerStateDesc& lhs, const SamplerStateDesc& rhs)
{
	return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
}

}