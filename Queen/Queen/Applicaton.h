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
#include "Timer.h"

class Applicaton
{
public:
	Applicaton(void);
	~Applicaton(void);

	bool Create(const std::wstring& title);
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

	// Directly use OpenGL to draw text.
	void DrawText(const std::string& str, float x, float y, const ColorRGBA& color);

	void CalculateFrameRate();

private:
	bool CreateGLWindow(const std::wstring& title, int width, int height, bool fullscreenflag);
	void KillGLWindow();	
	void Tick();
	void Present();

	bool BuildFont();
	void KillFont();

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
	float mFramePerSecond;

	Timer mTimer;

private:
	HDC			mhDC;		// Private GDI Device Context
	HGLRC		mhRC;		// Permanent Rendering Context
	HWND		mhWnd;		// Holds Our Window Handle
	HINSTANCE	mhInstance;		// Holds The Instance Of The Application

	uint32_t mPresentTexture;
	uint32_t mFontDisplayList;


	struct TextInfo
	{
		std::string Text;
		float X, Y;
		ColorRGBA Color;
	};

	std::vector<TextInfo> mTexts;
};


#endif // Applicaton_h__

