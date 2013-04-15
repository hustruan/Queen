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


class DiffuseMaterial : public Material
{
public:
	DiffuseMaterial(const ColorRGB& abedo);
	virtual ~DiffuseMaterial(void);

	virtual BSDF* GetBSDF(const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena);

private:
	ColorRGB mAbedo;
};


class PhongMaterial : public Material
{
public:
	PhongMaterial(const ColorRGB& abedo);
	virtual ~PhongMaterial(void);

	virtual BSDF* GetBSDF(const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena);

private:
	ColorRGB mAbedo;
};


}



#endif // Material_h__
