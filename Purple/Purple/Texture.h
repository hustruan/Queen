#ifndef Texture_h__
#define Texture_h__

#include "Prerequisites.h"

namespace Purple {

template <typename T> 
class Texture 
{
public:
	// Texture Interface
	virtual T Evaluate(const DifferentialGeometry &) const = 0;
	virtual ~Texture() { }
};

template <typename T> 
class ConstantTexture : public Texture<T>
{
public:
	ConstantTexture(const T &v) { value = v; }
	T Evaluate(const DifferentialGeometry &) const { return value;}

private:
	T value;
};


}



#endif // Texture_h__

