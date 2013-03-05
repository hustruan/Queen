#ifndef Shader_h__
#define Shader_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"
#include "RenderStage.h"
#include <Vector.hpp>
#include <ColorRGBA.hpp>

#define MaxVSInput 16
#define MaxVSOutput 32
#define MaxPSOutput 8

using RxLib::float4;
using RxLib::ColorRGBA;

typedef float4 ShaderRegister;

struct VS_Input
{
	std::array<ShaderRegister, MaxVSOutput> ShaderInputs;
};

struct VS_Output
{
	float4 Position;
	std::array<ShaderRegister, MaxVSOutput> ShaderOutputs;
};

typedef VS_Output PS_Input;

struct PS_Output
{
	std::array<ColorRGBA, MaxPSOutput> Color;
	float Depth;
};

class VertexShaderStage;
class PixelShaderStage;

class Shader
{
public:
	Shader(void);
	virtual ~Shader(void);

	virtual void Bind();
	virtual void Unbind();

	virtual uint32_t GetOutputCount() const= 0;

protected:
	ColorRGBA Sample(uint32_t texUint, uint32_t samplerUnit, float U, float V);
	ColorRGBA Sample(uint32_t texUint, uint32_t samplerUnit, float U, float V, float W);

	VertexShaderStage* VertexShaderStage();
	PixelShaderStage* PixelShaderStage();

protected:
	RenderDevice* mDevice;
};

class VertexShader : public Shader
{
public:
	VertexShader();
	virtual ~VertexShader();

	virtual void Execute(const VS_Input* input, VS_Output* output) = 0;
};

class PixelShader : public Shader
{
public:
	PixelShader();
	virtual ~PixelShader();

	/**
	 * return false if discard current pixel
	 */
	virtual bool Execute(const VS_Output* input, PS_Output* output, float* pDepthIO) = 0;
};

class VertexShaderStage : public RenderStage
{
public:
	VertexShaderStage(RenderDevice& device);
	~VertexShaderStage();

	void SetVertexShader(const shared_ptr<VertexShader>& vs);
	const shared_ptr<VertexShader>& GetVertexShader() const { return mShader; } 

public:
	std::array<uint32_t, MaxVSOutput> InterpolationModifiers;
	uint32_t VSOutputCount;

private:
	shared_ptr<VertexShader> mShader;
};


class PixelShaderStage : public RenderStage
{
public:
	PixelShaderStage(RenderDevice& device);
	~PixelShaderStage();

	void SetPixelShader(const shared_ptr<PixelShader>& ps);
	const shared_ptr<PixelShader>& GetPixelShader() const { return mShader; } 

private:
	shared_ptr<PixelShader> mShader;
};


/**
 *
 */
#define DeclareVarying(modifier, type, name, slot)		 VertexShaderStage()->InterpolationModifiers[slot] = modifier;
#define DefineVarying(type, name, slot)					 type& name = output->ShaderOutputs[slot];
#define DefineAttribute(type, name, slot)				 const type& name = input->ShaderInputs[slot];

#define DefineTexture(unit, name)						 enum {name = unit };
#define DefineSampler(unit, name)						 enum {name = unit };

#endif // Shader_h__

