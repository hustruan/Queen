Purple Queen
===============

Queen is a D3D10 like multi-thread software rasterizer renderer. I implement classic scanline algorithm and a simple tile-based half-space method. It supports programmable vertex and pixel shader which are writen directly in Cpp file.

### Simple Shader
```cpp   
       
class SimpleVertexShader : public VertexShader
{
public:

	void Bind()
	{
		DeclareVarying(InterpolationModifier::Linear, float4, oPosW, 0);
		DeclareVarying(InterpolationModifier::Linear, float4, oNormal, 1);
		DeclareVarying(InterpolationModifier::Linear, float2, oTex, 2);
		DeclareVarying(InterpolationModifier::Linear, float4, oColor, 3);
	}

	void Execute(const VS_Input* input, VS_Output* output)
	{
		DefineAttribute(float4, iPos, 0);
		DefineAttribute(float4, iNormal, 1);
		DefineAttribute(float2, iTex, 2);
		DefineAttribute(float4, iColor, 3);

		DefineVaryingOutput(float4, oPosW, 0);
		DefineVaryingOutput(float4, oNormal, 1);
		DefineVaryingOutput(float2, oTex, 2);
		DefineVaryingOutput(float4, oColor, 3);

		oPosW = iPos * World;
		oNormal = float4(iNormal.X(), iNormal.Y(), iNormal.Z(), 0.0) * World;
		oTex = iTex;
		oColor = iColor;

		output->Position = oPosW * View * Projection;
	}

	uint32_t GetOutputCount() const
	{
		return 4;
	}

public:
	float44 World;
	float44 View;
	float44 Projection;
};   

class SimplePixelShader : public PixelShader
{
public:

	float3 LightPos;
	
	DefineTexture(0, DiffuseTex);
	DefineSampler(0, LinearSampler);

	bool Execute(const VS_Output* input, PS_Output* output, float* pDepthIO)
	{
		DefineVaryingInput(float3, iPosW, 0);
		DefineVaryingInput(float3, iNormal, 1);
		DefineVaryingInput(float2, iTex, 2);
		DefineVaryingInput(float4, iColor, 3);

		float3 L = Normalize(LightPos - iPosW);
		float3 N = Normalize(iNormal);

		float NdotL = Dot(N, L);

		output->Color[0] = Saturate(ColorRGBA((float*)&iColor) * NdotL);

		//ColorRGBA diffuse = Sample(DiffuseTex, LinearSampler, iTex.X(), iTex.Y());
		//output->Color[0] = ColorRGBA(diffuse.R, diffuse.G, diffuse.B, 1.0f);

		return true;
	}

	uint32_t GetOutputCount() const
	{
		return 1;
	}
};

``` 

### Screenshort
![](https://github.com/hustruan/Queen/blob/master/Media/Head.png)

================
Purple is a physically based ray tracer.


Note: 
I have put all 3rd libraries in 3rdParty directory except for boost, it's a little bit huge, so I don't upload it. 
