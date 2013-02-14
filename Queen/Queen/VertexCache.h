#ifndef VertexCache_h__
#define VertexCache_h__

#include "Prerequisite.h"
#include "Shader.h"

#include <thread>
#include <atomic>

#define VertexCacheSize 32

// 
//class VertexCache
//{
//private:
//	struct VertexCacheEntry
//	{
//		uint32_t VertexIndex;
//		VS_Output VertexOutput;
//		uint32_t FetchTime;
//	};
//
//public:
//	VertexCache(void);
//	~VertexCache(void);
//
//	VS_Output& FetchVertex(uint32_t index);
//
//
//private:
//	std::array<VertexCacheEntry, VertexCacheSize> mVertexCache;
//};


#endif // VertexCache_h__


