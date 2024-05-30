#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tApplication.hpp"
#include "enet/enet.h"
#include "tGameSessionSearchResult.hpp"
#include "tProfiler.hpp"

namespace Sig
{
	devvar(bool, Cameras_UsePlayerPosAsFadePos, false);

	tGameInvite::tGameInvite( u32 localHwIndex, const XINVITE_INFO& xinvite )
		: XINVITE_INFO( xinvite )
		, mLocalHwIndex( localHwIndex )
	{
	}

	const tGameSessionInfo & tGameInvite::fSessionInfo( ) const { return ( const tGameSessionInfo & )hostInfo; }


	namespace
	{
		static void* fNoCacheHeapAlloc( u32 numBytes )
		{
			void* mem = XPhysicalAlloc( numBytes, MAXULONG_PTR, 0, PAGE_READWRITE | PAGE_NOCACHE );
			sigassert( mem && "OUT OF MEMORY" );
			return mem;
		}
		static void* fVramHeapAlloc( u32 numBytes )
		{
			void* mem = XPhysicalAlloc( numBytes, MAXULONG_PTR, 0, PAGE_READWRITE | PAGE_WRITECOMBINE | MEM_LARGE_PAGES );
			sigassert( mem && "OUT OF MEMORY" );
			return mem;
		}
		inline const XALLOC_ATTRIBUTES& fConvert( const DWORD& dwAllocAttributes )
		{
			return reinterpret_cast<const XALLOC_ATTRIBUTES&>( dwAllocAttributes );
		}
		inline u32 fConvertPhysicalAlignment( const XALLOC_ATTRIBUTES& allocAtts )
		{
			switch( allocAtts.dwAlignment )
			{
			case XALLOC_PHYSICAL_ALIGNMENT_DEFAULT:		return 4*1024; break;
			case XALLOC_PHYSICAL_ALIGNMENT_4:			return 4; break;
			case XALLOC_PHYSICAL_ALIGNMENT_8:			return 8; break;
			case XALLOC_PHYSICAL_ALIGNMENT_16:			return 16; break;
			case XALLOC_PHYSICAL_ALIGNMENT_32:			return 32; break;
			case XALLOC_PHYSICAL_ALIGNMENT_64:			return 64; break;
			case XALLOC_PHYSICAL_ALIGNMENT_128:			return 128; break;
			case XALLOC_PHYSICAL_ALIGNMENT_256:			return 256; break;
			case XALLOC_PHYSICAL_ALIGNMENT_512:			return 512; break;
			case XALLOC_PHYSICAL_ALIGNMENT_1K:			return 1*1024; break;
			case XALLOC_PHYSICAL_ALIGNMENT_2K:			return 2*1024; break;
			case XALLOC_PHYSICAL_ALIGNMENT_4K:			return 4*1024; break;
			case XALLOC_PHYSICAL_ALIGNMENT_8K:			return 8*1024; break;
			case XALLOC_PHYSICAL_ALIGNMENT_16K:			return 16*1024; break;
			case XALLOC_PHYSICAL_ALIGNMENT_32K:			return 32*1024; break;
			default: sigassert(!"invalid alignment!");		break;
			}

			return 0;
		}
	}

	LPVOID tApplicationPlatformBase::fXMemAlloc( SIZE_T dwSize, DWORD dwAllocAttributes )
	{
		const XALLOC_ATTRIBUTES& atts = fConvert( dwAllocAttributes );
		sigassert( dwSize < ( DWORD )std::numeric_limits<s32>::max( ) );

		if( XALLOC_IS_PHYSICAL( dwAllocAttributes ) )
		{
			const u32 alignment = fConvertPhysicalAlignment( atts );

			void* o=0;
			switch( atts.dwMemoryProtect )
			{
			case XALLOC_MEMPROTECT_NOCACHE:
				{
					o = tApplication::fInstance( ).fNoCacheHeap( ).fAllocInternal( dwSize, alignment, Memory::tAllocStamp::fNoContextStamp( ) );
					if( !o )
						Log::fFatalError( );
				}
				break;
			case XALLOC_MEMPROTECT_WRITECOMBINE:
			case XALLOC_MEMPROTECT_WRITECOMBINE_LARGE_PAGES:
				{
					sigassert( Memory::tHeap::mVramContext != Memory::tAllocStamp::fNoContextStamp( ) );
					Memory::tAllocStamp stampCopy = Memory::tHeap::mVramContext;
					stampCopy.mTypeName = stampCopy.mCppFile;
					stampCopy.mSize = dwSize;

					o = tApplication::fInstance( ).fVramHeap( ).fAllocInternal( dwSize, alignment, stampCopy /*, dwSize > 2*1024*1024 ? btrue : bfalse*/ );
					if( !o )
						Log::fFatalError( );

					// Track system vram.
#if defined(sig_memory_dump)
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextSysVram ) profile_mem( cProfileMemVramSystem, fXMemSize( o, dwAllocAttributes ) );
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextSysTextures ) profile_mem( cProfileMemVramSysTextures, fXMemSize( o, dwAllocAttributes ) );
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextSysGeometry ) profile_mem( cProfileMemVramSysGeom, fXMemSize( o, dwAllocAttributes ) );
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextSysShader ) profile_mem( cProfileMemVramSysShader, fXMemSize( o, dwAllocAttributes ) );
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextTexture ) profile_mem( cProfileMemTexture, fXMemSize( o, dwAllocAttributes ) );
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextGeometry ) profile_mem( cProfileMemGeometry, fXMemSize( o, dwAllocAttributes ) );
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextFui ) profile_mem( cProfileMemFui, fXMemSize( o, dwAllocAttributes ) );
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextMovie ) profile_mem( cProfileMemMovie, fXMemSize( o, dwAllocAttributes ) );
					if( stampCopy.mContext == Memory::AllocStampContext::cAllocStampContextGroundCover ) profile_mem( cProfileMemGroundCover, fXMemSize( o, dwAllocAttributes ) );
#endif
				}
				break;
			case XALLOC_MEMPROTECT_READWRITE:
				{
					void* mem = XMemAllocDefault( dwSize, dwAllocAttributes );
					if( !mem )
						Log::fFatalError( );
					return mem;
				}
				break;
			default:
				sigassert(0);
				break;
			}

			if( atts.dwZeroInitialize )
				fMemSet( o, 0, dwSize );

			sigassert( fIsAligned<size_t>( (size_t)o, alignment ) && "Memory mis-algined!" );

			return o;
		}
		else
		{
			sigassert( atts.dwMemoryProtect == XALLOC_MEMPROTECT_READWRITE );

			void* o = Memory::tHeap::fInstance( ).fAlloc( dwSize, Memory::tAllocStamp( __FILE__, __LINE__, "fXMemAlloc", "READWRITE", dwSize ) );
			if( atts.dwZeroInitialize )
				fMemSet( o, 0, dwSize );

			profile_mem( cProfileMemMain, fXMemSize( o, dwAllocAttributes ) );
			return o;
		}
	}

	VOID tApplicationPlatformBase::fXMemFree( PVOID pAddress, DWORD dwAllocAttributes )
	{
		const XALLOC_ATTRIBUTES& atts = fConvert( dwAllocAttributes );

		if( XALLOC_IS_PHYSICAL( dwAllocAttributes ) )
		{
			if( !pAddress )
				return;

			switch( atts.dwMemoryProtect )
			{
			case XALLOC_MEMPROTECT_NOCACHE:
				tApplication::fInstance( ).fNoCacheHeap( ).fFree( pAddress );
				break;
			case XALLOC_MEMPROTECT_WRITECOMBINE:
			case XALLOC_MEMPROTECT_WRITECOMBINE_LARGE_PAGES:
				{
					// track system vram consumption
#if defined(sig_memory_dump)
					const Memory::tAllocStamp* stamp = static_cast<Memory::tExternalHeap&>( tApplication::fInstance( ).fVramHeap( ) ).fGetStamp( pAddress );
					sigassert( stamp );

					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextSysVram ) profile_mem( cProfileMemVramSystem, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextSysTextures ) profile_mem( cProfileMemVramSysTextures, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextSysGeometry ) profile_mem( cProfileMemVramSysGeom, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextSysShader ) profile_mem( cProfileMemVramSysShader, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextTexture ) profile_mem( cProfileMemTexture, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextGeometry ) profile_mem( cProfileMemGeometry, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextFui ) profile_mem( cProfileMemFui, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextMovie ) profile_mem( cProfileMemMovie, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
					if( stamp->mContext == Memory::AllocStampContext::cAllocStampContextGroundCover ) profile_mem( cProfileMemGroundCover, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
#endif

					tApplication::fInstance( ).fVramHeap( ).fFree( pAddress );
				}
				break;
			case XALLOC_MEMPROTECT_READWRITE:
				XMemFreeDefault( pAddress, dwAllocAttributes );
				break;
			default:
				sigassert(0);
				break;
			}
		}
		else
		{
			profile_mem( cProfileMemMain, -(s32)fXMemSize( pAddress, dwAllocAttributes ) );
			Memory::tHeap::fInstance( ).fFree( pAddress );
		}
	}

	SIZE_T tApplicationPlatformBase::fXMemSize( PVOID pAddress, DWORD dwAllocAttributes )
	{
		const XALLOC_ATTRIBUTES& atts = fConvert( dwAllocAttributes );
		//assert( atts.dwHeapTracksAttributes == 0 );

		if( XALLOC_IS_PHYSICAL( dwAllocAttributes ) )
		{
			//return XMemSizeDefault( pAddress, dwAllocAttributes );

			if( !pAddress )
				return 0;
			switch( atts.dwMemoryProtect )
			{
			case XALLOC_MEMPROTECT_NOCACHE:
				return tApplication::fInstance( ).fNoCacheHeap( ).fSizeOfAlloc( pAddress );
			case XALLOC_MEMPROTECT_WRITECOMBINE:
			case XALLOC_MEMPROTECT_WRITECOMBINE_LARGE_PAGES:
				return tApplication::fInstance( ).fVramHeap( ).fSizeOfAlloc( pAddress );
			case XALLOC_MEMPROTECT_READWRITE:
				return XMemSizeDefault( pAddress, dwAllocAttributes );
			default:
				sigassert(0);
				return 0;
			}

		}
		else
			return Memory::tHeap::fInstance( ).fSizeOfAlloc( pAddress );
	}

	tApplicationPlatformBase::tApplicationPlatformBase( )
		: mNoCacheHeap( fNoCacheHeapAlloc, XPhysicalFree, tApplication::fMemoryOptions( ).mNoCache, "No Cache" )
		, mMainVramHeap( fVramHeapAlloc, XPhysicalFree, tApplication::fMemoryOptions( ).mVRam, Memory::tHeap::cVramHeapName )
		, mEventListener( 0 )
	{
	}

	void tApplicationPlatformBase::fShowWindow( u32 width, u32 height, s32 x, s32 y, b32 maximize )
	{
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
		if_devmenu( if( Cameras_UsePlayerPosAsFadePos ) return mFadePos; );
		return camera.fGetTripod( ).mEye;
	}

	void tApplication::fQuitAsync( b32 reboot )
	{
		mKeepRunning = false;

		enet_deinitialize( );
		XCloseHandle( mEventListener );
		XOnlineCleanup( );
		XNetCleanup( );

		if( reboot )
		{
			XLaunchNewImage( "default.xex", 0 );
		}
		else
		{
			//XLaunchNewImage( XLAUNCH_KEYWORD_DEFAULT_APP, 0 );
			XLaunchNewImage( XLAUNCH_KEYWORD_DASH_ARCADE, 0 );
		}
	}	

	b32 tApplication::fPreRun( )
	{
#ifdef sig_devmenu
		XNetStartupParams xnsp;
		fZeroOut( xnsp );
		xnsp.cfgSizeOfStruct = sizeof(XNetStartupParams);
		xnsp.cfgFlags = XNET_STARTUP_BYPASS_SECURITY;
		XNetStartup( &xnsp );
#else
		XNetStartup( 0 );
#endif

		const DWORD xerr = XOnlineStartup( );
		sigassert( xerr == ERROR_SUCCESS );
		const int err = enet_initialize( );
		sigassert( err != -1 );

		mEventListener = XNotifyCreateListener( XNOTIFY_SYSTEM | XNOTIFY_LIVE | XNOTIFY_PARTY );
		return true;
	}

	bool fIsSignInSystemCallLegit( )
	{
		bool legit = false;

		//if any person is signed in then we assume the system call is legit
		for( u32 i = 0; i < XUSER_MAX_COUNT; ++i )
		{
			XUSER_SIGNIN_STATE state = XUserGetSigninState( i );
			if( state != eXUserSigninState_NotSignedIn )
			{
				legit = true;
				break;
			}
		}
		return legit;
	}

	void tApplication::fPostRun( )
	{
	}

#ifdef sig_logging
	namespace{
		tStringPtr fEventIdToString( u32 id )
		{
			static tHashTable<DWORD,tStringPtr> sMsgToString;
			#define MSG(val) sMsgToString[ val ] = tStringPtr( #val );
				//sys
				MSG( XN_SYS_FIRST )
				MSG( XN_SYS_UI )
				MSG( XN_SYS_SIGNINCHANGED )
				MSG( XN_SYS_STORAGEDEVICESCHANGED )
				MSG( XN_SYS_PROFILESETTINGCHANGED )
				MSG( XN_SYS_MUTELISTCHANGED )
				MSG( XN_SYS_INPUTDEVICESCHANGED )
				MSG( XN_SYS_INPUTDEVICECONFIGCHANGED )
				MSG( XN_SYS_PLAYTIMERNOTICE )
				MSG( XN_SYS_AVATARCHANGED )
				MSG( XN_SYS_NUIHARDWARESTATUSCHANGED )
				MSG( XN_SYS_NUIPAUSE )
				MSG( XN_SYS_NUIUIAPPROACH )
				MSG( XN_SYS_DEVICEREMAP )
				MSG( XN_SYS_NUIBINDINGCHANGED )
				MSG( XN_SYS_AUDIOLATENCYCHANGED )
				MSG( XN_SYS_NUICHATBINDINGCHANGED )
				MSG( XN_SYS_INPUTACTIVITYCHANGED )
				MSG( XN_SYS_LAUNCHDATACHANGED )
				MSG( XN_SYS_FITNESSINFOCHANGED )
					//MSG( XN_SYS_LAST )
				//live
				MSG( XN_LIVE_FIRST )
				MSG( XN_LIVE_CONNECTIONCHANGED )
				MSG( XN_LIVE_INVITE_ACCEPTED )
				MSG( XN_LIVE_LINK_STATE_CHANGED )
				MSG( XN_LIVE_CONTENT_INSTALLED )
				MSG( XN_LIVE_MEMBERSHIP_PURCHASED )
				MSG( XN_LIVE_VOICECHAT_AWAY )
				MSG( XN_LIVE_PRESENCE_CHANGED )
					//MSG( XN_LIVE_LAST )
				//friend
				MSG( XN_FRIENDS_FIRST )
				MSG( XN_FRIENDS_PRESENCE_CHANGED )
				MSG( XN_FRIENDS_FRIEND_ADDED )
				MSG( XN_FRIENDS_FRIEND_REMOVED )
					//MSG( XN_FRIENDS_LAST )
				//custom
				MSG( XN_CUSTOM_FIRST )
				MSG( XN_CUSTOM_ACTIONPRESSED )
				MSG( XN_CUSTOM_GAMERCARD )
					//MSG( XN_CUSTOM_LAST )
				//xmp
				MSG( XN_XMP_FIRST )
				MSG( XN_XMP_STATECHANGED )
				MSG( XN_XMP_PLAYBACKBEHAVIORCHANGED )
				MSG( XN_XMP_PLAYBACKCONTROLLERCHANGED )
					//MSG( XN_XMP_LAST )
				//party
				MSG( XN_PARTY_FIRST )
				MSG( XN_PARTY_MEMBERS_CHANGED )
					//MSG( XN_PARTY_LAST )
			#undef MSG
			tStringPtr* msg = sMsgToString.fFind( id );
			if( msg )
				return *msg;
			static const tStringPtr sMsg( "UNKNOWN XBOX EVENT" );
			return sMsg;
		}
	}//unnamed namespace
#endif sig_logging

	devvar( bool, Debug_UI_IdleInputPause, true );

	void tApplication::fPreOnTick( )
	{
		profile_pix( "tApplication::fPreOnTick" );

		b32 refreshPlatformIds = false;

		DWORD notificationId = 0;
		ULONG_PTR notificationParam = 0;
		while( XNotifyGetNext( mEventListener, 0, &notificationId, &notificationParam ) )
		{
#ifdef sig_logging
			const tStringPtr msg = fEventIdToString( notificationId );
			log_output(0,"[XBOX EVENT] " << msg.fCStr( ) << " (" << notificationId << ")" << std::endl);
#endif//sig_logging
			switch( notificationId )
			{
			case XN_SYS_SIGNINCHANGED:
				{
					profile_pix( "XN_SYS_SIGNINCHANGED" );
					if( fIsSignInSystemCallLegit( ) )
						refreshPlatformIds = true;
					else
						mSecondSignInWaitTime = 0.f;
				}
				break;

			case XN_SYS_UI:
				{
					profile_pix( "XN_SYS_UI" );
					mSystemUiShowing = ( b32 )notificationParam;
					fOnSystemUiChange( mSystemUiShowing );
				}
				break;

			case XN_LIVE_INVITE_ACCEPTED:
				{
					profile_pix( "XN_LIVE_INVITE_ACCEPTED" );
					fOnGameInviteAccepted( ( u32 )notificationParam );
				}
				break;

			case XN_SYS_MUTELISTCHANGED:
				{
					profile_pix( "XN_SYS_MUTELISTCHANGED" );
					fOnMuteListChanged( );
				}
				break;

			case XN_SYS_INPUTACTIVITYCHANGED:
				{
					profile_pix( "XN_SYS_INPUTACTIVITYCHANGED" );
					if( Debug_UI_IdleInputPause )
						fOnInputActivityChanged( ((BOOL)notificationParam) != FALSE );
				}
				break;

			case XN_SYS_PROFILESETTINGCHANGED:
				{
					profile_pix( "XN_SYS_PROFILESETTINGCHANGED" );
					DWORD user = (DWORD)notificationParam;
					user &= 0xF;
					for( u32 i = 0; i < tUser::cMaxLocalUsers; ++i )
					{
						if( user & ( 1 << i ) )
						{
							mLocalUsers[ i ]->fPlatformSettingsChanged( );
						}
					}
				}
				break;

			case XN_SYS_STORAGEDEVICESCHANGED:
				{
					profile_pix( "XN_SYS_STORAGEDEVICESCHANGED" );
					//if( tGameApp::fGetInstance( ).fOnDlcInstalled( ) )
					//	mOnContentInstalled.fFire( );
					//mOnStorageDevicesChanged.fFire( );
					fOnStorageDeviceChanged( );
				}
				break;
			case XN_SYS_INPUTDEVICESCHANGED:
				{
					profile_pix( "XN_SYS_INPUTDEVICESCHANGED" );
					u32 connected = 0;
					for( u32 i = 0; i < tUser::cMaxLocalUsers; ++i )
					{
						XINPUT_STATE ignored;
						if( XInputGetState( i, &ignored ) != ERROR_DEVICE_NOT_CONNECTED )
							connected |= ( 1 << i );
					}

					fOnInputDevicesChanged( connected );
				}
				break;
			case XN_SYS_PLAYTIMERNOTICE:
				{
					profile_pix( "XN_SYS_PLAYTIMERNOTICE" );
					//mOnPlayTimerNotice.fFire( ( u32 )notificationParam );
				}
				break;

			case XN_LIVE_CONNECTIONCHANGED:
				{
					profile_pix( "XN_LIVE_CONNECTIONCHANGED" );
					//mOnConnectionChanged.fFire( ( tConnectionState )notificationParam );
					switch( notificationParam )
					{
					case XONLINE_S_LOGON_CONNECTION_ESTABLISHED: // if the connection is valid
						break;
					case XONLINE_S_LOGON_DISCONNECTED: // if the connection was closed gracefully
						break;
					// Otherwise, returns one of the following errors:
					case XONLINE_E_LOGON_NO_NETWORK_CONNECTION:
						break;
					case XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE:
						break;
					case XONLINE_E_LOGON_UPDATE_REQUIRED:
						break;
					case XONLINE_E_LOGON_SERVERS_TOO_BUSY:
						break;
					case XONLINE_E_LOGON_CONNECTION_LOST:
						break;
					case XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON:
						break;
					case XONLINE_E_LOGON_INVALID_USER:
						break;
					}
				}
				break;


			case XN_LIVE_LINK_STATE_CHANGED:
				{
					profile_pix( "XN_LIVE_LINK_STATE_CHANGED" );
					//mOnLinkStateChanged.fFire( ( b32 )notificationParam );
					const b32 connected = ( b32 )notificationParam;
				}
				break;

			case XN_LIVE_CONTENT_INSTALLED:
				{
					profile_pix( "XN_LIVE_CONTENT_INSTALLED" );
					fReadLicense( );
					fOnDlcInstalled( );
					//mOnContentInstalled.fFire( );
				}
				break;
			case XN_LIVE_MEMBERSHIP_PURCHASED:
				{
					profile_pix( "XN_LIVE_MEMBERSHIP_PURCHASED" );
					//mOnMembershipPurchased.fFire( );
				}
				break;

			case XN_PARTY_MEMBERS_CHANGED:
				{
					profile_pix( "XN_LIVE_MEMBERSHIP_PURCHASED" );
					fOnPartyMembersChanged( );
				}
				break;
			}
		}

		//This is all from xbox FAQ #17:
		/*
		https://developer.xboxlive.com/en-us/xbox/development/support/Pages/AccesstoFAQs.aspx

			17. I get multiple XN_SYS_SIGNINCHANGED messages. After receiving the first, everyone appears signed out. What gives?

			This is a known issue. The proper way to work around it is to wait roughly 1 second for a second XN_SYS_SIGNINCHANGED 
			message. If a second message comes in, trust the sign-in states returned from the second message instead of the first 
			before taking action in the title. If no second message comes, trust the state returned from the first. It’s a little 
			painful, and we apologize. This workaround should safely work around the issue, and should remain safe if the issue is 
			fixed in a future revision of the flash.
		*/
		if( mSecondSignInWaitTime >= 0.f )
		{
			mSecondSignInWaitTime += mFrameDeltaTime;
			if( mSecondSignInWaitTime > 1.1f ) //FAQ says to wait "roughly" 1s after receiving the notification to know whether or not it was legit
			{
				refreshPlatformIds = true;
			}
		}

		if( refreshPlatformIds )
		{
			profile_pix( "Refresh platform IDs" );
			refreshPlatformIds = false;
			mSecondSignInWaitTime = -1.f;

			// Cache the current signin info
			tUserSigninArray oldStates;
			fMemCpy( oldStates.fBegin( ), mLocalSigninInfo.fBegin( ), mLocalSigninInfo.fTotalSizeOf( ) );

			// Refresh info
			for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
				mLocalUsers[ i ]->fRefreshPlatformId( );

			// Update the signin info
			fGatherSigninInfo( );

			// Signal the change
			fOnUserSignInChange( oldStates.fBegin( ), mLocalSigninInfo.fBegin( ) );
		}

	}

	void tApplication::fPostOnTick( )
	{
	}

	void tApplication::fSetNetworkTimeouts( u32 peerMin, u32 peerMax )
	{
		enet_set_peer_timeouts( peerMin, peerMax );
	}

	b32 tApplication::fCacheCurrentGameInvite( u32 localHwIndex )
	{
		if( mGameInvite && mGameInvite->mLocalHwIndex != localHwIndex )
		{
			log_warning(	
					"User [" << localHwIndex << "] has attempted to accept a game invite while user [" 
				<< mGameInvite->mLocalHwIndex << "] is already in the process of trying to join a game - ignoring new User [" 
				<< localHwIndex << "]'s invite." );

			return false;
		}

		XINVITE_INFO inviteInfo = {0};
		const DWORD result = XInviteGetAcceptedInfo( localHwIndex, &inviteInfo );
		if( result == ERROR_SUCCESS )
		{
			// consider buffering these? i.e., the invite has been accepted, but might be awhile before we get into lobby - more
			// invites could conceivably be accepted before we get there - if so, always prefer the latest one? check with MSFT on acceptable behavior in this case
			mGameInvite.fReset( NEW tGameInvite( localHwIndex, inviteInfo ) );
			return true;
		}
		else
		{
			log_warning( "Error accepting game invite: " << result );
			return false;

			// TODO should probably clear existing invite if it exists, that said we need to be careful of current game state - 
			// i.e., are we in the process of trying to get to a lobby? in which case simply setting mGameInvite to null might cause problems
		}
	}

	void tApplication::fGatherSigninInfo( )
	{
		for( u32 u = 0; u < mLocalSigninInfo.cDimension; ++u )
		{
			mLocalSigninInfo[ u ].mState = mLocalUsers[ u ]->fSignInState( );
			mLocalSigninInfo[ u ].mOfflineId = mLocalUsers[ u ]->fOfflinePlatformId( );
			mLocalSigninInfo[ u ].mOnlineId = mLocalUsers[ u ]->fPlatformId( );
		}
	}

	void tApplication::fSetGameRoot( )
	{
		mGameRoot = tFilePathPtr( "game:\\" );
	}
}
#endif//#if defined( platform_xbox360 )
