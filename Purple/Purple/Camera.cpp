#include "Camera.h"
#include "Film.h"
#include "Sampler.h"
#include <MathUtil.hpp>

namespace Purple {

using namespace RxLib;

PerspectiveCamera::PerspectiveCamera( const float44& cam2world, float fov, uint32_t width, uint32_t height, float shutterOpen, float shutterClose )
	: Camera(cam2world, shutterOpen, shutterClose)
{
	// Compute perspective transform
	float aspect = float(width) / float(height);
	float44 perspective = CreatePerspectiveFovLH(fov, aspect, 1e-4f, 1e4f);
	
	mRasterToCamera = MatrixInverse(perspective * CreateTranslation(1.0f, 1.0f, 0.0f) * CreateScaling(0.5f * width, -0.5f * height, 1.0f));

	dxCamera = Transform(float3(1, 0, 0), mRasterToCamera) - Transform(float3(0, 0, 0), mRasterToCamera);
	dyCamera = Transform(float3(0, 1, 0), mRasterToCamera) - Transform(float3(0, 0, 0), mRasterToCamera);
}

PerspectiveCamera::~PerspectiveCamera()
{

}

float PerspectiveCamera::GenerateRay( const float2& rasterSample, const float2& lensSample, Ray* ray )
{
	float3 ptRaster(rasterSample.X(), rasterSample.Y(), 0.0f);
	float3 ptCamera = Transform(ptRaster, mRasterToCamera);

	ray->Origin = Transform(float3(0,0,0), mCameraToWorld);
	ray->Direction = Normalize(Transform(ptCamera, mCameraToWorld));
	ray->tMin = 0.0f;
	ray->tMax = std::numeric_limits<float>::infinity();

	return 1.0f;
}

float PerspectiveCamera::GenerateRayDifferential( const float2& rasterSample, const float2& lensSample, RayDifferential* ray )
{
	float3 ptRaster(rasterSample.X(), rasterSample.Y(), 0.0f);
	float3 ptCamera = Transform(ptRaster, mRasterToCamera);

	ray->tMin = 0.0f;
	ray->tMax = std::numeric_limits<float>::infinity();

	ray->Origin = Transform(float3(0,0,0), mCameraToWorld);
	ray->rxOrigin = ray->ryOrigin = ray->Origin;

	ray->Direction = Normalize(Transform(ptCamera, mCameraToWorld));
	ray->rxDirection = Normalize(Transform(ptCamera + dxCamera, mCameraToWorld));
	ray->ryDirection = Normalize(Transform(ptCamera + dyCamera, mCameraToWorld));

	ray->hasDifferentials = true;

	return 1.0f;
}


}