#include "SimpleGeometry.h"
#include "MentoCarlo.h"
#include <MathUtil.hpp>
#include <SoveEquation.hpp>

namespace Purple {

using namespace RxLib;

Sphere::Sphere( const float44& o2w, bool ro, float radius, float zmin, float zmax, float phiMax )
	: Shape(o2w, ro), mRadius(radius)
{
	mMinZ = Clamp(std::min(zmin, zmax), -radius, radius);
	mMaxZ = Clamp(std::max(zmin, zmax), -radius, radius);

	mMinTheta = acosf(Clamp(zmin/radius, -1.f, 1.f));
	mMaxTheta = acosf(Clamp(zmax/radius, -1.f, 1.f));
	mMaxPhi = Clamp(phiMax, 0.0f, Mathf::TWO_PI);
}

Sphere::~Sphere(void)
{
}

BoundingBoxf Sphere::GetLocalBound() const
{
	return BoundingBoxf( float3(-mRadius, -mRadius, -mRadius), float3(-mRadius, -mRadius, -mRadius) );
}

bool Sphere::Intersect( const Ray& r, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	// Transform ray into local frame
	Ray ray(r);
	ray.Origin = Transform(r.Origin, mWorldToLocal);
	ray.Direction = TransformDirection(r.Direction, mWorldToLocal);

	float A = Dot(ray.Direction, ray.Direction);
	float B = 2.0f * Dot(ray.Origin, ray.Direction);
	float C = Dot(ray.Origin, ray.Origin) - mRadius*mRadius;

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

	// hit point in local space
	float3 phit = ray.Eval(thit);
	float3 N = Normalize(phit);

	float phi = atan2f(phit.Y(), phit.X());
	if (phi < 0.0f)
		phi += 2.f * Mathf::PI;

	// Test sphere intersection against clipping parameters
	if ((mMinZ > -mRadius && phit.Z() < mRadius) || (mMaxZ < mRadius && phit.Z() > mMaxZ) || phi > mMaxPhi) 
	{
		if (thit == t1) return false;
		if (t1 > ray.tMax) return false;
		thit = t1;
	
		phit = ray.Eval(thit);
		phi = atan2f(phit.Y(), phit.X());
		if (phi < 0.0f)
			phi += 2.f * Mathf::PI;

		if ((mMinZ > -mRadius && phit.Z() < mMinZ) || (mMaxZ <  mRadius && phit.Z() > mMaxZ) || phi > mMaxPhi)
			return false;
	}

	// Find parametric representation of sphere hit
	float u = phi / mMaxPhi;
	float theta = acosf(Clamp(phit.Z() / mRadius, -1.0f, 1.0f));
	float v = (theta - mMinTheta) / (mMaxTheta - mMinTheta); 

	// Compute sphere dpdu and dpdv
	float zradius = sqrtf(phit.X()*phit.X() + phit.Y()*phit.Y());
	float invzradius = 1.f / zradius;
	float cosphi = phit.X() * invzradius;
	float sinphi = phit.Y() * invzradius;

	float3 dpdu(-mMaxPhi * phit.Y(), mMaxPhi * phit.X(), 0.0f);
	float3 dpdv = (mMaxTheta - mMinTheta) * float3(phit.Z() * cosphi, phit.Z() * sinphi, -mRadius * sinf(theta));

	float3 d2Pduu = -mMaxPhi * mMaxPhi * float3(phit.X(), phit.Y(), 0.0f);
	float3 d2Pduv = (mMaxTheta - mMinTheta) * phit.Z() *mMaxPhi * float3(-sinphi, cosphi, 0.0f);
	float3 d2Pdvv = -(mMaxTheta - mMinTheta) * (mMaxTheta - mMinTheta) * float3(phit.X(), phit.Y(), phit.Z());

	const float E = Dot(dpdu, dpdu);
	const float F = Dot(dpdu, dpdv);
	const float G = Dot(dpdv, dpdv);
	const float e = Dot(N, d2Pduu);
	const float f = Dot(N, d2Pduv);
	const float g = Dot(N, d2Pdvv);

	const float invEGF2 = 1.f / (E*G - F*F);

	float3 dndu((f*F - e*G) * invEGF2 * dpdu + (e*F - f*E) * invEGF2 * dpdv);
	float3 dndv((g*F - f*G) * invEGF2 * dpdu + (f*F - g*E) * invEGF2 * dpdv);

	diffGeoHit->dpdu = TransformDirection(dpdu, mLocalToWorld);
	diffGeoHit->dpdv = TransformDirection(dpdv, mLocalToWorld);
	diffGeoHit->dndu = TransformNormal(dndu, mLocalToWorld);
	diffGeoHit->dndv = TransformNormal(dndv, mLocalToWorld);
	diffGeoHit->Normal = TransformNormal(N, mLocalToWorld);
	diffGeoHit->Point = Transform(phit, mLocalToWorld);;
	diffGeoHit->UV = float2(u, v);
	diffGeoHit->Geometry = this;

	*tHit = thit;

	return true;
}

float Sphere::Area() const
{
	return mMaxPhi * mRadius * (mMaxZ-mMinZ);
}

float3 Sphere::Sample( float u1, float u2, float3* normal ) const
{
	float3 pt = float3(0, 0, 0) + mRadius * UniformSampleSphere(u1, u2);
	
	// normal 
	*normal = Normalize(TransformNormal(pt, mLocalToWorld));
	if (mReverseOrientation) 
		*normal *= -1.f;

	return Transform(pt, mLocalToWorld);
}

float3 Sphere::Sample( const float3& pt, float u1, float u2, float3* normal ) const
{
	float3 centerW = Transform(float3(0, 0, 0), mLocalToWorld);
	
	float3 wc = Normalize(centerW - pt);
	float3 wcX, wcY;
	CoordinateSystem(wc, &wcX, &wcY); 

	float distanceSquared = LengthSquared(pt - centerW);

	// pt is inside in the sphere
	if (distanceSquared - mRadius*mRadius < 1e-4f)
		return Sample(u1, u2, normal);

	float sinThetaMax2 = mRadius*mRadius /distanceSquared;
	float cosThetaMax = sqrtf(std::max(0.0f, 1.f - sinThetaMax2));

	Ray ray(pt, UniformSampleCone(u1, u2, cosThetaMax, wcX, wcY, wc), 1e-3f, Mathf::INFINITY);
	
	DifferentialGeometry dgSphere;
	float hit;
	if ( !Intersect(ray, &hit, &dgSphere) )
	{
		hit = Dot(centerW-pt, ray.Direction);
	}

	float3 ps = ray.Eval(hit);
	*normal = Normalize(ps - centerW);

	if (mReverseOrientation) 
		*normal *= -1.f;

	return ps;
}

float Sphere::Pdf( const float3& pt, const float3& wi ) const
{
	float3 centerW = Transform(float3(0, 0, 0), mLocalToWorld);
	float distanceSquared = LengthSquared(pt - centerW);

	// pt is inside in the sphere
	if (distanceSquared - mRadius*mRadius < 1e-4f)
		return UniformSpherePdf();

	float sinThetaMax2 = mRadius*mRadius /distanceSquared;
	float cosThetaMax = sqrtf(std::max(0.0f, 1.f - sinThetaMax2));

	return UniformConePdf(cosThetaMax);
}

bool Sphere::IntersectP(const Ray& r) const
{
	// Transform ray into local frame
	Ray ray(r);
	ray.Origin = Transform(r.Origin, mWorldToLocal);
	ray.Direction = TransformDirection(r.Direction, mWorldToLocal);

	float A = Dot(ray.Direction, ray.Direction);
	float B = 2.0f * Dot(ray.Origin, ray.Direction);
	float C = Dot(ray.Origin, ray.Origin) - mRadius*mRadius;

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

	// hit point in local space
	float3 phit = ray.Eval(thit);
	float3 N = Normalize(phit);

	float phi = atan2f(phit.Y(), phit.X());
	if (phi < 0.0f)
		phi += 2.f * Mathf::PI;

	// Test sphere intersection against clipping parameters
	if ((mMinZ > -mRadius && phit.Z() < mRadius) || (mMaxZ < mRadius && phit.Z() > mMaxZ) || phi > mMaxPhi) 
	{
		if (thit == t1) return false;
		if (t1 > ray.tMax) return false;
		thit = t1;

		phit = ray.Eval(thit);
		phi = atan2f(phit.Y(), phit.X());
		if (phi < 0.0f)
			phi += 2.f * Mathf::PI;

		if ((mMinZ > -mRadius && phit.Z() < mMinZ) || (mMaxZ <  mRadius && phit.Z() > mMaxZ) || phi > mMaxPhi)
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------
Cylinder::Cylinder( const float44& o2w, bool ro, float radius, float minZ, float mazZ, float maxPhi )
	: Shape(o2w, ro), mRadius(radius), mMaxZ(mazZ), mMinZ(minZ)
{
	mMaxPhi = Clamp(maxPhi, 0.0f, Mathf::TWO_PI);
}

Cylinder::~Cylinder()
{

}

BoundingBoxf Cylinder::GetLocalBound() const
{
	float3 min = float3(-mRadius, -mRadius, mMinZ);
	float3 max = float3( mRadius,  mRadius, mMaxZ);
	return BoundingBoxf(min, max);
}

bool Cylinder::Intersect( const Ray& r, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	// Transform ray into local frame
	Ray ray(r);
	ray.Origin = Transform(r.Origin, mWorldToLocal);
	ray.Direction = TransformDirection(r.Direction, mWorldToLocal);
	
	// Compute quadratic cylinder coefficients
	float A = ray.Direction.X() * ray.Direction.X() + ray.Direction.Y() * ray.Direction.Y();
	float B = 2.0f * (ray.Direction.X() * ray.Origin.X() + ray.Direction.Y() * ray.Origin.Y());
	float C = ray.Origin.X() * ray.Origin.X() + ray.Origin.Y() * ray.Origin.Y() - mRadius * mRadius;

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

	// hit point in local space
	float3 phit = ray.Eval(thit);

	float phi = atan2f(phit.Y(), phit.X());
	if (phi < 0.0f)
		phi += 2.f * Mathf::PI;

	// Test cylinder intersection against clipping parameters
	if (phit.Z() < mMinZ || phit.Z() > mMaxZ || phi > mMaxPhi) 
	{
		if (thit == t1) return false;
		thit = t1;
		if (t1 > ray.tMax) return false;
		
		// Compute cylinder hit point 
		phit =ray.Eval(thit);
		phi = atan2f(phit.Y(), phit.X());
		if (phi < 0.0f)	phi += 2.f * Mathf::PI;
		if (phit.Z() < mMinZ || phit.Z() > mMaxZ || phi > mMaxPhi) 
			return false;
	}

	// Find parametric representation of sphere hit
	float u = phi / mMaxPhi;
	float v = (phit.Z() - mMinZ) / (mMaxZ - mMinZ); 

	// Compute sphere dpdu and dpdv
	float zradius = sqrtf(phit.X()*phit.X() + phit.Y()*phit.Y());
	float invzradius = 1.f / zradius;
	float cosphi = phit.X() * invzradius;
	float sinphi = phit.Y() * invzradius;

	float3 dpdu(-mMaxPhi * phit.Y(), mMaxPhi * phit.X(), 0.0f);
	float3 dpdv(0.0f, 0.0f, mMaxZ - mMinZ);

	float3 d2Pduu = -mMaxPhi * mMaxPhi * float3(phit.X(), phit.Y(), 0);
	float3 d2Pduv(0, 0, 0);
	float3 d2Pdvv(0, 0, 0);

	const float E = Dot(dpdu, dpdu);
	const float F = Dot(dpdu, dpdv);
	const float G = Dot(dpdv, dpdv);
	
	// Compute N
	float3 N = Normalize(Cross(dpdu, dpdv));

	const float e = Dot(N, d2Pduu);
	const float f = Dot(N, d2Pduv);
	const float g = Dot(N, d2Pdvv);

	const float invEGF2 = 1.f / (E*G - F*F);

	float3 dndu((f*F - e*G) * invEGF2 * dpdu + (e*F - f*E) * invEGF2 * dpdv);
	float3 dndv((g*F - f*G) * invEGF2 * dpdu + (f*F - g*E) * invEGF2 * dpdv);

	diffGeoHit->dpdu = TransformDirection(dpdu, mLocalToWorld);
	diffGeoHit->dpdv = TransformDirection(dpdv, mLocalToWorld);
	diffGeoHit->dndu = TransformNormal(dndu, mLocalToWorld);
	diffGeoHit->dndv = TransformNormal(dndv, mLocalToWorld);
	diffGeoHit->Normal = TransformNormal(N, mLocalToWorld);
	diffGeoHit->Point = Transform(phit, mLocalToWorld);;
	diffGeoHit->UV = float2(u, v);
	diffGeoHit->Geometry = this;

	*tHit = thit;

	return true;
}

bool Cylinder::IntersectP( const Ray& r ) const
{
	// Transform ray into local frame
	Ray ray(r);
	ray.Origin = Transform(r.Origin, mWorldToLocal);
	ray.Direction = TransformDirection(r.Direction, mWorldToLocal);

	// Compute quadratic cylinder coefficients
	float A = ray.Direction.X() * ray.Direction.X() + ray.Direction.Y() * ray.Direction.Y();
	float B = 2.0f * (ray.Direction.X() * ray.Origin.X() + ray.Direction.Y() * ray.Origin.Y());
	float C = ray.Origin.X() * ray.Origin.X() + ray.Origin.Y() * ray.Origin.Y() - mRadius * mRadius;

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

	// hit point in local space
	float3 phit = ray.Eval(thit);

	float phi = atan2f(phit.Y(), phit.X());
	if (phi < 0.0f)
		phi += 2.f * Mathf::PI;

	// Test cylinder intersection against clipping parameters
	if (phit.Z() < mMinZ || phit.Z() > mMaxZ || phi > mMaxPhi) 
	{
		if (thit == t1) return false;
		thit = t1;
		if (t1 > ray.tMax) return false;

		// Compute cylinder hit point 
		phit =ray.Eval(thit);
		phi = atan2f(phit.Y(), phit.X());
		if (phi < 0.0f)	phi += 2.f * Mathf::PI;
		if (phit.Z() < mMinZ || phit.Z() > mMaxZ || phi > mMaxPhi) 
			return false;
	}

	return true;
}

float Cylinder::Area() const
{
	return (mMaxZ - mMinZ) * mMaxPhi * mRadius;
}

float3 Cylinder::Sample( float u1, float u2, float3* normal ) const
{
	float z = Lerp(u1, mMinZ, mMaxZ);
	float t = u2 * mMaxPhi;

	float3 pt = float3(mRadius * cosf(t), mRadius * sinf(t), z);
	*normal = Normalize(TransformNormal(float3(pt.X(), pt.Y(), 0.0f), mLocalToWorld)); 

	if (mReverseOrientation) 
		*normal *= -1.f;
	
	return Transform(pt, mLocalToWorld);
}


////----------------------------------------------------------------------------
//Triangle::Triangle( Mesh& mesh, uint32_t* index )
//	: mMesh(mesh), mIndex(index)
//{
//}
//
//Triangle::~Triangle()
//{
//
//}
//
//bool Triangle::Intersect( const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit ) const
//{
//	const float3& p0 = mMesh.mPositions[mIndex[0]];
//	const float3& p1 = mMesh.mPositions[mIndex[1]];
//	const float3& p2 = mMesh.mPositions[mIndex[2]];
//
//	float3 e1 = p1 - p0;
//	float3 e2 = p2 - p0;
//	
//	float3 s1 = Cross(ray.Direction, e2);
//
//	float divisor = Dot(s1, e1);
//
//	if (divisor == 0)
//		return false;
//
//	float invDivisor = 1.0f / divisor;
//
//	float3 s = ray.Origin - p0;
//	float b1 = Dot(s1, s) * invDivisor;
//	if (b1 < 0.0f || b1 > 1.0f)
//		return false;
//
//	float3 s2 = Cross(s, e1);
//	float b2 = Dot(s2, ray.Direction) * invDivisor;
//
//	if (b1 < 0.0f || b1 + b2 > 1.0f)
//		return false;
//
//	float t = Dot(s2, e2) * invDivisor;
//	if (t < ray.tMin || t > ray.tMax)
//		return false;
//
//	// Compute triangle partial derivatives
//	float2 UVs[3];
//	if (!mMesh.mTexcoords.empty())
//	{
//		UVs[0] = mMesh.mTexcoords[mIndex[0]];
//		UVs[1] = mMesh.mTexcoords[mIndex[1]];
//		UVs[2] = mMesh.mTexcoords[mIndex[2]];
//	}
//	else
//	{
//		UVs[0] = float2(0, 1);
//		UVs[1] = float2(1, 0);
//		UVs[2] = float2(1, 1);
//	}
//		
//	// calculate dPdU and dPdV
//	float du1 = UVs[0][0] - UVs[2][0];
//	float du2 = UVs[1][0] - UVs[2][0];
//	float dv1 = UVs[0][1] - UVs[2][1];
//	float dv2 = UVs[1][1] - UVs[2][1]; 
//	float invdet, det = du1 * dv2 - dv1 * du2;
//		
//	if (det == 0.f)
//	{
//		//diffGeoHit->
//		//sp.dPdV = vector3d_t(0.f);
//	}
//	else
//	{
//		float3 dp1 = p0 - p2;
//		float3 dp2 = p1 - p2;
//
//		invdet = 1.0f / det;
//		diffGeoHit->dpdu = ( dv2 * dp1 - dv1 * dp2) * invdet;
//		diffGeoHit->dpdv = (-du2 * dp1 + du1 * dp2) * invdet;
//	}
//
//	float b0 = 1 - b1 - b2;
//	diffGeoHit->UV = float2(b0 * UVs[0] + b1 * UVs[1] + b2 * UVs[2]);
//	diffGeoHit->Point = ray.Eval(t);
//	diffGeoHit->dndu = float3(0, 0, 0);
//	diffGeoHit->dndv = float3(0, 0, 0);
//	diffGeoHit->Normal = Normalize(Cross(diffGeoHit->dpdu, diffGeoHit->dpdv));
//	diffGeoHit->Geometry = this;
//
//	*tHit = t;
//	return true;
//}
//
//BoundingBoxf Triangle::GetLocalBound() const
//{
//	const float3& p1 = mMesh.mPositions[mIndex[0]];
//	const float3& p2 = mMesh.mPositions[mIndex[1]];
//	const float3& p3 = mMesh.mPositions[mIndex[2]];
//
//	BoundingBoxf box;
//	box.Merge(p1); box.Merge(p2); box.Merge(p3);
//
//	return box;
//}
//
//float Triangle::Area() const
//{
//	const float3& p0 = mMesh.mPositions[mIndex[0]];
//	const float3& p1 = mMesh.mPositions[mIndex[1]];
//	const float3& p2 = mMesh.mPositions[mIndex[2]];
//
//	return 0.5f * Length(Cross(p1-p0, p2-p0));
//}


}