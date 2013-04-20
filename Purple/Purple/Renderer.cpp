#include "Renderer.h"
#include "Camera.h"
#include "Sampler.h"
#include "Random.h"
#include "Shape.h"
#include "Scene.h"
#include "Light.h"
#include "Film.h"
#include "DifferentialGeometry.h"
#include "Integrator.h"
#include "MemoryArena.h"
#include "threadpool.h"
#include <chrono>

#define TilesPackageSize 16

namespace Purple {

GLuint SamplerRenderer::mTexture;
std::mutex SamplerRenderer::mMutex;

SamplerRenderer::SamplerRenderer( Sampler* sampler, Camera* cam, SurfaceIntegrator* si )
{
	mMainSampler = sampler;
	mCamera = cam;
	mSurfaceIntegrator = si;
}

SamplerRenderer::~SamplerRenderer()
{

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
		Li = mSurfaceIntegrator->Li(scene, this, ray, *isect, sample,rng, arena);
	}
	else 
	{
		// Handle ray that doesn't intersect any geometry
		for (uint32_t i = 0; i < scene->Lights.size(); ++i)
			Li += scene->Lights[i]->Le(ray);
	}
	
	return /**T **/ Li;
}

ColorRGB SamplerRenderer::Transmittance( const Scene *scene, const RayDifferential &ray, const Sample *sample, Random &rng, MemoryArena &arena ) const
{
	return ColorRGB::White;
}

void SamplerRenderer::Render( const Scene *scene )
{
	//// Compute number of _SamplerRendererTask_s to create for rendering
	Film* film = mCamera->GetFilm();
	int nPixels = film->xResolution * film->yResolution;

	int nTasks = (std::max)(int(32 * GetNumWorkThreads()), nPixels / (32*32));
	//int nTasks = nPixels / (128*128);

	// Allocate and initialize _sample_
	Sample* sample = new Sample(mMainSampler, mSurfaceIntegrator, scene);

	mFinishedTiles = 0;

	std::atomic<int> workingPackage = 0;
	pool& tp = GlobalThreadPool();

	auto start = clock();

	for (size_t iCore = 0; iCore < tp.size(); ++iCore)
	{
		tp.schedule(std::bind(&SamplerRenderer::TileRender, this, scene, sample, std::ref(workingPackage), nTasks));
		//std::bind(&SamplerRenderer::TileRender, this, scene, sample, std::ref(workingPackage), nTasks)();
	}	
	//tp.wait();

	// Show preview window
	glutMainLoop();

	tp.wait();

	auto end = clock();
	auto total = (double)(end - start) / CLOCKS_PER_SEC;

	std::cout << "\n\nMeasured from Renderer\n";
	std::cout << "Total: " << total / 60.0 <<" minutes\n";

#ifdef DEBUG

	auto isect =  (double)(scene->mTime) / CLOCKS_PER_SEC;
	auto isectP = (double)(scene->mTimeP) / CLOCKS_PER_SEC;


	auto isectTotal =  (double)(scene->mKDTree->mTimeTotal) / CLOCKS_PER_SEC;
	auto isectM = (double)(scene->mKDTree->mTimeM) / CLOCKS_PER_SEC;
	auto isectS = (double)(scene->mKDTree->mTimeS) / CLOCKS_PER_SEC;
	auto isectB = (double)(scene->mKDTree->mTimeB) / CLOCKS_PER_SEC;
	auto isectTr = (double)(scene->mKDTree->mTimeTotal - scene->mKDTree->mTimeTrav - scene->mKDTree->mTimeB ) / CLOCKS_PER_SEC;
	auto find = (double)(scene->mKDTree->mTimeF - scene->mKDTree->mTimeFP) / CLOCKS_PER_SEC;


	std::cout << "\nMeasured from Scene\n";
	std::cout << "Intersect: " <<isect << " seconds  " << 100.0 *  (double)isect / double(total) << "%\n";
	std::cout << "IntersectP: " <<isectP << " seconds  " << 100.0 * (double)isectP / double(total) << "%\n";

	std::cout << "\nMeasured from KdTree\n";
	std::cout << "Intersect Total: " <<isectTotal << " seconds  " << 100.0 *  (double)isectTotal / double(total) << "%\n";
	std::cout << "Intersect Mesh: " <<isectM << " seconds  " << 100.0 * (double)isectM / double(total) << "%\n";
	std::cout << "Intersect Sphere: " <<isectS << " seconds  " << 100.0 *  (double)isectS / double(total)  << "%\n";
	std::cout << "Intersect Box: " <<isectB << " seconds  " << 100.0 *  (double)isectB / double(total)  << "%\n";
	std::cout << "Intersect Travel: " <<isectTr << " seconds  " << 100.0 *  (double)isectTr / double(total)  << "%\n";
	std::cout << "Intersect Find Shape: " << find << " seconds  " << 100.0 *  find / double(total)  << "%\n";

#endif 

	mCamera->GetFilm()->WriteImage("test.pfm");

	delete sample;
}

void SamplerRenderer::TileRender( const Scene* scene, const Sample* sample, std::atomic<int32_t>& workingPackage, int32_t numTiles )
{
	int32_t numPackages = (numTiles + TilesPackageSize - 1) / TilesPackageSize;
	int32_t localWorkingPackage = workingPackage ++;

	Film* film = mCamera->GetFilm();

	while (localWorkingPackage < numPackages)
	{
	    int32_t start = localWorkingPackage * TilesPackageSize;
		int32_t end = (std::min)(numTiles, start + TilesPackageSize);
		
		for (int32_t iTile = start; iTile < end; ++iTile)
		{
			// Declare local variables used for rendering loop
			MemoryArena arena;
			Random rng(iTile);

			Sampler* sampler = mMainSampler->GetSubSampler(iTile, numTiles);

			// Allocate space for samples and intersections
			int maxSamples = sampler->GetSampleCount();

			Sample* samples = sample->Duplicate(maxSamples);

			RayDifferential* rays = new RayDifferential[maxSamples];
			ColorRGB* Ls = new ColorRGB[maxSamples];
			ColorRGB* Ts = new ColorRGB[maxSamples];
			DifferentialGeometry* isects = new DifferentialGeometry[maxSamples];

			// Get samples from _Sampler_ and update image
			int sampleCount;
			while ((sampleCount = sampler->GetMoreSamples(samples, rng)) > 0)
			{
				// Generate camera rays and compute radiance along rays
				for (int i = 0; i < sampleCount; ++i)
				{
					//float rayWeight = mCamera->GenerateRayDifferential(samples[i].ImageSample, samples[i].LensSample, &rays[i]);
					
					float rayWeight = mCamera->GenerateRay(samples[i].ImageSample, samples[i].LensSample, &rays[i]);
					rays[i].ScaleDifferentials(1.f / sqrtf(float(sampler->SamplesPerPixel)));

					// Evaluate radiance along camera ray
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

				// Add sample to film
				for (int i = 0; i < sampleCount; ++i)
				{
					film->AddSample(samples[i], Ls[i]);
				}

				arena.FreeAll();
			}

			delete sampler;
			delete [] samples;
			delete [] rays;
			delete [] Ls;
			delete [] Ts;
			delete [] isects;

			mFinishedTiles++;
			float percent = mFinishedTiles / float(numTiles);

			printf("Tile: %d finished, total: %0.1f%%\n", iTile ,percent*100);
		}

		localWorkingPackage = workingPackage++;
	}
}

void SamplerRenderer::DrawScreen()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, mTexture);

	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);
	glEnd();
}

void SamplerRenderer::ResizeWindow( int w, int h )
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

bool SamplerRenderer::InitPreviewWindow( Film* film, int argc, char** argv)
{
	glutInit(&argc, argv);

	const int32_t width = film->xResolution;
	const int32_t height = film->yResolution;

	glutInitWindowSize(width, height);
	glutCreateWindow("Ray Tracer");

	glutDisplayFunc(SamplerRenderer::DrawScreen);
	glutReshapeFunc(SamplerRenderer::ResizeWindow);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	/* Allocate texture memory for the rendered image */
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	return true;
}

void SamplerRenderer::PutTile( Sampler* sampler, Film* film )
{
	int32_t startX = sampler->PixelStartX;
	int32_t startY = sampler->PixelStartY;

	/*glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
	film->xResolution, film->yResolution, GL_RGB32F, GL_FLOAT, pBufferData);*/
}



}