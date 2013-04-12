#include "Material.h"
#include "Reflection.h"
#include "DifferentialGeometry.h"
#include "MemoryArena.h"

namespace Purple {

Material::Material(void)
{
}


Material::~Material(void)
{
}


DiffuseMaterial::DiffuseMaterial( const ColorRGB& abedo )
	: mAbedo(abedo)
{

}

DiffuseMaterial::~DiffuseMaterial( void )
{

}

BSDF* DiffuseMaterial::GetBSDF( const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena )
{
	BSDF* bsdf = BSDF_ALLOC(arena, BSDF)(dgShading, dgGeom.Normal, 1.5f);
	
	bsdf->Add(BSDF_ALLOC(arena, Lambertian)(mAbedo));

	return bsdf;
}

}

