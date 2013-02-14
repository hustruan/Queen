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
	
	// 从M32中直接接触Pitch
	Real sinPitch = -mat.M32;

	if (sinPitch <= Real(-0.999))
	{
		// 检查万向硕的情况，正在向上看
		pitch = -Math<Real>::HALF_PI; 
		roll = 0;
		yaw = atan2(-mat.M13, mat.M11);
	}
	else if( sinPitch >= Real(0.999))
	{
		// 检查万向硕的情况，正在向下看
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