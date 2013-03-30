#pragma once

#include "Prerequisites.h"

namespace Purple {

struct Distribution1D
{
	Distribution1D(const float* f, int n)
		: Count(n)
	{
		pdf = new float[n];
		memcpy(pdf, f, n*sizeof(float));
		
		cdf = new float[n+1];
		cdf[0] = 0.0f;
		for (int i = 1; i <= n; ++i)
			cdf[i] = cdf[i-1] + pdf[i-1] / n;

		C = cdf[n];
		if (C == 0.0f)
		{
			for (int i = 1; i <= n; ++i)
				cdf[i] = float(i) / float(n);
		}
		else
		{
			for (int i = 1; i <= n; ++i)
				cdf[i] = cdf[i] / C;
		}
	}

	~Distribution1D()
	{
		delete[] cdf;
		delete[] pdf;
	}

	float SampleContinuous(float u, float *pdf)
	{
		float* upPtr = std::upper_bound(cdf, cdf+Count+1, u);
		int offset = (std::max)(0, std::distance(cdf, upPtr) - 1);

		float t = (u - cdf[offset]) / (cdf[offset+1] - cdf[offset]);
		
		if(pdf)
			*pdf = pdf[offset] / C;

		return (offset + t) / float(Count);
	}


private:
	float* pdf;		//概率密度，并没有归一化到1,需要除以C
	float* cdf;	
	float C;
	int Count;
};


//
//class MentoCarlo
//{
//public:
//	MentoCarlo(void);
//	~MentoCarlo(void);
//};



}

