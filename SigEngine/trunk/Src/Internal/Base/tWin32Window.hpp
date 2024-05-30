#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#ifndef __tWin32Window__
#define __tWin32Window__

namespace Sig
{

	class base_export tWin32Window : public tUncopyable
	{
	public:

		tWin32Window( );
		~tWin32Window( );

		b32					fCreate(
								  const char* wndClassName,
								  int width, int height,
								  int icon_id=-1, int bkgnd=-1,
								  int style = WS_OVERLAPPEDWINDOW,
								  int styleex = WS_EX_APPWINDOW,
								  int x = CW_USEDEFAULT, int y = CW_USEDEFAULT );
		void				fAssume( HWND hwnd );
		void				fDestroy( );
		void				fShow( int width, int height, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, b32 maximize = false, b32 show = true );

		inline b32			fCreated( ) const { return mHwnd!=0; }
		inline HWND			fGetHwnd( ) const	{ return mHwnd; }
		inline HINSTANCE	fGetHinst( ) const	{ return mHinst; }

		static b32			fMessagePump( );

	private:

		std::string			mWndClassName;
		s32					mStyle;
		s32					mStyleEx;
		b32					mHasMenu;
		b32					mAssumed;
		HWND				mHwnd;
		HINSTANCE			mHinst;
	};
}

#endif//__tWin32Window__
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
