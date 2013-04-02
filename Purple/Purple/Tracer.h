#ifndef Tracer_h__
#define Tracer_h__

#include "Prerequisites.h"
#include "Ray.h"
#include <ColorRGBA.hpp>

namespace Purple {

using RxLib::ColorRGB;

class Renderer
{
public:
	virtual ~Renderer(void) {};

	virtual void Render(Scene* scene) = 0;

	/*virtual ColorRGB Li(const Scene *scene, const RayDifferential &ray, const Sample *sample, RNG &rng, MemoryArena &arena,
		Intersection *isect = NULL, Spectrum *T = NULL) const = 0;
	
	virtual ColorRGB Transmittance(const Scene *scene, const RayDifferential &ray, const Sample *sample,
		RNG &rng, MemoryArena &arena) const = 0;*/
};


class SamplerRenderer : public Renderer
{
public:
	SamplerRenderer();
	~SamplerRenderer();



protected:
	Camera* mCamera;	

};

}



#endif // Tracer_h__
