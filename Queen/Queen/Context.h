#ifndef Context_h__
#define Context_h__

#include "Prerequisite.h"
#include <Singleton.h>

class Context : public RxLib::Singleton<Context>
{
public:
	SINGLETON_DECL_HEADER(Context)

public:
	Context(void);
	~Context(void);

	void SetRenderFactory(RenderFactory* factory);
	RenderFactory& GetRenderFactory()						{ return *mRenderFactory; }
	RenderFactory* GetRenderFactoryPtr()					{ return mRenderFactory; }

	void SetRenderDevice(RenderDevice* device);
	RenderDevice& GetRenderDevice()							{ assert(mRenderDevice); return *mRenderDevice; }
	RenderDevice* GetRenderDevicePtr()						{ assert(mRenderDevice); return mRenderDevice; }

public:
	RenderDevice* mRenderDevice;
	RenderFactory* mRenderFactory;
	//InputSystem* mInputSystem;
};

#endif // Context_h__
