#ifndef Surface_h__
#define Surface_h__

#include "Prerequisite.h"
#include "PixelFormat.h"

class Surface
{
public:
	Surface(uint32_t width, uint32_t height, PixelFormat format);
	~Surface(void);


protected:
	std::vector<uint8_t> mBufferData;
};

#endif // Surface_h__
