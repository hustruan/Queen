#ifndef Quaternion_h__
#define Quaternion_h__

#include "Vector.hpp"

namespace RxLib {

template<typename Real>
class Quaternion
{
public:
	typedef Real value_type;

public:
	Quaternion() {}

	Quaternion(Real w, Real x, Real y, Real z) 
	{
		mTuple[0] = w; mTuple[1] = x; mTuple[2] = y; mTuple[3] = z;
	}

	Quaternion(const Quaternion& rhs)
	{
		mTuple[0] = rhs.mTuple[0];
		mTuple[1] = rhs.mTuple[1];
		mTuple[2] = rhs.mTuple[2];
		mTuple[3] = rhs.mTuple[3];
	}

	void MakeIdentity()
	{
		mTuple[0] = Real(1);
		mTuple[1] = Real(0);
		mTuple[2] = Real(0);
		mTuple[3] = Real(0);
	}

	inline Real operator[] (int i) const { return mTuple[i] }
	inline Real& operator[] (int i)      { return mTuple[i]; }
	inline Real W () const               { return mTuple[0]; }
	inline Real& W ()                    { return mTuple[0]; }
	inline Real X () const               { return mTuple[1]; }
	inline Real& X ()                    { return mTuple[1]; }
	inline Real Y () const               { return mTuple[2]; }
	inline Real& Y ()                    { return mTuple[2]; }
	inline Real Z () const               { return mTuple[3]; }
	inline Real& Z ()                    { return mTuple[3]; }

	// assignment operators
	inline Quaternion& operator += (const Quaternion& rhs)
	{
		for (int i = 0; i < 4; i++)
		{
			mTuple[i] += rhs.mTuple[i];
		}
		return *this;
	}

	inline Quaternion& operator -= (const Quaternion& rhs)
	{
		for (int i = 0; i < 4; i++)
		{
			mTuple[i] -= rhs.mTuple[i];
		}
		return *this;
	}

	inline Quaternion& operator *= (const Quaternion& rhs)
	{
		*this = (*this) * rhs;
		return *this;
	}

	inline Quaternion& operator *= (float scalar)
	{
		for (int i = 0; i < 4; i++)
		{
			mTuple[i] *= scalar;
		}
		return *this;
	}

	inline Quaternion& operator /= (float scalar)
	{
		int i;
		if (scalar != (Real)0.0)
		{
			Real invScalar = ((Real)1.0)/scalar;
			for (i = 0; i < 4; i++)
			{
				mTuple[i] *= invScalar;
			}
		}
		else
		{
			for (i = 0; i < 4; i++)
			{
				mTuple[i] = (std::numeric_limits<Real>::max)();
			}
		}
	}

	// binary operators
	inline Quaternion operator + (const Quaternion& rhs) const
	{
		Quaternion<Real> sum;
		for (int i = 0; i < 4; i++)
		{
			sum.mTuple[i] = mTuple[i] + rhs.mTuple[i];
		}
		return sum;
	}

	inline Quaternion operator - (const Quaternion& rhs) const
	{
		Quaternion<Real> diff;
		for (int i = 0; i < 4; i++)
		{
			diff.mTuple[i] = mTuple[i] - rhs.mTuple[i];
		}
		return diff;
	}

	inline Quaternion operator * (const Quaternion& rhs) const
	{
		return Quaternion<Real>(
			mTuple[0]*rhs.mTuple[0] - mTuple[1]*rhs.mTuple[1] - mTuple[2]*rhs.mTuple[2] - mTuple[3]*rhs.mTuple[3],
			mTuple[0]*rhs.mTuple[1] + mTuple[1]*rhs.mTuple[0] + mTuple[3]*rhs.mTuple[2] - mTuple[2]*rhs.mTuple[3],
			mTuple[0]*rhs.mTuple[2] + mTuple[2]*rhs.mTuple[0] + mTuple[1]*rhs.mTuple[3] - mTuple[3]*rhs.mTuple[1],
			mTuple[0]*rhs.mTuple[3] + mTuple[3]*rhs.mTuple[0] + mTuple[2]*rhs.mTuple[1] - mTuple[1]*rhs.mTuple[2] );
	}

	inline Quaternion operator * (float scalar) const
	{
		Quaternion<Real> prod;
		for (int i = 0; i < 4; i++)
		{
			prod.mTuple[i] = scalar*mTuple[i];
		}
		return prod;
	}

	inline Quaternion operator / (float scalar) const
	{
		Quaternion quot;
		int i;

		if (scalar != (Real)0.0)
		{
			Real invScalar = ((Real)1.0)/scalar;
			for (i = 0; i < 4; i++)
			{
				quot.mTuple[i] = invScalar*mTuple[i];
			}
		}
		else
		{
			for (i = 0; i < 4; i++)
			{
				quot.mTuple[i] = (std::numeric_limits<Real>::max)();
			}
		}

		return quot;
	}

	inline bool operator == (const Quaternion& rhs) const
	{
		return memcmp(mTuple,rhs.mTuple,4*sizeof(Real)) == 0;
	}

	inline bool operator != (const Quaternion& rhs) const
	{
		return memcmp(mTuple,rhs.mTuple,4*sizeof(Real)) != 0;
	}

	inline void Normalize()
	{
		Real mag = sqrt(quat[0]*quat[0]+quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3]);
		Real oneOverMag = Real(1) / mag;	//��ʱ������0;
		mTuple[0] *= oneOverMag;
		mTuple[1] *= oneOverMag;
		mTuple[2] *= oneOverMag;
		mTuple[3] *= oneOverMag;
	}

	inline Quaternion<Real> Conjugate() const
	{
		return Quaternion<Real>(W(), -X(), -Y(), -Z());
	}

	inline Quaternion<Real> Inverse() const
	{
		Real length = sqrt(mTuple[0]*mTuple[0]+mTuple[1]*mTuple[1]+mTuple[2]*mTuple[2]+mTuple[3]*mTuple[3]);
		Real inv = Real(1) / length;
		return Quaternion<Real>(W()* inv, -X()* inv, -Y()* inv, -Z()* inv);
	}


	inline static const Quaternion& Identity()
	{
		static const Quaternion out(1, 0, 0, 0);
		return out;
	}

private:
	Real mTuple[4];
};

template <typename Real>
inline Real QuaternionLength(const Quaternion<Real>& quat)
{
	return std::sqrt(quat[0]*quat[0]+quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3]);
}

template <typename Real>
inline Quaternion<Real>  QuaternionNormalize(const Quaternion<Real>& quat)
{
	Real mag = std::sqrt(quat[0]*quat[0]+quat[1]*quat[1]+quat[2]*quat[2]+quat[3]*quat[3]);
	Real oneOverMag = Real(1) / mag;	//��ʱ������0;
	return Quaternion<Real>(quat[0]*oneOverMag, quat[1]*oneOverMag, 
		quat[2]*oneOverMag, quat[3]*oneOverMag);
}

template <typename Real>
inline Real QuaternionDot(const Quaternion<Real>& quat1, const Quaternion<Real>& quat2)
{
	return quat1[0]*quat2[0]+quat1[1]*quat2[1]+quat1[2]*quat2[2]+quat1[3]*quat2[3];
}

template <typename Real>
inline Quaternion<Real> QuaternionConjugate(const Quaternion<Real>& quat)
{
	return Quaternion<Real>(quat.W(), -quat.X(), -quat.Y(), -quat.Z());
}

template <typename Real>
inline Quaternion<Real> QuaternionMultiply(const Quaternion<Real>& quat1, const Quaternion<Real>& quat2)
{
	return Quaternion<Real>(
		quat1[0]*quat2[0] - quat1[1]*quat2[1] - quat1[2]*quat2[2] - quat1[3]*quat2[3],
		quat1[0]*quat2[1] + quat1[1]*quat2[0] + quat1[2]*quat2[3] - quat1[3]*quat2[2],
		quat1[0]*quat2[2] + quat1[2]*quat2[0] + quat1[3]*quat2[1] - quat1[1]*quat2[3],
		quat1[0]*quat2[3] + quat1[3]*quat2[0] + quat1[1]*quat2[2] - quat1[2]*quat2[1] );
}

template <typename Real>
inline Quaternion<Real> QuaternionInverse(const Quaternion<Real>& quat)
{
	Real inv = Real(1) / QuaternionLength(quat);
	return QuaternionConjugate(quat) * inv;
}

template <typename Real>
Quaternion<Real> QuaternionSlerp(const Quaternion<Real>& quat1, const Quaternion<Real>& quat2, Real t)
{
	// �õ�˼�������Ԫ�ؼнǵ�cosֵ
	Real cosAngle = QuaternionDot(quat1, quat2);

	//������Ϊ������תһ����Ԫ����ȡ�ö̵�4D��
	Real dir = Real(1);
	if (cosAngle < Real(0))
	{
		cosAngle = -cosAngle;
		dir = -dir;
	}

	Real k1, k2;

	// ��������Ƿ���ڽӽ��Ա������
	if ( cosAngle > Real(1) - std::numeric_limits<Real>::epsilon() )
	{
		// �ǳ��ӽ��������Բ�ֵ
		k1 = Real(1) - t;
		k2 = t;
	}
	else
	{
		Real angle = std::acos(cosAngle);
		Real oneOverSinAngle = Real(1) / std::sin(angle);
		k1 = std::sin((Real(1) - t) * angle) * oneOverSinAngle;
		k2 = std::sin(t * angle) * oneOverSinAngle;
	}

	return quat1 * k1 + quat2 * k2 * dir;
}

typedef Quaternion<float> Quaternionf;
typedef Quaternion<double> Quaterniond;



}
#endif // Quaternion_h__
