#ifndef ColorRGBA_h__
#define ColorRGBA_h__

#include "Math.hpp"

namespace RxLib {

class ColorRGB 
{
public:
	ColorRGB ()
	{
		Tuple[0] = 0.0f;
		Tuple[1] = 0.0f;
		Tuple[2] = 0.0f;
	}

	ColorRGB (float f)
	{
		Tuple[0] = f;
		Tuple[1] = f;
		Tuple[2] = f;
	}

	ColorRGB (float fR, float fG, float fB)
	{
		Tuple[0] = fR;
		Tuple[1] = fG;
		Tuple[2] = fB;
	}

	ColorRGB (const float* tuple)
	{
		Tuple[0] = tuple[0];
		Tuple[1] = tuple[1];
		Tuple[2] = tuple[2];
	}

	inline bool HasNaNs() const
	{
		return isnan(Tuple[0]) && isnan(Tuple[1]) && isnan(Tuple[2]);
	}

	// member access
	inline const float* operator() (void) const			{ return Tuple; }
	inline float* operator() (void)						{ return Tuple; }
	inline float operator[] (size_t i) const				{ return Tuple[i]; }
	inline float& operator[] (size_t i)					{ return Tuple[i]; }


	// comparison
	inline bool operator==(const ColorRGB& rkC) const
	{
		return
			Tuple[0] == rkC.Tuple[0] &&
			Tuple[1] == rkC.Tuple[1] &&
			Tuple[2] == rkC.Tuple[2];
	}

	inline bool operator!=(const ColorRGB& rkC) const
	{
		return
			Tuple[0] != rkC.Tuple[0] ||
			Tuple[1] != rkC.Tuple[1] ||
			Tuple[2] != rkC.Tuple[2];
	}

	// arithmetic operations
	inline ColorRGB operator+ (const ColorRGB& rkC) const
	{
		return ColorRGB(
			Tuple[0] + rkC.Tuple[0],
			Tuple[1] + rkC.Tuple[1],
			Tuple[2] + rkC.Tuple[2]);
	}

	inline ColorRGB operator- (const ColorRGB& rkC) const
	{
		return ColorRGB(
			Tuple[0] - rkC.Tuple[0],
			Tuple[1] - rkC.Tuple[1],
			Tuple[2] - rkC.Tuple[2]);
	}

	inline ColorRGB operator* (const ColorRGB& rkC) const
	{
		return ColorRGB(
			Tuple[0]*rkC.Tuple[0],
			Tuple[1]*rkC.Tuple[1],
			Tuple[2]*rkC.Tuple[2]);
	}

	inline ColorRGB operator/ (const ColorRGB& rkC) const
	{
		return ColorRGB(
			Tuple[0]/rkC.Tuple[0],
			Tuple[1]/rkC.Tuple[1],
			Tuple[2]/rkC.Tuple[2]);
	}

	inline ColorRGB operator* (float fScalar) const
	{
		return ColorRGB(
			fScalar*Tuple[0],
			fScalar*Tuple[1],
			fScalar*Tuple[2]);
	}

	inline ColorRGB operator/ (float fScalar) const
	{
		float invScalar = 1.0f / fScalar;
		return ColorRGB(
			invScalar*Tuple[0],
			invScalar*Tuple[1],
			invScalar*Tuple[2]);
	}

	// arithmetic updates
	inline ColorRGB& operator+= (const ColorRGB& rkC)
	{
		Tuple[0] += rkC.Tuple[0];
		Tuple[1] += rkC.Tuple[1];
		Tuple[2] += rkC.Tuple[2];
		return *this;
	}

	inline ColorRGB& operator-= (const ColorRGB& rkC)
	{
		Tuple[0] -= rkC.Tuple[0];
		Tuple[1] -= rkC.Tuple[1];
		Tuple[2] -= rkC.Tuple[2];
		return *this;
	}

	inline ColorRGB& operator*= (const ColorRGB& rkC)
	{
		Tuple[0] *= rkC.Tuple[0];
		Tuple[1] *= rkC.Tuple[1];
		Tuple[2] *= rkC.Tuple[2];
		return *this;
	}

	inline ColorRGB& operator*= (float fScalar)
	{
		Tuple[0] *= fScalar;
		Tuple[1] *= fScalar;
		Tuple[2] *= fScalar;
		return *this;
	}

public:

	union
	{
		struct{ float Tuple[3]; };
		struct
		{
			float R, G, B;
		};	
	};

	static const ColorRGB Black;
	static const ColorRGB White;
	static const ColorRGB Red;
	static const ColorRGB Green;
	static const ColorRGB Blue;
};

class ColorRGBA
{
public:
	ColorRGBA ()
	{
		Tuple[0] = 0.0f;
		Tuple[1] = 0.0f;
		Tuple[2] = 0.0f;
		Tuple[3] = 0.0f;
	}

	ColorRGBA (float fR, float fG, float fB, float fA)
	{
		Tuple[0] = fR;
		Tuple[1] = fG;
		Tuple[2] = fB;
		Tuple[3] = fA;
	}

	ColorRGBA (const float* tuple)
	{
		Tuple[0] = tuple[0];
		Tuple[1] = tuple[1];
		Tuple[2] = tuple[2];
		Tuple[3] = tuple[3];
	}

	inline bool HasNaNs() const
	{
		return isnan(Tuple[0]) && isnan(Tuple[1]) && isnan(Tuple[2]);
	}

	// member access
	inline const float* operator() (void) const			{ return Tuple; }
	inline float* operator() (void)						{ return Tuple; }
	inline float operator[] (size_t i) const				{ return Tuple[i]; }
	inline float& operator[] (size_t i)					{ return Tuple[i]; }


	// comparison
	inline bool operator==(const ColorRGBA& rkC) const
	{
		return
			Tuple[0] == rkC.Tuple[0] &&
			Tuple[1] == rkC.Tuple[1] &&
			Tuple[2] == rkC.Tuple[2] &&
			Tuple[3] == rkC.Tuple[3];
	}

	inline bool operator!=(const ColorRGBA& rkC) const
	{
		return
			Tuple[0] != rkC.Tuple[0] ||
			Tuple[1] != rkC.Tuple[1] ||
			Tuple[2] != rkC.Tuple[2] ||
			Tuple[3] != rkC.Tuple[3];
	}

	// arithmetic operations
	inline ColorRGBA operator+ (const ColorRGBA& rkC) const
	{
		return ColorRGBA(
			Tuple[0] + rkC.Tuple[0],
			Tuple[1] + rkC.Tuple[1],
			Tuple[2] + rkC.Tuple[2],
			Tuple[3] + rkC.Tuple[3]);
	}

	inline ColorRGBA operator- (const ColorRGBA& rkC) const
	{
		return ColorRGBA(
			Tuple[0] - rkC.Tuple[0],
			Tuple[1] - rkC.Tuple[1],
			Tuple[2] - rkC.Tuple[2],
			Tuple[3] - rkC.Tuple[3]);
	}

	inline ColorRGBA operator* (const ColorRGBA& rkC) const
	{
		return ColorRGBA(
			Tuple[0]*rkC.Tuple[0],
			Tuple[1]*rkC.Tuple[1],
			Tuple[2]*rkC.Tuple[2],
			Tuple[3]*rkC.Tuple[3]);
	}

	inline ColorRGBA operator* (float fScalar) const
	{
		return ColorRGBA(
			fScalar*Tuple[0],
			fScalar*Tuple[1],
			fScalar*Tuple[2],
			fScalar*Tuple[3]);
	}

	inline ColorRGBA operator/ (float fScalar) const
	{
		return ColorRGBA(
			fScalar/Tuple[0],
			fScalar/Tuple[1],
			fScalar/Tuple[2],
			fScalar/Tuple[3]);
	}

	// arithmetic updates
	inline ColorRGBA& operator+= (const ColorRGBA& rkC)
	{
		Tuple[0] += rkC.Tuple[0];
		Tuple[1] += rkC.Tuple[1];
		Tuple[2] += rkC.Tuple[2];
		Tuple[3] += rkC.Tuple[3];
		return *this;
	}

	inline ColorRGBA& operator-= (const ColorRGBA& rkC)
	{
		Tuple[0] -= rkC.Tuple[0];
		Tuple[1] -= rkC.Tuple[1];
		Tuple[2] -= rkC.Tuple[2];
		Tuple[3] -= rkC.Tuple[3];
		return *this;
	}

	inline ColorRGBA& operator*= (const ColorRGBA& rkC)
	{
		Tuple[0] *= rkC.Tuple[0];
		Tuple[1] *= rkC.Tuple[1];
		Tuple[2] *= rkC.Tuple[2];
		Tuple[3] *= rkC.Tuple[3];
		return *this;
	}

	inline ColorRGBA& operator*= (float fScalar)
	{
		Tuple[0] *= fScalar;
		Tuple[1] *= fScalar;
		Tuple[2] *= fScalar;
		Tuple[3] *= fScalar;
		return *this;
	}

public:

	union
	{
		struct{ float Tuple[4]; };
		struct
		{
			float R, G, B, A;
		};	
	};

	static const ColorRGBA Black;
	static const ColorRGBA White;
	static const ColorRGBA Red;
	static const ColorRGBA Green;
	static const ColorRGBA Blue;
};

// To do: Use Clamp 
// Clamps color value to the range [0, 1].
inline ColorRGBA Saturate(const ColorRGBA& color)
{
	ColorRGBA retVal = color;

	for (int i = 0; i < 4; i++)
	{
		if (retVal.Tuple[i] > 1.0f)
		{
			retVal.Tuple[i] = 1.0f;
		}
		else if (retVal.Tuple[i] < 0.0f)
		{
			retVal.Tuple[i] = 0.0f;
		}
	}

	return retVal;
}

//---------------------------------------------------------------------------------------
inline ColorRGB Saturate(const ColorRGB& color)
{
	ColorRGB retVal = color;

	for (int i = 0; i < 3; i++)
	{
		if (retVal.Tuple[i] > 1.0f)
		{
			retVal.Tuple[i] = 1.0f;
		}
		else if (retVal.Tuple[i] < 0.0f)
		{
			retVal.Tuple[i] = 0.0f;
		}
	}

	return retVal;
}

inline float Luminance(const ColorRGB& c)
{
	 const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
	 return YWeight[0] * c[0] + YWeight[1] * c[1] + YWeight[2] * c[2];
}

inline float Luminance(const ColorRGBA& c) 
{
	 const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
	 return YWeight[0] * c[0] + YWeight[1] * c[1] + YWeight[2] * c[2];
}

//---------------------------------------------------------------------------------------
inline ColorRGB operator* (float fScalar, const ColorRGB& rhs)
{
	return ColorRGB(fScalar * rhs[0], fScalar * rhs[1], fScalar * rhs[2]);
}

//---------------------------------------------------------------------------------------
inline ColorRGBA operator* (float fScalar, const ColorRGBA& rhs)
{
	return ColorRGBA(fScalar * rhs[0], fScalar * rhs[1], fScalar * rhs[2], fScalar * rhs[3]);
}


}

#endif // ColorRGBA_h__
