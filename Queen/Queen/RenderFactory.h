#ifndef RenderFactory_h__
#define RenderFactory_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"
#include "VertexDeclaration.h"
#include "PixelFormat.h"

class RenderFactory
{
public:
	RenderFactory(void);
	~RenderFactory(void);

	shared_ptr<VertexDeclaration> CreateVertexDeclaration(const std::vector<VertexElement>& elems);
	shared_ptr<VertexDeclaration> CreateVertexDeclaration(VertexElement* elems, uint32_t count);

	template<typename Iterator>
	shared_ptr<VertexDeclaration> CreateVertexDeclaration(Iterator start, Iterator end)
	{
		return std::make_shared<VertexDeclaration>(start, end);
	}

	shared_ptr<GraphicsBuffer> CreateVertexBuffer(ElementInitData* initData);
	shared_ptr<GraphicsBuffer> CreateIndexBuffer(ElementInitData* initData);

	shared_ptr<Texture> CreateTextureFromFile(const std::string&  file);
};

void ExportToPfm(const std::string& filename, uint32_t width, uint32_t height, PixelFormat format, void* data);

#endif // RenderFactory_h__
