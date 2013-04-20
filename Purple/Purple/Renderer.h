#ifndef Tracer_h__
#define Tracer_h__

#include "Prerequisites.h"
#include "Ray.h"
#include <atomic>
#include <mutex>

#include <GL/glew.h>
#include <GL/glut.h>

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

	bool InitPreviewWindow(Film* film, int argc, char** argv);

	virtual ColorRGB Li(const Scene *scene, const RayDifferential &ray, const Sample *sample, Random& rng, MemoryArena &arena,
		DifferentialGeometry* isect = NULL, ColorRGB* T = NULL) const;

	virtual ColorRGB Transmittance(const Scene *scene, const RayDifferential &ray, const Sample *sample,
		Random &rng, MemoryArena &arena) const;

	void Render(const Scene *scene);


public:
	static void DrawScreen();
	static void ResizeWindow(int w, int h);
	static void Refresh();

private:
	void TileRender(const Scene* scene, const Sample* sample, std::atomic<int32_t>& workingPackage, int32_t numTiles);
	void PutTile(Sampler* sampler, Film* film);

protected:
	Camera* mCamera;	
	Sampler* mMainSampler;
	SurfaceIntegrator* mSurfaceIntegrator;

	static std::mutex mMutex;

	static GLuint mTexture;

	std::atomic<int> mFinishedTiles;
};

}



#endif // Tracer_h__
