#ifndef Integrator_h__
#define Integrator_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

class Intersection;

class Integrator
{
public:
	virtual ~Integrator(void) {}
	
	/**
	 * @brief Use this method to do some initialization after scene is loaded.
	 */
	virtual void Preprocess(const Random& rng, const Scene& scene) { }

	/**
	 * @brief Integrator may need samples to integral light or BRDF, use this method to init samples
	 */
	virtual void RequestSamples(Sampler* sampler, Sample* sample, const Scene& scene) {}
};

class SurfaceIntegrator : public Integrator
{
public:
	virtual ~SurfaceIntegrator(void) { }

	virtual ColorRGB Li(const Scene& scene, const Sample& sample, const RayDifferential &ray, const Intersection& isect) const = 0;

	//virtual ColorRGB Li(const Scene *scene, const Renderer *renderer, const RayDifferential &ray, const Intersection &isect,
	//	const Sample *sample, RNG &rng, MemoryArena &arena) const = 0;
};


class DirectLightingIntegrator : public SurfaceIntegrator
{
public:
	DirectLightingIntegrator();
};



}




#endif // Integrator_h__

