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
#include <exception>

using std::vector;
using std::array;
using std::shared_ptr;
using std::make_shared;

#define isnan _isnan
#define isinf(f) (!_finite((f)))

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

#define L1_CACHE_LINE_SIZE 64

namespace Purple {

class Shape;
class Camera;
class Sampler;
class Scene;
class Film;
class Random;
class Renderer;
class Mesh;
class Material;
class Light;
class AreaLight;
class MemoryArena;
class BSDF;
struct Sample;
struct CameraSample;
struct DifferentialGeometry;
}




#endif // Prerequisites_h__
