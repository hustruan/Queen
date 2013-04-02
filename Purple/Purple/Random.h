#ifndef Random_h__
#define Random_h__

#include <cstdint>

namespace Purple {

class Random
{
public:
	Random(uint32_t seed = 5489UL);

	void Seed(uint32_t seed);

	/// Generate an uniformly distributed 32-bit integer
	uint32_t RandomUInt();

	/// Generate an uniformly distributed single precision value on [0,1)
	float RandomFloat();

private:
	static const int N = 624;
	uint32_t mt[N]; /* the array for the state vector  */
	int mti;
};



}



#endif // Random_h__
