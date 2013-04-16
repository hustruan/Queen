#include "Shape.h"
#include "MemoryArena.h"
#include "Material.h"
#include "Light.h"
#include "DifferentialGeometry.h"
#include <MathUtil.hpp>


namespace Purple {

using namespace RxLib;

Shape::Shape( const float44& o2w, bool ro )
	: mLocalToWorld(o2w), mWorldToLocal(MatrixInverse(o2w)), ReverseOrientation(ro), mMaterial(NULL)
{

}

Shape::~Shape(void)
{
}

BoundingBoxf Shape::GetWorldBound() const
{
	return Transform(GetLocalBound(), mLocalToWorld);
}

float3 Shape::Sample( float u1, float u2, float u3, float3* n ) const
{
	throw std::exception("Unimplemented!");
	return float3();
}

float Shape::Pdf( const float3& pt ) const
{
	return 1.0f / Area();
}

float3 Shape::Sample( const float3& pt, float u1, float u2, float u3,  float3* n ) const
{
	return Sample(u1, u2, u3, n);
}

float Shape::Pdf( const float3& pt, const float3& wi ) const
{
	Ray ray(pt, wi, 1e-3f, Mathf::INFINITY);

	DifferentialGeometry dgLight;
	float hit;
	if( !Intersect(ray, &hit, &dgLight) )
		return 0.0f;

	float s = Area();

	/**
	 * Py(y) = (dy/dx)-1 * Px(x)
	 */
	float pdf = LengthSquared(pt - ray.Eval(hit)) / (AbsDot(dgLight.Normal, -wi) * Area());
	return pdf;
}

bool Shape::Intersect( const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	throw std::exception("Unimplemented!");
	return false;
}

bool Shape::IntersectP( const Ray& ray ) const
{
	throw std::exception("Unimplemented!");
	return false;
}

float Shape::Area() const
{
	throw std::exception("Unimplemented!");
	return 0.0f;
}

void Shape::GetShadingGeometry( const float44& local2world, const DifferentialGeometry &dg, DifferentialGeometry *dgShading ) const
{
	*dgShading = dg;
}

BSDF* Shape::GetBSDF( const DifferentialGeometry& dgGeom, const float44& ObjectToWorld, MemoryArena &arena ) const
{
	DifferentialGeometry dgShading;
	GetShadingGeometry(ObjectToWorld, dgGeom, &dgShading);
	return mMaterial->GetBSDF(dgGeom, dgShading, arena);
}

AreaLightShape::AreaLightShape( const shared_ptr<Shape>& shape, AreaLight* light )
	: Shape(shape->mLocalToWorld, shape->ReverseOrientation), mShape(shape), mAreaLight(light)
{

}

bool AreaLightShape::Intersect( const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit ) const
{
	return mShape->Intersect(ray, tHit, diffGeoHit);
}




}