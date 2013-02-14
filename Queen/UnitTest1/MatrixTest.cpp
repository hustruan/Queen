#include "stdafx.h"
#include "CppUnitTest.h"

#include <DirectXMath.h>
#include <D3DX10math.h>
#include <limits>

#include "Math.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"

#pragma comment(lib, "D3DX10.lib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace MathLib;
using namespace DirectX; 

//const float epsilon = std::numeric_limits<float>::epsilon();
const float epsilon =  0.00001f;

namespace UnitTest
{	
	bool EqualVec(const float4& vec, const XMVECTOR& d3dVec)
	{
		float x = abs(vec.X() - XMVectorGetX(d3dVec));

		if ( x > epsilon)
			return false;

		float y = abs(vec.Y() - XMVectorGetY(d3dVec));

		if ( x > epsilon)
			return false;

		float z = abs(vec.Z() - XMVectorGetZ(d3dVec));

		if ( z > epsilon)
			return false;

		float w = abs(vec.W() - XMVectorGetW(d3dVec));

		if ( w > epsilon)
			return false;

		return true;
	}

	bool Equal(const float44& mat, const XMMATRIX & d3dMat)
	{
		if (!EqualVec(mat.GetRow(0), d3dMat.r[0]))
		{
			return false;
		}

		if (!EqualVec(mat.GetRow(1), d3dMat.r[1]))
		{
			return false;
		}

		if (!EqualVec(mat.GetRow(2), d3dMat.r[2]))
		{
			return false;
		}

		if (!EqualVec(mat.GetRow(3), d3dMat.r[3]))
		{
			return false;
		}

		return true;
	}

	bool Equal(const float44& mat, const D3DXMATRIX & d3dMat)
	{
		float* m = (float*)d3dMat.m;
		for (int i = 0; i < 16; ++i)
		{
			if (abs(mat[i] - m[i]) > epsilon)
			{
				return false;
			}
		}
		return true;
	}

	bool Equal(const XMMATRIX& mat, const D3DXMATRIX & d3dMat)
	{
		float x = abs(d3dMat._11 - XMVectorGetX(mat.r[0]));

		if ( x > epsilon)
			return false;

		float y = abs(d3dMat._12 - XMVectorGetY(mat.r[0]));

		if ( x > epsilon)
			return false;

		float z = abs(d3dMat._13 - XMVectorGetZ(mat.r[0]));

		if ( z > epsilon)
			return false;

		float w = abs(d3dMat._14 - XMVectorGetW(mat.r[0]));

		if ( w > epsilon)
			return false;


		x = abs(d3dMat._21 - XMVectorGetX(mat.r[1]));

		if ( x > epsilon)
			return false;

		y = abs(d3dMat._22 - XMVectorGetY(mat.r[1]));

		if ( x > epsilon)
			return false;

		z = abs(d3dMat._23 - XMVectorGetZ(mat.r[1]));

		if ( z > epsilon)
			return false;

		w = abs(d3dMat._24 - XMVectorGetW(mat.r[1]));

		if ( w > epsilon)
			return false;




		x = abs(d3dMat._31 - XMVectorGetX(mat.r[2]));

		if ( x > epsilon)
			return false;

		y = abs(d3dMat._32 - XMVectorGetY(mat.r[2]));

		if ( x > epsilon)
			return false;

		z = abs(d3dMat._33 - XMVectorGetZ(mat.r[2]));

		if ( z > epsilon)
			return false;

		w = abs(d3dMat._34 - XMVectorGetW(mat.r[2]));

		if ( w > epsilon)
			return false;


		x = abs(d3dMat._41 - XMVectorGetX(mat.r[3]));

		if ( x > epsilon)
			return false;

		y = abs(d3dMat._42 - XMVectorGetY(mat.r[3]));

		if ( x > epsilon)
			return false;

		z = abs(d3dMat._43 - XMVectorGetZ(mat.r[3]));

		if ( z > epsilon)
			return false;

		w = abs(d3dMat._44 - XMVectorGetW(mat.r[3]));

		if ( w > epsilon)
			return false;

		return true;
	}

	void Generate(float44& mat, XMMATRIX & d3dMat)
	{
		float array[16];
		for (int i = 0; i < 16; ++i)
		{
			array[i] = (rand() % 19) / (float)5.146314f;
		}

		mat = float44(array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8],
			array[9], array[10], array[11], array[12], array[13], array[14], array[15]);

		d3dMat = XMMatrixSet(array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8],
			array[9], array[10], array[11], array[12], array[13], array[14], array[15]);

		assert(Equal(mat, d3dMat));
	}

	void Generate(float44& mat, XMMATRIX & d3dMat, D3DXMATRIX  & d3dMat1)
	{
		float array[16];
		for (int i = 0; i < 16; ++i)
		{
			array[i] = (rand() % 19) / (float)5.146314f;
		}

		mat = float44(array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8],
			array[9], array[10], array[11], array[12], array[13], array[14], array[15]);

		d3dMat = XMMatrixSet(array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8],
			array[9], array[10], array[11], array[12], array[13], array[14], array[15]);

		d3dMat1 = D3DXMATRIX(array);

		assert(Equal(mat, d3dMat));
		assert(Equal(mat, d3dMat1));
	}

	void Generate(float44& mat, D3DXMATRIX  & d3dMat)
	{
		float array[16];
		for (int i = 0; i < 16; ++i)
		{
			array[i] = (rand() % 19) / (float)5.146314f;
		}

		mat = float44(array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7], array[8],
			array[9], array[10], array[11], array[12], array[13], array[14], array[15]);
		
		d3dMat = D3DXMATRIX(array);
		assert(Equal(mat, d3dMat));
	}

	TEST_CLASS(MatrixTest)
	{
	public:

		TEST_METHOD(MatGen)
		{
			float44 mat;
			XMMATRIX d3dMat;

			Generate(mat, d3dMat);

			Assert::IsTrue(Equal(mat, d3dMat));
		}

		TEST_METHOD(MatMul)
		{
			float44 mat1, mat2;
			XMMATRIX d3dMat1, d3dMat2;

			Generate(mat1, d3dMat1);
			Generate(mat2, d3dMat2);

			float44 mat3 =  mat1 * mat2;
			XMMATRIX d3dMat3 = XMMatrixMultiply(d3dMat1, d3dMat2);

			//float v = mat1.M11 * mat2.M14 + mat1.M12 * mat2.M24 + mat1.M13 * mat2.M34 + mat1.M14 * mat2.M44;
			//float v2 = XMVectorGetX(d3dMat1.r[0]) * XMVectorGetW(d3dMat2.r[0]) +
			//	XMVectorGetY(d3dMat1.r[0]) * XMVectorGetW(d3dMat2.r[1]) +
			//	XMVectorGetZ(d3dMat1.r[0]) * XMVectorGetW(d3dMat2.r[2]) +
			//	XMVectorGetW(d3dMat1.r[0]) * XMVectorGetW(d3dMat2.r[3]);

			Assert::IsTrue(Equal(mat3, d3dMat3));
		}

		TEST_METHOD(MatIden)
		{
			float44 mat1;		
			mat1.MakeIdentity();

			Assert::IsTrue(Equal(mat1, XMMatrixIdentity()));
		}

		TEST_METHOD(MatScale)
		{
			float44 mat1, mat2, mat3;
			XMMATRIX d3dMat1, d3dMat2;

			Generate(mat1, d3dMat1);
			
			mat3 = mat1;

			mat2 = mat1 * 1.5634f;
			mat3 *= 1.5634f;
		
			Assert::IsTrue(mat2 == mat3);
		}

		TEST_METHOD(MatDiv)
		{
			float44 mat1, mat2, mat3;
			XMMATRIX d3dMat1, d3dMat2;

			Generate(mat1, d3dMat1);

			mat3 = mat1;

			mat2 = mat1 / 1.5634f;
			mat3 /= 1.5634f;

			Assert::IsTrue(mat2 == mat3);
		}

		TEST_METHOD(MatDert)
		{
			float44 mat1, mat2;
			XMMATRIX d3dMat1, d3dMat2;
			D3DXMATRIX d3d10Mat1, d3d10Mat2;
			
			Generate(mat1, d3dMat1, d3d10Mat1);
	

			float v1 = MatrixDeterminant(mat1);
			float v2 = XMVectorGetX(XMMatrixDeterminant(d3dMat1));
			float v3 = D3DXMatrixDeterminant(&d3d10Mat1);

			Assert::IsTrue(abs(v1 - v2) < epsilon);
			Assert::IsTrue(abs(v1 - v3) < epsilon);
		}

		TEST_METHOD(MatInverse)
		{
			float44 mat1, mat2;
			XMMATRIX d3dMat1, d3dMat2;
			D3DXMATRIX d3d10Mat1, d3d10Mat2;

			Generate(mat1, d3dMat1, d3d10Mat1);

			XMVECTOR d1;
			float d2;

			mat2 = MatrixInverse(mat1);
			d3dMat2 = XMMatrixInverse(&d1, d3dMat1);
			D3DXMatrixInverse(&d3d10Mat2, &d2, &d3d10Mat1);

			//float v = MatrixDeterminant(mat1);
			//float v2 = XMVectorGetX(XMMatrixDeterminant(d3dMat1));
			//float v3 = D3DXMatrixDeterminant(&d3d10Mat1);

			//bool b1 = Equal(mat2, d3dMat2);
			//bool b2 = Equal(mat2, d3d10Mat2);
			//bool b3 = Equal(d3dMat2, d3d10Mat2);

			Assert::IsTrue(Equal(mat2, d3dMat2));
			Assert::IsTrue(Equal(mat2, d3d10Mat2));
		}
	};
}