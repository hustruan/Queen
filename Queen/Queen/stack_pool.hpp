#ifndef stack_pool_h__
#define stack_pool_h__

#include <cassert>

template<typename ObjectT, int MaxCount>
class stack_pool
{
public:
	stack_pool()
	{
		memset(used, 0, sizeof(used));
	}
	
	ObjectT* malloc()
	{
		for (size_t i = 0; i < MaxCount; ++i)
		{
			if (!used[i])
			{
				used[i] = true;
				return &elements[i];
			}
		}

		return NULL;
	}

	void free(ObjectT* p)
	{
		assert(p != NULL);

		uintptr_t diff = (uintptr_t)p - (uintptr_t)elements;
		
		if ( 0 <= diff && diff <= sizeof(elements))
		{
			used[diff / ObjectSize] = false;
		}
	}

protected:

	enum { ObjectSize = sizeof(ObjectT) };

	ObjectT elements[MaxCount];
	bool used[MaxCount];
};



#endif // stack_pool_h__
