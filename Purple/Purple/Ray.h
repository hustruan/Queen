#ifndef Ray_h__
#define Ray_h__

#include <Vector.hpp>

namespace Purple {

using RxLib::float3;

struct Ray
{
	Ray()
		: Origin(0, 0, 0), Direction(0, 0, 0)
	{

	}

	Ray(const float3& origin, const float3& dir, float min, float max)
		: Origin(origin), Direction(dir), tMin(min), tMax(max)
	{

	}
	
	float3 Eval(float t) const 
	{
		return Origin + Direction * t;
	}

	float3 Origin;
	float3 Direction;
	float tMin, tMax;
};

struct RayDifferential : public Ray
{
	bool hasDifferentials;
	float3 rxOrigin, ryOrigin;
	float3 rxDirection, ryDirection;
};


}

#endif // Ray_h__

