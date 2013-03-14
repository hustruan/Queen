#include "Shader.h"
#include "RenderDevice.h"
#include "Context.h"
#include "Texture.h"


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

ColorRGBA Shader::Sample( uint32_t texUint, uint32_t samplerUnit, float U, float V )
{
	return mDevice->Sample(texUint, samplerUnit, U, V, 0.0f);
}

ColorRGBA Shader::Sample( uint32_t texUint, uint32_t samplerUnit, float U, float V, float W )
{
	return mDevice->Sample(texUint, samplerUnit, U, V, W);
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




