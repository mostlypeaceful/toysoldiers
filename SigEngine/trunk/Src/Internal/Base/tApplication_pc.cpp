#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tApplication.hpp"
#include "ToolsPaths.hpp"
#include "Win32Util.hpp"
#include "tGameSessionSearchResult.hpp"
#include "enet/enet.h"

namespace Sig
{
	const tGameSessionInfo & tGameInvite::fSessionInfo( ) const
	{
		static tGameSessionInfo info;
		return info;
	}

	b32 tApplicationPlatformBase::tWin32LogConsole::fLogFilter( const char* text )
	{
		return true;
	}

	void tApplicationPlatformBase::fShowWindow( u32 width, u32 height, s32 x, s32 y, b32 maximize )
	{
		mMainWindow.fShow( width, height, x, y, maximize );
	}

	void tApplicationPlatformBase::fGetLocalSystemTime( u32& hour, u32& minute, u32& second )
	{
		hour = 0;
		minute = 0;
		second = 0;

		SYSTEMTIME sysTime={0};
		GetLocalTime( &sysTime );
		hour = sysTime.wHour;
		minute = sysTime.wMinute;
		second = sysTime.wSecond;
	}

	//HACK: hack to help artists debug stuff fading out
	const Math::tVec3f& tApplication::fFadePos( const Gfx::tCamera& camera ) const
	{
		return camera.fGetTripod( ).mEye;
	}

	void tApplication::fQuitAsync( b32 reboot )
	{
		mKeepRunning = false;
	}	

	b32 tApplication::fPreRun( )
	{
		const int err = enet_initialize( );
		sigassert( err != -1 );

		if( mPlatformOptions.mCreateWindowOverride.fNull( ) )
		{
			// create the main window object; we don't worry about the size/position as the app can manage that later
			const u32 winStyles = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU; // basically your standard window, but don't allow resizing
			if( !mMainWindow.fCreate( mOptions.mGameName.c_str( ), 1, 1, mPlatformOptions.mIconId, BLACK_BRUSH, winStyles, WS_EX_APPWINDOW ) )
				return false;
		}
		else
		{
			// use window creation override
			HWND hwnd = mPlatformOptions.mCreateWindowOverride( );
			if( !hwnd )
				return false;
			mMainWindow.fAssume( hwnd );
		}


#ifdef sig_logging
		// create console app
		std::stringstream consoleHelloText;
		consoleHelloText << "-=-=-=-=-=- Log Window for [" << mOptions.mGameName << "] -=-=-=-=-=-" << std::endl;
		mLogWindow.fCreateConsole( consoleHelloText.str( ).c_str( ) );
#endif//sig_logging

		Log::tAssert::fSetOwnerWindow( ( u64 )mMainWindow.fGetHwnd( ) );

		return true;
	}

	void tApplication::fPostRun( )
	{
#ifdef sig_logging
		mLogWindow.fDestroyConsole( "", false );
#endif//sig_logging
	}

	void tApplication::fPreOnTick( )
	{
		if( !tWin32Window::fMessagePump( ) )
			fQuitAsync( );
	}

	void tApplication::fPostOnTick( )
	{
#ifdef target_game
		if( GetForegroundWindow( ) == fGetHwnd( ) ) 
		{
			// TODOHACK until we have a real way to quit
			static u32 gNumFramesInARow = 0;
			if( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 )	++gNumFramesInARow;
			else											  gNumFramesInARow = 0;
			if( gNumFramesInARow > 20 )						  PostQuitMessage( 0 );
		}
#endif//target_game
	}

	void tApplication::fSetNetworkTimeouts( u32 peerMin, u32 peerMax )
	{
		log_warning_unimplemented( );
	}

	b32 tApplication::fCacheCurrentGameInvite( u32 localHwIndex )
	{
		return false;
	}

	void tApplication::fSetGameRoot( )
	{
		if( !mOptions.mGameRootOverride.fNull( ) )
		{
			mGameRoot = mOptions.mGameRootOverride;
			Win32Util::fSetCurrentDirectory( mGameRoot.fCStr( ) );
		}
		else if( mOptions.mAutoDetectGameRoot )
		{
			std::string currentDirStr = StringUtil::fDirectoryFromPath( Win32Util::fGetCurrentApplicationFileName( ).c_str( ) );

			log_line( Log::cFlagResource, "Starting executable directory is [" << currentDirStr << "]" );

			const char* bin = StringUtil::fStrStrI( currentDirStr.c_str( ), "\\Bin\\" );
			if( !bin ) StringUtil::fStrStrI( currentDirStr.c_str( ), "\\Bin" );
			const char* src = StringUtil::fStrStrI( currentDirStr.c_str( ), "\\Src\\" );
			if( !src ) StringUtil::fStrStrI( currentDirStr.c_str( ), "\\Src" );

			if( src && bin )
			{
				// weird, somehow we're within both src and bin folders;
				// determine which of the two is deeper, and use that
				if( bin > src )
					src = 0;
				else
				{
					sigassert( src > bin );
					bin = 0;
				}
			}

			const char* projSubFolder = 0;
			if( src )
				projSubFolder = src;
			else if( bin )
				projSubFolder = bin;

			if( projSubFolder )
			{
				// we are required to be inside either a bin or a src folder in a non-shipped build,
				// so if this code is running, we can safely sigassert that it's true
				sigassert( projSubFolder && "Invalid project directory structure!" );

				// this should result in our current project path
				currentDirStr.resize( projSubFolder - currentDirStr.c_str( ) );

				// convert to path object
				const tFilePathPtr currentDir = tFilePathPtr( currentDirStr.c_str( ) );

				// get the game root folder from the project path
				mGameRoot = ToolsPaths::fGetGameRootPathFromProjectPath( currentDir );
			}
			else
			{
				// in a non-shipped build, we will be directly above all the assets, so just use the current folder
				mGameRoot = tFilePathPtr::fConstructPath( tFilePathPtr( currentDirStr ), tFilePathPtr( "Game" ), tFilePathPtr( ) );
			}

			// set as the current directory
			Win32Util::fSetCurrentDirectory( mGameRoot.fCStr( ) );
		}

		log_output( 0, "Gameroot set to: '" << mGameRoot << "'" );
	}
}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
