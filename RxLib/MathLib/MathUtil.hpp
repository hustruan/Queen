#ifndef MathUtil_h__
#define MathUtil_h__

#include "Math.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Plane.hpp"
#include "BoundingBox.hpp"

namespace RxLib {

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

/**
 * Transforms the vector by the matrix.
 */
template<typename Real>
inline Vector<Real, 3> 
Transform(const Vector<Real, 3>& vec, const Matrix4<Real>& mat);

/**
 * Transforms the vector by the matrix, projecting the result back into w=1.
 */
template<typename Real>
inline Vector<Real, 3> 
TransformCoord(const Vector<Real, 3>& vec, const Matrix4<Real>& mat);

/**
* Projects a 3D vector from object space into screen space.
 */
template<typename Real>
inline Vector<Real, 3> 
Project(const Vector<Real, 3>& vec, int x, int y, unsigned int width, unsigned int height, Real minDepth, Real maxDepth, const Matrix4<Real>& mvp);

template<typename Real>
inline Vector<Real, 3> 
Unproject(const Vector<Real, 3>& vec,  int x, int y, unsigned int width, unsigned int height, Real minDepth, Real maxDepth, const Matrix4<Real>& mvp);

template<typename Real>
inline Vector<Real, 3> 
TransformNormal(const Vector<Real, 3>& normal, const Matrix4<Real>& mat);

template<typename Real>
inline BoundingBox<Real>
Transform( const BoundingBox<Real>& box, const Matrix4<Real>& matrix );

template<typename Real>
void CoordinateSystem(const Vector<Real, 3>& v1, Vector<Real, 3>* v2, Vector<Real, 3>* v3);

#include "MathUtil.inl"

}

#endif // MathUtil_h__
