#ifndef Tracer_h__
#define Tracer_h__

#include "Prerequisites.h"
#include "Ray.h"
#include <atomic>

namespace Purple {

using RxLib::ColorRGB;

class Renderer
{
public:
	virtual ~Renderer(void) {};

	virtual void Render(const Scene* scene) = 0;

	virtual ColorRGB Li(const Scene *scene, const RayDifferential &ray, const Sample *sample, Random& rng, MemoryArena &arena,
		DifferentialGeometry* isect = NULL, ColorRGB* T = NULL) const = 0;
	
	virtual ColorRGB Transmittance(const Scene *scene, const RayDifferential &ray, const Sample *sample,
		Random &rng, MemoryArena &arena) const = 0;
};


class SamplerRenderer : public Renderer
{
public:
	SamplerRenderer(Sampler* sampler, Camera* cam, SurfaceIntegrator* si);
	~SamplerRenderer();

	virtual ColorRGB Li(const Scene *scene, const RayDifferential &ray, const Sample *sample, Random& rng, MemoryArena &arena,
		DifferentialGeometry* isect = NULL, ColorRGB* T = NULL) const;

	virtual ColorRGB Transmittance(const Scene *scene, const RayDifferential &ray, const Sample *sample,
		Random &rng, MemoryArena &arena) const;

	void Render(const Scene *scene);


private:

	void TileRender(const Scene *scene, std::atomic<int32_t>& workingPackage, int32_t numTiles);

protected:
	Camera* mCamera;	
	Sampler* mMainSampler;
	Sample* mMainSample;
	SurfaceIntegrator* mSurfaceIntegrator;
};

}



#endif // Tracer_h__
