#include "Prerequisites.h"
#include <ColorRGBA.hpp>
#include <MathUtil.hpp>
#include "Camera.h"
#include "Sampler.h"
#include <FloatCast.hpp>

using namespace RxLib;

int main()
{	
	float44 cameraToWorld = MatrixInverse(CreateLookAtMatrixLH(float3(0, 0, 0), float3(0, 0, 1), float3(0, 1, 0)));

	//PerspectiveCamera camera()



	return 0;
}