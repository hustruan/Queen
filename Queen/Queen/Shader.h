#ifndef Shader_h__
#define Shader_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"
#include "RenderStage.h"
#include "MathLib/Vector.hpp"
#include "MathLib/ColorRGBA.hpp"

#define MaxVSInput 16
#define MaxVSOutput 32
#define MaxPSOutput 8

using MathLib::float4;
using MathLib::ColorRGBA;

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

class Shader
{
public:
	Shader(void);
	virtual ~Shader(void);

	virtual uint32_t GetOutputCount() const= 0;

protected:
	ColorRGBA Sample(uint32_t texUint, uint32_t samplerUnit, float U, float V);
	ColorRGBA Sample(uint32_t texUint, uint32_t samplerUnit, float U, float V, float W);

private:
	RenderDevice* mDevice;
};

#define Varying(type, name, slot) type& name = output->ShaderOutputs[slot];
#define DefineTexture(unit, name) enum {name = unit };
#define DefineSampler(unit, name) enum {name = unit };

template<int N>
struct ShaderOutputCounter
{
	enum { Count = ShaderOutputCounter<N-1>::Count + 1; };
};

template<>
struct ShaderOutputCounter<0> {
public:
	enum { Count = 1 };
};

class VertexShader : public Shader
{
public:
	VertexShader()
		: mNumOutput(0)
	{

	}

	//virtual ~VertexShader();

	virtual void Execute(const VS_Input* input, VS_Output* output) = 0;

protected:
	uint32_t mNumOutput;
};

class PixelShader : public Shader
{
public:
	//PixelShader();
	//virtual ~PixelShader();

	/**
	 * return false if discard current pixel
	 */
	virtual bool Execute(const VS_Output* input, PS_Output* output, float* pDepthIO) = 0;

};

class VertexShaderStage : public RenderStage
{
//public:
//	typedef std::function<void(VS_Output&, const VS_Output&, const VS_Output&, float t)> InterpolateFunc;
//	typedef std::function<void(VS_Output*, float invW)> ProjectAttriFunc;
//	typedef std::function<void(VS_Output*, const VS_Output*, const VS_Output*)> SubFunc;
//	typedef std::function<void(VS_Output*, const VS_Output*, float invW)> MulFunc;
public:
	VertexShaderStage(RenderDevice& device);
	~VertexShaderStage();


	void SetVertexShader(const shared_ptr<VertexShader>& vs);
	const shared_ptr<VertexShader>& GetVertexShader() const { return mShader; } 

	//const InterpolateFunc& GetInterpolateFunc() const  { return mInterpolateFunc; }	
	//const ProjectAttriFunc& GetProjectAttriFunc() const  { return mProjectAttriFunc; }	
	//const SubFunc& GetSubFunc() const { return mAttribSubFunc; }
	//const MulFunc& GetMulFunc() const { return mMulFunc; }

private:
	shared_ptr<VertexShader> mShader;
	//InterpolateFunc mInterpolateFunc;
	//ProjectAttriFunc mProjectAttriFunc;
	//SubFunc mAttribSubFunc;
	//MulFunc mMulFunc;
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


#endif // Shader_h__

