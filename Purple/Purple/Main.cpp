#include "Prerequisites.h"
#include "Camera.h"
#include "Sampler.h"
#include "Scene.h"
#include "SimpleShape.h"
#include "Tracer.h"
#include <MathUtil.hpp>

using namespace RxLib;
using namespace Purple;

Camera* gCamera;
Sampler* gSampler;
Scene* gScene;
SamplerRenderer* mRenderer;

class TestScene1 : public Scene
{
public:
	
	void LoadScene()
	{
		shared_ptr<Sphere> sphere1 = std::make_shared<Sphere>(CreateTranslation(0, 0, 5), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		mKDTree->AddShape(sphere1);

		shared_ptr<Sphere> sphere2 = std::make_shared<Sphere>(CreateTranslation(5, 0, 10), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		mKDTree->AddShape(sphere2);

		shared_ptr<Sphere> sphere3 = std::make_shared<Sphere>(CreateTranslation(-5, 0, 15), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		mKDTree->AddShape(sphere3);

		mKDTree->BuildTree();
	}
};


void CreateScene()
{
	gScene = new TestScene1;
	gScene->LoadScene();

	gCamera = new PerspectiveCamera(float44::Identity(), ToRadian(60.0f), 512, 512, 0, 1);

	gSampler = new StratifiedSampler(0, 512, 0, 512, 4, 4);
}


int main()
{	
    CreateScene();

	mRenderer = new SamplerRenderer(gSampler, gCamera, NULL);
	mRenderer->Render(gScene);
	delete gScene;

	return 0;
}