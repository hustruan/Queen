#ifndef DifferentialGeometry_h__
#define DifferentialGeometry_h__

#include "Prerequisites.h"

namespace Purple {

struct RayDifferential;

struct DifferentialGeometry
{
	DifferentialGeometry()
	{
		dudx = dvdx = dudy = dvdy = 0.0f;
		Shape = NULL; 
	}

	DifferentialGeometry(const float3 &P, const float3 &DPDU, const float3 &DPDV, const float3 &DNDU, const float3 &DNDV, const float2& uv, const Shape *sh)
		: Point(P), dpdv(DPDV), dpdu(DPDU), dndv(DNDV), dndu(DNDU), UV(uv)
	{
		Normal = Normalize(Normalize(Cross(dpdu, dpdv)));

		Shape = sh;
		dudx = dvdx = dudy = dvdy = 0;
	}

	void ComputeDifferentials(const RayDifferential& ray) const;

	BSDF* GetBSDF(const RayDifferential &ray, MemoryArena &arena) const;

	ColorRGB Le(const float3& wo) const;

	const Shape* Shape;

	float3 Point;
	float3 Normal;
	float2 UV;

	float3 dpdu, dpdv;
	float3 dndu, dndv;

	mutable float3 dpdx, dpdy;
	mutable float dudx, dvdx, dudy, dvdy;
};



}

#endif // DifferentialGeometry_h__


