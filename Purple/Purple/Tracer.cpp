#include "Tracer.h"
#include "Camera.h"
#include "Sampler.h"
#include "Random.h"
#include "Shape.h"
#include "Scene.h"
#include "Light.h"
#include "Integrator.h"
#include "MemoryArena.h"

namespace Purple {

void SamplerRenderer::Render( const Scene *scene )
{
	// Compute number of _SamplerRendererTask_s to create for rendering
	int nPixels = mCamera->Width * mCamera->Height;

	//int nTasks = max(32 * NumSystemCores(), nPixels / (16*16));

	int nTasks = nPixels / (16*16);

	for (int taskNum = 0; taskNum < nTasks; ++taskNum)
	{
		 Sampler* sampler = mSampler->GetSubSampler(taskNum, nTasks);

		 // Declare local variables used for rendering loop
		 MemoryArena arena;
		 Random rng(taskNum);

		 // Allocate space for samples and intersections
		 int maxSamples = sampler->GetSampleCount();

		 Sample *samples;// = origSample->Duplicate(maxSamples);

		 RayDifferential *rays = new RayDifferential[maxSamples];
		 ColorRGB *Ls = new ColorRGB[maxSamples];
		 ColorRGB *Ts = new ColorRGB[maxSamples];
		 DifferentialGeometry* isects = new DifferentialGeometry[maxSamples];

		 // Get samples from _Sampler_ and update image
		 int sampleCount;
		 while ((sampleCount = sampler->GetMoreSamples(samples, rng)) > 0)
		 {
			 // Generate camera rays and compute radiance along rays
			 for (int i = 0; i < sampleCount; ++i)
			 {
				// Find camera ray for _sample[i]_
		
				float rayWeight = mCamera->GenerateRayDifferential(samples[i].ImageSample, samples[i].LensSample, &rays[i]);
				rays[i].ScaleDifferentials(1.f / sqrtf(float(sampler->SamplesPerPixel)));

				//	// Evaluate radiance along camera ray
				if (rayWeight > 0.f)
				{
					Ls[i] = rayWeight * Li(scene, rays[i], &samples[i], rng, arena, &isects[i], &Ts[i]);
				}
				else
				{
					Ls[i] = ColorRGB::Black;
					Ts[i] = ColorRGB::White;
				}
		
			 }

			 // Free _MemoryArena_ memory from computing image sample values
			 arena.FreeAll();
		 }
	}
}

ColorRGB SamplerRenderer::Li( const Scene *scene, const RayDifferential &ray, const Sample *sample, Random& rng, MemoryArena &arena, DifferentialGeometry* isect /*= NULL*/, ColorRGB* T /*= NULL*/ ) const
{
	// Allocate local variables for _isect_ and _T_ if needed
	ColorRGB localT;
	if (!T) T = &localT;
	DifferentialGeometry localIsect;
	if (!isect) isect = &localIsect;
	ColorRGB Li = ColorRGB::Black;
	if (scene->Intersect(ray, isect))
	{
		//Li = surfaceIntegrator->Li(scene, this, ray, *isect, sample,rng, arena);
	}
	else 
	{
		// Handle ray that doesn't intersect any geometry
		for (uint32_t i = 0; i < scene->mLights.size(); ++i)
			Li += scene->mLights[i]->Le(ray);
	}
	
	return *T * Li;
}

ColorRGB SamplerRenderer::Transmittance( const Scene *scene, const RayDifferential &ray, const Sample *sample, Random &rng, MemoryArena &arena ) const
{
	return ColorRGB::Black;
}


}