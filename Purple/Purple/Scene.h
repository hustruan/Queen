#ifndef Scene_h__
#define Scene_h__

#include "Prerequisites.h"
#include "Ray.h"
#include "Kdtree.h"

namespace Purple {

class Scene
{
public:
	Scene();
	virtual ~Scene(void);

	const BoundingBoxf& GetBoundingBox() const { return mWorldBound; } 

	bool Intersect(const Ray &ray, DifferentialGeometry* diffGeoHit) const;

	bool IntersectP(const Ray &ray) const;
	
	virtual void LoadScene() { }

public:
	vector<Light*> Lights;	

	std::string Name;

	Camera* mCamera;

	Sampler* mSampler;

	mutable clock_t mTime, mTimeP;

	BoundingBoxf mWorldBound;
	
	vector<shared_ptr<Shape> > mShapes;

	SurfaceIntegrator* mSurfaceIntegrator;

protected:
	KDTree* mKDTree;

};

}



#endif // Scene_h__
