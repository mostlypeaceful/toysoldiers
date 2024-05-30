#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#ifndef __tApplication_pc__
#define __tApplication_pc__
#include "tWin32Window.hpp"
#include "tConsoleApp.hpp"

namespace Sig
{
	struct tGameSessionInfo;
	
	class tGameInvite : public tRefCounter
	{
	public:
		explicit tGameInvite( u32 localHwIndex, const tAddr hostAddress );
		const tGameSessionInfo & fSessionInfo( ) const;
	public:
		u32 mLocalHwIndex;
		tAddr mHostAddress;
	};

	typedef tRefCounterPtr< tGameInvite > tGameInvitePtr;

	///
	/// \brief Provides a pc-specific implementation of tApplication.
	class base_export tApplicationPlatformBase
	{
	public:

		typedef tDelegate<HWND ( )> tCreateWindowOverride;

		struct base_export tPlatformStartupOptions
		{
			s32						mIconId;
			LPSTR					mDefaultCursor;
			tCreateWindowOverride	mCreateWindowOverride;

			tPlatformStartupOptions( )
				: mIconId( -1 )
				, mDefaultCursor( NULL )
			{
			}
		};

	protected:

		class tWin32LogConsole : public tConsoleApp
		{
		protected:
			virtual b32 fLogFilter( const char* text );
		};

		tWin32Window		mMainWindow;
		tWin32LogConsole	mLogWindow;
		tGameInvitePtr		mGameInvite;
		tAddr				mInviteHostAddress;

	public:

		inline HWND fGetHwnd( ) const { return mMainWindow.fGetHwnd( ); }
		inline u64 fGetWindowHandleGeneric( ) const { return ( u64 )fGetHwnd( ); }
		void fShowWindow( u32 width, u32 height, s32 x=CW_USEDEFAULT, s32 y=CW_USEDEFAULT, b32 maximize=false, b32 fullscreen=false );
		void fGetLocalSystemTime( u32& hour, u32& minute, u32& second );
		void fGetWindowPosition( int &x, int &y );

		void fReceivedGameInvite( tAddr hostAddress );
	};


#define implement_application( derivedAppClass ) \
	sig_static_assert( __ClassRegistration__ ); \
	int WINAPI WinMain( HINSTANCE h1, HINSTANCE h2, LPSTR cmdLine, int showCmd ) \
	{ \
		derivedAppClass* theApp = NEW derivedAppClass( ); \
		const int result = theApp->fRun( cmdLine ); \
		delete theApp; \
		return result; \
	}

}

#endif//__tApplication_pc__
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
