#ifndef MathUtil_h__
#define MathUtil_h__

#include "Math.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Plane.hpp"

namespace MathLib {

/************************************************************************/
/* Transform Matrix                                                     */
/************************************************************************/
template<typename Real>
inline Matrix4<Real> 
CreateLookAtMatrixLH(const Vector<Real,3>& vEye, const Vector<Real,3>& vAt, const Vector<Real,3>& vUp);
		
template<typename Real>
inline Matrix4<Real> 
CreateLookAtMatrixRH(const Vector<Real,3>& vEye, const Vector<Real,3>& vAt, const Vector<Real,3>& vUp);
		
template<typename Real>
inline Matrix4<Real>
CreatePerspectiveFovLH(Real fovy, Real aspect,  Real zNear,  Real zFar);
		
template<typename Real>
inline Matrix4<Real> 
CreatePerspectiveFovRH(Real fovy, Real aspect, Real zNear,  Real zFar);
		
template<typename Real>
inline Matrix4<Real> 
CreatePerspectiveLH(Real width, Real height, Real zNear,  Real zFar);
		
template<typename Real>
inline Matrix4<Real> 
CreatePerspectiveRH(Real width, Real height, Real zNear,  Real zFar);

template<typename Real>
inline Matrix4<Real> 
CreateOrthographicLH(Real width, Real height, Real zNear,  Real zFar);

template<typename Real>
inline Matrix4<Real> 
CreateViewportMatrixD3D(Real left, Real top, Real width, Real height, Real zMin = Real(0),  Real zMax = Real(1));

template<typename Real>
inline Matrix4<Real> 
CreateRotationX(Real angle);
		
template<typename Real>
inline Matrix4<Real> 
CreateRotationY(Real angle);
		
template<typename Real>
inline Matrix4<Real> 
CreateRotationZ(Real angle);

template<typename Real>
inline Matrix4<Real> 
CreateRotationAxis(const Vector<Real,3>& axis, Real angle);

template<typename Real>
inline Matrix4<Real> 
CreateRotationYawPitchRoll(Real yaw, Real pitch, Real roll);

template<typename Real>
inline Matrix4<Real> 
CreateScaling(Real sX, Real sY, Real sZ);
		
template<typename Real>
inline Matrix4<Real> 
CreateTranslation(const Vector<Real,3>& trans);
		
template<typename Real>
inline Matrix4<Real> 
CreateTranslation(Real x, Real y, Real z);


template<typename Real>
inline Matrix4<Real>
RotationFromMatrix( const Matrix4<Real>& transformMat );

template<typename Real>
inline Vector<Real,3>
TranslationFromMatrix( const Matrix4<Real>& transformMat );

template<typename Real>
inline Vector<Real,3>
ScaleFromMatrix( const Matrix4<Real>& transformMat );

#include "MathUtil.inl"

}

#endif // MathUtil_h__
