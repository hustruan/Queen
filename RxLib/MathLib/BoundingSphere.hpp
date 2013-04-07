#ifndef BoundingSphere_h__
#define BoundingSphere_h__

#include "Vector.hpp"

namespace RxLib {

template<typename Real>
class BoundingSphere
{
public:
	typedef Real value_type;

public:
	BoundingSphere(): Defined(false) { }

	BoundingSphere(const Vector<Real, 3>& center, Real radius)
		: Center(center), Radius(radius), Defined(true) {}

	BoundingSphere(const BoundingSphere<Real>& rhs)
		: Center(rhs.Center), Radius(rhs.Radius), Defined(rhs.Defined) {}

	BoundingSphere& operator = (const BoundingSphere<Real>& rhs)
	{
		if (this != &rhs)
		{
			Center = rhs.Center;
			Radius = rhs.Radius;
			Defined = rhs.Defined;
		}
		return *this;
	}

	bool operator == ( const BoundingSphere<Real>& rhs)
	{
		return (Center == rhs.Center) && (Radius == rhs.Radius);
	}

	/**
	 * Reset the bounding box to undefined.
	 */
	void SetNull()	{ Defined = false; }

	/**
	 * Merge a point
	 */
	void Merge( const Vector<Real, 3>& point )
	{
		if (!Defined)
		{
			Center = point;
			Radius = Real(0);
			Defined = true;
		}
		else
		{
			Vector<Real, 3> offset = point - Center;
			Real dist = offset.Length();

			if (dist > Radius)
			{
				Real half = (dist - Radius) / 2;
				Radius += half;
				Center += (half / dist) * offset;
			}
		}
	}
	
	/**
	 * Merge a sphere
	 */
	void Merge( const BoundingSphere<Real>& sphere )
	{
		// do nothing if the rhs sphere id undefined.
		if (!sphere.Defined)
		{
			return;
		}
		// Otherwise if current null, just take rhs sphere
		else if (!Defined)
		{
			Center = sphere.Center;
			Radius = sphere.Radius;
			Defined = true;
			return;
		}
		// Otherwise merge
		else
		{
			Vector<Real, 3> offset = sphere.Center - Center;
			Real distance = offset.Length();

			// If sphere fits inside, do nothing
			if (distance + sphere.Radius < Radius)
				return;

			// If we fit inside the other sphere, become it
			if (distance + Radius < sphere.Radius)
			{
				Center = sphere.Center;
				Radius = sphere.Radius;
				return;
			}

			Vector<Real, 3> normalizedOffset = offset * ( Real(1) / distance );

			Vector<Real, 3> min = Center - Radius * normalizedOffset;
			Vector<Real, 3> max = sphere.Center + sphere.Radius * normalizedOffset;
			Center = (min + max) * 0.5f;
			Radius = (max - Center).Length();
		}
	}

	/**
	 * Determines whether contains the specified sphere.
	 */
	ContainmentType Contains( const BoundingSphere<Real>& sphere )
	{
		Vector<Real, 3> difference = Center - sphere.Center;

		Real distance = difference.Length();

		if( Radius + sphere.Radius < distance )
			return CT_Disjoint;

		if( Radius - sphere.Radius < distance )
			return CT_Intersects;

		return CT_Contains;
	}

	/**
	 * Determines whether contains the specified point.
	 */
	ContainmentType Contains( const Vector<Real,3>& point )
	{
		Vector<Real, 3> offset = point - Center;
		Real distanceSquared = LengthSquared(offset);

		if( distanceSquared >= (Radius * Radius) )
			return CT_Disjoint;

		return CT_Contains;
	}

	/**
	 * Determines whether a sphere intersects the specified object.
	 */
	bool Intersects( const BoundingSphere<Real>& sphere )
	{
		float distance = LengthSquared( Center - sphere.Center );
		float radius = Radius;
		float radius2 = sphere.Radius;

		if( (radius * radius) + (2.0f * radius * radius2) + (radius2 * radius2) <= distance )
			return false;

		return true;
	}


public:
	Vector<Real, 3> Center;
	Real Radius;
	bool Defined;
};

/**
 * Return a new bounding sphere merged  from two given bounding sphere 
 */
template<typename Real>
BoundingSphere<Real> Merge(const BoundingSphere<Real>& sphere1, const BoundingSphere<Real>& sphere2 )
{
	BoundingSphere sphere;
	Vector<Real,3> difference = sphere2.Center - sphere1.Center;

	Real length = difference.Length();
	Real radius = sphere1.Radius;
	Real radius2 = sphere2.Radius;

	if( radius + radius2 >= length)
	{
		if( radius - radius2 >= length )
			return sphere1;

		if( radius2 - radius >= length )
			return sphere2;
	}

	Vector<Real,3> vector = difference * ( Real(1) / length );
	Real min = (std::min)( -radius, length - radius2 );
	Real max = ( (std::max)( radius, length + radius2 ) - min ) * Real(0.5);

	sphere.Center = sphere1.Center + vector * ( max + min );
	sphere.Radius = max;

	return sphere;
}


typedef BoundingSphere<float> BoundingSpheref;
typedef BoundingSphere<double> BoundingSphered;

}
#endif // BoundingSphere_h__
