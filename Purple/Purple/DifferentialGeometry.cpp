#include "DifferentialGeometry.h"
#include "Shape.h"
#include "Reflection.h"
#include "Light.h"
#include <SoveEquation.hpp>
#include <MathUtil.hpp>

namespace Purple {

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
	AreaLight* light = Shape->GetAreaLight();
	
	 return light ? light->L(Point, Normal, wo) : ColorRGB::Black;
}



}