#ifndef RenderFactory_h__
#define RenderFactory_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"
#include "VertexDeclaration.h"

class RenderFactory
{
public:
	RenderFactory(void);
	~RenderFactory(void);

	shared_ptr<VertexDeclaration> CreateVertexDeclaration(const std::vector<VertexElement>& elems);
	shared_ptr<VertexDeclaration> CreateVertexDeclaration(VertexElement* elems, uint32_t count);

	shared_ptr<GraphicsBuffer> CreateVertexBuffer(ElementInitData* initData);
	shared_ptr<GraphicsBuffer> CreateIndexBuffer(ElementInitData* initData);
};

#endif // RenderFactory_h__
