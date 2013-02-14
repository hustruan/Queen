#include "Shader.h"
#include "RenderDevice.h"
#include "Context.h"
#include "Texture.h"

//namespace {
//
//using namespace MathLib;
//
//template<int N>
//void VS_Output_Interpolate(VS_Output& out, const VS_Output& a, const VS_Output& b, float t)
//{
//	out.Position = Lerp(a.Position, b.Position, t);
//	for (int i = 0; i < N; ++i)
//	{
//		out.ShaderOutputs[i] = Lerp(a.ShaderOutputs[i], b.ShaderOutputs[i], t);
//	}
//}
//
//template<int N>
//void VS_Output_ProjectAttrib(VS_Output* out, float val)
//{
//	for (int i = 0; i < N; ++i)
//	{
//		out->ShaderOutputs[i] = out->ShaderOutputs[i] * val;
//	}
//}
//
//template<int N>
//void VS_Output_Mul(VS_Output* out, const VS_Output* in, float val)
//{
//	out->Position = in->Position * val;
//	for (int i = 0; i < N; ++i)
//	{
//		out->ShaderOutputs[i]  = in->ShaderOutputs[i] * val;
//	}
//}
//
//template<int N>
//void VS_Output_Sub(VS_Output* out, const VS_Output* a, const VS_Output* b)
//{
//	out->Position = a->Position - b->Position;
//	for (int i = 0; i < N; ++i)
//	{
//		out->ShaderOutputs[i] = a->ShaderOutputs[i] - b->ShaderOutputs[i];
//	}
//}
//
//}
//
//
//VertexShaderStage::InterpolateFunc gInterpolateFuncs[MaxVSOutput] = 
//{
//	VS_Output_Interpolate<0>,
//	VS_Output_Interpolate<1>,
//	VS_Output_Interpolate<2>,
//	VS_Output_Interpolate<3>,
//	VS_Output_Interpolate<4>,
//	VS_Output_Interpolate<5>,
//	VS_Output_Interpolate<6>,
//	VS_Output_Interpolate<7>,
//	VS_Output_Interpolate<8>,
//	VS_Output_Interpolate<9>,
//	VS_Output_Interpolate<10>,
//	VS_Output_Interpolate<11>,
//	VS_Output_Interpolate<12>,
//	VS_Output_Interpolate<13>,
//	VS_Output_Interpolate<14>,
//	VS_Output_Interpolate<15>,
//	VS_Output_Interpolate<16>,
//	VS_Output_Interpolate<17>,
//	VS_Output_Interpolate<18>,
//	VS_Output_Interpolate<19>,
//	VS_Output_Interpolate<20>,
//	VS_Output_Interpolate<21>,
//	VS_Output_Interpolate<22>,
//	VS_Output_Interpolate<23>,
//	VS_Output_Interpolate<24>,
//	VS_Output_Interpolate<25>,
//	VS_Output_Interpolate<26>,
//	VS_Output_Interpolate<27>,
//	VS_Output_Interpolate<28>,
//	VS_Output_Interpolate<29>,
//	VS_Output_Interpolate<30>,
//	VS_Output_Interpolate<31>,
//};
//
//VertexShaderStage::ProjectAttriFunc gProjectAttriFuncs[MaxVSOutput] = 
//{
//	VS_Output_ProjectAttrib<0>,
//	VS_Output_ProjectAttrib<1>,
//	VS_Output_ProjectAttrib<2>,
//	VS_Output_ProjectAttrib<3>,
//	VS_Output_ProjectAttrib<4>,
//	VS_Output_ProjectAttrib<5>,
//	VS_Output_ProjectAttrib<6>,
//	VS_Output_ProjectAttrib<7>,
//	VS_Output_ProjectAttrib<8>,
//	VS_Output_ProjectAttrib<9>,
//	VS_Output_ProjectAttrib<10>,
//	VS_Output_ProjectAttrib<11>,
//	VS_Output_ProjectAttrib<12>,
//	VS_Output_ProjectAttrib<13>,
//	VS_Output_ProjectAttrib<14>,
//	VS_Output_ProjectAttrib<15>,
//	VS_Output_ProjectAttrib<16>,
//	VS_Output_ProjectAttrib<17>,
//	VS_Output_ProjectAttrib<18>,
//	VS_Output_ProjectAttrib<19>,
//	VS_Output_ProjectAttrib<20>,
//	VS_Output_ProjectAttrib<21>,
//	VS_Output_ProjectAttrib<22>,
//	VS_Output_ProjectAttrib<23>,
//	VS_Output_ProjectAttrib<24>,
//	VS_Output_ProjectAttrib<25>,
//	VS_Output_ProjectAttrib<26>,
//	VS_Output_ProjectAttrib<27>,
//	VS_Output_ProjectAttrib<28>,
//	VS_Output_ProjectAttrib<29>,
//	VS_Output_ProjectAttrib<30>,
//	VS_Output_ProjectAttrib<31>,
//};
//
//VertexShaderStage::SubFunc gAttribSubFuncs[MaxVSOutput] = 
//{
//	VS_Output_Sub<0>,
//	VS_Output_Sub<1>,
//	VS_Output_Sub<2>,
//	VS_Output_Sub<3>,
//	VS_Output_Sub<4>,
//	VS_Output_Sub<5>,
//	VS_Output_Sub<6>,
//	VS_Output_Sub<7>,
//	VS_Output_Sub<8>,
//	VS_Output_Sub<9>,
//	VS_Output_Sub<10>,
//	VS_Output_Sub<11>,
//	VS_Output_Sub<12>,
//	VS_Output_Sub<13>,
//	VS_Output_Sub<14>,
//	VS_Output_Sub<15>,
//	VS_Output_Sub<16>,
//	VS_Output_Sub<17>,
//	VS_Output_Sub<18>,
//	VS_Output_Sub<19>,
//	VS_Output_Sub<20>,
//	VS_Output_Sub<21>,
//	VS_Output_Sub<22>,
//	VS_Output_Sub<23>,
//	VS_Output_Sub<24>,
//	VS_Output_Sub<25>,
//	VS_Output_Sub<26>,
//	VS_Output_Sub<27>,
//	VS_Output_Sub<28>,
//	VS_Output_Sub<29>,
//	VS_Output_Sub<30>,
//	VS_Output_Sub<31>,
//};
//
//VertexShaderStage::MulFunc gMulFuncs[MaxVSOutput] = 
//{
//	VS_Output_Mul<0>,
//	VS_Output_Mul<1>,
//	VS_Output_Mul<2>,
//	VS_Output_Mul<3>,
//	VS_Output_Mul<4>,
//	VS_Output_Mul<5>,
//	VS_Output_Mul<6>,
//	VS_Output_Mul<7>,
//	VS_Output_Mul<8>,
//	VS_Output_Mul<9>,
//	VS_Output_Mul<10>,
//	VS_Output_Mul<11>,
//	VS_Output_Mul<12>,
//	VS_Output_Mul<13>,
//	VS_Output_Mul<14>,
//	VS_Output_Mul<15>,
//	VS_Output_Mul<16>,
//	VS_Output_Mul<17>,
//	VS_Output_Mul<18>,
//	VS_Output_Mul<19>,
//	VS_Output_Mul<20>,
//	VS_Output_Mul<21>,
//	VS_Output_Mul<22>,
//	VS_Output_Mul<23>,
//	VS_Output_Mul<24>,
//	VS_Output_Mul<25>,
//	VS_Output_Mul<26>,
//	VS_Output_Mul<27>,
//	VS_Output_Mul<28>,
//	VS_Output_Mul<29>,
//	VS_Output_Mul<30>,
//	VS_Output_Mul<31>,
//};

namespace {

//struct Warp
//{
//	static int CalcCoord(float coord, uint32_t size)
//	{
//		return (coord- floorf(coord)) * size;
//	}
//};
//
//struct Mirror 
//{
//	static int CalcCoord(float coord, uint32_t size)
//	{
//		float select = floorf(coord);
//		float result = ((int)select & 1) ? (1.0f + select - coord) : (coord - select);
//		return result * size;
//	}
//};

struct Clamp
{
	//static int CalcCoord(float coord, uint32_t size)
	//{
	//	
	//}
};

}

Shader::Shader(void)
{
	mDevice = Context::GetSingleton().GetRenderDevicePtr();
}


Shader::~Shader(void)
{
}

ColorRGBA Shader::Sample( uint32_t texUint, uint32_t samplerUnit, float U, float V )
{
	ASSERT(mDevice->TextureUnit[texUint]->GetTextureType() == TT_Texture2D);

	const shared_ptr<Texture>& texture = mDevice->TextureUnit[texUint];
	const SamplerState& samplerState = mDevice->Samplers[samplerUnit];

	const uint32_t width = texture->GetWidth(0);
	const uint32_t height = texture->GetHeight(0);



	if(samplerState.Filter == TF_Min_Mag_Mip_Point)
	{

	}


	return ColorRGBA();
}

ColorRGBA Shader::Sample( uint32_t texUint, uint32_t samplerUnit, float U, float V, float W )
{
	return ColorRGBA();
}


//VertexShader::VertexShader()
//{
//
//}
//
//VertexShader::~VertexShader()
//{
//
//}


VertexShaderStage::VertexShaderStage( RenderDevice& device )
	: RenderStage(device)
{

}

void VertexShaderStage::SetVertexShader( const shared_ptr<VertexShader>& vs )
{
	uint32_t numOutput = vs->GetOutputCount();
//	mInterpolateFunc = gInterpolateFuncs[numOutput];
	//mProjectAttriFunc = gProjectAttriFuncs[numOutput];
	mShader = vs;
}

VertexShaderStage::~VertexShaderStage()
{

}

PixelShaderStage::PixelShaderStage( RenderDevice& device )
	: RenderStage(device)
{
}

PixelShaderStage::~PixelShaderStage()
{

}

void PixelShaderStage::SetPixelShader( const shared_ptr<PixelShader>& ps )
{
	uint32_t numOutput = ps->GetOutputCount();
	mShader = ps;
}