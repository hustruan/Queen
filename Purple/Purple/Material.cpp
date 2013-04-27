#include "Material.h"
#include "Reflection.h"
#include "DifferentialGeometry.h"
#include "MemoryArena.h"

namespace Purple {

using namespace Purple;

//-------------------------------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------------------------------
PhongMaterial::PhongMaterial( const shared_ptr<Texture<ColorRGB> >& kd, const shared_ptr<Texture<ColorRGB> >& ks, const shared_ptr<Texture<float> >& exp )
	:mKd(kd), mKs(ks), mExp(exp)
{

}

BSDF* PhongMaterial::GetBSDF( const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena )
{
	BSDF* bsdf = BSDF_ALLOC(arena, BSDF)(dgShading, dgGeom.Normal);

	// Evaluate textures for _MatteMaterial_ material and allocate BRDF
	ColorRGB kd = Saturate(mKd->Evaluate(dgShading));
	ColorRGB ks = Saturate(mKs->Evaluate(dgShading));
	float e = mExp->Evaluate(dgShading);

	bsdf->Add(BSDF_ALLOC(arena, Phong)(kd, ks, e));

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
	}

	if (T != ColorRGB::Black)
	{
		bsdf->Add(BSDF_ALLOC(arena, SpecularTransmission)(T, 1.0f, ior));
	}

	return bsdf;
}

//--------------------------------------------------------------------------------------------------------------
MirrorMaterial::MirrorMaterial( const shared_ptr<Texture<ColorRGB>>& r )
	: mKr(r)
{

}

BSDF* MirrorMaterial::GetBSDF( const DifferentialGeometry &dgGeom, const DifferentialGeometry &dgShading, MemoryArena &arena )
{
	BSDF *bsdf = BSDF_ALLOC(arena, BSDF)(dgShading, dgGeom.Normal);

	ColorRGB R = Saturate(mKr->Evaluate(dgShading));

	if (R != ColorRGB::Black)
	{
		bsdf->Add(BSDF_ALLOC(arena, SpecularReflection)(R, BSDF_ALLOC(arena, FresnelNoOp)));
	}

	return bsdf;
}

}

