#ifndef Film_h__
#define Film_h__

#include "Prerequisites.h"

namespace Purple {

class Filter;

class Film 
{
public:
	Film(int xRes, int yRes, Filter* filter)
		: xResolution(xRes), yResolution(yRes), mFilter(filter)
	{

	}

	virtual ~Film() 
	{
		delete mFilter;
	}

	virtual void AddSample(const Sample& sample, const ColorRGB& L) = 0;


	 const int xResolution, yResolution;

protected:
	 Filter* mFilter;
};


class ImageFilm : public Film
{
public:
	ImageFilm(int xRes, int yRes, Filter* filter);
	~ImageFilm();

	void AddSample(const Sample& sample, const ColorRGB& L);

};

}


#endif // Film_h__
