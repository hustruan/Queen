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

//class Sphere : public Shape
//{
//public:
//	Sphere(const float44& o2w, bool ro, float radius);
//	~Sphere();
//
//	BoundingBoxf GetLocalBound() const;
//	bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
//	bool IntersectP(const Ray& ray) const;
//	float Area() const;
//
//	float3 Sample(float u1, float u2, float u3,  float3* n) const;
//	float3 Sample(const float3& pt, float u1, float u2, float u3,  float3* n) const;
//	float Pdf(const float3& pt, const float3& wi) const;
//
//public:
//	float mRadius;
//};

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

class Disk : public Shape
{
public:
	Disk(const float44& o2w, bool ro, float height, float radius, float innerRadius, float phiMax);
	~Disk();

	BoundingBoxf GetLocalBound() const;
	bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	bool IntersectP(const Ray &ray) const;
	float Area() const;
	float3 Sample(float u1, float u2, float3* n) const;

private:
	// Disk Private Data
	float mHeight, mRadius, mInnerRadius, mMaxPhi;
};

}

#endif // SimpleShape_h__


