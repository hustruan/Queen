#include "SimpleShape.h"
#include "MentoCarlo.h"
#include "DifferentialGeometry.h"
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
	return BoundingBoxf( float3(-mRadius, -mRadius, mMinZ), float3(mRadius, mRadius, mMaxZ) );
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
	diffGeoHit->Instance = this;

	*tHit = thit;

	return true;
}

float Sphere::Area() const
{
	return mMaxPhi * mRadius * (mMaxZ-mMinZ);
}

float3 Sphere::Sample( float u1, float u2, float u3,  float3* normal ) const
{
	float3 pt = float3(0, 0, 0) + mRadius * UniformSampleSphere(u1, u2);
	
	// normal 
	*normal = Normalize(TransformNormal(pt, mLocalToWorld));
	if (ReverseOrientation) 
		*normal *= -1.f;

	return Transform(pt, mLocalToWorld);
}

float3 Sphere::Sample( const float3& pt, float u1, float u2, float u3,  float3* normal ) const
{
	float3 centerW = Transform(float3(0, 0, 0), mLocalToWorld);
	
	float3 wc = Normalize(centerW - pt);
	float3 wcX, wcY;
	CoordinateSystem(wc, &wcX, &wcY); 

	float distanceSquared = LengthSquared(pt - centerW);

	// pt is inside in the sphere
	if (distanceSquared - mRadius*mRadius < 1e-4f)
		return Sample(u1, u2, u3, normal);

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

	if (ReverseOrientation) 
		*normal *= -1.f;

	return ps;
}

float Sphere::Pdf( const float3& pt, const float3& wi ) const
{
	float3 centerW = Transform(float3(0, 0, 0), mLocalToWorld);
	float distanceSquared = LengthSquared(pt - centerW);

	// pt is inside in the sphere
	if (distanceSquared - mRadius*mRadius < 1e-4f)
		return Shape::Pdf(pt, wi);

	float sinThetaMax2 = mRadius*mRadius /distanceSquared;
	float cosThetaMax = sqrtf(std::max(0.0f, 1.f - sinThetaMax2));

	//return Shape::Pdf(pt, wi);
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
	diffGeoHit->Instance = this;

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

float3 Cylinder::Sample( float u1, float u2, float u3,  float3* normal ) const
{
	float z = Lerp(u1, mMinZ, mMaxZ);
	float t = u2 * mMaxPhi;

	float3 pt = float3(mRadius * cosf(t), mRadius * sinf(t), z);
	*normal = Normalize(TransformNormal(float3(pt.X(), pt.Y(), 0.0f), mLocalToWorld)); 

	if (ReverseOrientation) 
		*normal *= -1.f;
	
	return Transform(pt, mLocalToWorld);
}


Disk::Disk( const float44& o2w, bool ro, float height, float radius, float innerRadius, float maxPhi )
	: Shape(o2w, ro), mRadius(radius), mInnerRadius(innerRadius), mHeight(height)
{
	mMaxPhi = Clamp(maxPhi, 0.0f, Mathf::TWO_PI);
}

Disk::~Disk()
{

}

BoundingBoxf Disk::GetLocalBound() const
{
	return BoundingBoxf(float3(-mRadius, -mRadius, mHeight),
		float3( mRadius,  mRadius, mHeight));
}

bool Disk::Intersect( const Ray& r, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	// Transform ray into local frame
	Ray ray(r);
	ray.Origin = Transform(r.Origin, mWorldToLocal);
	ray.Direction = TransformDirection(r.Direction, mWorldToLocal);

	// Compute plane intersection for disk
	if (fabsf(ray.Direction.Z()) < 1e-7) return false;
	float thit = (mHeight - ray.Origin.Z()) / ray.Direction.Z();
	if (thit < ray.tMin || thit > ray.tMax)
		return false;

	// See if hit point is inside disk radii and $\phimax$
	float3 phit = ray.Eval(thit);
	float dist2 = phit.X() * phit.X() + phit.Y() * phit.Y();
	if (dist2 > mRadius * mRadius || dist2 < mInnerRadius * mInnerRadius)
		return false;

	// Test disk $\phi$ value against $\phimax$
	float phi = atan2f(phit.Y(), phit.X());
	if (phi < 0) phi += 2.0f * Mathf::PI;
	if (phi > mMaxPhi)
		return false;

	// Find parametric representation of disk hit
	float u = phi / mMaxPhi;
	float oneMinusV = ((sqrtf(dist2)-mInnerRadius) / (mRadius-mInnerRadius));
	float invOneMinusV = (oneMinusV > 0.f) ? (1.f / oneMinusV) : 0.f;
	float v = 1.f - oneMinusV;
	float3 dpdu(-mMaxPhi * phit.Y(), mMaxPhi * phit.X(), 0.);
	float3 dpdv(-phit.X() * invOneMinusV, -phit.Y() * invOneMinusV, 0.);
	dpdu *= mMaxPhi * Mathf::INV_TWO_PI;
	dpdv *= (mRadius - mInnerRadius) / mRadius;

	float3 dndu(0,0,0), dndv(0,0,0);

	// Initialize _DifferentialGeometry_ from parametric information
	
	*diffGeoHit = DifferentialGeometry(Transform(phit, mLocalToWorld), TransformDirection(dpdu, mLocalToWorld), 
		TransformDirection(dpdv, mLocalToWorld), TransformNormal(dndu, mLocalToWorld), TransformNormal(dndv, mLocalToWorld), float2(u,v), this);

	// Update _tHit_ for quadric intersection
	*tHit = thit;

	// Compute _rayEpsilon_ for quadric intersection
	//*rayEpsilon = 5e-4f * *tHit;
	return true;
}

bool Disk::IntersectP( const Ray &r ) const
{
	// Transform ray into local frame
	Ray ray(r);
	ray.Origin = Transform(r.Origin, mWorldToLocal);
	ray.Direction = TransformDirection(r.Direction, mWorldToLocal);

	// Compute plane intersection for disk
	if (fabsf(ray.Direction.Z()) < 1e-7) return false;
	float thit = (mHeight - ray.Origin.Z()) / ray.Direction.Z();
	if (thit < ray.tMin || thit > ray.tMax)
		return false;

	// See if hit point is inside disk radii and $\phimax$
	float3 phit = ray.Eval(thit);
	float dist2 = phit.X() * phit.X() + phit.Y() * phit.Y();
	if (dist2 > mRadius * mRadius || dist2 < mInnerRadius * mInnerRadius)
		return false;

	// Test disk $\phi$ value against $\phimax$
	float phi = atan2f(phit.Y(), phit.X());
	if (phi < 0) phi += 2.0f * Mathf::PI;
	if (phi > mMaxPhi)
		return false;

	return true;
}

float Disk::Area() const
{
	return mMaxPhi * 0.5f *
		(mRadius * mRadius - mInnerRadius * mInnerRadius);
}

float3 Disk::Sample( float u1, float u2, float3* n ) const
{
	float3 p;
	ConcentricSampleDisk(u1, u2, &p[0], &p[1]);
	
	p.X() *= mRadius;
	p.Y() *= mRadius;
	p.Z() = mHeight;

	*n = Normalize(TransformNormal(float3(0,0,1), mLocalToWorld));
	
	if (ReverseOrientation) *n  *= -1.f;
	
	return Transform(p, mWorldToLocal);
}

}