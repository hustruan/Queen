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
#include "Mesh.h"
#include "Texture.h"
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
		auto redTexture = std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB::Red);
		auto greenTexture = std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB::Green);
		auto blueTexture = std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB::Blue);
		auto whiteTexture = std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB::White);
		auto cTexture = std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB(0.4f, 0.8f, 0.1f));
		auto indexTexture = std::make_shared<ConstantTexture<float>>(1.5f);

		float44 light2World = CreateTranslation(0.0f, 10.0f, 0.0f);
		//shared_ptr<Sphere> areaLightShape = std::make_shared<Sphere>(light2World, false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		//AreaLight* areaLight = new AreaLight(areaLightShape->mLocalToWorld, ColorRGB::White, areaLightShape, 20);

		PointLight* light = new PointLight(light2World,  ColorRGB::White);
		Lights.push_back(light);
		//mKDTree->AddShape(std::make_shared<AreaLightShape>(areaLightShape, areaLight));

		float scale = 40.0f;
		//shared_ptr<Shape> mesh = LoadMesh("../../Media/test.md", CreateScaling(0.15f, 0.15f, 0.15f) * CreateTranslation(0.0f, -2.0f, 12.0f));
		//shared_ptr<Shape> mesh = LoadMesh("../../Media/bunny.md", CreateScaling(40.0f, 40.0f, 40.0f) * CreateTranslation(0.0f, -4.0f, 12.0f));
		//shared_ptr<Shape> mesh = LoadMesh("../../Media/sphere.md", CreateScaling(scale, scale, scale) * CreateRotationY(ToRadian(45.0f))* CreateTranslation(0.0f, 0.0f, 12.0f));
		//mesh->SetMaterial(std::make_shared<DiffuseMaterial>(ColorRGB::Red));
		//mKDTree->AddShape(mesh);
		
		shared_ptr<Shape> ceilWall = LoadMesh("../../Media/plane.md", 
			CreateScaling(40.0f, 40.0f, 40.0f) * CreateRotationX(Mathf::PI / 2) * CreateTranslation(0.0f, 0.0f, 40.0f), true);
		backWall->SetMaterial(std::make_shared<DiffuseMaterial>(whiteTexture));
		mKDTree->AddShape(backWall);


		float44 scale = CreateScaling(40.0f, 40.0f, 40.0f);
		shared_ptr<Shape> backWall = LoadMesh("../../Media/plane.md", 
			scale * CreateRotationX(Mathf::PI / 2) * CreateTranslation(0.0f, 0.0f, 40.0f), false);
		backWall->SetMaterial(std::make_shared<DiffuseMaterial>(whiteTexture));
	    mKDTree->AddShape(backWall);

		shared_ptr<Shape> leftWall = LoadMesh("../../Media/plane.md", 
			scale * CreateRotationZ(Mathf::PI / 2) * CreateTranslation(0.0f, 0.0f, 40.0f), false);
		backWall->SetMaterial(std::make_shared<DiffuseMaterial>(whiteTexture));
		mKDTree->AddShape(backWall);

		shared_ptr<Shape> rightWall = LoadMesh("../../Media/plane.md", 
			CreateScaling(40.0f, 40.0f, 40.0f) * CreateRotationZ(-Mathf::PI / 2) * CreateTranslation(0.0f, 0.0f, 40.0f), false);
		backWall->SetMaterial(std::make_shared<DiffuseMaterial>(whiteTexture));
		mKDTree->AddShape(backWall);

		

		shared_ptr<Shape> floorWall = LoadMesh("../../Media/plane.md", 
			CreateScaling(40.0f, 40.0f, 40.0f) * CreateRotationX(Mathf::PI / 2) * CreateTranslation(0.0f, 0.0f, 40.0f), false);
		backWall->SetMaterial(std::make_shared<DiffuseMaterial>(whiteTexture));
		mKDTree->AddShape(backWall);
		
		//shared_ptr<Shape> sphere1 = std::make_shared<Sphere>(CreateTranslation(0.0f, 0.0f, 7.5f), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		//sphere1->SetMaterial(std::make_shared<DiffuseMaterial>(redTexture));
		////sphere1->SetMaterial(std::make_shared<GlassMaterial>(whiteTexture, whiteTexture, indexTexture));
		//mKDTree->AddShape(sphere1);

		//shared_ptr<Sphere> sphere2 = std::make_shared<Sphere>(CreateTranslation(5.0f, 0.0f, 10.0f), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		//sphere2->SetMaterial(std::make_shared<DiffuseMaterial>(greenTexture));
		//mKDTree->AddShape(sphere2);

		//shared_ptr<Sphere> sphere3 = std::make_shared<Sphere>(CreateTranslation(-5.0f, 0.0f, 15.0f), false, 2.0f, -2.0f, 2.0f, Mathf::TWO_PI);
		//sphere3->SetMaterial(std::make_shared<DiffuseMaterial>(blueTexture));
		//mKDTree->AddShape(sphere3);

		//shared_ptr<Sphere> sphere4 = std::make_shared<Sphere>(CreateTranslation(0.0f, 0.0f, 25.0f), false, 4.0f, -4.0f, 4.0f, Mathf::TWO_PI);
		//sphere4->SetMaterial(std::make_shared<DiffuseMaterial>(cTexture));
		//mKDTree->AddShape(sphere4);

		mKDTree->BuildTree();
	}


private:
	shared_ptr<Mesh> LoadMesh(const char* filename, const float44& world, bool ro = false) const
	{
		FILE* pFile;
		if( fopen_s(&pFile, filename, "rb") )
			return NULL;
		
		int nunIndices, numVertices, hasTanget;
		fread(&nunIndices, sizeof(int), 1, pFile);
		fread(&numVertices, sizeof(int),1, pFile);
		fread(&hasTanget, sizeof(int), 1, pFile);

		unsigned int* indices = new unsigned int[nunIndices];	
		float3* positions = new float3[numVertices];
		float3* normals = new float3[numVertices];
		float2* texcoords = new float2[numVertices];
		float3* tangets = hasTanget ? (new float3[numVertices]) : NULL;

		fread(indices, sizeof(unsigned int), nunIndices, pFile);
		fread(positions, sizeof(float3), numVertices, pFile);
		fread(normals, sizeof(float3), numVertices, pFile);
		fread(texcoords, sizeof(float3), numVertices, pFile);

		if (hasTanget)
		{
			fread_s(tangets, sizeof(float3)*numVertices, sizeof(float), numVertices*3, pFile);
		}

		return std::make_shared<Mesh>(world, ro, nunIndices/3, numVertices, indices, positions, normals, tangets, texcoords);
	}
};


void CreateScene()
{
	gScene = new TestScene1;
	gScene->LoadScene();
	
	/*float3 cameraPos = float3(278, 273, -800);
	float3 lookAt = float3(278, 273, -799);
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float44 camTrans = MatrixInverse(CreateLookAtMatrixLH(cameraPos, lookAt, up));*/
	float44 camTrans = float44::Identity();

	gCamera = new PerspectiveCamera(camTrans, ToRadian(60.0f), 0, 1,
		new ImageFilm(512, 512, new GaussianFilter(4.0f, 4.0f, 1.0f)));

	gSampler = new StratifiedSampler(0, 512, 0, 512, 1, 1);
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