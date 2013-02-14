#include "RenderFactory.h"
#include "GraphicsBuffer.h"

RenderFactory::RenderFactory(void)
{
}


RenderFactory::~RenderFactory(void)
{
}

shared_ptr<GraphicsBuffer> RenderFactory::CreateVertexBuffer( ElementInitData* initData )
{
	assert(initData && initData->pData && initData->RowPitch > 0);
	
	shared_ptr<GraphicsBuffer> retVal =  std::make_shared<GraphicsBuffer>(initData->RowPitch);

	void* buffer = retVal->Map(0, initData->RowPitch, BA_Write_Only);
	memcpy(buffer, initData->pData, initData->RowPitch);
	retVal->UnMap();

	return retVal;
}

shared_ptr<GraphicsBuffer> RenderFactory::CreateIndexBuffer( ElementInitData* initData )
{
	ASSERT(initData && initData->pData && initData->RowPitch > 0);
	shared_ptr<GraphicsBuffer> retVal =  std::make_shared<GraphicsBuffer>(initData->RowPitch);

	void* buffer = retVal->Map(0, initData->RowPitch, BA_Write_Only);
	memcpy(buffer, initData->pData, initData->RowPitch);
	retVal->UnMap();

	return retVal;
}

shared_ptr<VertexDeclaration> RenderFactory::CreateVertexDeclaration( const std::vector<VertexElement>& elems )
{
	return std::make_shared<VertexDeclaration>(elems);
}

shared_ptr<VertexDeclaration> RenderFactory::CreateVertexDeclaration( VertexElement* elems, uint32_t count )
{
	return std::make_shared<VertexDeclaration>(elems, count);
}
