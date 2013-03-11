#include "RenderDevice.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "VertexDeclaration.h"
#include "Rasterizer.h"
#include "GraphicsBuffer.h"
#include "Shader.h"
#include "pfm.h"

RenderDevice::RenderDevice(void)
	: mUseIndex(false), mStartIndexLoc(0), mBaseVertexLoc(0)
{
	mVertexShaderStage = new VertexShaderStage(*this);
	mPixelShaderStage = new PixelShaderStage(*this);
	mRasterizerStage = new Rasterizer(*this);

	mScreenFrameBuffer = std::make_shared<FrameBuffer>(500, 500);

	shared_ptr<Texture2D> color0(new Texture2D(PF_A32B32G32R32F, 500, 500, 0, 1, 0, 0));
	mScreenFrameBuffer->Attach(ATT_Color0, color0);

	shared_ptr<Texture2D> depth(new Texture2D(PF_Depth32, 500, 500, 0, 1, 0, 0));
	mScreenFrameBuffer->Attach(ATT_DepthStencil, depth);

	// bind screen frame buffer 
	BindFrameBuffer(mScreenFrameBuffer);

	// todo 
	TextureFetch::Init();
}


RenderDevice::~RenderDevice(void)
{
}

void RenderDevice::SetVertexStream( uint32_t streamSlot, const shared_ptr<GraphicsBuffer>& vertexBuffer, uint32_t offset, uint32_t stride )
{
	ASSERT(streamSlot < MaxVertexStreams);
	ASSERT(stride > 0);

	mVertexStreams[streamSlot].VertexBuffer = vertexBuffer;
	mVertexStreams[streamSlot].Offset = offset;
	mVertexStreams[streamSlot].Stride = stride;
}

void RenderDevice::SetIndexBuffer( const shared_ptr<GraphicsBuffer>& indexBuffer, IndexBufferType format, uint32_t offset )
{
	mIndexBuffer = indexBuffer;
}

const shared_ptr<GraphicsBuffer>& RenderDevice::GetIndexBuffet() const
{
	return mIndexBuffer;
}

void RenderDevice::SetInputLayout( const shared_ptr<VertexDeclaration>& decl )
{
	mVertexDecl = decl;
}

void RenderDevice::DecodeVertexStream( VS_Input* oVSInput, uint32_t iVertex )
{
	uint8_t* pVertex[MaxVertexStreams];
	
	for (uint32_t slot = 0; slot <= mVertexDecl->GetVertexStreams(); ++slot)
	{
		const VertexStream& vertexStream = mVertexStreams[slot];
		const uint32_t offset = vertexStream.Offset + iVertex * vertexStream.Stride;

		pVertex[slot] = (uint8_t*)vertexStream.VertexBuffer->Map(offset, 0, BA_Read_Only);
	}

	const VertexElementList& elements = mVertexDecl->GetElements();
	for (size_t i = 0; i < elements.size(); ++i)
	{
		const VertexElement& ve = elements[i];
		uint32_t offset = ve.Offset;

		switch(ve.Type)
		{
		case VEF_Float:
			{
				float* pData = (float*)(pVertex[ve.Stream] + offset);
				oVSInput->ShaderInputs[i] = ShaderRegister(pData[0], 0, 0, 1);
			}
			break;
		case VEF_Float2:
			{
				float* pData = (float*)(pVertex[ve.Stream] + offset);
				oVSInput->ShaderInputs[i] = ShaderRegister(pData[0], pData[1], 0, 1);
			}	
			break;
		case VEF_Float3:
			{
				float* pData = (float*)(pVertex[ve.Stream] + offset);
				oVSInput->ShaderInputs[i] = ShaderRegister(pData[0], pData[1], pData[2], 1);
			}	
			break;
		case VEF_Float4:
			{
				float* pData = (float*)(pVertex[ve.Stream] + offset);
				oVSInput->ShaderInputs[i] = ShaderRegister(pData[0], pData[1], pData[2], pData[3]);
			}			
			break;
		default:
			ASSERT(false);
		}
	}
}

void RenderDevice::Draw( PrimitiveType primitiveType, uint32_t vertexCount, uint32_t startVertexLocation )
{
	ASSERT(primitiveType == PT_Triangle_List);
	uint32_t primitive = vertexCount / 3;

	// set up vertice data
	mUseIndex = false;

	mRasterizerStage->PreDraw();
	mRasterizerStage->Draw(primitiveType, primitive);
	mRasterizerStage->PostDraw();
}

void RenderDevice::DrawIndexed( PrimitiveType primitiveType, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation )
{
	ASSERT(primitiveType == PT_Triangle_List);

	uint32_t primitive = indexCount / 3;

	mUseIndex = true;
	mStartIndexLoc = startIndexLocation;
	mBaseVertexLoc = baseVertexLocation;

	mRasterizerStage->PreDraw();
	
	mRasterizerStage->Draw(primitiveType, primitive);
	
	//mRasterizerStage->DrawTiled(primitiveType, primitive);
	
	mRasterizerStage->PostDraw();
}


VS_Output RenderDevice::FetchVertex( uint32_t index )
{
	VS_Input vertexInput;
	VS_Output vertexOutput;

	DecodeVertexStream(&vertexInput, mBaseVertexLoc + index);
	mVertexShaderStage->GetVertexShader()->Execute(&vertexInput, &vertexOutput);

	return vertexOutput;
}

uint32_t RenderDevice::FetchIndex( uint32_t index )
{
	if (mUseIndex)
	{
		uint32_t* buffer = (uint32_t*) mIndexBuffer->Map(0, 0, BA_Read_Only);
		return buffer[mStartIndexLoc + index];
	}

	return index;
}

void RenderDevice::BindFrameBuffer( const shared_ptr<FrameBuffer>& fb )
{
	if( mCurrentFrameBuffer && (fb != mCurrentFrameBuffer) )
	{	
		mCurrentFrameBuffer->OnUnbind();	
	}

	mCurrentFrameBuffer = fb; 
	mRasterizerStage->OnBindFrameBuffer(fb);

	if(mCurrentFrameBuffer->IsDirty())
	{
		mCurrentFrameBuffer->OnBind();
	}
}

void RenderDevice::SaveScreenToPfm( const String& filename )
{
	auto texture = mCurrentFrameBuffer->GetRenderTarget(ATT_Color0);
	uint32_t width = texture->GetWidth(0);
	uint32_t height = texture->GetWidth(0);

	void* pData;
	uint32_t pitch;
	
	texture->Map2D(0, TMA_Read_Only, 0, 0, width, height, pData, pitch);

	float* pColor = (float*)pData;
	std::vector<float> pfmData(width * height * 3);
	float* pColorPfm = &pfmData[0];

	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			uint32_t invY = (height-y-1);
			pfmData[ invY * width * 3 + x*3 + 0] = pColor[0];
			pfmData[invY * width * 3 + x*3 + 1] = pColor[1];
			pfmData[invY * width * 3 + x*3 + 2] = pColor[2];

			pColor += 4;

		}
	}

	WritePfm(filename.c_str(), width, height, 3, &pfmData[0]);
}

