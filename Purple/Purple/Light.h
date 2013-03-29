#ifndef Light_h__
#define Light_h__

namespace Purple {

class Light
{
public:
	Light(void);
	virtual ~Light(void);
};



class PointLight : public Light
{

};


class SpotLight : public Light
{

};

class AreaLight : public Light
{

};



}

#endif // Light_h__


