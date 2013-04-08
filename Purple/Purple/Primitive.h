#ifndef Primitive_h__
#define Primitive_h__

#include "Prerequisites.h"
#include "Shape.h"

namespace Purple {

class Primitive;
class BSDF;

class Intersection
{
public:
	Intersection(void);
	~Intersection(void);
	
	BSDF* GetBSDF(const RayDifferential &ray) const;
	
	ColorRGB Le(const float3& wo) const;


public:
	const Primitive* Primitive;
	DifferentialGeometry DiffGeo;

};

class Primitive
{
public:
	Primitive(void);
	~Primitive(void);


protected:

};

}

#endif // Primitive_h__
