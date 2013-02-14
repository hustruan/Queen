#ifndef RenderStage_h__
#define RenderStage_h__

#include "Prerequisite.h"

class RenderStage
{
public:
	RenderStage(RenderDevice& device);
	virtual ~RenderStage();

protected:
	RenderDevice& mDevice;

};


#endif // RenderStage_h__
