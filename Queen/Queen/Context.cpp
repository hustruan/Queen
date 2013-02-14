#include "Context.h"
#include "RenderDevice.h"
#include "RenderFactory.h"

SINGLETON_DECL(Context)

Context::Context()
	:  mRenderFactory(nullptr)
{

}

Context::~Context()
{
	//Safe_Delete(mInputSystem);
}


void Context::SetRenderFactory( RenderFactory* factory )
{
	assert(factory);
	mRenderFactory = factory;
}

void Context::SetRenderDevice( RenderDevice* device )
{
	assert(device);
	mRenderDevice = device;
}

