#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tWin32Window.hpp"

namespace Sig
{
	namespace Input
	{
		extern void fMouseWheelWndProc( HWND hwnd, WPARAM wParam, LPARAM lParam );
	}

	namespace
	{
		static WNDPROC gAssumedWndProc = 0;

		LRESULT CALLBACK fWindowProcedure( HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam )
		{
			// always pass off to mouse wheel callback (no matter what)
			if( umsg == WM_MOUSEWHEEL )
				Input::fMouseWheelWndProc( hwnd, wparam, lparam );

			// if we've got an assumed wnd proc, pass the rest of execution to it
			if( gAssumedWndProc )
				return CallWindowProc( gAssumedWndProc, hwnd, umsg, wparam, lparam );

			// check for close message
			if( umsg == WM_CLOSE )
			{
				PostQuitMessage( 0 );
				return 0;
			}

			// pass off to default wnd proc
			return DefWindowProc( hwnd, umsg, wparam, lparam );
		}
	}

	tWin32Window::tWin32Window( ) 
		: mWndClassName("")
		, mStyle( 0 )
		, mStyleEx( 0 )
		, mHasMenu( false )
		, mAssumed( false )
		, mHwnd( 0 )
		, mHinst( 0 )
	{
	}

	tWin32Window::~tWin32Window( )
	{
		fDestroy( );
	}

	b32 tWin32Window::fCreate(
							  const char* wndClassName,
							  int width, int height,
							  int icon_id, int bkgnd,
							  int style,
							  int styleex,
							  int x, int y )
	{
		fDestroy( );

		mWndClassName = wndClassName;
		mHinst = GetModuleHandle(0);

		// setup the window class structure to register the window class
		// check for default values (-1), and provide defaults,
		// otherwise load specified resources
		WNDCLASSEX wcx = {0};
		wcx.cbSize = sizeof(WNDCLASSEX);
		wcx.style = CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS;
		wcx.lpfnWndProc = fWindowProcedure;
		wcx.cbClsExtra = 0;
		wcx.cbWndExtra = 0;
		wcx.hInstance = mHinst;
		wcx.hIcon = (icon_id == -1) ? LoadIcon(0, IDI_APPLICATION) : LoadIcon(mHinst, MAKEINTRESOURCE(icon_id));
		wcx.hCursor = LoadCursor(0, IDC_ARROW);
		wcx.hbrBackground = (bkgnd == -1) ? ((HBRUSH)GetStockObject(WHITE_BRUSH)) : ((HBRUSH)GetStockObject(bkgnd));
		wcx.lpszMenuName = 0;
		wcx.lpszClassName = mWndClassName.c_str( );
		wcx.hIconSm = (icon_id == -1) ? LoadIcon(0, IDI_APPLICATION) : LoadIcon(mHinst, MAKEINTRESOURCE(icon_id));

		// try to register the window class
		if ( !RegisterClassEx(&wcx) )
			return false;

		RECT clientRect = { 0 }; clientRect.right = width; clientRect.bottom = height;
		AdjustWindowRectEx( &clientRect, style, FALSE, styleex );

		// try to create the window
		mHwnd = CreateWindowEx (
				styleex,
				wndClassName,
				wndClassName,
				style,
				CW_USEDEFAULT, CW_USEDEFAULT,
				clientRect.right, clientRect.bottom,
				0,
				0,
				mHinst,
				0 );

		if ( !mHwnd )
		{
			UnregisterClass(mWndClassName.c_str( ), mHinst);
			mWndClassName.clear( );
			return false;
		}

		mStyle = style;
		mStyleEx = styleex;
		mHasMenu = (b32)(wcx.lpszMenuName!=0);

		fShow( width, height, x, y, false, ( style & WS_VISIBLE ) );

		return true;
	}

	void tWin32Window::fAssume( HWND hwnd )
	{
		fDestroy( );

		mAssumed = true;
		mHwnd = hwnd;
		mHinst = ( HINSTANCE )( size_t )GetWindowLong( hwnd, GWL_HINSTANCE );
		gAssumedWndProc = ( WNDPROC )( size_t )GetWindowLong( hwnd, GWL_WNDPROC );
		mStyle = GetWindowLong( hwnd, GWL_STYLE );
		mStyleEx = GetWindowLong( hwnd, GWL_EXSTYLE );

		SetWindowLong( hwnd, GWL_WNDPROC, ( LONG )( size_t )&fWindowProcedure );

		char classNameBuf[MAX_PATH]={0};
		GetClassName( hwnd, classNameBuf, array_length( classNameBuf ) - 1 );

		mWndClassName = classNameBuf;

		WNDCLASSEX wcx;
		GetClassInfoEx( mHinst, classNameBuf, &wcx );

		mHasMenu = (b32)(wcx.lpszMenuName!=0);
	}

	void tWin32Window::fDestroy( )
	{
		if( !fCreated( ) )
			return;

		if( !mAssumed )
		{
			DestroyWindow( mHwnd );
			UnregisterClass( mWndClassName.c_str( ), mHinst );
		}

		mWndClassName = "";
		mStyle = 0;
		mStyleEx = 0;
		mHasMenu = false;
		mHwnd = 0;
		mHinst = 0;
		gAssumedWndProc = 0;
	}

	void tWin32Window::fShow( int width, int height, int x, int y, b32 maximize, b32 show )
	{
		sigassert( mHwnd );

		// adjust width and height for styles
		RECT sizeRect = { 0 }; sizeRect.right = width; sizeRect.bottom = height;
		AdjustWindowRectEx(&sizeRect, mStyle, mHasMenu, mStyleEx);
		width = sizeRect.right - sizeRect.left;
		height = sizeRect.bottom - sizeRect.top;

		// compute x and y if default was specified
		if( x == CW_USEDEFAULT )
			x = GetSystemMetrics(SM_CXFULLSCREEN)/2 - width/2;
		if( y == CW_USEDEFAULT )
			y = GetSystemMetrics(SM_CYFULLSCREEN)/2 - height/2;

		SetWindowPos( mHwnd, HWND_TOP, x, y, width, height, show ? SWP_SHOWWINDOW : SWP_HIDEWINDOW );
		if( maximize )
			ShowWindow( mHwnd, SW_MAXIMIZE );
	}

	b32 tWin32Window::fMessagePump( )
	{
		MSG msg;
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if ( msg.message == WM_QUIT )
				return false;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return true;
	}

}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )

