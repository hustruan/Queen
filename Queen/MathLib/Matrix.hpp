#ifndef _Matrix__H
#define _Matrix__H

#include "Vector.hpp"

namespace MathLib {

#define MatrixItem(iRow, iCol) ( iRow*4 + iCol )

template<typename Real>
class Matrix4
{
public:
	typedef Real value_type;

public:
	Matrix4() 
	{
		MakeZero();
	}

	Matrix4(const Real* rhs)
	{
		memcpy(Elements, rhs, sizeof(Elements));
	}
	
	Matrix4(Real f11, Real f12, Real f13, Real f14,
		Real f21, Real f22, Real f23, Real f24,
		Real f31, Real f32, Real f33, Real f34,
		Real f41, Real f42, Real f43, Real f44)
	{
		M11	= f11; M12 = f12; M13 = f13; M14 = f14; 
		M21	= f21; M22 = f22; M23 = f23; M24 = f24; 
		M31	= f31; M32 = f32; M33 = f33; M34 = f34; 
		M41	= f41; M42 = f42; M43 = f43; M44 = f44; 
	}

	template<typename T>
	Matrix4(const Matrix4<T>& rhs)
	{
		for(int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
			{
				Elements[MatrixItem(i, j)] = static_cast<Real>(rhs.Elements[MatrixItem(i, j)]);
			}
	}

	void MakeIdentity()
	{
		Elements[ 0] = (Real)1.0;
		Elements[ 1] = (Real)0.0;
		Elements[ 2] = (Real)0.0;
		Elements[ 3] = (Real)0.0;
		Elements[ 4] = (Real)0.0;
		Elements[ 5] = (Real)1.0;
		Elements[ 6] = (Real)0.0;
		Elements[ 7] = (Real)0.0;
		Elements[ 8] = (Real)0.0;
		Elements[ 9] = (Real)0.0;
		Elements[10] = (Real)1.0;
		Elements[11] = (Real)0.0;
		Elements[12] = (Real)0.0;
		Elements[13] = (Real)0.0;
		Elements[14] = (Real)0.0;
		Elements[15] = (Real)1.0;
	}

	void MakeZero()
	{
		memset(Elements, 0, sizeof(Elements));
	}

	// member access
	inline const Real* operator() () const							{ return Elements; }
	inline Real* operator() ()										{ return Elements; }

	inline Real operator[] (int index) const					{ return Elements[index]; }
	inline Real& operator[] (int index)							{ return Elements[index]; }

	inline Real operator() (int iRow, int iCol) const		{ return Elements[MatrixItem(iRow, iCol)]; }
	inline Real& operator() (int iRow, int iCol)			{ return Elements[MatrixItem(iRow, iCol)]; }

	void SetRow (int iRow, const Vector<Real, 4>& rhs)
	{
		Elements[MatrixItem(iRow, 0)] = rhs[0];
		Elements[MatrixItem(iRow, 1)] = rhs[1];
		Elements[MatrixItem(iRow, 2)] = rhs[2];
		Elements[MatrixItem(iRow, 3)] = rhs[3];
	}

	Vector<Real, 4> GetRow (int iRow) const
	{
		return Vector<Real, 4>(Elements[MatrixItem(iRow, 0)], Elements[MatrixItem(iRow, 1)],
			Elements[MatrixItem(iRow, 2)], Elements[MatrixItem(iRow, 3)]);	
	}

	void SetColumn (int iCol, const Vector<Real, 4>& rhs)
	{
		Elements[MatrixItem(0, iCol)] = rhs[0];
		Elements[MatrixItem(1, iCol)]= rhs[1];
		Elements[MatrixItem(2, iCol)] = rhs[2];
		Elements[MatrixItem(3, iCol)] = rhs[3];
	}

	Vector<Real, 4> GetColumn (int iCol) const
	{
		return Vector<Real, 4>(Elements[MatrixItem(0, iCol)], Elements[MatrixItem(1, iCol)],
			Elements[MatrixItem(2, iCol)], Elements[MatrixItem(3, iCol)]);
	}
		
	// comparison
	bool operator== (const Matrix4& rhs) const
	{
		 return memcmp(Elements,rhs.Elements,sizeof(Elements)) == 0;
	}

	bool operator!= (const Matrix4& rhs) const
	{
		 return memcmp(Elements,rhs.Elements,sizeof(Elements)) != 0;
	}
		
	// arithmetic operations
	inline Matrix4 operator+ (const Matrix4& rhs) const
	{
		return Matrix4<Real>(
			Elements[ 0] + rhs.Elements[ 0],
			Elements[ 1] + rhs.Elements[ 1],
			Elements[ 2] + rhs.Elements[ 2],
			Elements[ 3] + rhs.Elements[ 3],
			Elements[ 4] + rhs.Elements[ 4],
			Elements[ 5] + rhs.Elements[ 5],
			Elements[ 6] + rhs.Elements[ 6],
			Elements[ 7] + rhs.Elements[ 7],
			Elements[ 8] + rhs.Elements[ 8],
			Elements[ 9] + rhs.Elements[ 9],
			Elements[10] + rhs.Elements[10],
			Elements[11] + rhs.Elements[11],
			Elements[12] + rhs.Elements[12],
			Elements[13] + rhs.Elements[13],
			Elements[14] + rhs.Elements[14],
			Elements[15] + rhs.Elements[15]);
	}

	inline Matrix4 operator- (const Matrix4& rhs) const
	{
		return Matrix4<Real>(
			Elements[ 0] - rhs.Elements[ 0],
			Elements[ 1] - rhs.Elements[ 1],
			Elements[ 2] - rhs.Elements[ 2],
			Elements[ 3] - rhs.Elements[ 3],
			Elements[ 4] - rhs.Elements[ 4],
			Elements[ 5] - rhs.Elements[ 5],
			Elements[ 6] - rhs.Elements[ 6],
			Elements[ 7] - rhs.Elements[ 7],
			Elements[ 8] - rhs.Elements[ 8],
			Elements[ 9] - rhs.Elements[ 9],
			Elements[10] - rhs.Elements[10],
			Elements[11] - rhs.Elements[11],
			Elements[12] - rhs.Elements[12],
			Elements[13] - rhs.Elements[13],
			Elements[14] - rhs.Elements[14],
			Elements[15] - rhs.Elements[15]);
	}

	inline Matrix4 operator* (Real fScalar) const
	{
		return Matrix4<Real>(
			fScalar*Elements[ 0],
			fScalar*Elements[ 1],
			fScalar*Elements[ 2],
			fScalar*Elements[ 3],
			fScalar*Elements[ 4],
			fScalar*Elements[ 5],
			fScalar*Elements[ 6],
			fScalar*Elements[ 7],
			fScalar*Elements[ 8],
			fScalar*Elements[ 9],
			fScalar*Elements[10],
			fScalar*Elements[11],
			fScalar*Elements[12],
			fScalar*Elements[13],
			fScalar*Elements[14],
			fScalar*Elements[15]);
	}

	inline Matrix4 operator/ (Real fScalar) const
	{
		Real fInvScalar = ((Real)1.0)/fScalar;
		return Matrix4<Real>(
			fInvScalar*Elements[ 0],
			fInvScalar*Elements[ 1],
			fInvScalar*Elements[ 2],
			fInvScalar*Elements[ 3],
			fInvScalar*Elements[ 4],
			fInvScalar*Elements[ 5],
			fInvScalar*Elements[ 6],
			fInvScalar*Elements[ 7],
			fInvScalar*Elements[ 8],
			fInvScalar*Elements[ 9],
			fInvScalar*Elements[10],
			fInvScalar*Elements[11],
			fInvScalar*Elements[12],
			fInvScalar*Elements[13],
			fInvScalar*Elements[14],
			fInvScalar*Elements[15]);
	}

	inline Matrix4 operator- () const
	{
		return Matrix4<Real>(-Elements[ 0], -Elements[ 1], -Elements[ 2], -Elements[ 3],
			-Elements[ 4], -Elements[ 5], -Elements[ 6], -Elements[ 7], 
			-Elements[ 8], -Elements[ 9], -Elements[10], -Elements[11], 
			-Elements[12], -Elements[13], -Elements[14], -Elements[15]);
	}

	// arithmetic updates
	inline Matrix4& operator+= (const Matrix4& rhs)
	{
		Elements[ 0] += rhs.Elements[ 0];
		Elements[ 1] += rhs.Elements[ 1];
		Elements[ 2] += rhs.Elements[ 2];
		Elements[ 3] += rhs.Elements[ 3];
		Elements[ 4] += rhs.Elements[ 4];
		Elements[ 5] += rhs.Elements[ 5];
		Elements[ 6] += rhs.Elements[ 6];
		Elements[ 7] += rhs.Elements[ 7];
		Elements[ 8] += rhs.Elements[ 8];
		Elements[ 9] += rhs.Elements[ 9];
		Elements[10] += rhs.Elements[10];
		Elements[11] += rhs.Elements[11];
		Elements[12] += rhs.Elements[12];
		Elements[13] += rhs.Elements[13];
		Elements[14] += rhs.Elements[14];
		Elements[15] += rhs.Elements[15];
		return *this;
	}

	inline Matrix4& operator-= (const Matrix4& rhs)
	{
		Elements[ 0] -= rhs.Elements[ 0];
		Elements[ 1] -= rhs.Elements[ 1];
		Elements[ 2] -= rhs.Elements[ 2];
		Elements[ 3] -= rhs.Elements[ 3];
		Elements[ 4] -= rhs.Elements[ 4];
		Elements[ 5] -= rhs.Elements[ 5];
		Elements[ 6] -= rhs.Elements[ 6];
		Elements[ 7] -= rhs.Elements[ 7];
		Elements[ 8] -= rhs.Elements[ 8];
		Elements[ 9] -= rhs.Elements[ 9];
		Elements[10] -= rhs.Elements[10];
		Elements[11] -= rhs.Elements[11];
		Elements[12] -= rhs.Elements[12];
		Elements[13] -= rhs.Elements[13];
		Elements[14] -= rhs.Elements[14];
		Elements[15] -= rhs.Elements[15];
		return *this;
	}

	inline Matrix4& operator*= (Real fScalar)
	{
		Elements[ 0] *= fScalar;
		Elements[ 1] *= fScalar;
		Elements[ 2] *= fScalar;
		Elements[ 3] *= fScalar;
		Elements[ 4] *= fScalar;
		Elements[ 5] *= fScalar;
		Elements[ 6] *= fScalar;
		Elements[ 7] *= fScalar;
		Elements[ 8] *= fScalar;
		Elements[ 9] *= fScalar;
		Elements[10] *= fScalar;
		Elements[11] *= fScalar;
		Elements[12] *= fScalar;
		Elements[13] *= fScalar;
		Elements[14] *= fScalar;
		Elements[15] *= fScalar;
		return *this;
	}

	inline Matrix4& operator/= (Real fScalar)
	{
		Real fInvScalar = ((Real)1.0)/fScalar;
		Elements[ 0] *= fInvScalar;
		Elements[ 1] *= fInvScalar;
		Elements[ 2] *= fInvScalar;
		Elements[ 3] *= fInvScalar;
		Elements[ 4] *= fInvScalar;
		Elements[ 5] *= fInvScalar;
		Elements[ 6] *= fInvScalar;
		Elements[ 7] *= fInvScalar;
		Elements[ 8] *= fInvScalar;
		Elements[ 9] *= fInvScalar;
		Elements[10] *= fInvScalar;
		Elements[11] *= fInvScalar;
		Elements[12] *= fInvScalar;
		Elements[13] *= fInvScalar;
		Elements[14] *= fInvScalar;
		Elements[15] *= fInvScalar;

		return *this;
	}

	// return an identity matrix
	inline static const Matrix4& Identity()
	{
		static Matrix4<Real> out( Real(1), Real(0), Real(0), Real(0),
			Real(0), Real(1), Real(0), Real(0),
			Real(0), Real(0), Real(1), Real(0),
			Real(0), Real(0), Real(0), Real(1));
		return out;
	}
	
public:

	union
	{
		struct{ Real Elements[16]; };
		struct
		{
			Real M11, M12, M13, M14;
			Real M21, M22, M23, M24;
			Real M31, M32, M33, M34;
			Real M41, M42, M43, M44;
		};
	};		
};

//---------------------------------------------------------------------------------------------
#ifdef USE_SIMD

/**
 * Matrix4 template specialization for SSE with float type
 */

template<>
class ALIGNED_16 Matrix4<float>
{
public:
	typedef float value_type;

public:
	Matrix4() {}
	Matrix4(const float* rhs)
	{
		memcpy(Elements, rhs, sizeof(Elements));
	}

	Matrix4(float f11, float f12, float f13, float f14,
		float f21, float f22, float f23, float f24,
		float f31, float f32, float f33, float f34,
		float f41, float f42, float f43, float f44)
	{
		M11	= f11; M12 = f12; M13 = f13; M14 = f14; 
		M21	= f21; M22 = f22; M23 = f23; M24 = f24; 
		M31	= f31; M32 = f32; M33 = f33; M34 = f34; 
		M41	= f41; M42 = f42; M43 = f43; M44 = f44; 
	}

	template<typename T>
	Matrix4(const Matrix4<T>& rhs)
	{
		for(int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
			{
				Elements[MatrixItem(i, j)] = static_cast<float>(rhs.Elements[MatrixItem(i, j)]);
			}
	}

	void MakeIdentity()
	{
		Elements[ 0] = (float)1.0;
		Elements[ 1] = (float)0.0;
		Elements[ 2] = (float)0.0;
		Elements[ 3] = (float)0.0;
		Elements[ 4] = (float)0.0;
		Elements[ 5] = (float)1.0;
		Elements[ 6] = (float)0.0;
		Elements[ 7] = (float)0.0;
		Elements[ 8] = (float)0.0;
		Elements[ 9] = (float)0.0;
		Elements[10] = (float)1.0;
		Elements[11] = (float)0.0;
		Elements[12] = (float)0.0;
		Elements[13] = (float)0.0;
		Elements[14] = (float)0.0;
		Elements[15] = (float)1.0;
	}

	void MakeZero()
	{
		memset(Elements, 0, sizeof(Elements));
	}

	// member access
	inline const float* operator() () const							{ return Elements; }
	inline float* operator() ()										{ return Elements; }

	inline float operator[] (int index) const					{ return Elements[index]; }
	inline float& operator[] (int index)						{ return Elements[index]; }

	inline float operator() (int iRow, int iCol) const		{ return Elements[MatrixItem(iRow, iCol)]; }
	inline float& operator() (int iRow, int iCol)			{ return Elements[MatrixItem(iRow, iCol)]; }

	void SetRow (int iRow, const Vector<float, 4>& rhs)			{ SSEData[iRow] = rhs.SSEData; }
	Vector<float, 4> GetRow (int iRow) const					{ return Vector<float, 4>(SSEData[iRow]); }

	void SetColumn (int iCol, const Vector<float, 4>& rhs)
	{
		Elements[MatrixItem(0, iCol)] = rhs[0];
		Elements[MatrixItem(1, iCol)]= rhs[1];
		Elements[MatrixItem(2, iCol)] = rhs[2];
		Elements[MatrixItem(3, iCol)] = rhs[3];
	}

	Vector<float, 4> GetColumn (int iCol) const
	{
		return Vector<float, 4>(Elements[MatrixItem(0, iCol)], Elements[MatrixItem(1, iCol)],
			Elements[MatrixItem(2, iCol)], Elements[MatrixItem(3, iCol)]);
	}

	// comparison
	bool operator== (const Matrix4& rhs) const
	{
		 return memcmp(Elements,rhs.Elements,16*sizeof(float)) == 0;
	}

	bool operator!= (const Matrix4& rhs) const
	{
		return memcmp(Elements,rhs.Elements,16*sizeof(float)) != 0;
	}

	// arithmetic operations
	inline Matrix4 operator+ (const Matrix4& rhs) const
	{
		return Matrix4(_mm_add_ps(SSEData[0], rhs.SSEData[0]),
			_mm_add_ps(SSEData[1], rhs.SSEData[1]),
			_mm_add_ps(SSEData[2], rhs.SSEData[2]),
			_mm_add_ps(SSEData[3], rhs.SSEData[3]));
	}

	inline Matrix4 operator- (const Matrix4& rhs) const
	{
		return Matrix4(_mm_sub_ps(SSEData[0], rhs.SSEData[0]),
			_mm_sub_ps(SSEData[1], rhs.SSEData[1]),
			_mm_sub_ps(SSEData[2], rhs.SSEData[2]),
			_mm_sub_ps(SSEData[3], rhs.SSEData[3]));
	}

	inline Matrix4 operator* (float fScalar) const
	{
		__m128 operand = _mm_set_ps1(fScalar);
		return Matrix4(_mm_mul_ps(SSEData[0], operand),
			_mm_mul_ps(SSEData[1], operand),
			_mm_mul_ps(SSEData[2], operand),
			_mm_mul_ps(SSEData[3], operand));
	}

	inline Matrix4 operator/ (float fScalar) const
	{
		__m128 operand = _mm_div_ps(_mm_set_ps1(1.0f), _mm_set_ps1(fScalar));
		return Matrix4(_mm_mul_ps(SSEData[0], operand),
			_mm_mul_ps(SSEData[1], operand),
			_mm_mul_ps(SSEData[2], operand),
			_mm_mul_ps(SSEData[3], operand));
	}

	inline Matrix4 operator- () const
	{
		const __m128 zero = _mm_setzero_ps();

		return Matrix4(_mm_sub_ps(zero, SSEData[0]),
			_mm_sub_ps(zero, SSEData[1]),
			_mm_sub_ps(zero, SSEData[2]),
			_mm_sub_ps(zero, SSEData[3]));	
	}

	// arithmetic updates
	inline Matrix4& operator+= (const Matrix4& rhs)
	{
		SSEData[0] = _mm_add_ps(SSEData[0], rhs.SSEData[0]);
		SSEData[1] = _mm_add_ps(SSEData[1], rhs.SSEData[1]);
		SSEData[2] = _mm_add_ps(SSEData[2], rhs.SSEData[2]);
		SSEData[3] = _mm_add_ps(SSEData[3], rhs.SSEData[3]);
		return *this;
	}

	inline Matrix4& operator-= (const Matrix4& rhs)
	{
		SSEData[0] = _mm_sub_ps(SSEData[0], rhs.SSEData[0]);
		SSEData[1] = _mm_sub_ps(SSEData[1], rhs.SSEData[1]);
		SSEData[2] = _mm_sub_ps(SSEData[2], rhs.SSEData[2]);
		SSEData[3] = _mm_sub_ps(SSEData[3], rhs.SSEData[3]);
		return *this;
	}

	inline Matrix4& operator*= (float fScalar)
	{
		__m128 operand = _mm_set_ps1(fScalar);
		SSEData[0] = _mm_mul_ps(SSEData[0], operand);
		SSEData[1] = _mm_mul_ps(SSEData[1], operand);
		SSEData[2] = _mm_mul_ps(SSEData[2], operand);
		SSEData[3] = _mm_mul_ps(SSEData[3], operand);
		return *this;
	}

	inline Matrix4& operator/= (float fScalar)
	{
		__m128 operand = _mm_div_ps(_mm_set_ps1(1.0f), _mm_set_ps1(fScalar));
		SSEData[0] = _mm_mul_ps(SSEData[0], operand);
		SSEData[1] = _mm_mul_ps(SSEData[1], operand);
		SSEData[2] = _mm_mul_ps(SSEData[2], operand);
		SSEData[3] = _mm_mul_ps(SSEData[3], operand);
		return *this;
	}

public:

	union
	{
		struct { __m128 SSEData[4]; };
		struct{ float Elements[16]; };
		struct
		{
			float M11, M12, M13, M14;
			float M21, M22, M23, M24;
			float M31, M32, M33, M34;
			float M41, M42, M43, M44;
		};
	};
	
private:
	Matrix4(const __m128& row1, const __m128& row2, const __m128& row3, const __m128& row4)
	{
		SSEData[0] = row1; 
		SSEData[1] = row2;
		SSEData[2] = row3;
		SSEData[3] = row4;
	}

};

#endif

//---------------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real, 4> operator* (const Vector<Real, 4>& vec, const Matrix4<Real>& mat)
{
	return Vector<Real, 4>(
		vec[0]*mat.M11 + vec[1]*mat.M21 + vec[2]*mat.M31 + vec[3]*mat.M41,
		vec[0]*mat.M12 + vec[1]*mat.M22 + vec[2]*mat.M32 + vec[3]*mat.M42,
		vec[0]*mat.M13 + vec[1]*mat.M23 + vec[2]*mat.M33 + vec[3]*mat.M43,
		vec[0]*mat.M14 + vec[1]*mat.M24 + vec[2]*mat.M34 + vec[3]*mat.M44);
}

template<typename Real>
inline Vector<Real, 4> operator* (const Matrix4<Real>& mat, const Vector<Real, 4>& vec)
{
	return Vector<Real, 4>(
				mat.M11*vec[0] + mat.M12*vec[1] + mat.M13*vec[2] + mat.M14*vec[3],
				mat.M21*vec[0] + mat.M22*vec[1] + mat.M23*vec[2] + mat.M24*vec[3],
				mat.M31*vec[0] + mat.M32*vec[1] + mat.M33*vec[2] + mat.M34*vec[3],
				mat.M41*vec[0] + mat.M42*vec[1] + mat.M43*vec[2] + mat.M44*vec[3]);
}

template<typename Real>
inline Matrix4<Real> operator*(const Matrix4<Real>& M1, Matrix4<Real>& M2)
{
	Matrix4<Real> result;

	// Cache the invariants in registers
	float x = M1.M11; 
	float y = M1.M12;
	float z = M1.M13;
	float w = M1.M14;

	// Perform the operation on the first row
	result.M11 = (M2.M11*x)+(M2.M21*y)+(M2.M31*z)+(M2.M41*w);
	result.M12 = (M2.M12*x)+(M2.M22*y)+(M2.M32*z)+(M2.M42*w);
	result.M13 = (M2.M13*x)+(M2.M23*y)+(M2.M33*z)+(M2.M43*w);
	result.M14 = (M2.M14*x)+(M2.M24*y)+(M2.M34*z)+(M2.M44*w);

	// Repeat for all the other rows
	x = M1.M21; 
	y = M1.M22; 
	z = M1.M23; 
	w = M1.M24; 
	result.M21 = (M2.M11*x)+(M2.M21*y)+(M2.M31*z)+(M2.M41*w);
	result.M22 = (M2.M12*x)+(M2.M22*y)+(M2.M32*z)+(M2.M42*w);
	result.M23 = (M2.M13*x)+(M2.M23*y)+(M2.M33*z)+(M2.M43*w);
	result.M24 = (M2.M14*x)+(M2.M24*y)+(M2.M34*z)+(M2.M44*w);

	x = M1.M31; 
	y = M1.M32; 
	z = M1.M33; 
	w = M1.M34; 
	result.M31 = (M2.M11*x)+(M2.M21*y)+(M2.M31*z)+(M2.M41*w);
	result.M32 = (M2.M12*x)+(M2.M22*y)+(M2.M32*z)+(M2.M42*w);
	result.M33 = (M2.M13*x)+(M2.M23*y)+(M2.M33*z)+(M2.M43*w);
	result.M34 = (M2.M14*x)+(M2.M24*y)+(M2.M34*z)+(M2.M44*w);

	x = M1.M41; 
	y = M1.M42; 
	z = M1.M43; 
	w = M1.M44; 
	result.M41 = (M2.M11*x)+(M2.M21*y)+(M2.M31*z)+(M2.M41*w);
	result.M42 = (M2.M12*x)+(M2.M22*y)+(M2.M32*z)+(M2.M42*w);
	result.M43 = (M2.M13*x)+(M2.M23*y)+(M2.M33*z)+(M2.M43*w);
	result.M44 = (M2.M14*x)+(M2.M24*y)+(M2.M34*z)+(M2.M44*w);

	return result;
}

template<typename Real>
Matrix4<Real> MatrixMultiply(const Matrix4<Real>& M1, Matrix4<Real>& M2)
{
	Matrix4<Real> result;

	// Cache the invariants in registers
	float x = M1.M11; 
	float y = M1.M12;
	float z = M1.M13;
	float w = M1.M14;

	// Perform the operation on the first row
	result.M11 = (M2.M11*x)+(M2.M21*y)+(M2.M31*z)+(M2.M41*w);
	result.M12 = (M2.M12*x)+(M2.M22*y)+(M2.M32*z)+(M2.M42*w);
	result.M13 = (M2.M13*x)+(M2.M23*y)+(M2.M33*z)+(M2.M43*w);
	result.M14 = (M2.M14*x)+(M2.M24*y)+(M2.M34*z)+(M2.M44*w);

	// Repeat for all the other rows
	x = M1.M21; 
	y = M1.M22; 
	z = M1.M23; 
	w = M1.M24; 
	result.M21 = (M2.M11*x)+(M2.M21*y)+(M2.M31*z)+(M2.M41*w);
	result.M22 = (M2.M12*x)+(M2.M22*y)+(M2.M32*z)+(M2.M42*w);
	result.M23 = (M2.M13*x)+(M2.M23*y)+(M2.M33*z)+(M2.M43*w);
	result.M24 = (M2.M14*x)+(M2.M24*y)+(M2.M34*z)+(M2.M44*w);

	x = M1.M31; 
	y = M1.M32; 
	z = M1.M33; 
	w = M1.M34; 
	result.M31 = (M2.M11*x)+(M2.M21*y)+(M2.M31*z)+(M2.M41*w);
	result.M32 = (M2.M12*x)+(M2.M22*y)+(M2.M32*z)+(M2.M42*w);
	result.M33 = (M2.M13*x)+(M2.M23*y)+(M2.M33*z)+(M2.M43*w);
	result.M34 = (M2.M14*x)+(M2.M24*y)+(M2.M34*z)+(M2.M44*w);

	x = M1.M41; 
	y = M1.M42; 
	z = M1.M43; 
	w = M1.M44; 
	result.M41 = (M2.M11*x)+(M2.M21*y)+(M2.M31*z)+(M2.M41*w);
	result.M42 = (M2.M12*x)+(M2.M22*y)+(M2.M32*z)+(M2.M42*w);
	result.M43 = (M2.M13*x)+(M2.M23*y)+(M2.M33*z)+(M2.M43*w);
	result.M44 = (M2.M14*x)+(M2.M24*y)+(M2.M34*z)+(M2.M44*w);

	return result;
}

// Compute inverse matrix
template<typename Real>
Matrix4<Real> MatrixInverse(const Matrix4<Real>& mat)
{
	Real v0 = mat.M31 * mat.M42 - mat.M32 * mat.M41;
	Real v1 = mat.M31 * mat.M43 - mat.M33 * mat.M41;
	Real v2 = mat.M31 * mat.M44 - mat.M34 * mat.M41;
	Real v3 = mat.M32 * mat.M43 - mat.M33 * mat.M42;
	Real v4 = mat.M32 * mat.M44 - mat.M34 * mat.M42;
	Real v5 = mat.M33 * mat.M44 - mat.M34 * mat.M43;

	Real i11 = (v5 * mat.M22 - v4 * mat.M23 + v3 * mat.M24);
	Real i21 = -(v5 * mat.M21 - v2 * mat.M23 + v1 * mat.M24);
	Real i31 = (v4 * mat.M21 - v2 * mat.M22 + v0 * mat.M24);
	Real i41 = -(v3 * mat.M21 - v1 * mat.M22 + v0 * mat.M23);

	Real invDet = 1.0f / (i11 * mat.M11 + i21 * mat.M12 + i31 * mat.M13 + i41 * mat.M14);

	i11 *= invDet;
	i21 *= invDet;
	i31 *= invDet;
	i41 *= invDet;

	Real i12 = -(v5 * mat.M12 - v4 * mat.M13 + v3 * mat.M14) * invDet;
	Real i22 = (v5 * mat.M11 - v2 * mat.M13 + v1 * mat.M14) * invDet;
	Real i32 = -(v4 * mat.M11 - v2 * mat.M12 + v0 * mat.M14) * invDet;
	Real i42 = (v3 * mat.M11 - v1 * mat.M12 + v0 * mat.M13) * invDet;

	v0 = mat.M21 * mat.M42 - mat.M22 * mat.M41;
	v1 = mat.M21 * mat.M43 - mat.M23 * mat.M41;
	v2 = mat.M21 * mat.M44 - mat.M24 * mat.M41;
	v3 = mat.M22 * mat.M43 - mat.M23 * mat.M42;
	v4 = mat.M22 * mat.M44 - mat.M24 * mat.M42;
	v5 = mat.M23 * mat.M44 - mat.M24 * mat.M43;

	Real i13 = (v5 * mat.M12 - v4 * mat.M13 + v3 * mat.M14) * invDet;
	Real i23 = -(v5 * mat.M11 - v2 * mat.M13 + v1 * mat.M14) * invDet;
	Real i33 = (v4 * mat.M11 - v2 * mat.M12 + v0 * mat.M14) * invDet;
	Real i43 = -(v3 * mat.M11 - v1 * mat.M12 + v0 * mat.M13) * invDet;

	v0 = mat.M32 * mat.M21 - mat.M31 * mat.M22;
	v1 = mat.M33 * mat.M21 - mat.M31 * mat.M23;
	v2 = mat.M34 * mat.M21 - mat.M31 * mat.M24;
	v3 = mat.M33 * mat.M22 - mat.M32 * mat.M23;
	v4 = mat.M34 * mat.M22 - mat.M32 * mat.M24;
	v5 = mat.M34 * mat.M23 - mat.M33 * mat.M24;

	Real i14 = -(v5 * mat.M12 - v4 * mat.M13 + v3 * mat.M14) * invDet;
	Real i24 = (v5 * mat.M11 - v2 * mat.M13 + v1 * mat.M14) * invDet;
	Real i34 = -(v4 * mat.M11 - v2 * mat.M12 + v0 * mat.M14) * invDet;
	Real i44 = (v3 * mat.M11 - v1 * mat.M12 + v0 * mat.M13) * invDet;

	return Matrix4<Real>(
		i11, i12, i13, i14,
		i21, i22, i23, i24,
		i31, i32, i33, i34,
		i41, i42, i43, i44);
}

// Compute inverse matrix
template<typename Real>
inline Matrix4<Real> MatrixTranspose(const Matrix4<Real>& mat)
{
	return Matrix4<Real>(mat.Elements[0], mat.Elements[4], mat.Elements[8],  mat.Elements[12],
						 mat.Elements[1], mat.Elements[5], mat.Elements[9],  mat.Elements[13], 
						 mat.Elements[2], mat.Elements[6], mat.Elements[10], mat.Elements[14],
						 mat.Elements[3], mat.Elements[7], mat.Elements[11], mat.Elements[15]);
}

template<typename Real>
Real MatrixDeterminant(const Matrix4<Real>& mat)
{
	Real fA0 = mat.Elements[ 0]*mat.Elements[ 5] - mat.Elements[ 1]*mat.Elements[ 4];
	Real fA1 = mat.Elements[ 0]*mat.Elements[ 6] - mat.Elements[ 2]*mat.Elements[ 4];
	Real fA2 = mat.Elements[ 0]*mat.Elements[ 7] - mat.Elements[ 3]*mat.Elements[ 4];
	Real fA3 = mat.Elements[ 1]*mat.Elements[ 6] - mat.Elements[ 2]*mat.Elements[ 5];
	Real fA4 = mat.Elements[ 1]*mat.Elements[ 7] - mat.Elements[ 3]*mat.Elements[ 5];
	Real fA5 = mat.Elements[ 2]*mat.Elements[ 7] - mat.Elements[ 3]*mat.Elements[ 6];
	Real fB0 = mat.Elements[ 8]*mat.Elements[13] - mat.Elements[ 9]*mat.Elements[12];
	Real fB1 = mat.Elements[ 8]*mat.Elements[14] - mat.Elements[10]*mat.Elements[12];
	Real fB2 = mat.Elements[ 8]*mat.Elements[15] - mat.Elements[11]*mat.Elements[12];
	Real fB3 = mat.Elements[ 9]*mat.Elements[14] - mat.Elements[10]*mat.Elements[13];
	Real fB4 = mat.Elements[ 9]*mat.Elements[15] - mat.Elements[11]*mat.Elements[13];
	Real fB5 = mat.Elements[10]*mat.Elements[15] - mat.Elements[11]*mat.Elements[14];
	Real fDet = fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
	return fDet;
}
//-----------------------------------------------------------------------------
#ifdef USE_SIMD

template<>
inline Matrix4<float> operator* (const Matrix4<float>& M1, Matrix4<float>& M2)
{
	Matrix4<float> result;
	sse_mul_ps(M1.SSEData, M2.SSEData, result.SSEData);
	return result;
}

template<>
inline Vector<float, 4> operator* (const Vector<float, 4>& vec, const Matrix4<float>& mat)
{
	return Vector<float, 4>(sse_mul_ps(vec.SSEData, mat.SSEData));
}

template<>
inline Vector<float, 4> operator* (const Matrix4<float>& mat, const Vector<float, 4>& vec)   
{
	return Vector<float, 4>(sse_mul_ps(mat.SSEData, vec.SSEData));
}

// 测试发现不用SSE好像更快
template<>
float MatrixDeterminant(const Matrix4<float>& mat)
{
	float result;
	_mm_store_ss(&result, sse_det_ps(mat.SSEData));
	return result;
}

//To do: Bug
//template<>
//Matrix4<float> MatrixMultiply(const Matrix4<float>& M1, Matrix4<float>& M2)
//{
//	Matrix4<float> result;
//	sse_mul_ps(M1.SSEData, M2.SSEData, result.SSEData);
//	return result;
//}

template<>
Matrix4<float> MatrixInverse(const Matrix4<float>& mat)
{
	Matrix4<float> result;
	sse_inverse_ps(mat.SSEData, result.SSEData);
	return result;
}

#endif

typedef Matrix4<float> float44;


} // Namespace RcEngine

#endif // _Matrix__H