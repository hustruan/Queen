#include "Prerequisite.h"
#include "MathLib/Vector.hpp"
#include "MathLib/Matrix.hpp"
#include "MathLib/MathUtil.hpp"
#include "MathLib/ColorRGBA.hpp"
#include "RenderDevice.h"
#include "Model.h"
#include "RenderFactory.h"
#include "FrameBuffer.h"
#include "Applicaton.h"

#include "GraphicsBuffer.h"
#include "Context.h"

using namespace MathLib;

#define Queue_PI    (3.14159265358979323846)

class SimpleVertexShader : public VertexShader
{
public:

	void Execute(const VS_Input* input, VS_Output* output)
	{
		const float4& iPos = input->ShaderInputs[0];
		const float4& iNormal = input->ShaderInputs[1];
		const float4& iColor = input->ShaderInputs[2]; 

		Varying(float4, oPosW, 0);
		Varying(float4, oNormal, 1);
		Varying(float4, oColor, 2);

		oPosW = iPos * World;
		oNormal = iNormal * World;
		oColor = iColor;

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
		float3 posW = float3((float*)&input->ShaderOutputs[0]);
		float3 normal = float3((float*)&input->ShaderOutputs[1]);
		ColorRGBA color = ColorRGBA((float*)&input->ShaderOutputs[2]);

		float3 L = Normalize(LightPos - posW);
		float3 N = Normalize(normal);

		float NdotL = Dot(N, L);

		//color = ColorRGBA(1, 0, 0, 1);
		output->Color[0] = /*color*/Saturate(color * NdotL);

		//ColorRGBA Diffuse = Sample(DiffuseTex, LinearSampler, uv.X(), uv.Y());

		/*const float4& iColor = input->ShaderOutputs[2];

		float R = iColor.X();
		float G = iColor.Y();
		float B = iColor.Z();
		float A = iColor.W();

		output->Color[0] = ColorRGBA(R, G, B, A);*/

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

	void LoadContent()
	{
		SimpleVertex vertices[] =
		{
			// Top 红色
			{ float3( -1.0f, 1.0f, -1.0f ), float3( 0.0f, 1.0f, 0.0f ), ColorRGBA(1, 0, 0, 1) },
			{ float3( 1.0f, 1.0f, -1.0f ), float3( 0.0f, 1.0f, 0.0f ), ColorRGBA(1, 1, 0, 1)  },
			{ float3( 1.0f, 1.0f, 1.0f ), float3( 0.0f, 1.0f, 0.0f ), ColorRGBA(1, 0, 1, 1)  },
			{ float3( -1.0f, 1.0f, 1.0f ), float3( 0.0f, 1.0f, 0.0f ), ColorRGBA(1, 0, 0, 1)  },

			// Bottom 黄色
			{ float3( -1.0f, -1.0f, -1.0f ), float3( 0.0f, -1.0f, 0.0f ), ColorRGBA(0, 1, 0, 1)  },
			{ float3( 1.0f, -1.0f, -1.0f ), float3( 0.0f, -1.0f, 0.0f ), ColorRGBA(1, 1, 0, 1)  },
			{ float3( 1.0f, -1.0f, 1.0f ), float3( 0.0f, -1.0f, 0.0f ), ColorRGBA(1, 0, 0, 1)  },
			{ float3( -1.0f, -1.0f, 1.0f ), float3( 0.0f, -1.0f, 0.0f ), ColorRGBA(1, 1, 1, 1)  },

			// Left 绿色
			{ float3( -1.0f, -1.0f, 1.0f ), float3( -1.0f, 0.0f, 0.0f ), ColorRGBA(0.5, 1, 0, 1)  },
			{ float3( -1.0f, -1.0f, -1.0f ), float3( -1.0f, 0.0f, 0.0f ), ColorRGBA(0, 1, 0.5, 1)  },
			{ float3( -1.0f, 1.0f, -1.0f ), float3( -1.0f, 0.0f, 0.0f ), ColorRGBA(0.56, 1, 0.0451, 1)  },
			{ float3( -1.0f, 1.0f, 1.0f ), float3( -1.0f, 0.0f, 0.0f ), ColorRGBA(1, 1, 0, 1)  },

			// Right 紫红色
			{ float3( 1.0f, -1.0f, 1.0f ), float3( 1.0f, 0.0f, 0.0f ), ColorRGBA(0.5, 0, 1, 1)  },
			{ float3( 1.0f, -1.0f, -1.0f ), float3( 1.0f, 0.0f, 0.0f ), ColorRGBA(1, 0, 1, 1)  },
			{ float3( 1.0f, 1.0f, -1.0f ), float3( 1.0f, 0.0f, 0.0f ), ColorRGBA(0.8, 0.5, 0.6, 1)  },
			{ float3( 1.0f, 1.0f, 1.0f ), float3( 1.0f, 0.0f, 0.0f ), ColorRGBA(1, 0.5, 0.4, 1)  },

			// Back 
			{ float3( -1.0f, -1.0f, -1.0f ), float3( 0.0f, 0.0f, -1.0f ), ColorRGBA(1, 0, 1, 1)  },
			{ float3( 1.0f, -1.0f, -1.0f ), float3( 0.0f, 0.0f, -1.0f ), ColorRGBA(1, 1, 1, 1)  },
			{ float3( 1.0f, 1.0f, -1.0f ), float3( 0.0f, 0.0f, -1.0f ), ColorRGBA(1, 0, 1, 1)  },
			{ float3( -1.0f, 1.0f, -1.0f ), float3( 0.0f, 0.0f, -1.0f ), ColorRGBA(1, 1, 0, 1)  },

			// Front
			{ float3( -1.0f, -1.0f, 1.0f ), float3( 0.0f, 0.0f, 1.0f ), ColorRGBA(1, 0, 0, 1)  },
			{ float3( 1.0f, -1.0f, 1.0f ), float3( 0.0f, 0.0f, 1.0f ), ColorRGBA(0, 1, 0, 1)  },
			{ float3( 1.0f, 1.0f, 1.0f ), float3( 0.0f, 0.0f, 1.0f ), ColorRGBA(0, 0, 1, 1)  },
			{ float3( -1.0f, 1.0f, 1.0f ), float3( 0.0f, 0.0f, 1.0f ), ColorRGBA(0, 0, 0, 1)  },
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

		mModel.loadModelFromFile("../Media/cube.obj");

		if(!mModel.hasNormals())
			mModel.computeNormals();

		mModel.compileModel( nv::Model::eptTriangles);

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
		ve[1].Offset = 12;

		ve[2].Stream = 0;
		ve[2].Type = VEF_Float4;
		ve[2].Usage = VEU_Color;
		ve[2].UsageIndex = 0;
		ve[2].Offset = 24;

		/*ve[2].Stream = 0;
		ve[2].Type = VEF_Float3;
		ve[2].Usage = VEU_TextureCoordinate;
		ve[2].UsageIndex = 0;
		ve[2].Offset = mModel.getCompiledTexCoordOffset() * sizeof(float);*/

		mVertexDecl = mRenderFactory->CreateVertexDeclaration(ve, 3);

		/*ElementInitData initData;
		initData.pData = mModel.getCompiledVertices();
		initData.RowPitch = static_cast<uint32_t>(mModel.getCompiledVertexCount() * mModel.getCompiledVertexSize() * sizeof(float));
		mVertexBuffer = mRenderFactory->CreateVertexBuffer(&initData);
		
		initData.pData = mModel.getCompiledIndices();
		initData.RowPitch = mModel.getCompiledIndexCount() * sizeof(uint32_t);
		mIndexBuffer = mRenderFactory->CreateIndexBuffer(&initData);*/

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

		//t = Queue_PI * 0.25f;


		mVertexShader->World = CreateRotationY(t);

		mRenderDevice->GetCurrentFrameBuffer()->Clear(CF_Color | CF_Depth,
			ColorRGBA(0.5f, 0.5f, 0.5f, 1.0f), 1.0f, 0);

		mRenderDevice->SetVertexStream(0, mVertexBuffer, 0, mVertexDecl->GetVertexSize());
		mRenderDevice->SetInputLayout(mVertexDecl);

		mRenderDevice->SetIndexBuffer(mIndexBuffer, IBT_Bit32, 0);

		mRenderDevice->SetVertexShader(mVertexShader);
		mRenderDevice->SetPixelShader(mPixelShader);

		mRenderDevice->DrawIndexed(PT_Triangle_List, /*mModel.getCompiledIndexCount()*/36, 0, 0); 

		//mRenderDevice->DrawIndexed(PT_Triangle_List, /*mModel.getCompiledIndexCount()*/3, 33, 0); 
	}

private:
	shared_ptr<SimpleVertexShader> mVertexShader;
	shared_ptr<SimplePixelShader> mPixelShader;
	shared_ptr<GraphicsBuffer> mVertexBuffer;
	shared_ptr<GraphicsBuffer> mIndexBuffer;
	shared_ptr<VertexDeclaration> mVertexDecl;
	nv::Model mModel;
};

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


