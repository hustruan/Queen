#include "Mesh.h"
#include "MentoCarlo.h"
#include "DifferentialGeometry.h"
#include <MathUtil.hpp>

namespace Purple {

using namespace RxLib;

class Triangle
{
public:
	bool Intersect(const Mesh* parentMesh, const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const
	{
		const float3& p1 = parentMesh->mPositions[mIdx[0]];
		const float3& p2 = parentMesh->mPositions[mIdx[1]];
		const float3& p3 = parentMesh->mPositions[mIdx[2]];
		
		float3 e1 = p2 - p1;
		float3 e2 = p3 - p1;
		float3 s1 = Cross(ray.Direction, e2);
		float divisor = Dot(s1, e1);

		if (divisor == 0.)
			return false;
		float invDivisor = 1.f / divisor;

		// Compute first barycentric coordinate
		float3 d = ray.Origin - p1;
		float b1 = Dot(d, s1) * invDivisor;
		if (b1 < 0. || b1 > 1.)
			return false;
		
		// Compute second barycentric coordinate
		float3 s2 = Cross(d, e1);
		float b2 = Dot(ray.Direction, s2) * invDivisor;
		if (b2 < 0.0f || b1 + b2 > 1.0f)
			return false;
		
		// Compute _t_ to intersection point
		float t = Dot(e2, s2) * invDivisor;
		if (t < ray.tMin || t > ray.tMax)
			return false;

		// Compute triangle partial derivatives
		float3 dpdu, dpdv;
			
		float2 uvs[3];
		if (parentMesh->mTexcoords) 
		{
			uvs[0] = parentMesh->mTexcoords[mIdx[0]];
			uvs[1] = parentMesh->mTexcoords[mIdx[1]];
			uvs[2] = parentMesh->mTexcoords[mIdx[2]];
		} 
		else 
		{
			uvs[0] = float2(0.0f, 0.0f);
			uvs[1] = float2(1.0f, 0.0f);
			uvs[2] = float2(1.0f, 1.0f);
		}

		// Compute deltas for triangle partial derivatives
		float du1 = uvs[0][0] - uvs[2][0];
		float du2 = uvs[1][0] - uvs[2][0];
		float dv1 = uvs[0][1] - uvs[2][1];
		float dv2 = uvs[1][1] - uvs[2][1];
		float3 dp1 = p1 - p3, dp2 = p2 - p3;
		float determinant = du1 * dv2 - dv1 * du2;
		if (determinant == 0.f) 
		{
			// Handle zero determinant for triangle partial derivative matrix
			CoordinateSystem(Normalize(Cross(e2, e1)), &dpdu, &dpdv);
		}
		else
		{
			float invdet = 1.f / determinant;
			dpdu = ( dv2 * dp1 - dv1 * dp2) * invdet;
			dpdv = (-du2 * dp1 + du1 * dp2) * invdet;
		}

		// Interpolate uv
		float b0 = 1 - b1 - b2;
		float2 tuv = b0*uvs[0] + b1*uvs[1] + b2*uvs[2];

		*diffGeoHit = DifferentialGeometry(ray.Eval(t), dpdu, dpdv, float3(0,0,0), float3(0,0,0), tuv, parentMesh);
		*tHit = t;

		return true;
	}

	bool IntersectP(const Mesh* parentMesh, const Ray& ray) const
	{
		const float3& p1 = parentMesh->mPositions[mIdx[0]];
		const float3& p2 = parentMesh->mPositions[mIdx[1]];
		const float3& p3 = parentMesh->mPositions[mIdx[2]];

		float3 e1 = p2 - p1;
		float3 e2 = p3 - p1;
		float3 s1 = Cross(ray.Direction, e2);
		float divisor = Dot(s1, e1);

		if (divisor == 0.)
			return false;
		float invDivisor = 1.f / divisor;

		// Compute first barycentric coordinate
		float3 d = ray.Origin - p1;
		float b1 = Dot(d, s1) * invDivisor;
		if (b1 < 0. || b1 > 1.)
			return false;

		// Compute second barycentric coordinate
		float3 s2 = Cross(d, e1);
		float b2 = Dot(ray.Direction, s2) * invDivisor;
		if (b2 < 0.0f || b1 + b2 > 1.0f)
			return false;

		// Compute _t_ to intersection point
		float t = Dot(e2, s2) * invDivisor;
		if (t < ray.tMin || t > ray.tMax)
			return false;

		return true;
	}

	void GetShadingGeometry(const Mesh* parentMesh, const float44& obj2world, const DifferentialGeometry &dg, DifferentialGeometry *dgShading) const
	{

	}

	float Area(const Mesh* parentMesh) const
	{
		const float3& p0 = parentMesh->mPositions[mIdx[0]];
		const float3& p1 = parentMesh->mPositions[mIdx[1]];
		const float3& p2 = parentMesh->mPositions[mIdx[2]];

		float3 sideA = p1 - p0, sideB = p2 - p0;
		return 0.5f * Length(Cross(sideA, sideB));
	}

	float3 Sample(const Mesh* parentMesh, float u1, float u2, float3* n) const
	{
		float b1, b2;
		UniformSampleTriangle(u1, u2, &b1, &b2);
		
		const float3& p1 = parentMesh->mPositions[mIdx[0]];
		const float3& p2 = parentMesh->mPositions[mIdx[1]];
		const float3& p3 = parentMesh->mPositions[mIdx[2]];

		float3 p = b1 * p1 + b2 * p2 + (1.f - b1 - b2) * p3;
		*n = Normalize(Cross(p2-p1, p3-p1));
		return p;
	}

	BoundingBoxf GetBoundingBox(const Mesh* parentMesh) const
	{
		const float3& p1 = parentMesh->mPositions[mIdx[0]];
		const float3& p2 = parentMesh->mPositions[mIdx[1]];
		const float3& p3 = parentMesh->mPositions[mIdx[2]];

		BoundingBoxf retVal(p1, p2);
		retVal.Merge(p3);

		return retVal;
	}

	

public:
	uint32_t mIdx[3];
};

//--------------------------------------------------------------------	
Mesh::Mesh( const float44& o2w, bool ro, int32_t numTriangle, int32_t numVertices, const uint32_t* indices, const float3* positions, const float3* normals, const float3* tangents, const float3* texcoords )
: Shape(o2w, ro), mNumTriangles(numTriangle), mNumVertices(numVertices), mSurfaceArea(-1)
{
	mTriangles = new Triangle[mNumTriangles];
	memcpy(reinterpret_cast<uint32_t *>(mTriangles), indices, mNumTriangles * 3 * sizeof(uint32_t));
	/*for (int32_t tri = 0; tri < numTriangle; ++tri)
	{
	mTriangles[tri].mIdx[0] =  indices[tri*3+0];
	mTriangles[tri].mIdx[0] =  indices[tri*3+0];
	mTriangles[tri].mIdx[0] =  indices[tri*3+0];
	}*/
	
	if (texcoords)
	{
		mTexcoords = new float2[numVertices];
		memcpy(mTexcoords, texcoords, numVertices*sizeof(float2));
	}
	else 
		mTexcoords = NULL;

	if (tangents)
	{
		mTangents = new float3[numVertices];
		memcpy(mNormals, tangents, numVertices*sizeof(float3));
	}
	else
		mTangents = NULL;
	
	if (normals) 
	{
		mNormals= new float3[numVertices];
		memcpy(mNormals, normals, numVertices*sizeof(float3));
	}
	else 
		mNormals = NULL;

	/*if (S) {
		s = new Vector[nverts];
		memcpy(s, S, nverts*sizeof(Vector));
	}
	else s = NULL;*/

	// Transform mesh vertices to world space
	mPositions = new float3[numVertices];
	for (int i = 0; i < numVertices; ++i)
	{
		mLocalBound.Merge(positions[i]);
		mPositions[i] = Transform(positions[i], o2w);
	}
}


Mesh::~Mesh(void)
{
	delete[] mTriangles;
	delete[] mPositions;
	delete[] mTexcoords;
	delete[] mNormals;
	delete mAreaDistrib;
}


void Mesh::PrepareSampleDistribution()
{
	if (mSurfaceArea < 0)
	{
		mSurfaceArea = 0;
		std::vector<float> areas(mNumTriangles);
		for (int32_t i = 0; i < mNumTriangles; ++i)
		{
			areas[i] = mTriangles[i].Area(this);
			mSurfaceArea += areas[i];
		}
		mAreaDistrib = new Distribution1D(&areas[0], mNumTriangles);
	}
}

float Mesh::Area() const
{
	if (mSurfaceArea < 0)
		const_cast<Mesh*>(this)->PrepareSampleDistribution();  // remove const restrict
	
	return mSurfaceArea;
}

float3 Mesh::Sample( float u1, float u2, float u3,  float3* n ) const
{
	// use u2 to sample triangle index
	int32_t index = mAreaDistrib->SampleDiscrete(u3, NULL);
	float3 pt = mTriangles[index].Sample(this, u1, u2, n);

	if (mReverseOrientation)
		*n *= -1;

	return pt;
}

float3 Mesh::Sample( const float3& p, float u1, float u2, float u3,  float3* n ) const
{
	int32_t index = mAreaDistrib->SampleDiscrete(u3, NULL);
	float3 pt = mTriangles[index].Sample(this, u1, u2, n);

	// Find closest intersection of ray with triangles 
	Ray r(p, pt-p, 1e-3f, Mathf::INFINITY);
	float thit = 1.f;
	bool anyHit = false;
	DifferentialGeometry dg;
	for (int32_t i = 0; i < mNumTriangles; ++i)
		anyHit |= mTriangles[i].Intersect(this, r, &thit, &dg);
	if (anyHit) *n = dg.Normal;
	return r.Eval(thit);
}

bool Mesh::Intersect( uint32_t index, const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	return mTriangles[index].Intersect(this, ray, tHit, diffGeoHit);
}

bool Mesh::IntersectP( uint32_t index, const Ray& ray ) const
{
	return mTriangles[index].IntersectP(this, ray);
}

BoundingBoxf Mesh::GetWorldBound( uint32_t index ) const
{
	return mTriangles[index].GetBoundingBox(this);
}

void Mesh::GetShadingGeometry( const float44& local2world, const DifferentialGeometry &dg, DifferentialGeometry *dgShading ) const
{
	/*	if (mNormals )
		{
		}*/
}

}