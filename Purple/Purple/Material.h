#ifndef Material_h__
#define Material_h__

#include "Prerequisites.h"

namespace Purple {

struct DifferentialGeometry;

class Material
{
public:
	Material(void);
	virtual ~Material(void);

	virtual BSDF* GetBSDF(const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena) = 0;
};





}



#endif // Material_h__
