#ifndef Mesh_h__
#define Mesh_h__

#include "Prerequisites.h"
#include "Shape.h"

namespace Purple {

class Triangle;
struct Distribution1D;

class Mesh : public Shape
{
public:
	Mesh(const float44& o2w, bool ro, int32_t numTriangle, int32_t numVertices, 
		const uint32_t* indices, const float3* positions, const float3* normals, const float3* tangent, const float2* texcoords);
	~Mesh(void);

	const Mesh* GetTriangleMesh() const { return this; }

	const float3* GetVertexPositions() const { return mPositions; }
	const float2* GetVertexTexCoords() const { return mTexcoords; }
	const float3* GetVertexNormals() const   { return mNormals; }

	uint32_t GetTriangleCount() const        { return mNumTriangles; }

	BoundingBoxf GetLocalBound() const       { return mLocalBound; }

	BoundingBoxf GetWorldBound(uint32_t index) const;

	bool Intersect(uint32_t index, const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	bool IntersectP(uint32_t index, const Ray& ray) const;

	void GetShadingGeometry(const float44& local2world, const DifferentialGeometry &dg, DifferentialGeometry *dgShading) const;

	float Area() const;

	float3 Sample(float u1, float u2, float u3,  float3* n) const;
	float3 Sample(const float3& pt, float u1, float u2, float u3,  float3* n) const;

protected:
	void PrepareSampleDistribution();

private:
	int32_t mNumTriangles;
	int32_t mNumVertices;

	Triangle* mTriangles;

	float3* mPositions;
	float3* mNormals;
	float3* mTangents;
	float2* mTexcoords;

	BoundingBoxf mLocalBound;

	float mSurfaceArea;;

	Distribution1D* mAreaDistrib;

	friend class Triangle;
};


}

#endif // Mesh_h__


