#ifndef Ray_h__
#define Ray_h__

#include <Vector.hpp>

namespace Purple {

using RxLib::float3;

struct Ray
{
	Ray()
		: Origin(0, 0, 0), Direction(0, 0, 0), tMin(0.0f), tMax(RxLib::Mathf::INFINITY), Time(0.0f), Depth(0)
	{

	}

	Ray(const float3& origin, const float3& dir, float min, float max = RxLib::Mathf::INFINITY, float time = 0.0f, int depth = 0)
		: Origin(origin), Direction(dir), tMin(min), tMax(max), Time(time), Depth(depth)
	{

	}
	
	float3 Eval(float t) const 
	{
		return Origin + Direction * t;
	}

	float3 Origin;
	float3 Direction;
	float Time;
	mutable float tMin, tMax;
	int Depth;
};

struct RayDifferential : public Ray
{
	RayDifferential()
	{
		hasDifferentials = false;
	}

	RayDifferential(const float3& org, const float3& dir, float start, float end = RxLib::Mathf::INFINITY, float t = 0.0f, int d = 0)
		: Ray(org, dir, start, end, t, d)
	{
			hasDifferentials = false;
	}

	RayDifferential(const float3& org, const float3& dir, const Ray& parent, float start, float end = RxLib::Mathf::INFINITY)
		: Ray(org, dir, start, end, parent.Time, parent.Depth+1) 
	{
			hasDifferentials = false;
	}

	explicit RayDifferential(const Ray &ray) : Ray(ray)
	{
		hasDifferentials = false;
	}

	void ScaleDifferentials(float s)
	{
		rxOrigin = Origin + (rxOrigin - Origin) * s;
		ryOrigin = Origin + (ryOrigin - Origin) * s;
		rxDirection = Direction + (rxDirection - Direction) * s;
		ryDirection = Direction + (ryDirection - Direction) * s;
	}

	bool hasDifferentials;
	float3 rxOrigin, ryOrigin;
	float3 rxDirection, ryDirection;
};

}

#endif // Ray_h__

