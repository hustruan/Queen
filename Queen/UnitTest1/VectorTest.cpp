#include "stdafx.h"
#include "CppUnitTest.h"

#include <DirectXMath.h>

#include "Math.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace MathLib;
using namespace DirectX; 

namespace UnitTest
{	
	void GenerateVector(float4& vec, XMVECTOR& d3dVec)
	{
		/*float x = rand() / (float)5.146314f;
		float y = rand() / (float)5.146314f;
		float z = rand() / (float)5.146314f;
		float w = rand() / (float)5.146314f;*/

		float x = 1.056f;
		float y = 4.046f; 
		float z = 6.1485f; 
		float w = 6.145f;

		vec = float4(x, y, z, w);
		d3dVec = XMVectorSet(x, y, z, w);
	}

	bool Equal(const float4& vec, const XMVECTOR& d3dVec)
	{
		if ( (vec.X() == XMVectorGetX(d3dVec)) &&
			 (vec.Y() == XMVectorGetY(d3dVec)) &&
			 (vec.Z() == XMVectorGetZ(d3dVec)) &&
			 (vec.W() == XMVectorGetW(d3dVec)) )
		{
			return true;
		}

		return false;
	}

	TEST_CLASS(VectorTest)
	{
	public:
		
		TEST_METHOD(VectorAdd)
		{
			// TODO: Your test code here

			float4 vec1, vec2, vec3;
			XMVECTOR d3dVec1, d3dVec2, d3dVec3;

			GenerateVector(vec1, d3dVec1);
			GenerateVector(vec2, d3dVec2);

			vec3 = vec1 + vec2;
			d3dVec3 = XMVectorAdd(d3dVec1, d3dVec2);

			Assert::IsTrue(Equal(vec3, d3dVec3));
		}

		TEST_METHOD(VectorSub)
		{
			// TODO: Your test code here

			float4 vec1, vec2, vec3;
			XMVECTOR d3dVec1, d3dVec2, d3dVec3;

			GenerateVector(vec1, d3dVec1);
			GenerateVector(vec2, d3dVec2);

			vec3 = vec1 - vec2;
			d3dVec3 = XMVectorSubtract(d3dVec1, d3dVec2);

			Assert::IsTrue(Equal(vec3, d3dVec3));
		}

		TEST_METHOD(VectorMul)
		{
			// TODO: Your test code here

			float4 vec1, vec2, vec3;
			XMVECTOR d3dVec1, d3dVec2, d3dVec3;

			GenerateVector(vec1, d3dVec1);
			GenerateVector(vec2, d3dVec2);

			vec3 = vec1 * vec2;
			d3dVec3 = XMVectorMultiply(d3dVec1, d3dVec2);

			Assert::IsTrue(Equal(vec3, d3dVec3));
		}

		TEST_METHOD(VectorMulScalar)
		{
			// TODO: Your test code here

			float4 vec1, vec2;
			XMVECTOR d3dVec1, d3dVec2;

			GenerateVector(vec1, d3dVec1);

			vec2 = vec1 * 1.3415f;
			d3dVec2 = XMVectorScale(d3dVec1, 1.3415f);

			Assert::IsTrue(Equal(vec2, d3dVec2));
		}

		//TEST_METHOD(VectorDivScalar)
		//{
		//	// TODO: Your test code here

		//	float4 vec1, vec2;
		//	XMVECTOR d3dVec1, d3dVec2;

		//	GenerateVector(vec1, d3dVec1);
		//	Assert::IsTrue(Equal(vec1, d3dVec1));

		//	vec2 = vec1 / 1.3415f;
		//	d3dVec2 = XMVectorScale(d3dVec1, 1.0f / 1.3415f);

		//	int a = 0;

		//	if ( (vec2.X() != XMVectorGetX(d3dVec2)) )
		//		a = 0;
		//	if ( (vec2.Y() != XMVectorGetY(d3dVec2)) )
		//		a = 1;
		//	if ( (vec2.Z() != XMVectorGetZ(d3dVec2)) )
		//		a = 2;
		//	if ( (vec2.W() != XMVectorGetW(d3dVec2)) )
		//	{
		//		float f = vec2.W();
		//		float ff = XMVectorGetW(d3dVec2);
		//		float l = abs(f - ff);
		//		a = 3;
		//	}
		//		

		//	Assert::IsTrue(Equal(vec2, d3dVec2));
		//}

		TEST_METHOD(VectorNeg)
		{
			// TODO: Your test code here

			float4 vec1, vec2;
			XMVECTOR d3dVec1, d3dVec2;

			GenerateVector(vec1, d3dVec1);

			vec2 = -vec1 ;
			d3dVec2 = XMVectorNegate(d3dVec1);

			Assert::IsTrue(Equal(vec2, d3dVec2));
		}

		TEST_METHOD(VectorAddE)
		{
			// TODO: Your test code here

			float4 vec1, vec2;
			XMVECTOR d3dVec1, d3dVec2;

			GenerateVector(vec1, d3dVec1);
			GenerateVector(vec2, d3dVec2);

			vec2 += vec1 ;
			d3dVec2 = XMVectorAdd(d3dVec1, d3dVec2);

			Assert::IsTrue(Equal(vec2, d3dVec2));
		}

		TEST_METHOD(VectorSubE)
		{
			// TODO: Your test code here

			float4 vec1, vec2;
			XMVECTOR d3dVec1, d3dVec2;

			GenerateVector(vec1, d3dVec1);
			GenerateVector(vec2, d3dVec2);

			vec2 -= vec1 ;
			d3dVec2 = XMVectorSubtract(d3dVec2, d3dVec1);

			Assert::IsTrue(Equal(vec2, d3dVec2));
		}

		TEST_METHOD(VectorScaleE)
		{
			// TODO: Your test code here

			float4 vec1;
			XMVECTOR d3dVec1;

			GenerateVector(vec1, d3dVec1);


			vec1 *= 5.01456f ;
			d3dVec1 = XMVectorScale(d3dVec1, 5.01456f);

			Assert::IsTrue(Equal(vec1, d3dVec1));
		}

		//TEST_METHOD(VectorDivE)
		//{
		//	// TODO: Your test code here

		//	float4 vec1;
		//	XMVECTOR d3dVec1;

		//	GenerateVector(vec1, d3dVec1);


		//	vec1 /= 5.01456f ;
		//	d3dVec1 = XMVectorScale(d3dVec1, 1.0f / 5.01456f);

		//	Assert::IsTrue(Equal(vec1, d3dVec1));
		//}

		TEST_METHOD(VectorLength)
		{
			// TODO: Your test code here

			float4 vec1;
			XMVECTOR d3dVec1;

			GenerateVector(vec1, d3dVec1);

			float vecLen = Length(vec1);
			float d3dVecLen = XMVectorGetX(XMVector4Length(d3dVec1));

			Assert::AreEqual(vecLen, d3dVecLen);
		}

		TEST_METHOD(VectorLengthSquared)
		{
			// TODO: Your test code here

			float4 vec1;
			XMVECTOR d3dVec1;

			GenerateVector(vec1, d3dVec1);

			float vecLen = LengthSquared(vec1);
			float d3dVecLen = XMVectorGetX(XMVector4LengthSq(d3dVec1));

			Assert::AreEqual(vecLen, d3dVecLen);
		}

		TEST_METHOD(VectorDot)
		{
			// TODO: Your test code here

			float4 vec1, vec2;
			XMVECTOR d3dVec1, d3dVec2;

			GenerateVector(vec1, d3dVec1);
			GenerateVector(vec2, d3dVec2);


			float v1 = Dot(vec1, vec2);
			float v2 = XMVectorGetX(XMVector4Dot(d3dVec1, d3dVec2));

			Assert::AreEqual(v1, v2);
		}

		TEST_METHOD(VectorLerp)
		{
			// TODO: Your test code here

			float4 vec1, vec2;
			XMVECTOR d3dVec1, d3dVec2;

			GenerateVector(vec1, d3dVec1);
			GenerateVector(vec2, d3dVec2);

			Assert::IsTrue(Equal(Lerp(vec1, vec2, 0.3654f), XMVectorLerp(d3dVec1, d3dVec2, 0.3654f)));
		}

		TEST_METHOD(VectorNormalize)
		{
			// TODO: Your test code here

			float4 vec1, vec2;
			XMVECTOR d3dVec1, d3dVec2;

			GenerateVector(vec1, d3dVec1);
			GenerateVector(vec2, d3dVec2);

			vec1 = Normalize(vec1);
			d3dVec1 = XMVector4Normalize(d3dVec1);

			float l1 = LengthSquared(vec1);
			float l2 = XMVectorGetX(XMVector4LengthSq(d3dVec1));

			Assert::AreEqual(l1, l2);
		}

		//TEST_METHOD(VectorCross)
		//{
		//	// TODO: Your test code here

		//	float4 vec1, vec2;
		//	XMVECTOR d3dVec1, d3dVec2;

		//	GenerateVector(vec1, d3dVec1);
		//	GenerateVector(vec2, d3dVec2);

		//	Assert::IsTrue(Equal(Cross(vec1, vec2), XMVector4Cross(d3dVec1, d3dVec2)));
		//}


	};
}

