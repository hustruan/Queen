#ifndef Scene_h__
#define Scene_h__

#include "Prerequisites.h"
#include <BoundingBox.hpp>

namespace Purple {


class Scene
{
public:
	Scene(void);
	virtual ~Scene(void);

	const BoundingBoxf& GetBoundingBox() const { return mWorldBound; } 

protected:
	BoundingBoxf mWorldBound;

	vector<shared_ptr<Shape>> mGeometries;
};

}



#endif // Scene_h__
