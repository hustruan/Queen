#include "Prerequisite.h"
#include "RenderDevice.h"
#include "Model.h"
#include "RenderFactory.h"
#include "FrameBuffer.h"
#include "Applicaton.h"
#include <MathUtil.hpp>

#include "GraphicsBuffer.h"
#include "Context.h"

using namespace RxLib;

#define Queue_PI    (3.14159265358979323846)

class SimpleVertexShader : public VertexShader
{
public:

	void Bind()
	{
		DeclareVarying(InterpolationModifier::Linear, float4, oPosW, 0);
		DeclareVarying(InterpolationModifier::Linear, float4, oNormal, 1);
	}

	void Execute(const VS_Input* input, VS_Output* output)
	{
		DefineAttribute(float4, iPos, 0);
		DefineAttribute(float4, iNormal, 1);

		DefineVaryingOutput(float4, oPosW, 0);
		DefineVaryingOutput(float4, oNormal, 1);

		oPosW = iPos * World;
		oNormal = float4(iNormal.X(), iNormal.Y(), iNormal.Z(), 0.0) * World;

		output->Position = oPosW * View * Projection;
	}

	uint32_t GetOutputCount() const
	{
		return 2;
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
		//float3 posW = float3((float*)&input->ShaderOutputs[0]);
		//float3 normal = float3((float*)&input->ShaderOutputs[1]);

		DefineVaryingInput(float3, posW, 0);
		DefineVaryingInput(float3, normal, 1);


		float3 L = Normalize(LightPos - posW);
		float3 N = Normalize(normal);

		float NdotL = Dot(N, L);

		output->Color[0] = Saturate(ColorRGBA::White * NdotL);

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
		bool loaded = mModel.loadModelFromFile("../../Media/Infinite-Level_02.OBJ");

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
		mVertexShader->View = CreateLookAtMatrixLH(float3(0, 0, 1.6), float3(0, 0, 0), float3(0, 1, 0));
		mVertexShader->Projection =  CreatePerspectiveFovLH<float>(Queue_PI / 9, 1.0f, 0.1f, 100.0f ); 
		
		mPixelShader = std::make_shared<SimplePixelShader>();
		mPixelShader->LightPos = float3(10, 10, 10);
		
		Center = CreateTranslation(float3(-center.x, -center.y, -center.z));
	}

	void Render()
	{

		static float t = 0.0f;
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if( dwTimeStart == 0 )
			dwTimeStart = dwTimeCur;
		t = ( dwTimeCur - dwTimeStart ) / 1000.0f;

		mVertexShader->World = Center * CreateRotationY(t);

		mRenderDevice->GetCurrentFrameBuffer()->Clear(CF_Color | CF_Depth,
			ColorRGBA(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);

		mRenderDevice->SetVertexStream(0, mVertexBuffer, 0, mVertexDecl->GetVertexSize());
		mRenderDevice->SetInputLayout(mVertexDecl);

		mRenderDevice->SetIndexBuffer(mIndexBuffer, IBT_Bit32, 0);

		mRenderDevice->SetVertexShader(mVertexShader);
		mRenderDevice->SetPixelShader(mPixelShader);

		mRenderDevice->DrawIndexed(PT_Triangle_List, mModel.getCompiledIndexCount(), 0, 0);
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


