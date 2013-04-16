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
		Instance = NULL; 
		PrimIndex = std::numeric_limits<uint32_t>::max();
	}

	DifferentialGeometry(const float3 &P, const float3 &DPDU, const float3 &DPDV, const float3 &DNDU, const float3 &DNDV, const float2& uv, const Shape *sh);

	void ComputeDifferentials(const RayDifferential& ray) const;

	BSDF* GetBSDF(const RayDifferential &ray, MemoryArena &arena) const;

	ColorRGB Le(const float3& wo) const;

	const Shape* Instance;

	/// Primitive index, e.g. the triangle ID (if applicable)
	uint32_t PrimIndex;

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


