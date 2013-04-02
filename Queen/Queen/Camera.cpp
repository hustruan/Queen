#include "Camera.h"
#include "MathUtil.hpp"

using namespace RxLib;

Camera::Camera(void)
	: mDistance(0.0f),
	mDistanceVelocity(0.0f),
	mAngle(0.0f, 0.0f),
	mAngularVelocity(0.0f, 0.0f),
	mPanPosition(0.0f, 0.0f),
	mPanVelocity(0.0f, 0.0f),
	mViewportSize(1.0f, 1.0f),
	mMousePos(0.0f, 0.0f),
	mAttenuation(0.0f),
	mDraggingLeft(false),
	mDraggingMiddle(false),
	mDraggingRight(false)
{
	UpdateMatrices();
}

Camera::~Camera(void)
{
}

LRESULT Camera::HandleMessages(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
	case WM_LBUTTONDOWN:
		{
			POINT point;
			GetCursorPos(&point);
			mMousePos = float2(float(point.x), float(point.y));
			mDraggingLeft = true;
			mAttenuation = 4.0f;
			SetCapture(hwnd);
			return true;
		}
	case WM_LBUTTONUP:
		{
			mDraggingLeft = false;
			if (wparam & MK_CONTROL)
				mAttenuation = 0.0f;
			else
				mAttenuation = 4.0f;
			ReleaseCapture();
			return true;
		}		
	case WM_RBUTTONDOWN:
		{
			POINT point;
			GetCursorPos(&point);
			mMousePos = float2(float(point.x), float(point.y));
			mDraggingRight = true;
			SetCapture(hwnd);
			return true;
		}
	case WM_RBUTTONUP: 
		{
			mDraggingRight = false;
			ReleaseCapture();
			return true;
		}
	case WM_MBUTTONDOWN: 
		{
			POINT point;
			GetCursorPos(&point);
			mMousePos = float2(float(point.x), float(point.y));
			mDraggingMiddle = true;
			SetCapture(hwnd);
			return true;
		}
	case WM_MBUTTONUP: {
		mDraggingMiddle = false;
		ReleaseCapture();
		return true;
					   }
	case WM_MOUSEMOVE: 
		{
			POINT point;
			GetCursorPos(&point);
			float2 newMousePos = float2(float(point.x), float(point.y));
			if (mDraggingLeft) {
				float2 delta = newMousePos - mMousePos;
				mAngularVelocity -= delta;
				mMousePos = newMousePos;
			}
			if (mDraggingMiddle) {
				float2 delta = newMousePos - mMousePos;
				UpdatePosition(delta);
				mMousePos = newMousePos;
			}
			if (mDraggingRight) {
				mDistance += (newMousePos.Y() - mMousePos.Y()) / 75.0f;
				mMousePos = newMousePos;
			}
			return true;
		}
	case WM_MOUSEWHEEL:
		{
			short value = short(HIWORD(wparam));
			mDistance -= float(value) / 400.0f;
			return 0;
		}
	case WM_CAPTURECHANGED: 
		{
			if ((HWND) lparam != hwnd)
			{
				mDraggingLeft = false;
				mDraggingMiddle = false;
				mDraggingRight = false;
			}
			break;
		}
	}
	return 0;
}


void Camera::UpdatePosition(float2 delta)
{
	delta.X() /= mViewportSize.X() / 2.0f;
	delta.Y() /= mViewportSize.Y() / 2.0f;

	float44 transform = CreateTranslation(float3(0.0f, 0.0f, mDistance));
	transform = transform * mProjection;

	float44 inverse = MatrixInverse(transform);

	float4 t = float4(mPanPosition.X(), mPanPosition.Y(), 0.0f, 1.0f);
	t = t * transform;

	t.X() -= delta.X() * t.W();
	t.Y() += delta.Y() * t.W();
	t = t * inverse;

	mPanPosition = float2(t.X(), t.Y());
}

void Camera::UpdateMatrices()
{
	mView = CreateTranslation(float3(-mPanPosition.X(), -mPanPosition.Y(), mDistance));

	float44 t = CreateRotationX(mAngle.Y());
	mView = t * mView;

	t = CreateRotationY(mAngle.X());
	mView = t * mView;

	float44 viewInverse = MatrixInverse(mView);

	float4 lookAtPosition4 = float4(0.0f, 0.0f, mDistance, 1.0f);
	lookAtPosition4 = lookAtPosition4 * viewInverse;
	mLookAt = float3(lookAtPosition4.X(), lookAtPosition4.Y(), lookAtPosition4.Z());

	mLookAt = Transform(float3(0.0f, 0.0f, mDistance), viewInverse);

	float4 eyePosition4 = float4(0.0f, 0.0f, 0.0f, 1.0f);
	eyePosition4 = eyePosition4 * viewInverse;
	mEyePosition = float3(eyePosition4.X(), eyePosition4.Y(), eyePosition4.Z());

	mEyePosition = Transform(float3(0.0f, 0.0f, 0.0f), viewInverse);
}

void Camera::Update( float elapsedTime )
{
	mAngle += mAngularVelocity * elapsedTime / 150.0f;
	mAngularVelocity = mAngularVelocity / (1.0f + mAttenuation * elapsedTime);
	mDistance += mDistanceVelocity * elapsedTime / 150.0f;
	mPanPosition += mPanVelocity * elapsedTime / 150.0f;

	UpdateMatrices();
}

void Camera::SetProjection( float fov, float aspect, float nearPlane, float farPlane )
{
	mFov = fov; 
	mZFar = farPlane;
	mZNear = nearPlane;

	mProjection = CreatePerspectiveFovLH(ToRadian(fov), aspect, nearPlane, farPlane);
}


std::ostream &operator <<(std::ostream &os, const Camera &camera) 
{
	//const float scale = 180.0f / 3.1415926f;

	os << camera.mDistance << std::endl;
	os << camera.mAngle.X() << " " << camera.mAngle.Y() << std::endl;
	os << camera.mPanPosition.X() << " " << camera.mPanPosition.Y() << std::endl;
	os << camera.mAngularVelocity.X() << " " << camera.mAngularVelocity.Y() << std::endl;
	return os;
}


std::istream &operator >>(std::istream &is, Camera &camera)
{
	const float scale = 180.0f / 3.1415926f;

	is >> camera.mDistance;  
	is >> camera.mAngle.X() >> camera.mAngle.Y();  /*camera.mAngle.x *= scale;  camera.mAngle.y *= scale;*/
	is >> camera.mPanPosition.X() >> camera.mPanPosition.Y();
	is >> camera.mAngularVelocity.X() >> camera.mAngularVelocity.Y();  //camera.mAngularVelocity.x *= scale;  camera.mAngularVelocity.y *= scale;
	return is;
}