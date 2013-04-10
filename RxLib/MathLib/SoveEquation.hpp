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

template<typename Real>
inline bool SolveLinearSystem2x2(const Real A[2][2], const Real B[2], Real *x0, Real *x1)
{
	Real det = A[0][0]*A[1][1] - A[0][1]*A[1][0];
    if (fabs(det) < Real(1e-10))
        return false;
    
    *x0 = (A[1][1]*B[0] - A[0][1]*B[1]) / det;
    *x1 = (A[0][0]*B[1] - A[1][0]*B[0]) / det;
    
    return true;

}


#endif // SoveEquation_h__
