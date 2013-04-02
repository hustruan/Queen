#ifndef Geometry_h__
#define Geometry_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

struct DifferentialGeometry
{
	const Geometry* Geometry;

	float3 Point;
	float3 Normal;
	float2 UV;
	
	float3 dpdu, dpdv, dndu, dndv;
	float dudx, dvdx, dudy, dvdy;
};

class Geometry 
{
public:
	Geometry(void);
	virtual ~Geometry(void);

	virtual BoundingBoxf GetLocalBound() const = 0;
	virtual BoundingBoxf GetWorldBound() const;

	virtual bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const = 0;
	virtual bool FastIntersect(const Ray& ray);

	virtual float Area() const = 0;

protected:
	float44 mLocalToWorld, mWorldToLocal;
};

}

#endif // Geometry_h__

