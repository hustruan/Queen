#ifndef Camera_h__
#define Camera_h__

#include "Prerequisite.h"
#include <Vector.hpp>
#include <Matrix.hpp>

using namespace RxLib;

class Camera
{
public:
	Camera(void);
	~Camera(void);

	//void Reset() 
	//{
	//	mDraggingLeft = false;
	//	mDraggingMiddle = false;
	//	mDraggingRight = false;
	//}

	LRESULT Camera::HandleMessages(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	void Update(float elapsedTime);

	void SetDistance(float distance) { mDistance = distance; }
	float GetDistance() const { return mDistance; }

	void SetDistanceVelocity(float distanceVelocity) { mDistanceVelocity = distanceVelocity; }
	float GetDistanceVelocity() const { return mDistanceVelocity; }

	void SetPanPosition(const float2 &panPosition) { mPanPosition = panPosition; }
	const float2& GetPanPosition() const { return mPanPosition; }

	void SetPanVelocity(const float2 &panVelocity) { mPanVelocity = panVelocity; }
	const float2& GetPanVelocity() const { return mPanVelocity; }

	void SetAngle(const float2 &angle) { mAngle = angle; }
	const float2& GetAngle() const { return mAngle; }

	void SetAngularVelocity(const float2 &angularVelocity) { mAngularVelocity = angularVelocity; }
	const float2& GetAngularVelocity() const { return mAngularVelocity; }

	void SetProjection(float fov, float aspect, float nearPlane, float farPlane);
	void SetViewportSize(const float2& viewportSize) { mViewportSize = viewportSize; }

	const float44& GetViewMatrix() const { return mView; }
	const float44& GetProjectionMatrix() const { return mProjection; }
	const float44& GetViewProjectionMatrix() const { return mViewProj; }

	const float3& GetLookAtPosition() { return mLookAt; }  
	const float3& GetEyePosition() { return mEyePosition; }  

	float GetNearPlane() const { return mZNear; }
	float GetFarPlane() const { return mZFar; }


	friend std::ostream& operator <<(std::ostream &os, const Camera &camera);
	friend std::istream& operator >>(std::istream &is, Camera &camera);

private:
	void UpdatePosition(float2 delta);
	void UpdateMatrices();

private:
	float mDistance, mDistanceVelocity;

	float mZNear, mZFar, mFov;

	float2 mPanPosition, mPanVelocity;
	float2 mAngle, mAngularVelocity;

	float2 mViewportSize;

	float44 mView, mProjection, mViewProj;

	float3 mLookAt, mEyePosition;

	float2 mMousePos;

	bool mDraggingLeft, mDraggingMiddle, mDraggingRight;
	float mAttenuation;
};


#endif // Camera_h__