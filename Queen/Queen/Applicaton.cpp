#include "Applicaton.h"
#include <GL/GL.h>

#pragma comment(lib, "opengl32")

Applicaton* Applicaton::msApp = 0;

LRESULT CALLBACK Applicaton::WndProcStatic( HWND mhWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	if (msApp)
	{
		return msApp->WndProc(mhWnd, message, wParam, lParam);
	}
	return 0;
}

//----------------------------------------------------------------------------
Applicaton::Applicaton(void)
	: mhInstance(NULL), mhDC(NULL),mhRC(NULL), mhWnd(NULL), mPaused(false), mActice(false)
{
	msApp = this;

	Context::Initialize();

	mRenderDevice = new RenderDevice;
	mRenderFactory = new RenderFactory;

	Context::GetSingleton().SetRenderDevice(mRenderDevice);
	Context::GetSingleton().SetRenderFactory(mRenderFactory);
}


Applicaton::~Applicaton(void)
{
	SAFE_DELETE(mRenderDevice);
	SAFE_DELETE(mRenderFactory);

	Context::Finalize();
}

bool Applicaton::CreateGLWindow( const std::wstring& title, int width, int height, bool fullscreenflag )
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left=(long)0;			// Set Left Value To 0
	WindowRect.right=(long)width;		// Set Right Value To Requested Width
	WindowRect.top=(long)0;				// Set Top Value To 0
	WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

	mFullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

	mhInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) Applicaton::WndProcStatic;	// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= mhInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= L"OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, L"Failed To Register The Window Class.", L"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (mFullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL,L"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", 
				L"NeHe GL",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
			{
				mFullscreen=FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL,L"Program Will Now Close.",L"ERROR",MB_OK|MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (mFullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle=WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(mhWnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
		L"OpenGL",							// Class Name
		title.c_str(),						// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right-WindowRect.left,	// Calculate Window Width
		WindowRect.bottom-WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		mhInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,L"Window Creation Error.",L"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,										    // Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		0,											// No Z-Buffer
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(mhDC=GetDC(mhWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,L"Can't Create A GL Device Context.",L"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat=ChoosePixelFormat(mhDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,L"Can't Find A Suitable PixelFormat.",L"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!SetPixelFormat(mhDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,L"Can't Set The PixelFormat.",L"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(mhRC=wglCreateContext(mhDC)))			   // Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,L"Can't Create A GL Rendering Context.",L"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if(!wglMakeCurrent(mhDC, mhRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,L"Can't Activate The GL Rendering Context.",L"ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &mPresentTexture);

	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();									
	glOrtho(-1, 1, -1, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);						
	glLoadIdentity();		

	ShowWindow(mhWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(mhWnd);						// Slightly Higher Priority
	SetFocus(mhWnd);								// Sets Keyboard Focus To The Window
	
	return TRUE;									// Success
}

void Applicaton::KillGLWindow()
{
	if (mFullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (mhRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL,L"Release Of DC And RC Failed.",L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(mhRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL,L"Release Rendering Context Failed.",L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		mhRC=NULL;										// Set RC To NULL
	}

	if (mhDC && !ReleaseDC(mhWnd,mhDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL,L"Release Device Context Failed.",L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		mhDC=NULL;										// Set DC To NULL
	}

	if (mhWnd && !DestroyWindow(mhWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL,L"Could Not Release mhWnd.",L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		mhWnd=NULL;										// Set mhWnd To NULL
	}

	if (!UnregisterClass(L"OpenGL",mhInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL,L"Could Not Unregister Class.",L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		mhInstance=NULL;									// Set hInstance To NULL
	}
}

void Applicaton::OnSuspend()
{
	mPaused = true;
}

void Applicaton::OnResume()
{
	mPaused = false;
}

void Applicaton::OnResized( int width, int height )
{
	mWidth = width;
	mHeight = height;
}


LRESULT Applicaton::WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static bool minimized = false;
	static bool maximized = false;
	static bool inMove = false;

	switch (message)									// Check For Windows Messages
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
		{
			if (!HIWORD(wParam))					// Check Minimization State
			{
				mActice=TRUE;						// Program Is Active
			}
			else
			{
				mActice=FALSE;						// Program Is No Longer Active
			}							// Return To The Message Loop
		}
		break;

	case WM_CLOSE:								// Did We Receive A Close Message?
		{
			PostQuitMessage(0);						// Send A Quit Message								// Jump Back
		}
		break;

	case WM_KEYDOWN:							// Is A Key Being Held Down?
		{
			//keys[wParam] = TRUE;					// If So, Mark It As TRUE
		}
		break;

	case WM_KEYUP:								// Has A Key Been Released?
		{
			//keys[wParam] = FALSE;					// If So, Mark It As FALSE
		}
		break;

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED)
			{
				minimized = true;
				maximized = false;
				OnSuspend();
			}
			else if(wParam == SIZE_MAXIMIZED)
			{
				minimized = false;
				maximized = true;
				OnResized(LOWORD(lParam),HIWORD(lParam));
			}
			else
			{
				if (minimized)
					OnResume();

				minimized = false;
				minimized = false;
	
				if(!inMove)
					OnResized(LOWORD(lParam),HIWORD(lParam));
			}
		}
		break;

	case WM_ENTERSIZEMOVE:
		{
			OnSuspend();
			inMove = true;
		}
		break;

	case WM_EXITSIZEMOVE:
		{
			RECT rect;
			::GetWindowRect(mhWnd, &rect);

			inMove = false;		
			OnResized(rect.right - rect.left, rect.bottom - rect.top);
			OnResume();
		}
		break;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hwnd,message,wParam,lParam);
}

void Applicaton::Tick()
{
	if (mActice && !mPaused)
	{
		Update(0);

		Render();

		Present();
	}
}

void Applicaton::Initialize()
{

}

void Applicaton::LoadContent()
{

}

void Applicaton::UnloadContent()
{

}

void Applicaton::Render()
{
	
}

void Applicaton::Update( float deltaTime )
{

}

bool Applicaton::Create()
{
	if( !CreateGLWindow(L"Queue", 500, 500, false) )
		return false;

	return true;
}

void Applicaton::Run()
{
	Initialize();

	LoadContent();

	MSG msg;
	ZeroMemory( &msg, sizeof( msg ) );
	while( msg.message != WM_QUIT)
	{   
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );

		}
		else
		{
			Tick();
		}
	}

	UnloadContent();

	//KillGLWindow();
}

void Applicaton::Present()
{
	static bool first = true;

	// Present
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	const shared_ptr<Texture2D>& target = mRenderDevice->GetCurrentFrameBuffer()->GetRenderTarget(ATT_Color0);
	const uint32_t targetWidth = target->GetWidth(0);
	const uint32_t targetHeight = target->GetWidth(0);

	void* pBufferData;
	uint32_t pitch;
	target->Map2D(0, TMA_Read_Only, 0, 0, targetWidth, targetHeight, pBufferData, pitch);

	glBindTexture(GL_TEXTURE_2D, mPresentTexture);	

	if ((mWidth < targetWidth) || (mHeight < targetHeight) || first)
	{
		mWidth = targetWidth;
		mHeight = targetHeight;

		first = false;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_FLOAT, pBufferData);
	}
	else
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, targetWidth, targetHeight, GL_RGBA, GL_FLOAT, pBufferData);
	}

	float fw = static_cast<float>(targetWidth) / mWidth;
	float fh = static_cast<float>(targetHeight) / mHeight;

	glViewport(0, 0, targetWidth, targetHeight);

	glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, 0);
		glVertex3f(-1, +1, 0);

		glTexCoord2f(fw, 0);
		glVertex3f(+1, +1, 0);

		glTexCoord2f(0, fh);
		glVertex3f(-1, -1, 0);

		glTexCoord2f(fw, fh);
		glVertex3f(+1, -1, 0);
	glEnd();

	SwapBuffers(mhDC);
}
