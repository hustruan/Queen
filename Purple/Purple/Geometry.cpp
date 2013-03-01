#include "Geometry.h"
#include <MathUtil.hpp>

namespace Purple {

Geometry::Geometry(void)
{
}


Geometry::~Geometry(void)
{
}

BoundingBoxf Geometry::GetWorldBound() const
{
	return Transform(GetLocalBound(), mLocalToWorld);
}

bool Geometry::FastIntersect( const Ray& ray )
{
	return false;
}

}