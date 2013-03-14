#include "Prerequisite.h"
#include "RenderDevice.h"
#include "RenderFactory.h"
#include "FrameBuffer.h"
#include "Applicaton.h"
#include "GraphicsBuffer.h"
#include "Context.h"

#include <MathUtil.hpp>
#include <nvModel.h>
//#include <nvSDKPath.h>
//
//using namespace RxLib;

#define Queue_PI    float(3.14159265358979323846)

//nv::SDKPath gAppPath;

//#define CubeDemo

#ifdef CubeDemo

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

		//output->Color[0] = Saturate(ColorRGBA((float*)&iColor) * NdotL);

		ColorRGBA diffuse = Sample(DiffuseTex, LinearSampler, iTex.X(), iTex.Y());
		output->Color[0] = ColorRGBA(diffuse.R, diffuse.G, diffuse.B, 1.0f);

		return true;
	}

	uint32_t GetOutputCount() const
	{
		return 1;
	}
};

class TestApp : public Applicaton
{
public:
	TestApp() 
		: Applicaton()
	{
		std::unordered_map<std::string, int> test;
	}

	struct SimpleVertex
	{
		float3 Pos;
		float3 Normal;
		float2 Tex;
		ColorRGBA Color;
	};

	void LoadContent()
	{
		mDiffuseTexture = mRenderFactory->CreateTextureFromFile("../../Media/Glass.dds");

		void* pData; uint32_t pitch;
		mDiffuseTexture->Map2D(0, TMA_Read_Only, 0, 0, 0, 0, pData, pitch);
		//ExportToPfm("E:/test.pfm", 256, 256, mDiffuseTexture->GetTextureFormat(), pData);

		SimpleVertex vertices[] =
		{
			// Top 红色
			{ float3( -1.0f, 1.0f, -1.0f ), float3( 0.0f, 1.0f, 0.0f ), float2( 0.0f, 0.0f ), ColorRGBA(1, 0, 0, 1) },
			{ float3( 1.0f, 1.0f, -1.0f ), float3( 0.0f, 1.0f, 0.0f ), float2( 1.0f, 0.0f ), ColorRGBA(1, 1, 0, 1)  },
			{ float3( 1.0f, 1.0f, 1.0f ), float3( 0.0f, 1.0f, 0.0f ), float2( 1.0f, 1.0f ), ColorRGBA(1, 0, 1, 1)  },
			{ float3( -1.0f, 1.0f, 1.0f ), float3( 0.0f, 1.0f, 0.0f ), float2( 0.0f, 1.0f ),ColorRGBA(1, 0, 0, 1)  },

			// Bottom 黄色
			{ float3( -1.0f, -1.0f, -1.0f ), float3( 0.0f, -1.0f, 0.0f ), float2( 0.0f, 0.0f ), ColorRGBA(0, 1, 0, 1)  },
			{ float3( 1.0f, -1.0f, -1.0f ), float3( 0.0f, -1.0f, 0.0f ), float2( 1.0f, 0.0f ), ColorRGBA(1, 1, 0, 1)  },
			{ float3( 1.0f, -1.0f, 1.0f ), float3( 0.0f, -1.0f, 0.0f ), float2( 1.0f, 1.0f ), ColorRGBA(1, 0, 0, 1)  },
			{ float3( -1.0f, -1.0f, 1.0f ), float3( 0.0f, -1.0f, 0.0f ), float2( 0.0f, 1.0f ),ColorRGBA(1, 1, 1, 1)  },

			// Left 绿色
			{ float3( -1.0f, -1.0f, 1.0f ), float3( -1.0f, 0.0f, 0.0f ), float2( 0.0f, 0.0f ), ColorRGBA(0.5f, 1, 0, 1)  },
			{ float3( -1.0f, -1.0f, -1.0f ), float3( -1.0f, 0.0f, 0.0f ), float2( 1.0f, 0.0f ),ColorRGBA(0, 1, 0.5f, 1)  },
			{ float3( -1.0f, 1.0f, -1.0f ), float3( -1.0f, 0.0f, 0.0f ), float2( 1.0f, 1.0f ), ColorRGBA(0.56f, 1, 0.0451f, 1)  },
			{ float3( -1.0f, 1.0f, 1.0f ), float3( -1.0f, 0.0f, 0.0f ), float2( 0.0f, 1.0f ), ColorRGBA(1, 1, 0, 1)  },

			// Right 紫红色
			{ float3( 1.0f, -1.0f, 1.0f ), float3( 1.0f, 0.0f, 0.0f ), float2( 0.0f, 0.0f ), ColorRGBA(0.5, 0, 1, 1)  },
			{ float3( 1.0f, -1.0f, -1.0f ), float3( 1.0f, 0.0f, 0.0f ), float2( 1.0f, 0.0f ) , ColorRGBA(1, 0, 1, 1)  },
			{ float3( 1.0f, 1.0f, -1.0f ), float3( 1.0f, 0.0f, 0.0f ), float2( 1.0f, 1.0f ), ColorRGBA(0.8, 0.5, 0.6, 1)  },
			{ float3( 1.0f, 1.0f, 1.0f ), float3( 1.0f, 0.0f, 0.0f ), float2( 0.0f, 1.0f ), ColorRGBA(1, 0.5, 0.4, 1)  },

			// Back 
			{ float3( -1.0f, -1.0f, -1.0f ), float3( 0.0f, 0.0f, -1.0f ), float2( 0.0f, 0.0f ), ColorRGBA(1, 0, 1, 1)  },
			{ float3( 1.0f, -1.0f, -1.0f ), float3( 0.0f, 0.0f, -1.0f ), float2( 1.0f, 0.0f ), ColorRGBA(1, 1, 1, 1)  },
			{ float3( 1.0f, 1.0f, -1.0f ), float3( 0.0f, 0.0f, -1.0f ), float2( 1.0f, 1.0f ), ColorRGBA(1, 0, 1, 1)  },
			{ float3( -1.0f, 1.0f, -1.0f ), float3( 0.0f, 0.0f, -1.0f ), float2( 0.0f, 1.0f ), ColorRGBA(1, 1, 0, 1)  },

			// Front
			{ float3( -1.0f, -1.0f, 1.0f ), float3( 0.0f, 0.0f, 1.0f ), float2( 0.0f, 0.0f ), ColorRGBA(1, 0, 0, 1)  },
			{ float3( 1.0f, -1.0f, 1.0f ), float3( 0.0f, 0.0f, 1.0f ), float2( 1.0f, 0.0f ), ColorRGBA(0, 1, 0, 1)  },
			{ float3( 1.0f, 1.0f, 1.0f ), float3( 0.0f, 0.0f, 1.0f ), float2( 1.0f, 1.0f ), ColorRGBA(0, 0, 1, 1)  },
			{ float3( -1.0f, 1.0f, 1.0f ), float3( 0.0f, 0.0f, 1.0f ), float2( 0.0f, 1.0f ), ColorRGBA(0, 0, 0, 1)  },
		};

		uint32_t indices[] =
		{
			3,1,0,
			2,1,3,

			6,4,5,
			7,4,6,

			11,9,8,
			10,9,11,

			14,12,13,
			15,12,14,

			19,17,16,
			18,17,19,

			22,20,21,
			23,20,22
		};

		// Create buffers
		VertexElement ve[4] = 
		{
			VertexElement(0, 0, VEF_Float3, VEU_Position, 0),
			VertexElement(0, 12, VEF_Float3, VEU_Normal, 0),
			VertexElement(0, 24, VEF_Float2, VEU_TextureCoordinate, 0),
			VertexElement(0, 32, VEF_Float4, VEU_Color, 0),
		};
		mVertexDecl = mRenderFactory->CreateVertexDeclaration(ve, 4);

		ElementInitData initData;
		initData.pData = vertices;
		initData.RowPitch = sizeof(vertices);
		mVertexBuffer = mRenderFactory->CreateVertexBuffer(&initData);

		initData.pData = indices;
		initData.RowPitch = sizeof(indices);
		mIndexBuffer = mRenderFactory->CreateIndexBuffer(&initData);


		mVertexShader = std::make_shared<SimpleVertexShader>();
		mVertexShader->View = CreateLookAtMatrixLH(float3(0, 3, 3), float3(0, 0, 0), float3(0, 1, 0));
		mVertexShader->Projection =  CreatePerspectiveFovLH<float>(Queue_PI * 0.5f, 1.0f, 0.1f, 100.0f ); 
		
		mPixelShader = std::make_shared<SimplePixelShader>();
		mPixelShader->LightPos = float3(10, 10, 10);
		
		mVertexShader->World.MakeIdentity();
	}

	void Render()
	{
		static float t = 0.0f;
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if( dwTimeStart == 0 )
		dwTimeStart = dwTimeCur;
		t = ( dwTimeCur - dwTimeStart ) / 1000.0f;

		//t = 0.54f;


		mVertexShader->World = CreateRotationY(t);

		mRenderDevice->GetCurrentFrameBuffer()->Clear(CF_Color | CF_Depth,
			ColorRGBA(0.5f, 0.5f, 0.5f, 1.0f), 1.0f, 0);

		mRenderDevice->SetVertexStream(0, mVertexBuffer, 0, mVertexDecl->GetVertexSize());
		mRenderDevice->SetInputLayout(mVertexDecl);

		mRenderDevice->SetIndexBuffer(mIndexBuffer, IBT_Bit32, 0);

		mRenderDevice->SetVertexShader(mVertexShader);
		mRenderDevice->SetPixelShader(mPixelShader);

		mRenderDevice->SampleStates[0].AddressU = TAM_Wrap;
		mRenderDevice->SampleStates[0].AddressW = TAM_Wrap;
		mRenderDevice->SampleStates[0].BindStage = ST_Pixel;
		mRenderDevice->SampleStates[0].Filter = TF_Min_Mag_Linear_Mip_Point;

		mRenderDevice->TextureUnits[0] = mDiffuseTexture;

		mRenderDevice->DrawIndexed(PT_Triangle_List, /*mModel.getCompiledIndexCount()*/36, 0, 0); 
		//mRenderDevice->DrawIndexed(PT_Triangle_List, /*mModel.getCompiledIndexCount()*/6, 30, 0); 

	   //mRenderDevice->SaveScreenToPfm("scanline.pfm");	
	}

private:
	shared_ptr<SimpleVertexShader> mVertexShader;
	shared_ptr<SimplePixelShader> mPixelShader;
	shared_ptr<GraphicsBuffer> mVertexBuffer;
	shared_ptr<GraphicsBuffer> mIndexBuffer;
	shared_ptr<VertexDeclaration> mVertexDecl;
	shared_ptr<Texture> mDiffuseTexture;
	nv::Model mModel;
};

#else

class SimpleVertexShader : public VertexShader
{
public:

	void Bind()
	{
		DeclareVarying(InterpolationModifier::Linear, float4, oPosW, 0);
		DeclareVarying(InterpolationModifier::Linear, float4, oNormal, 1);
		DeclareVarying(InterpolationModifier::Linear, float2, oTex, 2);
	}

	void Execute(const VS_Input* input, VS_Output* output)
	{
		DefineAttribute(float4, iPos, 0);
		DefineAttribute(float4, iNormal, 1);
		DefineAttribute(float2, iTex, 2);

		DefineVaryingOutput(float4, oPosW, 0);
		DefineVaryingOutput(float4, oNormal, 1);
		DefineVaryingOutput(float2, oTex, 2);

		oPosW = iPos * World;
		oNormal = float4(iNormal.X(), iNormal.Y(), iNormal.Z(), 0.0) * World;
		oTex = iTex;

		output->Position = oPosW * View * Projection;
	}

	uint32_t GetOutputCount() const
	{
		return 3;
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

		float3 L = Normalize(LightPos - iPosW);
		float3 N = Normalize(iNormal);
		float NdotL = Dot(N, L);

		ColorRGBA diffuse = Sample(DiffuseTex, LinearSampler, iTex.X(), iTex.Y());

		diffuse.R = powf(diffuse.R, 1.0f / 1.2f);
		diffuse.G = powf(diffuse.G, 1.0f / 1.2f);
		diffuse.B = powf(diffuse.B, 1.0f / 1.2f);

		output->Color[0] = Saturate(diffuse * NdotL);

		//output->Color[0] = Saturate(ColorRGBA::White * NdotL);

		return true;
	}

	uint32_t GetOutputCount() const
	{
		return 1;
	}
};

class TestApp : public Applicaton
{
public:
	TestApp() 
		: Applicaton()
	{

	}

	struct SimpleVertex
	{
		float3 Pos;
		float3 Normal;
		ColorRGBA Color;
	};

	float44 Center;

	void LoadContent()
	{
		mDiffuseTexture = mRenderFactory->CreateTextureFromFile("../Media/Map-COL.png");

		bool loaded = mModel.loadModelFromFile("../Media/Infinite-Level_02.OBJ");

		if(!mModel.hasNormals())
			mModel.computeNormals();

		mModel.compileModel( nv::Model::eptTriangles);

		nv::vec3f min, max, center;
		mModel.computeBoundingBox(min, max);
		center = (min + max) / 2;

		// Create buffers
		VertexElement ve[3];

		ve[0].Stream = 0;
		ve[0].Type = VEF_Float3;
		ve[0].Usage = VEU_Position;
		ve[0].UsageIndex = 0;
		ve[0].Offset = 0;

		ve[1].Stream = 0;
		ve[1].Type = VEF_Float3;
		ve[1].Usage = VEU_Normal;
		ve[1].UsageIndex = 0;
		ve[1].Offset = mModel.getCompiledNormalOffset() * sizeof(float);

		ve[2].Stream = 0;
		ve[2].Type = VEF_Float2;
		ve[2].Usage = VEU_TextureCoordinate;
		ve[2].UsageIndex = 0;
		ve[2].Offset = mModel.getCompiledTexCoordOffset() * sizeof(float);

		mVertexDecl = mRenderFactory->CreateVertexDeclaration(ve, 3);

		auto vc = mModel.getCompiledVertexCount();
		auto vs = mModel.getCompiledVertexSize();

		ElementInitData initData;
		initData.pData = mModel.getCompiledVertices();
		initData.RowPitch = static_cast<uint32_t>(mModel.getCompiledVertexCount() * mModel.getCompiledVertexSize() * sizeof(float));
		mVertexBuffer = mRenderFactory->CreateVertexBuffer(&initData);

		initData.pData = mModel.getCompiledIndices();
		initData.RowPitch = mModel.getCompiledIndexCount() * sizeof(uint32_t);
		mIndexBuffer = mRenderFactory->CreateIndexBuffer(&initData);


		mVertexShader = std::make_shared<SimpleVertexShader>();
		mVertexShader->View = CreateLookAtMatrixLH(float3(0, 0, 1.6f), float3(0, 0, 0), float3(0, 1, 0));
		mVertexShader->Projection =  CreatePerspectiveFovLH<float>(Queue_PI / 9, 1.0f, 0.1f, 100.0f ); 

		mPixelShader = std::make_shared<SimplePixelShader>();
		mPixelShader->LightPos = float3(10, 10, 10);

		Center = CreateTranslation(float3(-center.x, -center.y, -center.z));
		mVertexShader->World = Center;
	}

	void Render()
	{

		/*static float t = 0.0f;
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if( dwTimeStart == 0 )
			dwTimeStart = dwTimeCur;
		t = ( dwTimeCur - dwTimeStart ) / 1000.0f;

		mVertexShader->World = Center * CreateRotationY(t);*/

		mRenderDevice->GetCurrentFrameBuffer()->Clear(CF_Color | CF_Depth,
			ColorRGBA(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);

		mRenderDevice->SetVertexStream(0, mVertexBuffer, 0, mVertexDecl->GetVertexSize());
		mRenderDevice->SetInputLayout(mVertexDecl);

		mRenderDevice->SetIndexBuffer(mIndexBuffer, IBT_Bit32, 0);

		mRenderDevice->SetVertexShader(mVertexShader);
		mRenderDevice->SetPixelShader(mPixelShader);

		mRenderDevice->SampleStates[0].AddressU = TAM_Wrap;
		mRenderDevice->SampleStates[0].AddressW = TAM_Wrap;
		mRenderDevice->SampleStates[0].BindStage = ST_Pixel;
		mRenderDevice->SampleStates[0].Filter = TF_Min_Mag_Linear_Mip_Point;

		mRenderDevice->TextureUnits[0] = mDiffuseTexture;

		mRenderDevice->DrawIndexed(PT_Triangle_List, mModel.getCompiledIndexCount(), /*mModel.getCompiledIndexCount()/2*/0, 0);
	
		//mRenderDevice->SaveScreenToPfm("E:/screen.pfm");
	}

private:
	shared_ptr<SimpleVertexShader> mVertexShader;
	shared_ptr<SimplePixelShader> mPixelShader;
	shared_ptr<GraphicsBuffer> mVertexBuffer;
	shared_ptr<GraphicsBuffer> mIndexBuffer;
	shared_ptr<VertexDeclaration> mVertexDecl;
	shared_ptr<Texture> mDiffuseTexture;
	nv::Model mModel;
};

#endif

int WINAPI WinMain(	HINSTANCE	hInstance,			// Instance
				   HINSTANCE	hPrevInstance,		// Previous Instance
				   LPSTR		lpCmdLine,			// Command Line Parameters
				   int			nCmdShow)			// Window Show State
{
	TestApp app;

	app.Create();
	app.Run();

	return 0;
}


