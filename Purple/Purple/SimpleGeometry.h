#ifndef SimpleGeometry_h__
#define SimpleGeometry_h__

#include "Geometry.h"

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

	float3 Sample(float u1, float u2, float3* n) const;
	float3 Sample(const float3& pt, float u1, float u2, float3* n) const;
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

	float3 Sample(float u1, float u2, float3* n) const;

public:
	float mRadius;
	float mMinZ, mMaxZ, mMaxPhi;

};

//class Mesh;
//
//class Triangle : public Shape
//{
//public:
//	Triangle(Mesh& mesh, uint32_t* index);
//	~Triangle();
//
//	virtual BoundingBoxf GetLocalBound() const;
//	virtual bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
//	virtual float Area() const;
//
//private:
//	Mesh& mMesh;
//	uint32_t* mIndex;
//};
//
//class Mesh : public Shape
//{
//public:
//	Mesh();
//	~Mesh();
//
//protected:
//	vector<float3> mPositions;
//	vector<float3> mNormals;
//	vector<float2> mTexcoords;
//	vector<uint32_t> mIndices;
//
//	friend class Triangle;
//};

}

#endif // SimpleGeometry_h__


