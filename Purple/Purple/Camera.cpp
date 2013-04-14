#include "Camera.h"
#include "Film.h"
#include "Sampler.h"
#include <MathUtil.hpp>

namespace Purple {

using namespace RxLib;

Camera::Camera( const float44& cam2world, float shutterOpen, float shutterClose, Film* film ) : mCameraToWorld(cam2world), mShutterOpen(shutterOpen), mShutterClose(shutterClose), mFilm(film)
{

}

//-----------------------------------------------------------------------------------------------------------------------
PerspectiveCamera::PerspectiveCamera( const float44& cam2world, float fov, float shutterOpen, float shutterClose, Film* film )
	: Camera(cam2world, shutterOpen, shutterClose, film)
{
	float width = (float)film->xResolution;
	float height = (float)film->yResolution;

	// Compute perspective transform
	float aspect = float(width) / float(height);
	float44 perspective = CreatePerspectiveFovLH(fov, aspect, 1e-4f, 1e4f);

	mRasterToCamera = MatrixInverse(perspective * CreateTranslation(1.0f, -1.0f, 0.0f) * CreateScaling(0.5f * width, -0.5f * height, 1.0f));

	//float3 right = float3(1, 0, 0);
	//float3 up = float3(0, 1, 0);
	//float3 front = float3(0, 0, 1);
	//float fovScale = tanf(ToRadian(60.0f / 2)) * 2;

	//float x = 256.0f;
	//float y = 128.0f;

	//float3 r = right * ((x / Width - 0.5) * fovScale);
	//float3 u = up * ((1.0f - y / height - 0.5) * fovScale);
	//float3 dir = Normalize(front + r + u);

	//float3 ptRaster(x, y, 0.0f);
	//float3 ptCamera = Normalize(TransformCoord(ptRaster, mRasterToCamera));

	dxCamera = TransformCoord(float3(1, 0, 0), mRasterToCamera) - TransformCoord(float3(0, 0, 0), mRasterToCamera);
	dyCamera = TransformCoord(float3(0, 1, 0), mRasterToCamera) - TransformCoord(float3(0, 0, 0), mRasterToCamera);
}


PerspectiveCamera::~PerspectiveCamera()
{

}

float PerspectiveCamera::GenerateRay( const float2& rasterSample, const float2& lensSample, Ray* ray )
{
	float3 ptRaster(rasterSample.X(), rasterSample.Y(), 0.0f);
	float3 ptCamera = TransformCoord(ptRaster, mRasterToCamera);

	ray->Origin = Transform(float3(0,0,0), mCameraToWorld);
	ray->Direction = Normalize(TransformDirection(ptCamera, mCameraToWorld));
	ray->tMin = 0.0f;
	ray->tMax = std::numeric_limits<float>::infinity();

	return 1.0f;
}

float PerspectiveCamera::GenerateRayDifferential( const float2& rasterSample, const float2& lensSample, RayDifferential* ray )
{
	float3 ptRaster(rasterSample.X(), rasterSample.Y(), 0.0f);
	float3 ptCamera = TransformCoord(ptRaster, mRasterToCamera);

	ray->tMin = 0.0f;
	ray->tMax = Mathf::INFINITY;

	ray->Origin = Transform(float3(0,0,0), mCameraToWorld);
	ray->rxOrigin = ray->ryOrigin = ray->Origin;

	ray->Direction = Normalize(TransformDirection(ptCamera, mCameraToWorld));
	ray->rxDirection = Normalize(TransformDirection(ptCamera + dxCamera, mCameraToWorld));
	ray->ryDirection = Normalize(TransformDirection(ptCamera + dyCamera, mCameraToWorld));

	ray->hasDifferentials = true;

	return 1.0f;
}


}