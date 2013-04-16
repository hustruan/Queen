#include "Material.h"
#include "Reflection.h"
#include "DifferentialGeometry.h"
#include "MemoryArena.h"

namespace Purple {

using namespace Purple;


DiffuseMaterial::DiffuseMaterial( const shared_ptr<Texture<ColorRGB>>& kd )
	: mKd(kd)
{

}

BSDF* DiffuseMaterial::GetBSDF( const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena )
{
	BSDF* bsdf = BSDF_ALLOC(arena, BSDF)(dgShading, dgGeom.Normal);
	
	// Evaluate textures for _MatteMaterial_ material and allocate BRDF
	ColorRGB r = Saturate(mKd->Evaluate(dgShading));
	bsdf->Add(BSDF_ALLOC(arena, Lambertian)(r));

	return bsdf;
}

BSDF* PhongMaterial::GetBSDF( const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena )
{
	BSDF *bsdf = BSDF_ALLOC(arena, BSDF)(dgShading, dgGeom.Normal);
	

	return bsdf;
}

//--------------------------------------------------------------------------------------------
GlassMaterial::GlassMaterial( const shared_ptr<Texture<ColorRGB>>& r, const shared_ptr<Texture<ColorRGB>>& t, const shared_ptr<Texture<float>>& index )
	: mKr(r), mKt(t), mIndex(index)
{

}


BSDF* GlassMaterial::GetBSDF( const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena )
{
	// eval index of refraction
	float ior = mIndex->Evaluate(dgShading);

	BSDF *bsdf = BSDF_ALLOC(arena, BSDF)(dgShading, dgGeom.Normal, ior);

	ColorRGB R = Saturate(mKr->Evaluate(dgShading));
	ColorRGB T = Saturate(mKt->Evaluate(dgShading));

	if (R != ColorRGB::Black)
	{
		FresnelDielectric* fresnel = BSDF_ALLOC(arena, FresnelDielectric)(1.0f, ior);
		bsdf->Add(BSDF_ALLOC(arena, SpecularReflection)(R, fresnel));

		bsdf->Add(BSDF_ALLOC(arena, Lambertian)(R));
	}

	if (T != ColorRGB::Black)
	{
		bsdf->Add(BSDF_ALLOC(arena, SpecularTransmission)(T, 1.0f, ior));
	}

	return bsdf;
}

//--------------------------------------------------------------------------------------------------------------



}

