#ifndef Prerequisite_h__
#define Prerequisite_h__

#if !defined(_VARIADIC_MAX)
	#define _VARIADIC_MAX    10
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <unordered_map>
#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <atomic>
#include <functional>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <windows.h>

using std::vector;
using std::shared_ptr;

typedef std::string String;

#ifndef NDEBUG 
	#include <cassert>
	#define ASSERT(condition) { if(!(condition)) { std::cerr << "Assertion Failed: " << __FILE__ << ":(" << __LINE__ << ") \n\tCondition: " << #condition << std::endl; abort(); } }
#else
	#define ASSERT(condition) (condition)
#endif

#define SAFE_DELETE(p) if(p) { delete p; p = NULL; }
#define SAFE_DELETE_ARRAY if(p) { delete[] p; p = NULL; }

class GraphicsBuffer;
class VertexDeclaration;
class RenderFactory;
class RenderDevice;
class VertexShader;
class PixelShader;
class VertexShader;
class PixelShader;
class FrameBuffer;
class Texture;
class Texture2D;

#endif // Prerequisite_h__
