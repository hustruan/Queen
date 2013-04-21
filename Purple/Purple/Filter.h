#ifndef Filter_h__
#define Filter_h__

#include "Prerequisites.h"

namespace Purple {

class Filter
{
public:
	Filter(float radius) : Radius(radius) { }
	
	virtual float Eval(float x) const = 0;

public:
	const float Radius;
};

class GaussianFilter : public Filter
{
public:
	GaussianFilter(float radius, float alpha)
		: Filter(radius), Alpha(alpha), Exp(expf(-alpha*radius*radius)) { }

	float Eval(float x) const 
	{
		return std::max(0.0f, expf(-Alpha * x * x) - Exp);
	}

public:
	const float Alpha, Exp;
};

}




#endif // Filter_h__