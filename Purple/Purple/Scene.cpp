#include "Scene.h"


namespace Purple {

Scene::Scene(void)
{
	mKDTree = new KDTree;
}


Scene::~Scene(void)
{
	delete mKDTree;
}

bool Scene::Intersect( const Ray &ray, DifferentialGeometry* diffGeoHit ) const
{
	return mKDTree->Intersect(ray, diffGeoHit);
}

bool Scene::IntersectP( const Ray &ray ) const
{
	return mKDTree->IntersectP(ray);
}

}
