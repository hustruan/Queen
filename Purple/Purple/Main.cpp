#include "Prerequisites.h"
#include <BlockedArray.h>
#include <ColorRGBA.hpp>

using namespace RxLib;

int main()
{
	double zero = 0.0;

	auto div = 1.0 / zero;
	
	BlockedArray<ColorRGBA, 5> level(252, 216);

	return 0;
}