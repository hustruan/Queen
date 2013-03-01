#ifndef SoveEquation_h__
#define SoveEquation_h__

#include <cmath>
#include <algorithm>

template <typename Real>
inline bool SoveQuadratic(Real A, Real B, Real C, Real* t0, Real* t1)
{
	Real discrim = B * B - Real(4) * A * C;
	if (discrim <= Real(0)) 
		return false;

	Real rootDiscrim = std::sqrt(discrim);

	Real q;
	if (B < Real(0)) 
		q = Real(-0.5) * (B - rootDiscrim);
	else
		q = Real(-0.5) * (B + rootDiscrim);
	
	*t0 = q / A;
	*t1 = C / q;
	
	if (*t0 > *t1) std::swap(*t0, *t1);

	return true;
}


#endif // SoveEquation_h__
