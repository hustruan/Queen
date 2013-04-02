#ifndef Integrator_h__
#define Integrator_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

class Integrator
{
public:
	Integrator(void);
	virtual ~Integrator(void);
};

class SurfaceIntegrator : public Integrator
{
public:
	SurfaceIntegrator(void);
	~SurfaceIntegrator(void);

	/*virtual ColorRGB Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray, const Intersection &isect,
		const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;*/
};

}




#endif // Integrator_h__

