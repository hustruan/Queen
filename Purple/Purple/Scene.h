#ifndef Scene_h__
#define Scene_h__

#include "Prerequisites.h"
#include <BoundingBox.hpp>

namespace Purple {

using namespace RxLib;

class Scene
{
public:
	Scene(void);
	virtual ~Scene(void);


protected:
	BoundingBoxf mWorldBound;

	vector<shared_ptr<Geometry>> mGeometries;
};

}



#endif // Scene_h__
