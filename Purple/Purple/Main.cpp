#include "Prerequisites.h"
#include "Camera.h"
#include "Sampler.h"
#include "Scene.h"
#include "SimpleShape.h"
#include "Integrator.h"
#include "Renderer.h"
#include "Material.h"
#include "Light.h"
#include "Filter.h"
#include "Film.h"
#include <MathUtil.hpp>

using namespace RxLib;
using namespace Purple;

Camera* gCamera;
Sampler* gSampler;
Scene* gScene;
SamplerRenderer* gRenderer;
SurfaceIntegrator* gSurfaceIntegrator;
Film* gFilm;

class TestScene1 : public Scene
{
public:
	
	void LoadScene()
	{
		float44 light2World = CreateTranslation(0, 10, 10);
		shared_ptr<Sphere> areaLightShape = std::make_shared<Sphere>(light2World, false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		AreaLight* areaLight = new AreaLight(areaLightShape->mLocalToWorld, ColorRGB::White, areaLightShape, 20);
		Lights.push_back(areaLight);
		mKDTree->AddShape(std::make_shared<AreaLightShape>(areaLightShape, areaLight));

		shared_ptr<Sphere> sphere1 = std::make_shared<Sphere>(CreateTranslation(0, 0, 5), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		sphere1->SetMaterial(std::make_shared<DiffuseMaterial>(ColorRGB::Red));
		mKDTree->AddShape(sphere1);

		shared_ptr<Sphere> sphere2 = std::make_shared<Sphere>(CreateTranslation(5, 0, 10), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		sphere2->SetMaterial(std::make_shared<DiffuseMaterial>(ColorRGB::Green));
		mKDTree->AddShape(sphere2);

		shared_ptr<Sphere> sphere3 = std::make_shared<Sphere>(CreateTranslation(-5, 0, 15), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		sphere3->SetMaterial(std::make_shared<DiffuseMaterial>(ColorRGB::Blue));
		mKDTree->AddShape(sphere3);

		mKDTree->BuildTree();
	}
};


void CreateScene()
{
	gScene = new TestScene1;
	gScene->LoadScene();

	gCamera = new PerspectiveCamera(float44::Identity(), ToRadian(60.0f), 0, 1,
		new ImageFilm(512, 512, new GaussianFilter(4.0f, 4.0f, 1.0f)));

	gSampler = new StratifiedSampler(0, 512, 0, 512, 2, 2);
	gSurfaceIntegrator = new WhittedIntegrator();
}


int main()
{	
    CreateScene();

	gRenderer = new SamplerRenderer(gSampler, gCamera, gSurfaceIntegrator);
	gRenderer->Render(gScene);
	delete gScene;

	return 0;
}