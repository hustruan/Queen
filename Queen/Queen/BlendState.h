#ifndef BlendState_h__
#define BlendState_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"

struct BlendStateDesc
{
public:
	struct RenderTargetBlendDesc
	{
		RenderTargetBlendDesc();
				
		bool				BlendEnable;
		AlphaBlendFactor    SrcBlend;
		AlphaBlendFactor    DestBlend;
		BlendOperation		BlendOp;
		AlphaBlendFactor    SrcBlendAlpha;
		AlphaBlendFactor    DestBlendAlpha;
		BlendOperation		BlendOpAlpha;
		uint8_t				ColorWriteMask;
	};

public:
	BlendStateDesc();
	
	bool AlphaToCoverageEnable;
	bool IndependentBlendEnable;
	RenderTargetBlendDesc RenderTarget[8];

	friend bool operator< (const BlendStateDesc& lhs, const BlendStateDesc& rhs);
};



class BlendState
{
public:
	BlendState(const BlendStateDesc& desc) 
		: mDesc(desc)
	{

	}

	const BlendStateDesc& GetDesc() const					{ return mDesc; }

protected:
	BlendStateDesc mDesc;
};

} // Namespace RcEngine

#endif // BlendState_h__
