#ifndef _VertexDeclaration__H
#define _VertexDeclaration__H

#include "Prerequisite.h"
#include "GraphicCommon.h"

struct VertexElement
{
public:
	VertexElement() {}
	VertexElement(uint32_t stream, uint32_t offset, VertexElementFormat theType, VertexElementUsage semantic, uint16_t index = 0)
	: Offset(offset), Stream(stream), Type(theType), Usage(semantic), UsageIndex(index)
	{

	}

public:
	uint32_t Stream;
	uint32_t Offset;
	VertexElementFormat Type;
	VertexElementUsage Usage;
	uint16_t UsageIndex;
};

typedef std::vector<VertexElement> VertexElementList;

class VertexDeclaration
{

public:
	VertexDeclaration() { }
	VertexDeclaration(const std::vector<VertexElement>& elements);
	VertexDeclaration(const VertexElement* elements, uint32_t count);

	template <typename InputIterator >
	VertexDeclaration(InputIterator  first, InputIterator  last)
	{
		AssignVertexElements(first, last);
	}

	uint32_t GetElementCount(void) { return static_cast<uint32_t>(mElementList.size()); }

	const VertexElementList& GetElements(void) const { return mElementList; }

	const VertexElement& GetElement(uint32_t index)	{ return mElementList.at(index); }

	uint32_t GetVertexSize() const;
	uint32_t GetVertexStreams() const { return mStreams; }

protected:

	template <typename InputIterator >
	void AssignVertexElements(InputIterator  first, InputIterator  last)
	{
		mElementList.assign(first, last);
		auto iter  =std::max_element(mElementList.begin(), mElementList.end(), [&](const VertexElement& e1, const VertexElement& e2)
					{
						return e1.Stream < e2.Stream;
					});

		mStreams = iter->Stream;

	}

	VertexElementList mElementList;
	uint32_t mStreams;
};

#endif // _VertexDeclaration__H
