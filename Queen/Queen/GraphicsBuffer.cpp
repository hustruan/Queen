#include "GraphicsBuffer.h"


GraphicsBuffer::GraphicsBuffer(uint32_t bufferSize)
{
	mBufferData.resize(bufferSize);
}


GraphicsBuffer::~GraphicsBuffer(void)
{
}

void* GraphicsBuffer::Map( uint32_t offset, uint32_t length, BufferAccess options )
{
	const size_t bufferLength = mBufferData.size();

	if (offset > bufferLength)
		return NULL;

	if (offset + length > bufferLength)
		return NULL;

	return (void*)(&mBufferData[offset]);
}

void GraphicsBuffer::UnMap()
{

}
