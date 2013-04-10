#include "Scene.h"

namespace Purple {

Scene::Scene(void)
{
}


Scene::~Scene(void)
{
}

bool Scene::Intersect( const Ray &ray, DifferentialGeometry* diffGeoHit ) const
{
	return mKDTree.Intersect(ray, diffGeoHit);
}

bool Scene::IntersectP( const Ray &ray ) const
{
	return mKDTree.IntersectP(ray);
}

}
