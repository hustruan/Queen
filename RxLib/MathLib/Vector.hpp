#ifndef _Vector__H
#define _Vector__H

#include "Math.hpp"

namespace RxLib {

template< typename Real, int Size >
class Vector
{
public:
	typedef Real value_type;

public:
	Vector() { }

	Vector(Real fX, Real fY)
	{
		static_assert(Size == 2, "Vector dimension error.");

		mTuple[0] = fX;
		mTuple[1] = fY;
	}

	Vector(Real fX, Real fY, Real fZ)
	{
		static_assert(Size == 3, "Vector dimension error.");

		mTuple[0] = fX;
		mTuple[1] = fY;
		mTuple[2] = fZ;
	}

	Vector(Real fX, Real fY, Real fZ, Real fW)
	{
		static_assert(Size == 4, "Vector dimension error.");

		mTuple[0] = fX;
		mTuple[1] = fY;
		mTuple[2] = fZ;
		mTuple[3] = fW;
	}

	Vector(const Real* fTuple)
	{
		for(int i = 0; i < Size; i++)
			mTuple[i] = fTuple[i];
	}

	Vector(Real fScalar)
	{
		for(int i = 0; i < Size; i++)
			mTuple[i] = fScalar;
	}
		
	template<typename T>
	Vector(const Vector<T, Size>& rhs)
	{
		for(int i = 0; i < Size; i++)
			mTuple[i] = rhs[i];
	}

	// assignment
	inline Vector& operator= (const Vector& rhs)
	{
		for(int i = 0; i < Size; i++)
			mTuple[i] = rhs[i];
		return *this;
	}

	inline bool HasNaNs() const
	{
		bool anyHit = false;
		for(int i = 0; i < Size && !anyHit; i++)
			anyHit |= isnan(mTuple[i]);
		return anyHit;
	}

	// coordinate access
	inline const Real* operator() (void)const			{ return mTuple; }
	inline Real* operator() (void)						{ return mTuple; }
	inline Real operator[] (size_t i) const				{ return mTuple[i]; }
	inline Real& operator[] (size_t i)					{ return mTuple[i]; }
	inline Real X () const								{ return mTuple[0]; }
	inline Real& X ()									{ return mTuple[0]; }
	inline Real Y () const								{ static_assert(Size >= 2, "Vector dimension error."); return mTuple[1]; }	
	inline Real& Y ()									{ static_assert(Size >= 2, "Vector dimension error."); return mTuple[1]; }
	inline Real Z () const								{ static_assert(Size >= 3, "Vector dimension error."); return mTuple[2]; }
	inline Real& Z ()									{ static_assert(Size >= 3, "Vector dimension error."); return mTuple[2]; }
	inline Real W () const								{ static_assert(Size == 4, "Vector dimension error."); return mTuple[3]; }
	inline Real& W ()									{ static_assert(Size == 4, "Vector dimension error."); return mTuple[3]; }

	// arithmetic operations
	inline Vector operator+ (const Vector<Real, Size>& rhs) const
	{
		Vector<Real, Size> result;
		for(int i = 0; i < Size; i++)
			result[i] = rhs[i] + mTuple[i];

		return result;
	}

	inline Vector operator- (const Vector<Real, Size>& rhs) const
	{
		Vector<Real, Size> result;
		for(int i = 0; i < Size; i++)
			result[i] = mTuple[i]- rhs[i];

		return result;
	}

	inline Vector operator* (const Vector<Real, Size>& rhs) const
	{
		Vector<Real, Size> result;
		for(int i = 0; i < Size; i++)
			result[i] =  mTuple[i] * rhs[i];

		return result;
	}

	inline Vector operator* (Real fScalar) const
	{
		Vector<Real, Size> result;
		for(int i = 0; i < Size; i++)
			result[i] =  mTuple[i] * fScalar;

		return result;
	}

	inline Vector operator/ (Real scalar) const
	{
		Vector<Real, Size> result;

		for(int i = 0; i < Size; i++)
			result[i] = mTuple[i] / scalar;

		return result;
	}

	inline Vector operator- () const
	{
		Vector<Real, Size> result;
		for(int i = 0; i < Size; i++)
			result[i] =  -mTuple[i];

		return result;
	}

	// arithmetic updates
	inline Vector& operator+= (const Vector& rhs)
	{
		for(int i = 0; i < Size; i++)
			mTuple[i]  += rhs[i];
		return *this;
	}

	inline Vector& operator-= (const Vector& rhs)
	{
		for(int i = 0; i < Size; i++)
			mTuple[i]  -= rhs[i];
		return *this;
	}

	inline Vector& operator*= (Real scalar)
	{
		for(int i = 0; i < Size; i++)
			mTuple[i]  *= scalar;
		return *this;
	}

	inline Vector& operator/= (Real scalar)
	{
		for(int i = 0; i < Size; i++)
			mTuple[i]  /= scalar;

		return *this;
	}

	// comparison
	bool operator== (const Vector& rhs) const
	{
		return memcmp(mTuple,rhs.mTuple,Size*sizeof(Real)) == 0;
	}

	bool operator!= (const Vector& rhs) const
	{
		return memcmp(mTuple,rhs.mTuple,Size*sizeof(Real)) != 0;
	}

	// Zero Vector
	inline static const Vector& Zero()
	{
		static const Vector zero = Vector(Real(0));
		return zero;
	}

private:
	Real mTuple[Size];
};

//----------------------------------------------------------------------------
#ifdef USE_SIMD
#include "SSEUtility.hpp"

/**
 * Vector4 template specialization for SSE with float type
 */

template<>
class ALIGNED_16 Vector<float, 4>
{
public:
	typedef float value_type;

public:
	Vector() {}

	Vector(const __m128& v) : SSEData(v) {}

	Vector(float fX, float fY): SSEData(_mm_set_ps(0, 0, fY, fX)) {}

	Vector(float fX, float fY, float fZ) : SSEData(_mm_set_ps(0, fZ, fY, fX)) {}

	Vector(float fX, float fY, float fZ, float fW) : SSEData(_mm_set_ps(fW, fZ, fY, fX)) {}

	Vector(const float* fTuple) : SSEData(_mm_set_ps(fTuple[3], fTuple[2], fTuple[1], fTuple[0])) {}

	Vector(float fScalar) : SSEData(_mm_set1_ps(fScalar)) {}

	Vector(const Vector& rhs) :SSEData(rhs.SSEData) {}

	// assignment
	inline Vector& operator= (const Vector& rhs)
	{
		SSEData = rhs.SSEData;
		return *this;
	}

	// coordinate access
	inline const float* operator() (void)const				{ return mTuple; }
	inline float* operator() (void)							{ return mTuple; }
	inline float operator[] (size_t i) const				{ return mTuple[i]; }
	inline float& operator[] (size_t i)						{ return mTuple[i]; }
	inline float X () const									{ return mTuple[0]; }
	inline float& X ()										{ return mTuple[0]; }
	inline float Y () const									{ return mTuple[1]; }
	inline float& Y ()										{ return mTuple[1]; }
	inline float Z () const									{ return mTuple[2]; }
	inline float& Z ()										{ return mTuple[2]; }
	inline float W () const									{ return mTuple[3]; }
	inline float& W ()										{ return mTuple[3]; }

	// arithmetic operations
	inline Vector operator+ (const Vector& rhs) const 
	{ 
		return Vector(_mm_add_ps(rhs.SSEData, SSEData));
	}

	inline Vector operator- (const Vector& rhs) const
	{
		return Vector(_mm_sub_ps(SSEData, rhs.SSEData));
	}

	inline Vector operator* (const Vector& rhs) const 
	{
		return Vector(_mm_mul_ps(rhs.SSEData, SSEData));
	}

	inline Vector operator* (float fScalar) const
	{
		return Vector(_mm_mul_ps(SSEData, _mm_set1_ps(fScalar)));
	}

	inline Vector operator/ (float fScalar) const
	{
		return Vector(_mm_div_ps(SSEData, _mm_set1_ps(fScalar)));
	}

	inline Vector operator- () const
	{
		return Vector(_mm_sub_ps(_mm_setzero_ps(), SSEData));
	}

	// arithmetic updates
	inline Vector& operator+= (const Vector& rhs)
	{
		SSEData = _mm_add_ps(SSEData, rhs.SSEData);
		return *this;
	}

	inline Vector& operator-= (const Vector& rhs)
	{
		SSEData = _mm_sub_ps(SSEData, rhs.SSEData);
		return *this;
	}

	inline Vector& operator*= (float fScalar)
	{
		SSEData = _mm_mul_ps(SSEData, _mm_set1_ps(fScalar));
		return *this;
	}

	inline Vector& operator/= (float fScalar)
	{
		SSEData = _mm_div_ps(SSEData, _mm_set1_ps(fScalar));
		return *this;
	}

	// comparison
	bool operator== (const Vector& rhs) const
	{
		return memcmp(mTuple, rhs.mTuple, 4*sizeof(float)) == 0;
	}

	bool operator!= (const Vector& rhs) const
	{
		return memcmp(mTuple, rhs.mTuple, 4*sizeof(float)) != 0;
	}

	// Zero Vector
	inline static const Vector& Zero()
	{
		static const Vector zero = Vector(0, 0, 0, 0);
		return zero;
	}

public:
	union 
	{
		__m128 SSEData;
		float mTuple[4];
	};	
};

#endif

//---------------------------------------------------------------------------------------
template <class Real, int Size>
inline Vector<Real, Size> operator* (Real fScalar, const Vector<Real, Size>& rhs)
{
	Vector<Real, Size> result;
	for(int i = 0; i < Size; i++)
		result[i] =  fScalar * rhs[i];

	return result;
}

template< typename Real, int Size >
inline Real Length( const Vector<Real, Size>& vec )
{
	Real result = Real(0);
	for(int i = 0; i < Size; i++)
		result +=  vec[i] * vec[i] ;
	return sqrt(result);
}

template< typename Real, int Size >
inline Real LengthSquared( const Vector<Real, Size>& vec )
{
	Real result = Real(0);
	for(int i = 0; i < Size; i++)
		result +=  vec[i] * vec[i] ;
	return result;
}

template< typename Real, int Size >
inline Real Dot( const Vector<Real, Size>& lfs, const Vector<Real, Size>& rhs )
{
	Real result = Real(0);
	for(int i = 0; i < Size; i++)
		result += lfs[i] * rhs[i];
	return result;
}

template< typename Real, int Size >
inline Vector<Real, Size> Normalize(const Vector<Real, Size>& vec)
{
	Real fLength = LengthSquared(vec);
	Real fInvScalar = ((Real)1.0)/ ((Real)sqrt((double)fLength));
	return vec * fInvScalar;
}

template< typename Real, int Size >
inline Vector<Real, Size> Lerp(const Vector<Real, Size>& vec1, const Vector<Real, Size>& vec2, float t)
{
	Vector<Real, Size> retVal;

	for (uint32_t i = 0; i < Size; ++i)
	{
		retVal[i] = vec1[i] + (vec2[i] - vec1[i]) * t;
	}

	return retVal;
}

template<typename Real>
inline Vector<Real, 3> Cross( const Vector<Real, 3>& vec1, const Vector<Real, 3>& vec2 )
{
	return Vector<Real, 3>(
		vec1.Y() * vec2.Z() - vec1.Z() * vec2.Y(),
		vec1.Z() * vec2.X() - vec1.X() * vec2.Z(), 
		vec1.X() * vec2.Y() - vec1.Y() * vec2.X());
}

template<typename Real>
inline Vector<Real, 4> Cross( const Vector<Real, 4>& vec1, const Vector<Real, 4>& vec2 )
{
	return Vector<Real, 4>(
		vec1.Y() * vec2.Z() - vec1.Z() * vec2.Y(),
		vec1.Z() * vec2.X() - vec1.X() * vec2.Z(), 
		vec1.X() * vec2.Y() - vec1.Y() * vec2.X(),
		Real(0));
}

//-----------------------------------------------------------------------
#ifdef USE_SIMD

template<>
inline float Length( const Vector<float, 4>& vec )
{
	float result;
	__m128 dot = sse_dot_ss(vec.SSEData, vec.SSEData);
	_mm_store_ss(&result, _mm_sqrt_ps(dot));
	return result;
}

template<>
inline float LengthSquared( const Vector<float, 4>& vec )
{
	float result;
	_mm_store_ss(&result, sse_dot_ss(vec.SSEData, vec.SSEData));
	return result;
}

template<>
inline float Dot( const Vector<float, 4>& lfs, const Vector<float, 4>& rhs )
{
	float result;
	_mm_store_ss(&result, sse_dot_ps(lfs.SSEData, rhs.SSEData));
	return result;
}

template<>
inline Vector<float, 4> Lerp(const Vector<float, 4>& vec1, const Vector<float, 4>& vec2, float t)
{
	__m128 result;

	__m128 L = _mm_sub_ps( vec2.SSEData, vec1.SSEData );
	__m128 S = _mm_set_ps1( t );
	result = _mm_mul_ps( L, S );
	result = _mm_add_ps( vec1.SSEData, result );

	return Vector<float, 4>(result);
}

template<>
inline Vector<float, 4> Normalize(const Vector<float, 4>& vec)
{
	return Vector<float, 4>(sse_nrm_ps(vec.SSEData));
}

template<>
inline Vector<float, 4> Cross( const Vector<float, 4>& lfs, const Vector<float, 4>& rhs )
{
	return Vector<float, 4>(sse_xpd_ps( lfs.SSEData, rhs.SSEData));
}

#endif

typedef Vector<float, 2> float2;
typedef Vector<float,3> float3;
typedef Vector<float,4> float4;

typedef Vector<int, 2> int2;


} // Namespace RcEngine
#endif