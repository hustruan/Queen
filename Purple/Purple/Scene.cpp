#include "Scene.h"
#include "Ray.h"

#include "Profiler.h"

namespace Purple {

Scene::Scene(void)
{
	mKDTree = new KDTree;
	
	auto now = clock();

	mTime = mTimeP = (now - now);
}


Scene::~Scene(void)
{
	delete mKDTree;
}

bool Scene::Intersect( const Ray &ray, DifferentialGeometry* diffGeoHit ) const
{	
	FUNCTION_CALL_TIME(guard, mTime);
	return mKDTree->Intersect(ray, diffGeoHit);
}

bool Scene::IntersectP( const Ray &ray ) const
{
	FUNCTION_CALL_TIME(guard, mTimeP);
	return mKDTree->IntersectP(ray);
}

}
