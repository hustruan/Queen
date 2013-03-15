#ifndef Renderer_h__
#define Renderer_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"
#include "RenderState.h"
#include "SampleState.h"
#include "Shader.h"

#define MaxTextureUnits 8
#define MaxVertexStreams 8
#define MaxVertexBufferSize 18000
#define MaxVertexBufferClip (MaxVertexBufferSize * 5)
#define MaxBinQueueSize     MaxVertexBufferClip

using namespace RxLib;

class Rasterizer;

class RenderDevice
{
	friend class Rasterizer;
	friend class Shader;

public:
	RenderDevice(void);
	~RenderDevice(void);

	void SetIndexBuffer(const shared_ptr<GraphicsBuffer>& indexBuffer, IndexBufferType format, uint32_t offset);
	const shared_ptr<GraphicsBuffer>& GetIndexBuffet() const;
	
	void SetVertexStream(uint32_t streamSlot,  const shared_ptr<GraphicsBuffer>& vertexBuffer, uint32_t offset, uint32_t stride );
	void SetInputLayout(const shared_ptr<VertexDeclaration>& decl);

	void SetVertexShader(const shared_ptr<VertexShader>& vs) { mVertexShaderStage->SetVertexShader(vs); }
	void SetPixelShader(const shared_ptr<PixelShader>& ps) { mPixelShaderStage->SetPixelShader(ps); }

	void Draw(PrimitiveType primitiveType, uint32_t vertexCount, uint32_t startVertexLocation);
	void DrawIndexed(PrimitiveType primitiveType, uint32_t indexCount, uint32_t startIndexLocation, int32_t baseVertexLocation);
	
	const shared_ptr<FrameBuffer>& GetCurrentFrameBuffer() const	{ return mCurrentFrameBuffer; } 
	void BindFrameBuffer(const shared_ptr<FrameBuffer>& fb);

	// Debug save screen
	void SaveScreenToPfm(const String& filename);

private:
	void SetInputLayout();

	/**
	 * 根据 Vertex format 从 Vertex Stream 中取顶点数据
	 */
	void DecodeVertexStream(VS_Input* oVSInput, uint32_t iVertex);

	VS_Output FetchVertex(uint32_t index);
	uint32_t FetchIndex(uint32_t index);


	ColorRGBA Sample(uint32_t texUint, uint32_t samplerUnit, float U, float V, float W);

public:
	RasterizerState RasterizerState;
	DepthStencilState DepthStencilState;
	BlendState BlendState;
	ColorRGBA CurrentBlendFactor;

	SamplerState SampleStates[MaxTextureUnits];
	shared_ptr<Texture> TextureUnits[MaxTextureUnits];

private:

	shared_ptr<GraphicsBuffer> mIndexBuffer;
	
	// Use index buffer
	bool mUseIndex;
	uint32_t mStartIndexLoc;
	uint32_t mBaseVertexLoc;

	struct VertexStream
	{
		shared_ptr<GraphicsBuffer> VertexBuffer;
		uint32_t	Offset;	///< Offset from the beginning of the vertex buffer in bytes.
		uint32_t	Stride;	///< Stride in bytes.
	} mVertexStreams[MaxVertexStreams];	///< The vertex streams; 

	shared_ptr<VertexDeclaration> mVertexDecl;

	shared_ptr<FrameBuffer> mCurrentFrameBuffer;
	shared_ptr<FrameBuffer> mScreenFrameBuffer;

	VertexShaderStage* mVertexShaderStage;
	PixelShaderStage* mPixelShaderStage;
	Rasterizer* mRasterizerStage;
};

#endif // Renderer_h__
