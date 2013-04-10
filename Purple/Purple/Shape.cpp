#include "Shape.h"
#include "MemoryArena.h"
#include "Material.h"
#include "Light.h"
#include <SoveEquation.hpp>
#include <MathUtil.hpp>

namespace Purple {

using namespace RxLib;

BSDF* DifferentialGeometry::GetBSDF( const RayDifferential &ray, MemoryArena &arena ) const
{
	ComputeDifferentials(ray);
	return Shape->GetBSDF(*this, Shape->mLocalToWorld, arena);
}

void DifferentialGeometry::ComputeDifferentials( const RayDifferential& ray ) const
{
	if (ray.hasDifferentials)
	{
		float d = -Dot(Normal, Point);
		float tx = -(Dot(Normal, ray.rxOrigin) + d) / Dot(Normal, ray.rxDirection); 
		if(isnan(tx)) goto fail;

		float ty = -(Dot(Normal, ray.ryOrigin) + d) / Dot(Normal, ray.ryDirection); 
		if(isnan(ty)) goto fail;

		float3 px = ray.rxOrigin + tx * ray.rxDirection;
		float3 py = ray.ryOrigin + ty * ray.ryDirection;

		float A[2][2], Bx[2], By[2];
		int axes[2];
		if (fabsf(Normal.X()) > fabsf(Normal.Y()) && fabsf(Normal.X()) > fabsf(Normal.Z()))
		{
			axes[0] = 1; axes[1] = 2;
		}
		else if (fabsf(Normal.Y()) > fabsf(Normal.Z()))
		{
			axes[0] = 0; axes[1] = 2;
		}
		else
		{
			axes[0] = 0; axes[1] = 1;
		}

		A[0][0] = dpdu[axes[0]];
		A[0][1] = dpdv[axes[0]];
		A[1][0] = dpdu[axes[1]];
		A[1][1] = dpdv[axes[1]];
		Bx[0] = px[axes[0]] - Point[axes[0]];
		Bx[1] = px[axes[1]] - Point[axes[1]];
		By[0] = py[axes[0]] - Point[axes[0]];
		By[1] = py[axes[1]] - Point[axes[1]];

		if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx))
		{
			dudx = 0.0f; dvdx = 0.0f;
		}
		if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy))
		{
			dudy =0.0f; dvdy = 0.0f;
		}
	}
	else
	{
fail:
		dudx = dvdx = 0.0f;
		dudy = dvdy = 0.0f;
		dpdx = dpdy = float3(0,0,0);
	}
}

ColorRGB DifferentialGeometry::Le( const float3& wo ) const
{
	const AreaLight *area = Shape->GetAreaLight();
	return area ? area->L(Point, Normal, wo) : ColorRGB::Black;
}


//------------------------------------------------------------------------------------
Shape::Shape( const float44& o2w, bool ro )
	: mLocalToWorld(o2w), mWorldToLocal(MatrixInverse(o2w)), mReverseOrientation(ro)
{

}


Shape::~Shape(void)
{
}

BoundingBoxf Shape::GetWorldBound() const
{
	return Transform(GetLocalBound(), mLocalToWorld);
}

float3 Shape::Sample( float u1, float u2, float u3, float3* n ) const
{
	throw std::exception("Unimplemented!");
	return float3();
}

float Shape::Pdf( const float3& pt ) const
{
	return 1.0f / Area();
}

float3 Shape::Sample( const float3& pt, float u1, float u2, float u3,  float3* n ) const
{
	return Sample(u1, u2, u3, n);
}

float Shape::Pdf( const float3& pt, const float3& wi ) const
{
	Ray ray(pt, wi, 1e-3f, Mathf::INFINITY);

	DifferentialGeometry dgLight;
	float hit;
	if( !Intersect(ray, &hit, &dgLight) )
		return 0.0f;

	/**
	 * Py(y) = (dy/dx)-1 * Px(x)
	 */
	float pdf = LengthSquared(pt - ray.Eval(hit)) / (fabsf(Dot(dgLight.Normal, -wi)) * Area());
	return pdf;
}

bool Shape::Intersect( const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	throw std::exception("Unimplemented!");
	return false;
}

bool Shape::IntersectP( const Ray& ray ) const
{
	throw std::exception("Unimplemented!");
	return false;
}

float Shape::Area() const
{
	throw std::exception("Unimplemented!");
	return 0.0f;
}

void Shape::GetShadingGeometry( const float44& local2world, const DifferentialGeometry &dg, DifferentialGeometry *dgShading ) const
{
	*dgShading = dg;
}

BSDF* Shape::GetBSDF( const DifferentialGeometry& dgGeom, const float44& ObjectToWorld, MemoryArena &arena ) const
{
	DifferentialGeometry dgShading;
	GetShadingGeometry(ObjectToWorld, dgGeom, &dgShading);
	return mMaterial->GetBSDF(dgGeom, dgShading, arena);
}


bool AreaLightShape::Intersect( const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	bool retVal = mShape->Intersect(ray, tHit, diffGeoHit);

	return retVal;
}




}