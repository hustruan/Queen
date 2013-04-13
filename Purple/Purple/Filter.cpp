#include "Filter.h"

namespace Purple {

GaussianFilter::GaussianFilter( float xw, float yw, float a ) : Filter(xw, yw), alpha(a), expX(expf(-alpha * xWidth * xWidth)),
	expY(expf(-alpha * yWidth * yWidth))
{

}

float GaussianFilter::Evaluate( float x, float y ) const
{
	return Gaussian(x, expX) * Gaussian(y, expY);
}

}


