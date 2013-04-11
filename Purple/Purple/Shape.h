#ifndef Shape_h__
#define Shape_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

class Shape 
{
public:
	Shape(const float44& o2w, bool ro);
	virtual ~Shape(void);

	virtual BoundingBoxf GetLocalBound() const = 0;
	virtual BoundingBoxf GetWorldBound() const;

	virtual const Mesh* GetTriangleMesh() const { return NULL; }

	virtual AreaLight* GetAreaLight() const { return NULL; }

	virtual BSDF* GetBSDF(const DifferentialGeometry& dgGeom, const float44& ObjectToWorld, MemoryArena &arena) const;

	virtual void GetShadingGeometry(const float44& local2world, const DifferentialGeometry &dg, DifferentialGeometry *dgShading) const;

	virtual bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;
	virtual bool IntersectP(const Ray& ray) const;

	virtual float Area() const;

	virtual float3 Sample(float u1, float u2, float u3, float3* n) const;
	virtual float Pdf(const float3& pt) const;

	/**
	 * @brief Only sample points from those should be integral. eg. for area light, only the portion visible to lit 
	 *        point should be sampled. The default implementation doesn't consider the additional point, just call
	 *        this original sample method.
	 */
	virtual float3 Sample(const float3& pt, float u1, float u2, float u3, float3* n) const;
	virtual float Pdf(const float3& pt, const float3& wi) const;


public:
	float44 mLocalToWorld, mWorldToLocal;

protected:
	
	bool mReverseOrientation;

	shared_ptr<Material> mMaterial;
};


class AreaLightShape : Shape
{
public:
	AreaLightShape(const shared_ptr<Shape>& shape, Light* light);
	~AreaLightShape() { }

	BoundingBoxf GetLocalBound() const     { return mShape->GetLocalBound(); }
	BoundingBoxf GetWorldBound() const     { return mShape->GetWorldBound(); }

	const Mesh* GetTriangleMesh() const    { return mShape->GetTriangleMesh(); }

	bool Intersect(const Ray& ray, float* tHit, DifferentialGeometry* diffGeoHit) const;

	bool IntersectP(const Ray& ray) const  { return mShape->IntersectP(ray); }

	float Area() const                     { return mShape->Area(); }

	float Pdf(const float3& pt, const float3& wi) const { return mShape->Pdf(pt, wi); }
	float Pdf(const float3& pt) const                   { return mShape->Pdf(pt); }

	float3 Sample(float u1, float u2, float u3, float3* n) const
	{
		return mShape->Sample(u1, u2, u3, n); 
	}

	float3 Sample(const float3& pt, float u1, float u2, float u3, float3* n) const
	{ 
		return mShape->Sample(pt, u1, u2, u3, n);
	}

protected:
	const shared_ptr<Shape> mShape;
	Light* mAreaLight;
};

}

#endif // Shape_h__

