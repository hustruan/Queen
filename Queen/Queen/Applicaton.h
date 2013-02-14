#ifndef Applicaton_h__
#define Applicaton_h__

#include "Prerequisite.h"
#include "Context.h"
#include "RenderDevice.h"
#include "RenderFactory.h"
#include "GraphicsBuffer.h"
#include "FrameBuffer.h"
#include "Texture.h"
#include "Shader.h"

#include <GL/GL.h>

class Applicaton
{
public:
	Applicaton(void);
	~Applicaton(void);

	bool Create();
	void Run();


protected:
	void Resize(int width, int height);

	virtual void Initialize();
	virtual void LoadContent();
	virtual void UnloadContent();
	virtual void Render();
	virtual void Update(float deltaTime);


	virtual void OnSuspend();
	virtual void OnResized(int width, int height);
	virtual void OnResume();

private:
	bool CreateGLWindow(const std::wstring& title, int width, int height, bool fullscreenflag);
	void KillGLWindow();	
	void Tick();
	void Present();

public:
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static Applicaton* msApp;

protected:
	bool		mFullscreen;
	bool		mActice;
	bool		mPaused;

	RenderDevice* mRenderDevice;
	RenderFactory* mRenderFactory;

	uint32_t mWidth, mHeight;

private:
	HDC			mhDC;		// Private GDI Device Context
	HGLRC		mhRC;		// Permanent Rendering Context
	HWND		mhWnd;		// Holds Our Window Handle
	HINSTANCE	mhInstance;		// Holds The Instance Of The Application

	GLuint mPresentTexture;
};


#endif // Applicaton_h__

