#pragma once
#ifndef Ray_h__
#define Ray_h__

#include <Vector.hpp>

namespace Purple {

using namespace RxLib;

class Ray
{
public:
	Ray()
		: Origin(0, 0, 0), Direction(0, 0, 0)
	{

	}

	Ray(const float3& origin, const float3& dir)
		: Origin(origin), Direction(dir)
	{

	}
	
	float3 Eval(float t) const 
	{
		return Origin + Direction * t;
	}

public:
	float3 Origin;
	float3 Direction;
	float tMin, tMax;
};


}

#endif // Ray_h__

