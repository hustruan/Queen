#ifndef Shape_h__
#define Shape_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

struct DifferentialGeometry
{
	DifferentialGeometry()
	{
		dudx = dvdx = dudy = dvdy = 0.0f;
		Shape = NULL; 
	}

	DifferentialGeometry(const float3 &P, const float3 &DPDU, const float3 &DPDV, const float3 &DNDU, const float3 &DNDV, const float2& uv, const Shape *sh)
		: Point(P), dpdv(DPDV), dpdu(DPDU), dndv(DNDV), dndu(DNDU), UV(uv)
	{
		Normal = Normalize(Normalize(Cross(dpdu, dpdv)));
		
		Shape = sh;
		dudx = dvdx = dudy = dvdy = 0;
	}

	const Shape* Shape;

	float3 Point;
	float3 Normal;
	float2 UV;
	
	float3 dpdu, dpdv, dndu, dndv;
	float dudx, dvdx, dudy, dvdy;
};

class Mesh;

class Shape 
{
public:
	Shape(const float44& o2w, bool ro);
	virtual ~Shape(void);

	virtual BoundingBoxf GetLocalBound() const = 0;
	virtual BoundingBoxf GetWorldBound() const;

	virtual const Mesh* GetTriangleMesh() const { return NULL; }

	virtual bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	virtual bool IntersectP(const Ray& ray) const;

	virtual float Area() const;

	virtual float3 Sample(float u1, float u2, float3* n) const;
	virtual float Pdf(const float3& pt) const;

	/**
	 * @brief Only sample points from those should be integral. eg. for area light, only the portion visible to lit 
	 *        point should be sampled. The default implementation doesn't consider the additional point, just call
	 *        this original sample method.
	 */
	virtual float3 Sample(const float3& pt, float u1, float u2, float3* n) const;
	virtual float Pdf(const float3& pt, const float3& wi) const;

protected:
	float44 mLocalToWorld, mWorldToLocal;
	bool mReverseOrientation;
};

}

#endif // Shape_h__

