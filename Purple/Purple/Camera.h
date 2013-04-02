#ifndef Camera_h__
#define Camera_h__

#include "Prerequisites.h"
#include "Ray.h"
#include <Matrix.hpp>

namespace Purple {

using RxLib::float44;
using RxLib::float3;

class Camera
{
public:
	Camera(const float44& cam2world, float shutterOpen, float shutterClose)
		: mCameraToWorld(cam2world), mShutterOpen(shutterOpen), mShutterClose(shutterClose)
	{

	}

	virtual ~Camera(void) { }

	/**
	 * Generate world space ray corresponding to a sample on the image plane
	 */
	virtual float GenerateRay(const float2& rasterSample, const float2& lensSample, Ray* ray) = 0;
	
	virtual float GenerateRayDifferential(const float2& rasterSample, const float2& lensSample, RayDifferential* ray) = 0;

protected:
	float44 mCameraToWorld;
	const float mShutterOpen, mShutterClose;
};

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(const float44& cam2world, float fov, uint32_t width, uint32_t height, float shutterOpen, float shutterClose);
	~PerspectiveCamera();

	float GenerateRay(const float2& rasterSample, const float2& lensSample, Ray* ray);
	float GenerateRayDifferential(const float2& rasterSample, const float2& lensSample, RayDifferential* ray); 

private:
	float44 mRasterToCamera;
	float3 dxCamera, dyCamera; 
};

}


#endif // Camera_h__

