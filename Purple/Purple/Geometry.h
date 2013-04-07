#ifndef Geometry_h__
#define Geometry_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

struct DifferentialGeometry
{
	const Shape* Geometry;

	float3 Point;
	float3 Normal;
	float2 UV;
	
	float3 dpdu, dpdv, dndu, dndv;
	float dudx, dvdx, dudy, dvdy;
};

class Shape 
{
public:
	Shape(const float44& o2w, bool ro);
	virtual ~Shape(void);

	virtual BoundingBoxf GetLocalBound() const = 0;
	virtual BoundingBoxf GetWorldBound() const;

	virtual bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	virtual bool IntersectP(const Ray& ray) const;

	virtual float Area() const;

	virtual float3 Sample(float u1, float u2, float3* n) const;
	virtual float Pdf(const float3& pt) const	{ return 1.0f / Area(); }

	/**
	 * @brief Only sample points from those should be integral. eg. for area light, only the portion visible to lit 
	 *        point should be sampled. The default implementation doesn't consider the additional point, just call
	 *        this original sample method.
	 */
	virtual float3 Sample(const float3& pt, float u1, float u2, float3* n) const { return Sample(u1, u2, n); }
	virtual float Pdf(const float3& pt, const float3& wi) const;

protected:
	float44 mLocalToWorld, mWorldToLocal;
	bool mReverseOrientation;
};

}

#endif // Geometry_h__

