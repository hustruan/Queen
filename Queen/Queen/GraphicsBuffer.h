#ifndef GraphicBuffer_h__
#define GraphicBuffer_h__

#include "Prerequisite.h"
#include "GraphicCommon.h"

class GraphicsBuffer
{
public:
	GraphicsBuffer(uint32_t bufferSize);
	~GraphicsBuffer(void);

	uint32_t GetBufferSize() const { return mBufferData.size(); }

	void* Map(uint32_t offset, uint32_t length, BufferAccess options);
	void UnMap();

private:
	std::vector<uint8_t> mBufferData;
};

#endif // GraphicBuffer_h__
