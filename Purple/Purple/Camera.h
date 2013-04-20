#ifndef Camera_h__
#define Camera_h__

#include "Prerequisites.h"
#include "Ray.h"

namespace Purple {

class Camera
{
public:
	Camera(const float44& cam2world, float shutterOpen, float shutterClose, Film* film);

	virtual ~Camera(void);

	Film* GetFilm() const { return mFilm; }

	/**
	 * Generate world space ray corresponding to a sample on the image plane
	 */
	virtual float GenerateRay(const float2& rasterSample, const float2& lensSample, Ray* ray) = 0;
	
	virtual float GenerateRayDifferential(const float2& rasterSample, const float2& lensSample, RayDifferential* ray) = 0;

protected:
	float44 mCameraToWorld;
	const float mShutterOpen, mShutterClose;
	Film* mFilm;
};

class PerspectiveCamera : public Camera
{
public:
	PerspectiveCamera(const float44& cam2world, float fov, float shutterOpen, float shutterClose, Film* film);
	~PerspectiveCamera();

	float GenerateRay(const float2& rasterSample, const float2& lensSample, Ray* ray);
	float GenerateRayDifferential(const float2& rasterSample, const float2& lensSample, RayDifferential* ray); 

private:
	float44 mRasterToCamera;
	float3 dxCamera, dyCamera; 
};

}


#endif // Camera_h__

