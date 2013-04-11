#ifndef Integrator_h__
#define Integrator_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

enum LightStrategy 
{ 
	LS_Sample_All_Uniform,
	LS_Sample_One_Uniform 
};

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
	virtual void RequestSamples(Sampler* sampler, Sample* sample, const Scene* scene) {}
};

class SurfaceIntegrator : public Integrator
{
public:
	virtual ~SurfaceIntegrator(void) { }

	virtual ColorRGB Li(const Scene* scene, const Renderer* renderer, const RayDifferential& ray, const DifferentialGeometry& isect,
			const Sample* sample, Random& rng, MemoryArena& arena) const = 0;
};


class WhittedIntegrator : public SurfaceIntegrator 
{
public:
	WhittedIntegrator(int32_t md) : mMaxDepth(md) {}

	virtual ColorRGB Li(const Scene* scene, const Renderer* renderer, const RayDifferential& ray, const DifferentialGeometry& isect,
		const Sample* sample, Random& rng, MemoryArena& arena) const;

private:
	int32_t mMaxDepth;
};


class DirectLightingIntegrator : public SurfaceIntegrator
{
public:
	DirectLightingIntegrator(LightStrategy ls = LS_Sample_All_Uniform, int32_t md = 5);
	~DirectLightingIntegrator();

	virtual ColorRGB Li(const Scene* scene, const Renderer* renderer, const RayDifferential& ray, const DifferentialGeometry& isect,
		const Sample* sample, Random& rng, MemoryArena& arena) const;

	void RequestSamples(Sampler* sampler, Sample* sample, const Scene* scene);

private:
	LightSampleOffsets* mLightSampleOffsets;
	BSDFSampleOffsets* mBSDFSampleOffsets;
	uint32_t mLightNumOffset;

	LightStrategy mLightStrategy;
	int32_t mMaxDepth;
};



}




#endif // Integrator_h__

