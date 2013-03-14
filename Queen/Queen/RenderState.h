#ifndef RenderState_h__
#define RenderState_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"
#include <ColorRGBA.hpp>

struct RasterizerState
{
	RasterizerState()
		: PolygonFillMode(FM_Solid),
		  PolygonCullMode(CM_Back),
		  FrontCounterClockwise(false),
		  DepthBias(0.0f), 
		  SlopeScaledDepthBias(0.0f),
		  DepthClipEnable(true),
		  ScissorEnable(false), 
		  MultisampleEnable(false)
	{

	}

	FillMode		PolygonFillMode;
	CullMode		PolygonCullMode;
	bool            FrontCounterClockwise;
	float           DepthBias;
	float           SlopeScaledDepthBias;
	bool            DepthClipEnable;
	bool            ScissorEnable;
	bool            MultisampleEnable;
};


struct DepthStencilState
{
	DepthStencilState()
		: DepthEnable(true), 
		  DepthWriteMask(true),
		  DepthFunc(CF_Less),
		  StencilEnable(true),
		  StencilReadMask(0xFFFF),
		  StencilWriteMask(0xFFFF), 
		  FrontStencilFailOp(SOP_Keep), 
		  FrontStencilDepthFailOp(SOP_Keep),
		  FrontStencilPassOp(SOP_Keep), 
		  FrontStencilFunc(CF_AlwaysPass),
		  BackStencilFailOp(SOP_Keep), 
		  BackStencilDepthFailOp(SOP_Keep),
		  BackStencilPassOp(SOP_Keep),
		  BackStencilFunc(CF_AlwaysPass)
	{

	}


	bool                       DepthEnable;
	bool					   DepthWriteMask;
	CompareFunction		       DepthFunc;

	bool                       StencilEnable;
	uint16_t                   StencilReadMask;
	uint16_t                   StencilWriteMask;

	StencilOperation		   FrontStencilFailOp;
	StencilOperation		   FrontStencilDepthFailOp;
	StencilOperation		   FrontStencilPassOp;
	CompareFunction			   FrontStencilFunc;

	StencilOperation		   BackStencilFailOp;
	StencilOperation		   BackStencilDepthFailOp;
	StencilOperation		   BackStencilPassOp;
	CompareFunction			   BackStencilFunc;
};

struct BlendState
{
public:
	struct RenderTargetBlend
	{
		RenderTargetBlend()
			: BlendEnable(false),
			  SrcBlend(ABF_One),
			  DestBlend(ABF_Zero),
			  BlendOp(BOP_Add),
			  SrcBlendAlpha(ABF_One),
			  DestBlendAlpha(ABF_Zero), 
			  BlendOpAlpha(BOP_Add),
			  ColorWriteMask(CWM_All)
		{

		}

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
	BlendState() 
		: AlphaToCoverageEnable(false), IndependentBlendEnable(false)
	{ }

	bool AlphaToCoverageEnable;
	bool IndependentBlendEnable;
	RenderTargetBlend RenderTarget[8];
};

#endif // RenderState_h__
