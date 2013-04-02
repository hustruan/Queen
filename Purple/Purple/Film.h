#ifndef Film_h__
#define Film_h__

namespace Purple {

class Film 
{
public:
	Film(int xRes, int yRes)
		: xResolution(xRes), yResolution(yRes)
	{

	}



	 const int xResolution, yResolution;
};



}


#endif // Film_h__
