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

	bool Debug;
};

class BlockGenerator;

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

	inline Camera* GetCamera() const { return mCamera; }


public:
	static void DrawScreen();
	static void ResizeWindow(int w, int h);
	static void Refresh(int timer);
	static void Close();

private:
	void BlockRender(const Scene* scene, const Sample* sample, Film* film, BlockGenerator& bng);

public:
	static SamplerRenderer* msRenderer;
	
	static SamplerRenderer& Instance() { return *msRenderer; }

protected:
	Camera* mCamera;	
	Sampler* mMainSampler;
	
	SurfaceIntegrator* mSurfaceIntegrator;

	static std::mutex mMutex;

	static GLuint mTexture;
	static std::vector<ColorRGB> mColorBuffer;
};

}



#endif // Tracer_h__
