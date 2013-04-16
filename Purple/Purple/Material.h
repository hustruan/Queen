#ifndef Material_h__
#define Material_h__

#include "Prerequisites.h"
#include "Texture.h"

namespace Purple {

struct DifferentialGeometry;

class Material
{
public:
	virtual ~Material(void) { }
	virtual BSDF* GetBSDF(const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena) = 0;
};

class DiffuseMaterial : public Material
{
public:
	DiffuseMaterial(const shared_ptr<Texture<ColorRGB>>& kd);

	BSDF* GetBSDF(const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena);

private:
	shared_ptr<Texture<ColorRGB>> mKd;
};

class GlassMaterial : public Material
{
public:
	GlassMaterial(const shared_ptr<Texture<ColorRGB>>& r, const shared_ptr<Texture<ColorRGB>>& t,
		const shared_ptr<Texture<float>>& index);

	BSDF* GetBSDF(const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena);

private:
	 shared_ptr<Texture<ColorRGB>> mKr, mKt;
	 shared_ptr<Texture<float>> mIndex;
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
