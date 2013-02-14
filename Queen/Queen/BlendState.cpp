#include <Graphics/BlendState.h>

namespace RcEngine {

BlendStateDesc::BlendStateDesc()
	: AlphaToCoverageEnable(false), IndependentBlendEnable(false)
{

}

bool operator< (const BlendStateDesc& lhs, const BlendStateDesc& rhs)
{
	return std::memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
}


BlendStateDesc::RenderTargetBlendDesc::RenderTargetBlendDesc()
	: BlendEnable(false), SrcBlend(ABF_One), DestBlend(ABF_Zero), BlendOp(BOP_Add),
		SrcBlendAlpha(ABF_One), DestBlendAlpha(ABF_Zero), BlendOpAlpha(BOP_Add),
		ColorWriteMask(CWM_All)
{

}


}