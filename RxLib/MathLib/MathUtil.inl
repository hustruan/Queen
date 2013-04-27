//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateLookAtMatrixLH(const Vector<Real,3>& vEye, const Vector<Real,3>& vAt, const Vector<Real,3>& vUp)
{
	Vector<Real, 3> zAxis = Normalize(vAt - vEye);
	Vector<Real, 3> xAxis = Normalize(Cross(vUp, zAxis));
	Vector<Real, 3> yAxis = Cross(zAxis, xAxis);

	return Matrix4<Real>(xAxis.X(), yAxis.X(), zAxis.X(), (Real)0, 
		xAxis.Y(), yAxis.Y(), zAxis.Y(), (Real)0, 
		xAxis.Z(), yAxis.Z(), zAxis.Z(), (Real)0,
		- Dot(xAxis, vEye), - Dot(yAxis, vEye), - Dot(zAxis, vEye), (Real)1);
}


//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateLookAtMatrixRH(const Vector<Real,3>& vEye, const Vector<Real,3>& vAt, const Vector<Real,3>& vUp)
{
	Vector<Real, 3> zAxis = Normalize(vEye - vAt);
	Vector<Real, 3> xAxis = Normalize(Cross(vUp, zAxis));
	Vector<Real, 3> yAxis = Cross(zAxis, xAxis);

	return Matrix4<Real>(xAxis.X(), yAxis.X(), zAxis.X(), (Real)0, 
		xAxis.Y(), yAxis.Y(), zAxis.Y(), (Real)0, 
		xAxis.Z(), yAxis.Z(), zAxis.Z(), (Real)0,
		- Dot(xAxis, vEye), - Dot(yAxis, vEye), - Dot(zAxis, vEye), (Real)1);

}

//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateOrthographicLH(Real width, Real height, Real zNear,  Real zFar)
{
/**
	2/w  0    0           0
	0    2/h  0           0
	0    0    1/(zf-zn)   0
	0    0    zn/(zn-zf)  1
 */

	return Matrix4<Real>((Real)2 / width, (Real)0, (Real)0, (Real)0, 
		(Real)0, (Real)2 / height, (Real)0, (Real)0, 
		(Real)0, (Real)0, (Real)1 / (zFar - zNear), (Real)0,
		(Real)0, (Real)0, zNear / (zNear - zFar), (Real)1);

}

//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreatePerspectiveFovLH(Real fovy, Real aspect, Real zNear,  Real zFar)
{
/*
	xScale     0          0               0
	0        yScale       0               0
	0          0       zf/(zf-zn)         1
	0          0       -zn*zf/(zf-zn)     0
	where:
	yScale = cot(fovY/2)

	xScale = yScale / aspect ratio
*/
	Real yScale = Real(1) / (tan(fovy / 2));
	Real xScale = yScale / aspect;
	
	return Matrix4<Real>(xScale, (Real)0, (Real)0, (Real)0, 
		(Real)0, yScale, (Real)0, (Real)0, 
		(Real)0, (Real)0, zFar / (zFar - zNear), (Real)1,
		(Real)0, (Real)0, zNear * zFar / (zNear - zFar),(Real)0);
}

//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreatePerspectiveFovRH(Real fovy, Real aspect,  Real zNear,  Real zFar)
{
/*
	xScale     0          0              0
	0        yScale       0              0
	0          0      zf/(zn-zf)        -1
	0          0      zn*zf/(zn-zf)      0
	where:
	yScale = cot(fovY/2)

	xScale = yScale / aspect ratio
*/
	Real yScale = Real(1) / (tan(fovy / 2));
	Real xScale = yScale / aspect;

	return Matrix4<Real>(xScale, (Real)0, (Real)0, (Real)0, 
		(Real)0, yScale, (Real)0, (Real)0, 
		(Real)0, (Real)0, zFar / (zNear - zFar), (Real)-1,
		(Real)0, (Real)0, zNear * zFar / (zNear - zFar),(Real)0);

}

//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real>
CreatePerspectiveLH( Real width, Real height, Real zNear, Real zFar )
{
/**
	2*zn/w  0       0              0
	0       2*zn/h  0              0
	0       0       zf/(zf-zn)     1
	0       0       zn*zf/(zn-zf)  0
*/

	return Matrix4<Real>((Real)2*zNear/width,  (Real)0,   (Real)0, (Real)0, 
		                (Real)0, (Real)2*zNear/height, (Real)0, (Real)0, 
		                (Real)0, (Real)0, zFar/(zFar - zNear), (Real)1,
		                (Real)0, (Real)0, zNear*zFar/(zNear - zFar),(Real)0);
}

//----------------------------------------------------------------------------
//template<typename Real>
//inline Matrix4<Real> 
//CreatePerspectiveLH(Real left, Real right, Real top, Real bottom, Real zNear,  Real zFar)
//{
//	return Matrix4<Real>((Real)2*zNear/width,  (Real)0,   (Real)0, (Real)0, 
//		(Real)0, (Real)2*zNear/height, (Real)0, (Real)0, 
//	    (right+left)/(left-right), (top+bottom)/(bottom-top), zFar/(zFar - zNear), (Real)1,
//		(Real)0, (Real)0, zNear*zFar/(zNear - zFar),(Real)0);
//}


//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateRotationX(Real angle)
{
	Real cos = std::cos(angle);
	Real sin = std::sin(angle);
	
	return Matrix4<Real>((Real)1, (Real)0, (Real)0, (Real)0, 
						 (Real)0,  cos,     sin,    (Real)0, 
						 (Real)0,  -sin,    cos,    (Real)0,
						 (Real)0, (Real)0, (Real)0, (Real)1 );
}

//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateRotationY(Real angle)
{
	Real cos = std::cos(angle);
	Real sin = std::sin(angle);

	return Matrix4<Real>( cos,   (Real)0,  -sin,   (Real)0, 
		                (Real)0, (Real)1, (Real)0, (Real)0, 
		                  sin,   (Real)0,  cos,    (Real)0,
		                (Real)0, (Real)0, (Real)0, (Real)1 );
}

//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateRotationZ(Real angle)
{
	Real cos = std::cos(angle);
	Real sin = std::sin(angle);

	return Matrix4<Real>( cos,   sin,    (Real)0,  (Real)0, 
					     -sin,   cos,    (Real)0,  (Real)0, 
					   (Real)0, (Real)0, (Real)1,  (Real)0,
					   (Real)0, (Real)0, (Real)0,  (Real)1 );
}


//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateRotationAxis(const Vector<Real,3>& axis, Real angle)
{
	const Real cosAngle = cos(angle);
	const Real sinAngle = sin(angle);	
	const Real lengthInv = (Real)1 / axis.Length();
	const Real nx = axis.X() * lengthInv;
	const Real ny = axis.Y() * lengthInv;
	const Real nz = axis.Z() * lengthInv;
	const Real oneMinusCos = (Real)1-cosAngle;

	return Matrix4<Real>(nx*nx*oneMinusCos+cosAngle,    nx*ny*oneMinusCos+nz*sinAngle, nx*nz*oneMinusCos-ny*sinAngle, (Real)0, 
		                 nx*ny*oneMinusCos-nz*sinAngle, ny*ny*oneMinusCos+cosAngle,    ny*nz*oneMinusCos+nx*sinAngle, (Real)0, 
		                 nx*nz*oneMinusCos+ny*sinAngle, ny*nz*oneMinusCos-nx*sinAngle, nz*nz*oneMinusCos+cosAngle,    (Real)0, 
						 (Real)0,                      (Real)0,                       (Real)0,                        (Real)1 );
}

//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateRotationYawPitchRoll(Real yaw, Real pitch, Real roll)
{
/**
 * The order of transformations is roll first, then pitch, then yaw. Relative to the object's local coordinate axis,
 * this is equivalent to rotation around the z-axis, followed by rotation around the x-axis, followed by rotation around 
 * the y-axis, as shown in the following illustration.
 * Equal to CreateRotationZ * CreateRotationX * CreateRotationY
 */ 
 
 const Real sinYaw = sin(yaw);
 const Real cosYaw = cos(yaw);
 const Real sinPitch = sin(pitch);
 const Real cosPitch =  cos(pitch);
 const Real sinRoll = sin(roll);
 const Real cosRoll = cos(roll); 

 return Matrix4<Real>( cosYaw*cosRoll+sinYaw*sinPitch*sinRoll,   sinRoll*cosPitch,   -sinYaw*cosRoll+cosYaw*sinPitch*sinRoll,  (Real)0, 
					   -cosYaw*sinRoll+sinYaw*sinPitch*cosRoll,  cosRoll*cosPitch,    sinRoll*sinYaw+cosYaw*sinPitch*cosRoll,  (Real)0, 
					   sinYaw*cosPitch,                          -sinPitch,           cosYaw*cosPitch,                          (Real)0,
					   (Real)0,                                 (Real)0,             (Real)0,                                  (Real)1 );
}

//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateScaling(Real sX, Real sY, Real sZ)
{
	return Matrix4<Real>( sX,    (Real)0, (Real)0,  (Real)0, 
					    (Real)0,  sY,     (Real)0,  (Real)0, 
					    (Real)0, (Real)0,   sZ,     (Real)0,
						(Real)0, (Real)0, (Real)0,  (Real)1 );
}

//----------------------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateTranslation(const Vector<Real,3>& trans)
{
	return Matrix4<Real>( (Real)1,    (Real)0,  (Real)0,   (Real)0, 
						  (Real)0,    (Real)1,  (Real)0,   (Real)0, 
						  (Real)0,    (Real)0,  (Real)1,   (Real)0,
						 trans.X(),  trans.Y(), trans.Z(), (Real)1 );
}

//----------------------------------------------------------------------------------------
template<typename Real>
inline void 
RotationMatrixToYawPitchRoll(Real& yaw, Real& pitch, Real& roll, const Matrix4<Real>& mat)
{
/**
 * R = Rz * Ry * Rx 
 * | cosYcosR+sinYsinPsinR   sinRcosP    -sinYcosR+cosYsinPsinR |
 * | -cosYsinR+sinYsinPcosR  cosRcosP    sinRsinY+cosYsinPcosR  |
 * |   sinYcosP               -sinP      cosYcosP               |
 */
	
	// ��M32��ֱ�ӽӴ�Pitch
	Real sinPitch = -mat.M32;

	if (sinPitch <= Real(-0.999))
	{
		// �������˶��������������Ͽ�
		pitch = -Math<Real>::HALF_PI; 
		roll = 0;
		yaw = atan2(-mat.M13, mat.M11);
	}
	else if( sinPitch >= Real(0.999))
	{
		// �������˶��������������¿�
		pitch = Math<Real>::HALF_PI; 
		roll = 0;
		yaw = atan2(-mat.M13, mat.M11);
	}
	else
	{
		pitch = asin(sinPitch);
		yaw = atan2(mat.M31, mat.M33);
		roll = atan2(mat.M12, mat.M22);
		
	}
}


//----------------------------------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real>
RotationFromMatrix( const Matrix4<Real>& transformMat)
{
	Vector<Real,3> cols[] =
	{
		Vector<Real,3>(transformMat.M11, transformMat.M12, transformMat.M13),
		Vector<Real,3>(transformMat.M21, transformMat.M22, transformMat.M23),
		Vector<Real,3>(transformMat.M31, transformMat.M32, transformMat.M33)
	};

	Vector<Real,3> scale;
	scale.X() = cols[0].Length();
	scale.Y() = cols[1].Length();
	scale.Z() = cols[2].Length();

	if (scale.X() != 0)
	{
		cols[0] = cols[0] / scale.X();
	}

	if (scale.Y() != 0)
	{
		cols[1] = cols[1] / scale.Y();
	}

	if (scale.Z() != 0)
	{
		cols[2] = cols[2] / scale.Z();
	}

	return Matrix4<Real>(
		cols[0].X(), cols[0].Y(), cols[0].Z(), (Real)0,
		cols[1].X(), cols[1].Y(), cols[1].Z(), (Real)0,
		cols[2].X(), cols[2].Y(), cols[2].Z(), (Real)0,
		(Real)0,     (Real)0,     (Real)0,     (Real)1
		);
}

//----------------------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real,3>
TranslationFromMatrix( const Matrix4<Real>& transformMat)
{
	return Vector<Real,3>(transformMat.M41, transformMat.M42, transformMat.M43);
}

//----------------------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real,3>
ScaleFromMatrix( const Matrix4<Real>& transformMat)
{
	return Vector<Real,3>(
		sqrt(transformMat.M11 * transformMat.M11 + transformMat.M12 * transformMat.M12 + transformMat.M13 * transformMat.M13),
		sqrt(transformMat.M21 * transformMat.M21 + transformMat.M22 * transformMat.M22 + transformMat.M23 * transformMat.M23),
		sqrt(transformMat.M31 * transformMat.M31 + transformMat.M32 * transformMat.M32 + transformMat.M33 * transformMat.M33)
		);
}


//----------------------------------------------------------------------------
template<typename Real>
inline Matrix4<Real> 
CreateTranslation(Real x, Real y, Real z)
{
	return Matrix4<Real>( (Real)1,    (Real)0,  (Real)0,   (Real)0, 
						  (Real)0,    (Real)1,  (Real)0,   (Real)0, 
						  (Real)0,    (Real)0,  (Real)1,   (Real)0,
							x,          y,        z,       (Real)1 );

}

template<typename Real>
inline Matrix4<Real> 
CreateViewportMatrixD3D(Real left, Real top, Real width, Real height, Real zMin,  Real zMax)
{
	 Matrix4<Real> retVal;

	 const Real Zero = Real(0);
	 const Real Half = Real(0.5);

	 retVal.M11 = width * Half;
	 retVal.M12 = Zero; 
	 retVal.M13 = Zero; 
	 retVal.M14 = Zero; 

	 retVal.M21 = Zero;          
	 retVal.M22 = -height * Half; 
	 retVal.M13 = Zero; 
	 retVal.M14 = Zero; 

	 retVal.M31 = Zero; 
	 retVal.M32 = Zero; 
	 retVal.M33 = zMax - zMin; 
	 retVal.M34 = Zero; 

	 retVal.M41 = left + width * Half ; 
	 retVal.M42 = height * Half + top; 
	 retVal.M43 = zMin;
	 retVal.M44 = Real(1); 

	 return retVal;
}

//----------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real, 3> 
Transform(const Vector<Real, 3>& vec, const Matrix4<Real>& mat)
{
	return Vector<Real, 3>(vec[0] * mat.M11 + vec[1] * mat.M21 + vec[2] * mat.M31 + mat.M41,
						   vec[0] * mat.M12 + vec[1] * mat.M22 + vec[2] * mat.M32 + mat.M42,
						   vec[0] * mat.M13 + vec[1] * mat.M23 + vec[2] * mat.M33 + mat.M43);
}

//----------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real, 3>
TransformCoord( const Vector<Real, 3>& vec, const Matrix4<Real>& mat )
{
	Real inverseW = Real(1) / (vec[0] * mat.M14 + vec[1] * mat.M24 + vec[2] * mat.M34 + mat.M44);

	return Vector<Real, 3>((vec[0] * mat.M11 + vec[1] * mat.M21 + vec[2] * mat.M31 + mat.M41) * inverseW,
		(vec[0] * mat.M12 + vec[1] * mat.M22 + vec[2] * mat.M32 + mat.M42) * inverseW,
		(vec[0] * mat.M13 + vec[1] * mat.M23 + vec[2] * mat.M33 + mat.M43) * inverseW);
}

//----------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real, 3> 
TransformNormal(const Vector<Real, 3>& normal, const Matrix4<Real>& mat)
{
	return Vector<Real, 3>(normal[0] * mat.M11 + normal[1] * mat.M21 + normal[2] * mat.M31,
						   normal[0] * mat.M12 + normal[1] * mat.M22 + normal[2] * mat.M32,
						   normal[0] * mat.M13 + normal[1] * mat.M23 + normal[2] * mat.M33);
}

//----------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real, 3> 
TransformDirection(const Vector<Real, 3>& dir, const Matrix4<Real>& mat)
{
	return Vector<Real, 3>(dir[0] * mat.M11 + dir[1] * mat.M21 + dir[2] * mat.M31,
		                   dir[0] * mat.M12 + dir[1] * mat.M22 + dir[2] * mat.M32,
		                   dir[0] * mat.M13 + dir[1] * mat.M23 + dir[2] * mat.M33);
}


//----------------------------------------------------------------------------------------------------
template<typename Real>
inline BoundingBox<Real>
Transform( const BoundingBox<Real>& box, const Matrix4<Real>& matrix )
{
	if (!box.Defined)
	{
		return box;
	}

	BoundingBox<Real> result;

	const Vector<Real,3>& oldMin = box.Min;
	const Vector<Real,3>& oldMax = box.Max;

	Vector<Real,3> currentCorner;

	// min min min
	currentCorner = oldMin;
	result.Merge( Transform(currentCorner, matrix) );

	// min,min,max
	currentCorner.Z() = oldMax.Z();
	result.Merge( Transform(currentCorner, matrix) );

	// min max max
	currentCorner.Y() = oldMax.Y();
	result.Merge( Transform(currentCorner, matrix) );

	// min max min
	currentCorner.Z() = oldMin.Z();
	result.Merge( Transform(currentCorner, matrix) );

	// max max min
	currentCorner.X() = oldMax.X();
	result.Merge( Transform(currentCorner, matrix) );

	// max max max
	currentCorner.Z() = oldMax.Z();
	result.Merge( Transform(currentCorner, matrix) );

	// max min max
	currentCorner.Y() = oldMin.Y();
	result.Merge( Transform(currentCorner, matrix) );

	// max min min
	currentCorner.Z() = oldMin.Z();
	result.Merge( Transform(currentCorner, matrix) );

	return result; 
}

//----------------------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real, 3> 
Project(const Vector<Real, 3>& vec,  int x, int y, unsigned int width, unsigned int height, Real minDepth, Real maxDepth, const Matrix4<Real>& mvp)
{
	Vector<Real, 3> posH = TransformCoord(vec, mvp);

	return Vector<Real, 3>( Real(0.5) * width * (posH.X() + Real(1)) + x,
							Real(0.5) * height * (Real(1) - posH.X()) + y,
							posH.Z() * (maxDepth - minDepth)  + minDepth);
	
}

//----------------------------------------------------------------------------------------------------
template<typename Real>
inline Vector<Real, 3> 
Unproject(const Vector<Real, 3>& vec,  int x, int y, unsigned int width, unsigned int height, Real minDepth, Real maxDepth, const Matrix4<Real>& mvp)
{
	Vector<Real, 3> posH;

	posH.X() = 2 * (vec.X() - x) / width - 1; 
	posH.Y() = 1 - 2 * (vec.Y() - y) / height; 
	posH.Z() = (vec.Z() - minDepth) / (maxDepth - minDepth);

	return TransformCoord(posH, MatrixInverse(mvp));
}

//----------------------------------------------------------------------------------------------------
template<typename Real>
void CoordinateSystem( const Vector<Real, 3>& v1, Vector<Real, 3>* v2, Vector<Real, 3>* v3 )
{
	if (fabs(v1.X()) > fabs(v1.Y()))
	{
		Real invLen = Real(1) / sqrt(v1.X()*v1.X() + v1.Z()*v1.Z());
		*v2 = Vector<Real, 3>(-v1.Z()*invLen, Real(0), v1.X()*invLen);
	}
	else
	{
		Real invLen = Real(1) / sqrt(v1.Y()*v1.Y() + v1.Z()*v1.Z());
		*v2 = Vector<Real, 3>(Real(0), v1.Z()*invLen, -v1.Y()*invLen);
	}

	*v3 = Cross(v1, *v2);
}

//////////////////////////////////////////////////////////////////////////
template<typename Real>
BoundingBox<Real> FromSphere( const BoundingSphere<Real>& sphere )
{
	BoundingBox box;
	box.Min = Vector<Real, 3>( sphere.Center.X() - sphere.Radius, sphere.Center.Y() - sphere.Radius, sphere.Center.Z() - sphere.Radius );
	box.Max = Vector<Real, 3>( sphere.Center.X() + sphere.Radius, sphere.Center.Y() + sphere.Radius, sphere.Center.Z() + sphere.Radius );
	return box;
}

template<typename Real>
BoundingSphere<Real> FromBox( const BoundingBox<Real>& box )
{
	Vector<Real,3> difference = box.Max - box.Min;
	Vector<Real,3> center = (box.Min + box.Max) * Real(0.5);
	Real radius = Length(difference) * Real(0.5);

	return BoundingSphere<Real>(center, radius);
}

//---------------------------------------------------------------------------------------
template<typename Real>
Matrix4<Real> 
CreateTransformMatrix( const Vector<Real, 3>& sacle, const Quaternion<Real>& rotation, const Vector<Real, 3>& translation )
{
	// Ordering:
	//    1. Scale
	//    2. Rotate
	//    3. Translate

	Matrix4<Real> result = QuaternionToRotationMatrix(rotation);

	result.M11 *= sacle.X(); result.M12 *= sacle.X(); result.M13 *= sacle.X();
	result.M21 *= sacle.Y(); result.M22 *= sacle.Y(); result.M23 *= sacle.Y();
	result.M31 *= sacle.Z(); result.M32 *= sacle.Z(); result.M33 *= sacle.Z();

	result.M41 = translation.X();
	result.M42 = translation.Y();
	result.M43 = translation.Z();

	return result;
}

//----------------------------------------------------------------------------------------------------
template <typename Real>
inline Quaternion<Real> 
QuaternionFromRotationMatrix(const Matrix4<Real>& mat)
{
	Quaternion<Real> ret;
	
	Real trace = mat.M11 + mat.M22 + mat.M33;

	Real S;

	if (trace > 0) { 
		 S = sqrt(trace+(Real)1) * 2; // S=4*qw 
		ret.W() = (Real)0.25 * S;
		ret.X() = (mat.M23 - mat.M32) / S;
		ret.Y() = (mat.M31 - mat.M13) / S; 
		ret.Z() = (mat.M12 - mat.M21) / S; 
	} else if ((mat.M11 > mat.M22)&(mat.M11 > mat.M33)) { 
		 S = sqrt((Real)1.0 + mat.M11 - mat.M22 - mat.M33) * 2; // S=4*qx 
		ret.W() = (mat.M23 - mat.M32) / S;
		ret.X() = (Real)0.25 * S;
		ret.Y() = (mat.M21 + mat.M12) / S; 
		ret.Z() = (mat.M13 + mat.M31) / S; 
	} else if (mat.M22 > mat.M33) { 
		 S = sqrt((Real)1 + mat.M22 - mat.M11 - mat.M33) * 2; // S=4*qy
		ret.W() = (mat.M31 - mat.M13) / S;
		ret.X() = (mat.M12 + mat.M21) / S; 
		ret.Y() = (Real)0.25 * S;
		ret.Z() = (mat.M23 + mat.M32) / S; 
	} else { 
		 S = sqrt((Real)1 + mat.M33 - mat.M11 - mat.M22) * 2; // S=4*qz
		ret.W() = (mat.M12 - mat.M21) / S;
		ret.X() = (mat.M31 + mat.M13) / S;
		ret.Y() = (mat.M32 + mat.M23) / S;
		ret.Z() = (Real)0.25 * S;
	}

	/*Real s;
	if (trace > (Real)0)
	{
		s = sqrt((trace + (Real)1));
		ret.W() =s * (Real)0.5;
		s = (Real)0.5 / s;
		ret.X() = (mat.M23 - mat.M32) * s;
		ret.Y() = (mat.M31 - mat.M13) * s;
		ret.Z() = (mat.M12 - mat.M21) * s;
	}
	else
	{
		if ((mat.M22 > mat.M11) && (mat.M33 <= mat.M22))
		{
			s = sqrt((mat.M22 - (mat.M33 + mat.M11)) + (Real)1);
			ret.Y() = s * (Real)0.5;
			s = (Real)0.5 / s;
			ret.W() = (mat.M31 - mat.M13) * s;
			ret.Z() = (mat.M32 + mat.M23) * s;
			ret.X() = (mat.M12 + mat.M21) * s;
		}
		else
		{
			if (((mat.M22 <= mat.M11) && (mat.M33 > mat.M11)) || (mat.M33 > mat.M22))
			{
				s = sqrt((mat.M33 - (mat.M11 + mat.M22)) + 1);
				ret.Z() =s * (Real)0.5;
				s = (Real)0.5 / s;
				ret.W() = (mat.M12 - mat.M21) * s;
				ret.X() = (mat.M13 + mat.M31) * s;
				ret.Y() = (mat.M23 + mat.M32) * s;
			}
			else
			{
				s = sqrt((mat.M11 - (mat.M22 + mat.M33)) + 1);
				ret.X() = s * (Real)0.5;
				s = (Real)0.5 / s;
				ret.W() = (mat.M23 - mat.M32) * s;
				ret.Y() = (mat.M21 + mat.M12) * s;
				ret.Z() = (mat.M31 + mat.M13) * s;
			}
		}
	}*/
	return ret;
}

//----------------------------------------------------------------------------------------------------
template <typename Real>
inline Matrix4<Real>
QuaternionToRotationMatrix(const Quaternion<Real>& rot)
{
	Real norm = (rot.W() * rot.W()) + (rot.X() * rot.X()) + (rot.Y() * rot.Y()) + (rot.Z() * rot.Z());
	Real s = (Real)0;
	//We must ensure the quaternion is normalized first. We also use 2 / norm to save mults later
	if(norm == (Real)1.0) {
		s = (Real)2;
	} else if(norm > 0.0f) {
		s = (Real)2 / norm;
	}

	//Compute xs/ys/zs since we use them 2-4 times, saves 6 mults
	float xs = rot.X() * s;
	float ys = rot.Y() * s;
	float zs = rot.Z() * s;

	float xx = rot.X() * xs;
	float xy = rot.X() * ys;
	float xz = rot.X() * zs;
	float xw = rot.W() * xs;

	float yy = rot.Y() * ys;
	float yz = rot.Y() * zs;
	float yw = rot.W() * ys;

	float zz = rot.Z() * zs;
	float zw = rot.W() * zs;

	return Matrix4<Real>(
		1-yy-zz,  xy+zw,   xz-yw,   (Real)0,
		xy-zw,    1-xx-zz, yz+xw,   (Real)0,
		xz+yw,    yz-xw,   1-xx-yy, (Real)0,
	    (Real)0,  (Real)0, (Real)0, (Real)1);
}

//----------------------------------------------------------------------------------------------------
template <typename Real>
inline Quaternion<Real> 
QuaternionFromRotationAxis(const Vector<Real, 3>& axis, Real angle)
{
	// assert:  axis[] is unit length
	//
	// The quaternion representing the rotation is
	//   q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

	Real halfAngle = ((Real)0.5)*angle;
	Real sn = sin(halfAngle); 
	return Quaternion<Real>(cos(halfAngle), sn*axis[0], sn*axis[1], sn*axis[2]);
}

//----------------------------------------------------------------------------------------------------
template <typename Real>
inline void
QuaternionToAxisAngle(const Quaternion<Real>& quat, Vector<Real, 3>& axis, Real& angle)
{
	Real squareLength = quat[0]*quat[0] + quat[1]*quat[1] + quat[2]*quat[2] + quat[3]*quat[3];
	
	if (squareLength > (Real)(1e-06))
	{
		angle = ((Real)2.0)*acos(quat[0]);
		Real invLength = ((Real)1.0) / sqrt(squareLength);
		axis[0] = quat[1]*invLength;
		axis[1] = quat[2]*invLength;
		axis[2] = quat[3]*invLength;
	}
	else
	{
		// angle is 0 (mod 2*pi), so any axis will do
		angle = (Real)0.0;
		axis[0] = (Real)1.0;
		axis[1] = (Real)0.0;
		axis[2] = (Real)0.0;
	}

}

//----------------------------------------------------------------------------------------------------
template <typename Real>
inline Quaternion<Real> 
QuaternionFromRotationYawPitchRoll(Real yaw, Real pitch, Real roll)
{
	const Real sinPitch(sin(pitch*((Real)0.5)));
	const Real cosPitch(cos(pitch*((Real)0.5)));
	const Real sinYaw(sin(yaw*((Real)0.5)));
	const Real cosYaw(cos(yaw*((Real)0.5)));
	const Real sinRoll(sin(roll*((Real)0.5)));
	const Real cosRoll(cos(roll*((Real)0.5)));
	const Real cosPitchCosYaw(cosPitch*cosYaw);
	const Real sinPitchSinYaw(sinPitch*sinYaw);

	return Quaternion<Real>(
		cosRoll * cosPitchCosYaw     + sinRoll * sinPitchSinYaw,
		cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
		cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
		sinRoll * cosPitchCosYaw     - cosRoll * sinPitchSinYaw);	
}

//----------------------------------------------------------------------------------------------------
template <typename Real>
inline void 
QuaternionToRotationYawPitchRoll(Real& yaw, Real& pitch, Real& roll, const Quaternion<Real>& quat)
{
	RotationMatrixToYawPitchRoll(yaw, pitch, roll, QuaternionToRotationMatrix(quat));
	//Real sqx = quat.X()*quat.X();
	//Real sqy = quat.Y()*quat.Y();
	//Real sqz = quat.Z()*quat.Z();
	//Real sqw = quat.W()*quat.W();
	//Real unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
	//
	//// sinPitch 
	//Real test = Real(-2.0) * ( quat.Y() * quat.Z() - quat.X() * quat.W() );

	//if(test >  Real(0.999) * unit)
	//{
	//	// singularity at north pole
	//	yaw = atan2(quat.W()*quat.Y()-quat.X()*quat.Z(), Real(0.5)-quat.Y()*quat.Y()-quat.Z()*quat.Z());
	//	pitch = Real(PI/2);
	//	roll = 0;
	//}
	//else if(test <  Real(-0.999) * unit)
	//{
	//	// singularity at south pole
	//	yaw = -2 * atan2(quat.Z(), quat.W());
	//	pitch = -Real(PI/2);
	//	roll = 0;
	//}
	//else
	//{
	//	yaw = atan2((quat.Y() * quat.W() + quat.X() * quat.Z()), -sqx - sqy + sqz + sqw);
	//	pitch = asin(test / unit);
	//	roll = atan2((quat.Z() * quat.W() + quat.X() * quat.Y()), -sqx + sqy - sqz + sqw);
	//}
	//	

	//if (sinPitch <= Real(-0.999))
	//{
	//	// �������˶��������������Ͽ�
	//	pitch = Real(-PI/2);
	//	roll = 0;
	//	yaw = atan2(-mat.M13, mat.M11);
	//}
	//else if( sinPitch >= Real(0.999))
	//{
	//	// �������˶��������������¿�
	//	pitch = Real(PI/2);
	//	roll = 0;
	//	yaw = atan2(-mat.M13, mat.M11);
	//}
	//else
	//{
	//	pitch = asin(sinPitch);
	//	yaw = atan2(mat.M31, mat.M33);
	//	roll = atan2(mat.M12, mat.M22);

	//}
		
	//	(quat.X() * quat.W() + quat.Y() * quat.Z();

	//if(test >  Real(0.499) * unit)
	//{
	//	// singularity at north pole
	//	yaw = 2 * atan2(quat.Z(), quat.W());
	//	pitch = PI / 2;
	//	roll = 0;
	//}
	//else if(test <  Real(-0.499) * unit)
	//{
	//	// singularity at south pole
	//	yaw = -2 * atan2(quat.Z(), quat.W());
	//	pitch = -PI / 2;
	//	roll = 0;
	//}
	//else
	//{
	//	yaw = atan2(2 * (quat.Y() * quat.W() - quat.X() * quat.Z()), -sqx - sqy + sqz + sqw);
	//	pitch = asin(2 * test / unit);
	//	roll = atan2(2 * (quat.Z() * quat.W() - quat.X() * quat.Y()), -sqx + sqy - sqz + sqw);
	//}
}