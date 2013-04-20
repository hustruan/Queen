#ifndef Filter_h__
#define Filter_h__

#include "Prerequisites.h"

namespace Purple {

//class Filter
//{
//public:
//	Filter(float xw, float yw)
//		: xWidth(xw), yWidth(yw), invXWidth(1.f/xw), invYWidth(1.f/yw) {
//	}
//	virtual ~Filter() {}
//	virtual float Evaluate(float x, float y) const = 0;
//
//	// Filter Public Data
//	const float xWidth, yWidth;
//	const float invXWidth, invYWidth;
//};
//
//
//class GaussianFilter : public Filter
//{
//public:
//	// GaussianFilter Public Methods
//	GaussianFilter(float xw, float yw, float a);
//	float Evaluate(float x, float y) const;
//private:
//	// GaussianFilter Private Data
//	const float alpha;
//	const float expX, expY;
//
//	// GaussianFilter Utility Functions
//	float Gaussian(float d, float expv) const {
//		return std::max(0.f, float(expf(-alpha * d * d) - expv));
//	}
//};


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

	float Evaluate(float x) const 
	{
		return std::max(0.0f, expf(-Alpha * x * x) - Exp);
	}

public:
	const float Alpha, Exp;
};

}




#endif // Filter_h__