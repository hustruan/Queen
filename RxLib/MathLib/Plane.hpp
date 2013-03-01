#ifndef Plane_h__
#define Plane_h__

#include "Vector.hpp"

namespace RxLib {

enum PlaneSide
{
	No_Side,
	Positive_Side,
	Negative_Side,
	Both_Side
};

template< typename Real >
class Plane
{
public:
	Plane() { }

	Plane(Real a, Real b, Real c, Real d)
		: Normal(Vector<Real, 3>(a, b, c)), Distance(d)
	{

	}

	Plane(const Vector<Real, 3>& noraml, Real distance)
		: Normal(noraml), Distance(distance) { }

	Plane(const Vector<Real, 3>& noraml, const Vector<Real, 3>& point)
		:Normal(noraml)
	{
		Distance = -Dot(noraml, point);
	}

	Plane(const Vector<Real, 3>& point1, const Vector<Real, 3>& point2, const Vector<Real, 3>& point3)
	{
		Vector<Real, 3> edge1 = point2 - point1;
		Vector<Real, 3> edge2 = point3 - point1;
		Normal = Normalize(Cross(edge1, edge2));
		Distance = -Dot(Normal, point1);
	}

	PlaneSide WhichSide (const Vector<Real, 3>& point) const
	{
		Real dist = DistanceTo(point);

		if (dist < (Real)0.0)
		{
			return Negative_Side;
		}

		if (dist > (Real)0.0)
		{
			return Positive_Side;
		}

		return No_Side;
	}

	Real DistanceTo(const Vector<Real, 3>& point) const
	{
		return Dot(Normal, point) + Distance;
	}

	void Normalize(Real epsilon = Math<Real>::ZERO_TOLERANCE)
	{
		Real length = Length(Normal);

		if (length > epsilon)
		{
			Real invLength = Real(1) / length;
			Normal *= invLength;
			Distance *= invLength;
		}
	}

public:
	Vector<Real, 3> Normal;
	Real Distance;
};

typedef Plane<float> Planef;

}


#endif // Plane_h__
