#ifndef Prerequisites_h__
#define Prerequisites_h__

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <memory>
#include <cassert>
#include <algorithm>

using std::vector;
using std::array;
using std::shared_ptr;
using std::make_shared;

#include <Vector.hpp>
#include <ColorRGBA.hpp>
#include <Matrix.hpp>
#include <BoundingBox.hpp>

using RxLib::float2;
using RxLib::float3;
using RxLib::float4;
using RxLib::float44;
using RxLib::ColorRGB;
using RxLib::BoundingBoxf;

namespace Purple {

class Geometry;
class Camera;
class Sampler;
class Scene;
class Film;
class Random;
class Renderer;

}




#endif // Prerequisites_h__
