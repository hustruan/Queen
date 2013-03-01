#ifndef aligned_allocator_h__
#define aligned_allocator_h__

#include <limits>
#include <cstdint>

namespace RxLib {

template<typename T, int alignment>
class aligned_allocator
{
public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	enum { Alignment = alignment };

	template<class U>
	struct rebind
	{
		typedef aligned_allocator<U, alignment> other;
	};

	pointer address( reference value ) const
	{
		return &value;
	}

	aligned_allocator() {}

	aligned_allocator( const aligned_allocator<T, alignment>& ) {}

	template<class U, int alignment2>
	aligned_allocator( const aligned_allocator<U, alignment2>& )
	{
		return *this;
	}

	~aligned_allocator() { }

	size_type max_size() const
	{
		return (std::numeric_limits<size_type>::max)() / sizeof(T);
	}

	pointer allocate( size_type num, const void* hint = 0 )
	{
		uint8_t *pStart , *pAligned;

		pStart = (uint8_t*)malloc(num*sizeof(T) + (alignment-1) + sizeof(uint16_t));

		if(pStart == NULL)
			return NULL;

		pAligned = (uint8_t*)(((size_t)pStart + (alignment-1) + sizeof(uint16_t)) & ~(alignment-1));

		*((uint16_t*)pAligned -1) = uint16_t(pAligned - pStart); 

		return reinterpret_cast<pointer>(pAligned);
	}

	void deallocate(pointer p, size_type /*num*/)
	{
		uint16_t* p16 = (uint16_t*)p;
		uint8_t* pTrue = (uint8_t*)(p16) - *((uint16_t*)p16 -1);
		free(pTrue);
	}

	void construct( pointer p, const T& value )
	{
		::new( p ) T( value );
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	template<typename U, int alignment2>
	bool operator!=(const aligned_allocator<U, alignment2>& ) const
	{ 
		return false;
	}

	template<typename U, int alignment2>
	bool operator==(const aligned_allocator<U, alignment2>& ) const
	{ 
		return true;
	}
};


}



#endif // aligned_allocator_h__
