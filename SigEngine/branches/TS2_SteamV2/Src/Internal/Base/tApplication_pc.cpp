#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tApplication.hpp"
#include "ToolsPaths.hpp"
#include "Win32Util.hpp"
#include "tGameSessionSearchResult.hpp"
#include "enet/enet.h"

namespace Sig
{
	tGameInvite::tGameInvite( u32 localHwIndex, const tAddr hostAddress )
		: mHostAddress( hostAddress )
		, mLocalHwIndex( localHwIndex )
	{
	}

	const tGameSessionInfo & tGameInvite::fSessionInfo( ) const
	{
		static tGameSessionInfo invite;
#if defined( use_steam )
		invite.mId = mHostAddress;
#endif
		return invite;
	}

	b32 tApplicationPlatformBase::tWin32LogConsole::fLogFilter( const char* text )
	{
		return true;
	}

	void tApplicationPlatformBase::fShowWindow( u32 width, u32 height, s32 x, s32 y, b32 maximize, b32 fullscreen )
	{
		mMainWindow.fShow( width, height, x, y, maximize, true, fullscreen );
	}

	void tApplicationPlatformBase::fGetWindowPosition( int &x, int &y )
	{
		mMainWindow.fGetPosition( x, y );
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

	void tApplicationPlatformBase::fReceivedGameInvite( tAddr hostAddress )
	{
		mInviteHostAddress = hostAddress;
	}

	void tApplication::fQuitAsync( b32 reboot )
	{
		mKeepRunning = false;
	}	

	b32 tApplication::fPreRun( )
	{
		if( mPlatformOptions.mCreateWindowOverride.fNull( ) )
		{
			// create the main window object; we don't worry about the size/position as the app can manage that later
			const u32 winStyles = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU; // basically your standard window, but don't allow resizing
			if( !mMainWindow.fCreate( mOptions.mGameName.c_str( ), 1280, 720, mPlatformOptions.mIconId, mPlatformOptions.mDefaultCursor, BLACK_BRUSH, winStyles, WS_EX_APPWINDOW ) )
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
		//std::stringstream consoleHelloText;
		//consoleHelloText << "-=-=-=-=-=- Log Window for [" << mOptions.mGameName << "] -=-=-=-=-=-" << std::endl;
		//mLogWindow.fCreateConsole( consoleHelloText.str( ).c_str( ) );
#endif//sig_logging

		Log::tAssert::fSetOwnerWindow( ( u64 )mMainWindow.fGetHwnd( ) );

		return true;
	}

	void tApplication::fPostRun( )
	{
#ifdef sig_logging
		//mLogWindow.fDestroyConsole( "", false );
#endif//sig_logging
	}

	void tApplication::fPreOnTick( )
	{
		if( !tWin32Window::fMessagePump( ) )
			fQuitAsync( );
	}

	void tApplication::fPostOnTick( )
	{
//#ifdef target_game
//		if( GetForegroundWindow( ) == fGetHwnd( ) ) 
//		{
//			// TODOHACK until we have a real way to quit
//			static u32 gNumFramesInARow = 0;
//			if( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 )	++gNumFramesInARow;
//			else											  gNumFramesInARow = 0;
//			if( gNumFramesInARow > 20 )						  PostQuitMessage( 0 );
//		}
//#endif//target_game
	}

	void tApplication::fSetNetworkTimeouts( u32 peerMin, u32 peerMax )
	{
		enet_set_peer_timeouts( peerMin, peerMax );
	}

	b32 tApplication::fCacheCurrentGameInvite( u32 localHwIndex )
	{
		if ( mGameInvite && mGameInvite->mLocalHwIndex != localHwIndex )
		{
			log_warning( 0,
				"User [" << localHwIndex << "] has attempted to accept a game invite while user ["
				<< mGameInvite->mLocalHwIndex << "] is already in the process of trying to join a game - ignoring new User ["
				<< localHwIndex << "]'s invite." );

			return false;
		}

		mGameInvite.fReset( NEW tGameInvite( localHwIndex, mInviteHostAddress ) );
		return true;
	}

	void tApplication::fSetGameRoot( )
	{
		tFilePathPtr gameRoot;

		if( !mOptions.mGameRootOverride.fNull( ) )
		{
			gameRoot = mOptions.mGameRootOverride;
			Win32Util::fSetCurrentDirectory( gameRoot.fCStr( ) );
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
				gameRoot = ToolsPaths::fGetGameRootPathFromProjectPath( currentDir );
			}
			else
			{
				// in a non-shipped build, we will be directly above all the assets, so just use the current folder
				gameRoot = tFilePathPtr( currentDirStr );
			}

			// set as the current directory
			Win32Util::fSetCurrentDirectory( gameRoot.fCStr( ) );
		}

		fResourceDepot( )->fSetRootPath( gameRoot );
	}

#if defined( platform_pcdx ) && defined( target_game )
	void tApplication::fOnSteamOverlayToggled( GameOverlayActivated_t* gameOverlayActivated )
	{
		mSystemUiShowing = gameOverlayActivated->m_bActive;
	}
#endif // defined( platform_pcdx ) && defined( target_game )
}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
