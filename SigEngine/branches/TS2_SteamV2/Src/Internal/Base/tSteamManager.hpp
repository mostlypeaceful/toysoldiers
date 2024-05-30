#ifndef __tSteamManager__
#define __tSteamManager__

#if defined(use_steam)

#include "tAchievements.hpp"
#include "tGameSessionSearchResult.hpp"
#include "tHashTable.hpp"

namespace Sig
{
	class tSteamManager
	{
		declare_singleton_define_own_ctor_dtor( tSteamManager );

	public:
		typedef tDelegate< void ( u32 appId, s64 orderId, b32 authorized ) > tMicroTxnAuthorizationCallback;

		// returns true if running in Steam's Big Picture Mode
		static b32 fIsBigPictureMode( );

		// enable steam systems - connects with the steam client and checks the APP ID
		void fStartup( );

		// showdown steam systems - application should call this before closing
		void fShutdown( );

		// this needs to be called every frame. it pumps the steam callback queue
		void fUpdate( );

		// Is steam client link available?
		b32 fIsSteamAvailable( );

		// get 'gametag' of this player
		const char *fGetGamerTag( );

		// get 'language' set in steam
		const char *fGetLanguage( );

		// get name of a user
		const char *fGetUserName( CSteamID userID );

		// get user id
		// return FALSE if steam is not ready to give an ID yet
		b32 fGetPlatformUserId( tPlatformUserId &userId );

		u32 fGetAppId( ) { return mAppID; }

		// these static conversion routines help convert tPlatformUserIDs to/from CSteamIDs safely
		static CSteamID fCastPlatformUserIdToSteamID( tPlatformUserId userId )			{ return *( CSteamID* )&userId; }
		static tPlatformUserId fCastSteamIDToPlatformUserId( CSteamID steamId )			{ return *( tPlatformUserId* )&steamId; }

		// Show the on-screen keyboard for Steam Big Picture mode
		b32 fShowGamepadTextInput( const tStringPtr& title, u32 charMax );
		b32 fIsGamepadTextInputCompleted( );
		b32 fIsGamepadTextInputCancelled( );
		tStringPtr fGetEnteredGamepadTextInput( );

		void fRegisterMicroTxnAuthorizationCallback( const tMicroTxnAuthorizationCallback& microTxnAuthorizationCallback );
		void fDeregisterMicroTxnAuthorizationCallback( const tMicroTxnAuthorizationCallback& microTxnAuthorizationCallback );

	private:
		tSteamManager( );
		~tSteamManager( );

		b32 mInitialised;
		b32 mInitialiseAttempted;
		u32 mAppID;

		// ==== Gamepad Text Input ====
		u32 mCharMax;
		b32 mTextInputCompleted;
		b32 mTextInputCancelled;
		tStringPtr mTextInput;

		// Called when the on-screen keyboard (Big Picture mode) is dismissed
		STEAM_CALLBACK( tSteamManager, fOnGamepadTextDismissed, GamepadTextInputDismissed_t, mCallbackGamepadTextDismissed );

		// A microtransaction authorization response
		STEAM_CALLBACK( tSteamManager, fOnMicroTxnAuthorizationResponse, MicroTxnAuthorizationResponse_t, mCallbackMicroTxnAuthorizationResponse );

		// Called when another Steam user invites us to a multiplayer game
		STEAM_CALLBACK( tSteamManager, fOnGameLobbyJoinRequested, GameLobbyJoinRequested_t, mCallbackGameLobbyJoinRequested );

		tGrowableArray<tMicroTxnAuthorizationCallback> mMicroTxnAuthorizationCallbacks;
	};
}

#endif
#endif
