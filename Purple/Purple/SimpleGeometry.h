#ifndef SimpleGeometry_h__
#define SimpleGeometry_h__

#include "Geometry.h"

namespace Purple {

class Sphere : public Geometry
{
public:
	Sphere(float radius);
	~Sphere(void);

	virtual BoundingBoxf GetLocalBound() const;
	virtual bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	virtual float Area() const;

public:
	float Radius;
};

class TriMesh;

class Triangle : public Geometry
{
public:
	Triangle(TriMesh& mesh, uint32_t* index);
	~Triangle();

	virtual BoundingBoxf GetLocalBound() const;
	virtual bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	virtual float Area() const;

private:
	TriMesh& mMesh;
	uint32_t* mIndex;
};

class TriMesh : public Geometry
{
public:
	TriMesh();
	~TriMesh();

protected:
	vector<float3> mPositions;
	vector<float3> mNormals;
	vector<float2> mTexcoords;
	vector<uint32_t> mIndices;

	friend class Triangle;
};

}

#endif // SimpleGeometry_h__


