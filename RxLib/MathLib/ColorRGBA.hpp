#ifndef ColorRGBA_h__
#define ColorRGBA_h__

namespace RxLib {

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

	// member access
	const float* operator() (void) const			{ return Tuple; }
	float* operator() (void)						{ return Tuple; }
	float operator[] (size_t i) const				{ return Tuple[i]; }
	float& operator[] (size_t i)					{ return Tuple[i]; }


	// comparison
	bool operator==(const ColorRGBA& rkC) const
	{
		return
			Tuple[0] == rkC.Tuple[0] &&
			Tuple[1] == rkC.Tuple[1] &&
			Tuple[2] == rkC.Tuple[2] &&
			Tuple[3] == rkC.Tuple[3];
	}

	bool operator!=(const ColorRGBA& rkC) const
	{
		return
			Tuple[0] != rkC.Tuple[0] ||
			Tuple[1] != rkC.Tuple[1] ||
			Tuple[2] != rkC.Tuple[2] ||
			Tuple[3] != rkC.Tuple[3];
	}

	// arithmetic operations
	ColorRGBA operator+ (const ColorRGBA& rkC) const
	{
		return ColorRGBA(
			Tuple[0] + rkC.Tuple[0],
			Tuple[1] + rkC.Tuple[1],
			Tuple[2] + rkC.Tuple[2],
			Tuple[3] + rkC.Tuple[3]);
	}

	ColorRGBA operator- (const ColorRGBA& rkC) const
	{
		return ColorRGBA(
			Tuple[0] - rkC.Tuple[0],
			Tuple[1] - rkC.Tuple[1],
			Tuple[2] - rkC.Tuple[2],
			Tuple[3] - rkC.Tuple[3]);
	}

	ColorRGBA operator* (const ColorRGBA& rkC) const
	{
		return ColorRGBA(
			Tuple[0]*rkC.Tuple[0],
			Tuple[1]*rkC.Tuple[1],
			Tuple[2]*rkC.Tuple[2],
			Tuple[3]*rkC.Tuple[3]);
	}

	ColorRGBA operator* (float fScalar) const
	{
		return ColorRGBA(
			fScalar*Tuple[0],
			fScalar*Tuple[1],
			fScalar*Tuple[2],
			fScalar*Tuple[3]);
	}

	// arithmetic updates
	ColorRGBA& operator+= (const ColorRGBA& rkC)
	{
		Tuple[0] += rkC.Tuple[0];
		Tuple[1] += rkC.Tuple[1];
		Tuple[2] += rkC.Tuple[2];
		Tuple[3] += rkC.Tuple[3];
		return *this;
	}

	ColorRGBA& operator-= (const ColorRGBA& rkC)
	{
		Tuple[0] -= rkC.Tuple[0];
		Tuple[1] -= rkC.Tuple[1];
		Tuple[2] -= rkC.Tuple[2];
		Tuple[3] -= rkC.Tuple[3];
		return *this;
	}

	ColorRGBA& operator*= (const ColorRGBA& rkC)
	{
		Tuple[0] *= rkC.Tuple[0];
		Tuple[1] *= rkC.Tuple[1];
		Tuple[2] *= rkC.Tuple[2];
		Tuple[3] *= rkC.Tuple[3];
		return *this;
	}

	ColorRGBA& operator*= (float fScalar)
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
inline ColorRGBA operator* (float fScalar, const ColorRGBA& rhs)
{
	ColorRGBA result;
	for(int i = 0; i < 4; i++)
		result[i] =  fScalar * rhs[i];

	return result;
}
}

#endif // ColorRGBA_h__
