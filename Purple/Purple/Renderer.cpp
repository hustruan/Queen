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
#include <sstream>
#include <iomanip>

#define FILM_BLOCK_SIZE 64

namespace Purple {

GLuint SamplerRenderer::mTexture;
std::mutex SamplerRenderer::mMutex;
SamplerRenderer* SamplerRenderer::msRenderer;
std::vector<ColorRGB> SamplerRenderer::mColorBuffer;

SamplerRenderer::SamplerRenderer( Sampler* sampler, Camera* cam, SurfaceIntegrator* si )
{
	mMainSampler = sampler;
	mCamera = cam;
	mSurfaceIntegrator = si;

	msRenderer = this;
	atexit(SamplerRenderer::Close);
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
	Film* film = mCamera->GetFilm();

	// Allocate and initialize sample
	Sample* sampleTemplate = new Sample(mMainSampler, mSurfaceIntegrator, scene);

	BlockGenerator blockGenerator(film->GetSize(), FILM_BLOCK_SIZE);

	film->Clear();
	
	//const uint32_t CpuCores = std::thread::hardware_concurrency();

	pool& tp = GlobalThreadPool();
	//for (size_t iCore = 0; iCore < tp.size(); ++iCore)
	{
		tp.schedule(std::bind(&SamplerRenderer::BlockRender, this, scene, sampleTemplate, film, std::ref(blockGenerator)));
		//std::bind(&SamplerRenderer::BlockRender, this, scene, sampleTemplate, film, std::ref(blockGenerator))();
	}	

	// Show preview window
	glutMainLoop();

	//tp.wait();

	//std::vector<std::thread*> workers;
	//for (size_t iCore = 0; iCore < CpuCores; ++iCore)
	//{
	//	workers.push_back(new std::thread(std::bind(&SamplerRenderer::BlockRender, this, scene, sampleTemplate, film, std::ref(blockGenerator))));
	//	//std::bind(&SamplerRenderer::BlockRender, this, scene, sampleTemplate, film, std::ref(blockGenerator))();
	//}		

	// Wait for finished
	/*for (size_t iCore = 0; iCore < CpuCores; ++iCore)
	{
		workers[iCore]->join();
		delete workers[iCore];
	}*/

	delete sampleTemplate;
}

void SamplerRenderer::BlockRender( const Scene* scene, const Sample* sample, Film* film, BlockGenerator& bng )
{
	FilmBlock block(int2(FILM_BLOCK_SIZE), film->GetFilter());

	// Allocate space for samples and intersections
	int maxSamples = mMainSampler->GetSampleCount();

	// Create Samples
	Sample* samples = sample->Duplicate(maxSamples);

	RayDifferential* rays = new RayDifferential[maxSamples];
	ColorRGB* Ls = new ColorRGB[maxSamples];
	ColorRGB* Ts = new ColorRGB[maxSamples];
	DifferentialGeometry* isects = new DifferentialGeometry[maxSamples];

	MemoryArena arena;

	while (bng.Next(block))
	{
		// Clear its contents 
		block.Clear();
	
		int2 offset = block.GetOffset();
		int2 size  = block.GetSize();

		// Declare local variables used for rendering loop
		Random rng(offset.Y()*FILM_BLOCK_SIZE + offset.X());

		// Create sampler for this block
		Sampler* sampler = mMainSampler->Clone(offset.X(), offset.X() + size.X(), offset.Y(), offset.Y() + size.Y());

		// Get samples from Sampler and update image
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
				block.AddSample(samples[i], Ls[i]);

			arena.FreeAll();
		}

		delete sampler;

		/* The image block has been processed. Now add it to the "big"
			block that represents the entire image */
		film->AddBlock(block);
	}
	
	delete [] samples;
	delete [] rays;
	delete [] Ls;
	delete [] Ts;
	delete [] isects;
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

	glutSwapBuffers();
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
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

bool SamplerRenderer::InitPreviewWindow( Film* film, int argc, char** argv)
{
	const int32_t width = film->GetSize().X();
	const int32_t height = film->GetSize().Y();

	glutInit(&argc, argv);

	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(600, 100);
	glutCreateWindow("Ray Tracer");

	glewInit();

	glutDisplayFunc(&SamplerRenderer::DrawScreen);
	glutReshapeFunc(&SamplerRenderer::ResizeWindow);
	glutTimerFunc(2000, SamplerRenderer::Refresh,  0);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glClearColor(0.0, 0.0, 0.0, 1.0);

	/* Allocate texture memory for the rendered image */
	glGenTextures(1, &mTexture);
	glBindTexture(GL_TEXTURE_2D, mTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);

	// Init Color Buffer
	mColorBuffer.resize(width*height);

	return true;
}

void SamplerRenderer::Refresh( int timer )
{
	FilmBlock* filmBlock = Instance().GetCamera()->GetFilm()->GetFilmBlock();
	const int2& size = filmBlock->GetSize();

	filmBlock->Lock();

	filmBlock->FillBuffer(&mColorBuffer[0]);

	//glPixelStorei(GL_UNPACK_ROW_LENGTH, m_output->cols());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, size.X(), size.Y(), 0, GL_RGB, GL_FLOAT, &mColorBuffer[0]);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	filmBlock->Unlock();

	//printf("Rendering.....\n");

	glutTimerFunc(2000, SamplerRenderer::Refresh,  timer);

	glutPostRedisplay();
}

void SamplerRenderer::Close()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);

	auto spp = Instance().mMainSampler->GetSampleCount();

	std::stringstream sss;
	sss <<  std::put_time(std::localtime(&now_c), "%m-%d-%H-%M-%S ")  <<
		Instance().mSurfaceIntegrator->GetIntegratorName() << "-" << spp << "spp.pfm";

	GlobalThreadPool().wait();
	Instance().GetCamera()->GetFilm()->WriteImage(sss.str().c_str());
}

}