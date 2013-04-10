#ifndef SimpleShape_h__
#define SimpleShape_h__

#include "Shape.h"

namespace Purple {

class Sphere : public Shape
{
public:
	Sphere(const float44& o2w, bool ro, float radius, float minZ, float mazZ, float maxPhi);
	~Sphere();

	BoundingBoxf GetLocalBound() const;
	bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	bool IntersectP(const Ray& ray) const;
	float Area() const;

	float3 Sample(float u1, float u2, float u3,  float3* n) const;
	float3 Sample(const float3& pt, float u1, float u2, float u3,  float3* n) const;
	float Pdf(const float3& pt, const float3& wi) const;

public:
	float mRadius;
	float mMinZ, mMaxZ;
	float mMaxPhi, mMinTheta, mMaxTheta;
};

class Cylinder : public Shape 
{
public:
	Cylinder(const float44& o2w, bool ro, float radius, float minZ, float mazZ, float maxPhi);
	~Cylinder();

	BoundingBoxf GetLocalBound() const;
	bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	bool IntersectP(const Ray& ray) const;
	float Area() const;

	float3 Sample(float u1, float u2, float u3,  float3* n) const;

public:
	float mRadius;
	float mMinZ, mMaxZ, mMaxPhi;

};


}

#endif // SimpleShape_h__


