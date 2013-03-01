#include "SimpleGeometry.h"
#include <Math.hpp>
#include <MathUtil.hpp>
#include <SoveEquation.hpp>

namespace Purple {

using namespace RxLib;

Sphere::Sphere( float radius )
	: Radius(radius)
{

}


Sphere::~Sphere(void)
{
}

BoundingBoxf Sphere::GetLocalBound() const
{
	return BoundingBoxf( float3(-Radius, -Radius, -Radius), float3(-Radius, -Radius, -Radius) );
}

bool Sphere::Intersect( const Ray& r, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	// Transform ray into local frame
	Ray ray(r);
	ray.Origin = Transform(r.Origin, mWorldToLocal);
	ray.Direction = TransformNormal(r.Direction, mWorldToLocal);

	float A = Dot(ray.Direction, ray.Direction);
	float B = 2.0f * Dot(ray.Origin, ray.Direction);
	float C = Dot(ray.Origin, ray.Origin) - Radius*Radius;

	float t0, t1;
	if( !SoveQuadratic(A, B, C, &t0, &t1) )
		return false;

	if (t0 > ray.tMax || t1 < ray.tMin)
		return false;

	float thit = t0;
	if (thit < ray.tMin)
	{
		thit = t1;
		if (thit > ray.tMax )
			return false;
	}

	float3 phit = ray.Eval(thit);
	float3 N = Normalize(phit);

	if (phit.X() == 0.f && phit.Y() == 0.f) 
		phit.X() = 1e-5f * Radius;

	float phi = atan2f(phit.Y(), phit.X());
	if (phi < 0.)
		phi += 2.f * Mathf::PI;

	// Find parametric representation of sphere hit
	float u = phi / Mathf::TWO_PI;
	float theta = acosf(Clamp(phit.Z() / Radius, -1.f, 1.f));
	float v = theta / Mathf::PI;

	// Compute sphere dpdu and dpdv
	float zradius = sqrtf(phit.X()*phit.X() + phit.Y()*phit.Y());
	float invzradius = 1.f / zradius;
	float cosphi = phit.X() * invzradius;
	float sinphi = phit.Y() * invzradius;

    float3 dpdu(-Mathf::TWO_PI * phit.Y(), Mathf::TWO_PI * phit.X(), 0);
	float3 dpdv = Mathf::PI * float3(phit.Z() * cosphi, phit.Z() * sinphi, -Radius * sinf(theta));

	float3 d2Pduu = -Mathf::TWO_PI * Mathf::TWO_PI * float3(phit.X(), phit.Y(), 0);
	float3 d2Pduv = Mathf::PI * phit.Z() * Mathf::TWO_PI * float3(-sinphi, cosphi, 0.);
	float3 d2Pdvv = -Mathf::PI * Mathf::PI * float3(phit.X(), phit.Y(), phit.Z());

	const float E = Dot(dpdu, dpdu);
	const float F = Dot(dpdu, dpdv);
	const float G = Dot(dpdv, dpdv);
	const float e = Dot(N, d2Pduu);
	const float f = Dot(N, d2Pduv);
	const float g = Dot(N, d2Pdvv);

	const float invEGF2 = 1.f / (E*G - F*F);

	float3 dndu((f*F - e*G) * invEGF2 * dpdu + (e*F - f*E) * invEGF2 * dpdv);
	float3 dndv((g*F - f*G) * invEGF2 * dpdu + (f*F - g*E) * invEGF2 * dpdv);

	diffGeoHit->dpdu = TransformNormal(dpdu, mWorldToLocal);
	diffGeoHit->dpdv = TransformNormal(dpdu, mWorldToLocal);
	diffGeoHit->dndu = TransformNormal(dndu, mWorldToLocal);
	diffGeoHit->dndv = TransformNormal(dndv, mWorldToLocal);
	diffGeoHit->Normal = TransformNormal(N, mWorldToLocal);
	diffGeoHit->Point = Transform(phit, mWorldToLocal);;
	diffGeoHit->UV = float2(u, v);
	diffGeoHit->Geometry = this;

	*tHit = thit;
	return true;
}

float Sphere::Area() const
{
	return 4.0f * Mathf::PI * Radius * Radius;
}


//----------------------------------------------------------------------------
Triangle::Triangle( TriMesh& mesh, uint32_t* index )
	: mMesh(mesh), mIndex(index)
{
}

Triangle::~Triangle()
{

}

bool Triangle::Intersect( const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	const float3& p0 = mMesh.mPositions[mIndex[0]];
	const float3& p1 = mMesh.mPositions[mIndex[1]];
	const float3& p2 = mMesh.mPositions[mIndex[2]];

	float3 e1 = p1 - p0;
	float3 e2 = p2 - p0;
	
	float3 s1 = Cross(ray.Direction, e2);

	float divisor = Dot(s1, e1);

	if (divisor == 0)
		return false;

	float invDivisor = 1.0f / divisor;

	float3 s = ray.Origin - p0;
	float b1 = Dot(s1, s) * invDivisor;
	if (b1 < 0.0f || b1 > 1.0f)
		return false;

	float3 s2 = Cross(s, e1);
	float b2 = Dot(s2, ray.Direction) * invDivisor;

	if (b1 < 0.0f || b1 + b2 > 1.0f)
		return false;

	float t = Dot(s2, e2) * invDivisor;
	if (t < ray.tMin || t > ray.tMax)
		return false;

	// Compute triangle partial derivatives
	float2 UVs[3];
	if (!mMesh.mTexcoords.empty())
	{
		UVs[0] = mMesh.mTexcoords[mIndex[0]];
		UVs[1] = mMesh.mTexcoords[mIndex[1]];
		UVs[2] = mMesh.mTexcoords[mIndex[2]];
	}
	else
	{
		UVs[0] = float2(0, 1);
		UVs[1] = float2(1, 0);
		UVs[2] = float2(1, 1);
	}
		
	// calculate dPdU and dPdV
	float du1 = UVs[0][0] - UVs[2][0];
	float du2 = UVs[1][0] - UVs[2][0];
	float dv1 = UVs[0][1] - UVs[2][1];
	float dv2 = UVs[1][1] - UVs[2][1]; 
	float invdet, det = du1 * dv2 - dv1 * du2;
		
	if (det == 0.f)
	{
		//diffGeoHit->
		//sp.dPdV = vector3d_t(0.f);
	}
	else
	{
		float3 dp1 = p0 - p2;
		float3 dp2 = p1 - p2;

		invdet = 1.0f / det;
		diffGeoHit->dpdu = ( dv2 * dp1 - dv1 * dp2) * invdet;
		diffGeoHit->dpdv = (-du2 * dp1 + du1 * dp2) * invdet;
	}

	float b0 = 1 - b1 - b2;
	diffGeoHit->UV = float2(b0 * UVs[0] + b1 * UVs[1] + b2 * UVs[2]);
	diffGeoHit->Point = ray.Eval(t);
	diffGeoHit->dndu = float3(0, 0, 0);
	diffGeoHit->dndv = float3(0, 0, 0);
	diffGeoHit->Normal = Normalize(Cross(diffGeoHit->dpdu, diffGeoHit->dpdv));
	diffGeoHit->Geometry = this;

	*tHit = t;
	return true;
}

BoundingBoxf Triangle::GetLocalBound() const
{
	const float3& p1 = mMesh.mPositions[mIndex[0]];
	const float3& p2 = mMesh.mPositions[mIndex[1]];
	const float3& p3 = mMesh.mPositions[mIndex[2]];

	BoundingBoxf box;
	box.Merge(p1); box.Merge(p2); box.Merge(p3);

	return box;
}

float Triangle::Area() const
{
	const float3& p0 = mMesh.mPositions[mIndex[0]];
	const float3& p1 = mMesh.mPositions[mIndex[1]];
	const float3& p2 = mMesh.mPositions[mIndex[2]];

	return 0.5f * Length(Cross(p1-p0, p2-p0));
}

}