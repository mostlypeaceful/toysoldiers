#include "BasePch.hpp"
#include "tSteamManager.hpp"
#include "tBase64.hpp"
#include "tGameAppBase.hpp"

#if defined(use_steam)

namespace Sig
{
	devvar( bool, Steam_BigPictureMode, false );
	static b32 sBigPictureMode = false;

#pragma warning( push )
#pragma warning( disable : 4355 )

	//-------------------------------------------------------------------------------------------------
	b32 tSteamManager::fIsBigPictureMode( )
	{
		return sBigPictureMode;
	}

	//-------------------------------------------------------------------------------------------------
	// tSteamManager
	//-------------------------------------------------------------------------------------------------
	tSteamManager::tSteamManager( )
		: mInitialised( false )
		, mInitialiseAttempted( false )
		, mCallbackGamepadTextDismissed( this, &tSteamManager::fOnGamepadTextDismissed )
		, mCallbackMicroTxnAuthorizationResponse( this, &tSteamManager::fOnMicroTxnAuthorizationResponse )
		, mCallbackGameLobbyJoinRequested( this, &tSteamManager::fOnGameLobbyJoinRequested )
	{
		// Steam sets an environment variable called SteamTenfoot to 1 when launching apps from Big Picture Mode.
		// If we want to test in Big Picture Mode, set the Steam client to Big Picture Mode and set Steam_BigPictureMode to 1 in res\user.ini
		if( Steam_BigPictureMode )
		{
			_putenv_s( "SteamTenfoot", "1" );
			sBigPictureMode = true;
		}
		else
		{
			auto bpm = getenv( "SteamTenfoot" );
			if( bpm )
			{
				sBigPictureMode = strcmp( bpm, "1" ) == 0;
#if defined( sig_devmenu )
				Steam_BigPictureMode.fSetFromString( bpm );
#endif
			}
		}
	}

#pragma warning( pop )

	//-------------------------------------------------------------------------------------------------
	tSteamManager::~tSteamManager( )
	{
	}

	//-------------------------------------------------------------------------------------------------
	static void fWarningMessageHook(int errorCode, const char* message)
	{
		log_warning( Log::cFlagNone, "Steam error code " << errorCode << ": " << message );
	}

	//-------------------------------------------------------------------------------------------------
	void tSteamManager::fStartup( )
	{
		if( SteamAPI_RestartAppIfNecessary( k_uAppIdInvalid ) )
		{
			// JP - Haven't been able to test if this is the correct response yet..
			exit(1);
		}

		mInitialiseAttempted = true;
		mInitialised = SteamAPI_Init( );

		if( mInitialised )
		{
			mAppID = SteamUtils( )->GetAppID( );
			log_line( Log::cFlagNone, "Steam initialized. AppId " << mAppID << "  SteamID " << std::hex << SteamUser( )->GetSteamID( ).ConvertToUint64( ) );
			SteamUtils( )->SetWarningMessageHook( fWarningMessageHook );
		}
		else
		{
			log_warning( Log::cFlagNone, "Unable to initalise steam - perhaps steam client is not running?" );
		}
	}

	//-------------------------------------------------------------------------------------------------
	const char *tSteamManager::fGetLanguage( )
	{
		return SteamApps( ) ? SteamApps( )->GetCurrentGameLanguage( ) : "english";
	}

	//-------------------------------------------------------------------------------------------------
	b32 tSteamManager::fGetPlatformUserId( tPlatformUserId &userId )
	{
		if (mInitialised)
		{
			userId = fCastSteamIDToPlatformUserId( SteamUser( )->GetSteamID( ) );
			return true;
		}

		userId = tPlatformUserId( k_steamIDNil );
		return false;
	}

	//-------------------------------------------------------------------------------------------------
	b32 tSteamManager::fIsSteamAvailable( )
	{
		return mInitialised;
	}

	//-------------------------------------------------------------------------------------------------
	const char *tSteamManager::fGetUserName( CSteamID userID )
	{
		return SteamFriends( )->GetFriendPersonaName( userID );
	}

	//-------------------------------------------------------------------------------------------------
	void tSteamManager::fShutdown( )
	{
		if( mInitialised )
			SteamAPI_Shutdown( );
	}

	//-------------------------------------------------------------------------------------------------
	void tSteamManager::fUpdate( )
	{
		if( mInitialised )
		{
			SteamAPI_RunCallbacks();
		}
	}

	//-------------------------------------------------------------------------------------------------
	b32 tSteamManager::fShowGamepadTextInput( const tStringPtr& title, u32 charMax )
	{
		if (!mInitialised)
			return false;

		mTextInputCompleted = false;
		mTextInputCancelled = false;
		mTextInput = tStringPtr::cNullPtr;
		mCharMax = charMax > 255 ? 255 : charMax;
		return SteamUtils( )->ShowGamepadTextInput( k_EGamepadTextInputModeNormal, k_EGamepadTextInputLineModeSingleLine, title.fCStr( ), mCharMax, "" );
	}

	//-------------------------------------------------------------------------------------------------
	b32 tSteamManager::fIsGamepadTextInputCompleted( )
	{
		return mTextInputCompleted;
	}

	//-------------------------------------------------------------------------------------------------
	b32 tSteamManager::fIsGamepadTextInputCancelled( )
	{
		return mTextInputCancelled;
	}

	//-------------------------------------------------------------------------------------------------
	tStringPtr tSteamManager::fGetEnteredGamepadTextInput( )
	{
		return mTextInput;
	}

	//-------------------------------------------------------------------------------------------------
	void tSteamManager::fOnGamepadTextDismissed( GamepadTextInputDismissed_t *pResult )
	{
		mTextInputCompleted = pResult->m_bSubmitted;
		mTextInputCancelled = !mTextInputCompleted;
		char buffer[ 256 ];
		if( SteamUtils( )->GetEnteredGamepadTextInput( buffer, mCharMax ) )
			mTextInput = tStringPtr( buffer );
	}

	//-------------------------------------------------------------------------------------------------
	void tSteamManager::fOnMicroTxnAuthorizationResponse( MicroTxnAuthorizationResponse_t *pResult )
	{
		for( u32 i = 0; i < mMicroTxnAuthorizationCallbacks.fCount( ); i++ )
		{
			mMicroTxnAuthorizationCallbacks[i]( pResult->m_unAppID, pResult->m_ulOrderID, pResult->m_bAuthorized );
		}
	}

	//-------------------------------------------------------------------------------------------------
	void tSteamManager::fRegisterMicroTxnAuthorizationCallback( const tMicroTxnAuthorizationCallback& microTxnAuthorizationCallback )
	{
		mMicroTxnAuthorizationCallbacks.fPushBack( microTxnAuthorizationCallback );
	}

	//-------------------------------------------------------------------------------------------------
	void tSteamManager::fDeregisterMicroTxnAuthorizationCallback( const tMicroTxnAuthorizationCallback& microTxnAuthorizationCallback )
	{
		mMicroTxnAuthorizationCallbacks.fFindAndErase( microTxnAuthorizationCallback );
	}

	//-------------------------------------------------------------------------------------------------
	void tSteamManager::fOnGameLobbyJoinRequested( GameLobbyJoinRequested_t *pResult )
	{
		CSteamID lobby = pResult->m_steamIDLobby;
		log_line( Log::cFlagSession, __FUNCTION__ " Received invite to lobby " << lobby.ConvertToUint64( ) );
		tGameAppBase::fInstance( ).fReceivedGameInvite( lobby.ConvertToUint64( ) );
		tGameAppBase::fInstance( ).fOnGameInviteAccepted( 0 );
	}
}
#endif
