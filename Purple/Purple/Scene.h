#ifndef Scene_h__
#define Scene_h__

#include "Prerequisites.h"
#include "Ray.h"
#include <BoundingBox.hpp>

namespace Purple {

class DifferentialGeometry;

class Scene
{
public:
	Scene(void);
	virtual ~Scene(void);

	const BoundingBoxf& GetBoundingBox() const { return mWorldBound; } 

	bool Intersect(const Ray &ray, DifferentialGeometry *isect) const;

	bool IntersectP(const Ray &ray) const;

protected:
	BoundingBoxf mWorldBound;

	vector<shared_ptr<Shape>> mGeometries;
};

}



#endif // Scene_h__
