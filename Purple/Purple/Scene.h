#ifndef Scene_h__
#define Scene_h__

#include "Prerequisites.h"
#include "Ray.h"
#include "Kdtree.h"
#include <BoundingBox.hpp>

namespace Purple {

struct DifferentialGeometry;

class Scene
{
public:
	Scene(void);
	virtual ~Scene(void);

	const BoundingBoxf& GetBoundingBox() const { return mWorldBound; } 

	bool Intersect(const Ray &ray, DifferentialGeometry* diffGeoHit) const;

	bool IntersectP(const Ray &ray) const;

public:
	BoundingBoxf mWorldBound;

	KDTree mKDTree;
	
	vector<Light*> mLights;
	vector<shared_ptr<Shape> > mGeometries;
};

}



#endif // Scene_h__
