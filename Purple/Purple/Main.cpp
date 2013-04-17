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

		// Area Light 
		float44 light2World = CreateScaling(40.0f, 40.0f, 40.0f) * CreateTranslation(30.0f, 98.0f, 30.0f);
		shared_ptr<Shape> areaLightShape = LoadMesh("../../Media/plane.md", light2World, true);	
		
		AreaLight* areaLight = new AreaLight(areaLightShape->mLocalToWorld, ColorRGB::White * 10.0f, areaLightShape, 10);
		Lights.push_back(areaLight);

		shared_ptr<Shape> areaLightShapeAdapter = std::make_shared<AreaLightShape>(areaLightShape, areaLight);
		areaLightShapeAdapter->SetMaterial(std::make_shared<DiffuseMaterial>(whiteTexture));
		mKDTree->AddShape(areaLightShapeAdapter);

		
		// Walls
		float44 ceilTrans = CreateScaling(100.0f, 100.0f, 100.0f) * CreateTranslation(0.0f, 100.0f, 0.0f);
		shared_ptr<Shape> ceilWall = LoadMesh("../../Media/plane.md", ceilTrans, true);
		ceilWall->SetMaterial(std::make_shared<DiffuseMaterial>(
			std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB(.75,.75,.75))));
		mKDTree->AddShape(ceilWall);

		float44 floorTrans = CreateScaling(100.0f, 100.0f, 100.0f);
		shared_ptr<Shape> floorWall = LoadMesh("../../Media/plane.md", floorTrans, false);
		floorWall->SetMaterial(std::make_shared<DiffuseMaterial>(
			std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB(.75,.75,.75))));
		mKDTree->AddShape(floorWall);

		float44 backTrans = CreateScaling(100.0f, 100.0f, 100.0f) * CreateRotationX(-Mathf::PI / 2) * CreateTranslation(0.0f, 0.0f, 100.0f);;
		shared_ptr<Shape> backWall = LoadMesh("../../Media/plane.md", backTrans, false);
		backWall->SetMaterial(std::make_shared<DiffuseMaterial>(
			std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB(.75,.75,.75))));
	    mKDTree->AddShape(backWall);

		float44 leftTrans = CreateScaling(100.0f, 100.0f, 100.0f) *  CreateRotationZ(Mathf::PI / 2);
		shared_ptr<Shape> leftWall = LoadMesh("../../Media/plane.md", leftTrans, true);
		leftWall->SetMaterial(std::make_shared<DiffuseMaterial>(
			std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB(.75,.25,.25))));
		mKDTree->AddShape(leftWall);

		float44 rightTrans = CreateScaling(100.0f, 100.0f, 100.0f) *  CreateRotationZ(Mathf::PI / 2)  * CreateTranslation(100.0f, 0.0f, 0.0f);;
		shared_ptr<Shape> rightWall = LoadMesh("../../Media/plane.md", rightTrans, false);
		rightWall->SetMaterial(std::make_shared<DiffuseMaterial>(
			std::make_shared<ConstantTexture<ColorRGB>>(ColorRGB(.25,.25,.75))));
		mKDTree->AddShape(rightWall);

		//// standford bunny
		//shared_ptr<Shape> bunny = LoadMesh("../../Media/bunny.md", CreateScaling(300.0f, 300.0f, 300.0f) * CreateRotationY(Mathf::PI) * CreateTranslation(50.0f, 0.0f, 50.0f));
		//bunny->SetMaterial(std::make_shared<DiffuseMaterial>(cTexture));
		//mKDTree->AddShape(bunny);

		shared_ptr<Shape> sphere1 = std::make_shared<Sphere>(CreateTranslation(75.0f, 20.0f, 44.4f), false, 20.0f, -20.0f, 20.0f, Mathf::TWO_PI);
		sphere1->SetMaterial(std::make_shared<GlassMaterial>(whiteTexture, whiteTexture, indexTexture));
		mKDTree->AddShape(sphere1);

		shared_ptr<Shape> sphere2 = std::make_shared<Sphere>(CreateTranslation(25.0f, 20.0f, 75.0f), false, 20.0f, -20.0f, 20.0f, Mathf::TWO_PI);
		sphere2->SetMaterial(std::make_shared<MirrorMaterial>(whiteTexture));
		mKDTree->AddShape(sphere2);

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
	
	float3 cameraPos = float3(50, 50, -80);
	float3 lookAt = float3(50, 50, 1);
	float3 up = float3(0.0f, 1.0f, 0.0f);
	float44 camTrans = MatrixInverse(CreateLookAtMatrixLH(cameraPos, lookAt, up));
	//float44 camTrans = float44::Identity();

	gCamera = new PerspectiveCamera(camTrans, ToRadian(60.0f), 0, 1,
		new ImageFilm(512, 512, new GaussianFilter(4.0f, 4.0f, 1.0f)));

	gSampler = new StratifiedSampler(0, 512, 0, 512, 1, 1);
	//gSampler = new StratifiedSampler(0, 512, 0, 512, 2, 2);
	gSurfaceIntegrator = new DirectLightingIntegrator();
	//gSurfaceIntegrator = new WhittedIntegrator;
}


int main()
{	
    CreateScene();

	gRenderer = new SamplerRenderer(gSampler, gCamera, gSurfaceIntegrator);
	gRenderer->Render(gScene);
	delete gScene;

	return 0;
}