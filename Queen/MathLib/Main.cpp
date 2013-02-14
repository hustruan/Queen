//#include "Vector.hpp"
//#include "Matrix.hpp"
//
//#include <iostream>
//#include <chrono>
//#include <ctime>
//#include <vector>
//
//#include <glm/glm.hpp>
//
//#include <DirectXMath.h>
////#include <D3DX10math.h>
////#include <limits>
////
////
////#pragma comment(lib, "D3DX10.lib")
//
//using namespace MathLib;
//using namespace std;
//using namespace DirectX; 

#if !defined(_VARIADIC_MAX)
#define _VARIADIC_MAX    10
#endif

#include <random>
#include <iostream>
#include <functional>

void f(int n1, int n2, int n3, const int& n4, int n5, int n6, int n7)
{
	std::cout << n1 << ' ' << n2 << ' ' << n3 << ' ' << n4 << ' ' << n5 << '\n';
}

int g(int n1)
{
	return n1;
}

struct Foo {
	void print_sum(int n1, int n2)
	{
		std::cout << n1+n2 << '\n';
	}
	int data;
};

int main()
{
	using namespace std::placeholders;

	// demonstrates argument reordering and pass-by-reference
	int n = 7;
	auto f1 = std::bind(f, _2, _1, 42, std::cref(n), n, n);

}