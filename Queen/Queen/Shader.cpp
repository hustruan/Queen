#include "Shader.h"
#include "RenderDevice.h"
#include "Context.h"
#include "Texture.h"
#include <Math.hpp>

namespace {

using namespace RxLib;

struct Warp
{
	static int CalcCoord(float coord, uint32_t size)
	{
		return (coord- floorf(coord)) * size;
	}
};

struct Mirror 
{
	static int CalcCoord(float coord, uint32_t size)
	{
		float select = floorf(coord);
		float result = ((int)select & 1) ? (1.0f + select - coord) : (coord - select);
		return result * size;
	}
};

struct Clamp
{
	static int CalcCoord(float coord, uint32_t size)
	{
		return RxLib::Clamp(coord, 0.0f, 1.0f) * size;
	}
};

}

Shader::Shader(void)
{
	mDevice = Context::GetSingleton().GetRenderDevicePtr();
}


Shader::~Shader(void)
{
}

void Shader::Bind()
{

}

void Shader::Unbind()
{

}

float4 Shader::Sample( uint32_t texUint, uint32_t samplerUnit, float U, float V )
{
	ASSERT(mDevice->TextureUnit[texUint]->GetTextureType() == TT_Texture2D);

	const shared_ptr<Texture>& texture = mDevice->TextureUnit[texUint];
	const SamplerState& samplerState = mDevice->Samplers[samplerUnit];

	const uint32_t width = texture->GetWidth(0);
	const uint32_t height = texture->GetHeight(0);

	if(samplerState.Filter == TF_Min_Mag_Mip_Point)
	{
	}


	return float4();
}

float4 Shader::Sample( uint32_t texUint, uint32_t samplerUnit, float U, float V, float W )
{
	return float4();
}

VertexShaderStage* Shader::VertexShaderStage()
{
	return mDevice->mVertexShaderStage;
}

PixelShaderStage* Shader::PixelShaderStage()
{
	return mDevice->mPixelShaderStage;
}


VertexShader::VertexShader() 
{

}

VertexShader::~VertexShader()
{

}

PixelShader::PixelShader()
{

}

PixelShader::~PixelShader()
{

}

VertexShaderStage::VertexShaderStage( RenderDevice& device )
	: RenderStage(device)
{

}

void VertexShaderStage::SetVertexShader( const shared_ptr<VertexShader>& vs )
{
	mShader = vs;
	
	mShader->Bind();
	VSOutputCount = mShader->GetOutputCount();
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




