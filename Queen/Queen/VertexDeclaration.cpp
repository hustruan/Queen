#include "VertexDeclaration.h"

uint32_t GetTypeSize( VertexElementFormat etype )
{
	switch(etype)
	{
	case VEF_Float:
		return sizeof(float);

	case VEF_Float2:
		return sizeof(float)*2;

	case VEF_Float3:
		return sizeof(float)*3;

	case VEF_Float4:
		return sizeof(float)*4;

	case VEF_Int:
		return sizeof(int32_t);

	case VEF_Int2:
		return sizeof(int32_t)*2;

	case VEF_Int3:
		return sizeof(int32_t)*3;

	case VEF_Int4:
		return sizeof(int32_t)*4;

	case VEF_UInt:
		return sizeof(uint32_t);

	case VEF_UInt2:
		return sizeof(uint32_t)*2;

	case VEF_UInt3:
		return sizeof(uint32_t)*3;

	case VEF_UInt4:
		return sizeof(uint32_t)*4;

	case VEF_Bool:
		return sizeof(bool);

	case VEF_Bool2:
		return sizeof(bool)*2;

	case VEF_Bool3:
		return sizeof(bool)*3;

	case VEF_Bool4:
		return sizeof(bool)*4;
	}

	throw std::exception("Vertex Format Error");
}

uint16_t GetTypeCount( VertexElementFormat etype )
{
	switch(etype)
	{
	case VEF_Float:
	case VEF_Int:
	case VEF_UInt:
	case VEF_Bool:
		return 1;

	case VEF_Float2:
	case VEF_Int2:
	case VEF_UInt2:
	case VEF_Bool2:
		return 2;

	case VEF_UInt3:
	case VEF_Float3:
	case VEF_Int3:
	case VEF_Bool3:
		return 3;

	case VEF_Float4:
	case VEF_Int4:
	case VEF_Bool4:
	case VEF_UInt4:
		return 4;
	}

	throw std::exception("VertexElement::GetTypeCount");
}


VertexDeclaration::VertexDeclaration( const std::vector<VertexElement>& elements )
{
	AssignVertexElements(elements.begin(), elements.end());
}

VertexDeclaration::VertexDeclaration( const VertexElement* elements, uint32_t count )
{
	AssignVertexElements(elements, elements + count);
}

uint32_t VertexDeclaration::GetVertexSize() const
{
	uint32_t size = 0;
	VertexElementList::const_iterator ei, eiend;
	eiend = mElementList.end();
	for (ei = mElementList.begin(); ei != eiend; ++ei)
	{
		size += GetTypeSize(ei->Type);
	}

	return size;
}


