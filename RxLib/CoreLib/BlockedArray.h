#ifndef BlockedArray_h__
#define BlockedArray_h__

#include "aligned_allocator.h"

#define CACHE_LINE_SIZE 64

namespace RxLib {

template<typename T, int logBlockSize>
class BlockedArray
{
	enum { BlockSize = (1 << logBlockSize) };

public:
	BlockedArray(uint32_t nu, uint32_t nv, const T* d = nullptr)
	{
		uRes = nu; 
		vRes = nv;
		uBlocks = RoundUp(nu) >> logBlockSize;
		
		uint32_t nAlloc = RoundUp(nu) * RoundUp(nv);
		data = (T*)aligned_malloc(nAlloc * sizeof(T), CACHE_LINE_SIZE);

		for (uint32_t i = 0; i < nAlloc; ++i)
			new (&data[i]) T();

		if (d)
		{
			for (uint32_t v = 0; v < vRes; ++v)
				for (uint32_t u = 0; u < uRes; ++u)
					(*this)(u, v) = d[v * uRes + u];
		}
	}

	BlockedArray()
	{
		for (uint32_t i = 0; i < uRes * vRes; ++i)
			data[i].~T();
		aligned_free(data);
	}

	uint32_t uSize() const { return uRes; }
	uint32_t vSize() const { return vRes; }


	T& operator()(uint32_t u, uint32_t v) 
	{
		uint32_t ub = Block(u);
		uint32_t vb = Block(v);
		uint32_t uOffset = Offset(u);
		uint32_t vOffset = Offset(v);
		
		uint32_t offset = (vb * uBlocks + ub) * BlockSize * BlockSize +
						  vOffset * BlockSize + uOffset;

		return data[offset];
	}

	const T& operator()(uint32_t u, uint32_t v)  const 
	{
		uint32_t ub = Block(u);
		uint32_t vb = Block(v);
		uint32_t uOffset = Offset(u);
		uint32_t vOffset = Offset(v);

		uint32_t offset = (vb * uBlocks + ub) * BlockSize * BlockSize +
			vOffset * BlockSize + uOffset;

		return data[offset];
	}


private:
	uint32_t RoundUp(uint32_t val) const { return (val + BlockSize - 1) & ~(BlockSize - 1); }
	uint32_t Block(uint32_t val) const { return val >> logBlockSize; }
	uint32_t Offset(uint16_t val) const { return val & (BlockSize - 1); }

private:
	uint32_t uRes, vRes;
	uint32_t uBlocks;

	T* data;
};

}

#endif // BlockedArray_h__
