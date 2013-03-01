#ifndef _Singleton__H
#define _Singleton__H

#include <assert.h>

namespace RxLib {

template <typename T> 
class Singleton
{
private:
	Singleton(const Singleton<T> &);
	Singleton& operator=(const Singleton<T> &);

protected:
	static T* ms_Singleton;

public:
	Singleton( void )
	{
		assert( !ms_Singleton );
		ms_Singleton = static_cast< T* >( this );
	}

	virtual ~Singleton( void )
	{  
		assert( ms_Singleton ); 
		ms_Singleton = 0; 
	}

	static void Initialize()
	{
		new T();
	}

	static void Finalize()
	{
		delete ms_Singleton;
	}

	static T& GetSingleton( void )
	{	
		assert( ms_Singleton ); 
		return ( *ms_Singleton ); 
	}

	static T* GetSingletonPtr( void )
	{ 
		return ms_Singleton;
	}

};

#define SINGLETON_DECL_HEADER(T) static T& GetSingleton( void ); static T* GetSingletonPtr( void );

#define SINGLETON_DECL(T) template<> T* Singleton<T>::ms_Singleton = 0;						 \
	T& T::GetSingleton( void ) { assert( ms_Singleton );  return ( *ms_Singleton );  }		     \
	T* T::GetSingletonPtr( void ) { return ms_Singleton; }  


}

#endif