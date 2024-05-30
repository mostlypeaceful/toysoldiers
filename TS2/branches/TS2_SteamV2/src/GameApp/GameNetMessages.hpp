#ifndef __GameNetMessages__
#define __GameNetMessages__
#include "Input/tGamepad.hpp"
#include "tLevelLoadInfo.hpp"

namespace Sig { namespace Net
{
	enum tGameMessageId
	{
		cMessageStartMap = 0,
		cMessageLoadFromSave,
		cMessageReadyForInput,
		cMessagePlayerInfo,
		cMessageInputFrame,
		cMessageSyncCheck,
		cMessageDesyncDetected,
		cMessageClientState,
		cMessageLevelSelectStatus,
		cMessageCount
	};

	///
	/// \class tBaseMessage
	/// \brief 
	struct tBaseMessage
	{
		u32 mMessageType;

		explicit tBaseMessage( u32 id = cMessageCount )
			: mMessageType( id ) { }

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mMessageType );
		}
	};

	///
	/// \class tStartMap
	/// \brief 
	struct tStartMap : public tBaseMessage
	{
		enum { cNormal, cRestart, cNext };
		u32 mObjectiveSeed;
		tLevelLoadInfo mLoadLevelInfo;
		u32 mType;

		tStartMap( )
			: tBaseMessage( cMessageStartMap )
			, mObjectiveSeed( 0 )
			, mType( cNormal )
		{
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			tBaseMessage::fSaveLoad( archive );
			archive.fSaveLoad( mLoadLevelInfo );
			archive.fSaveLoad( mObjectiveSeed );
			archive.fSaveLoad( mType );
		}
	};

	///
	/// \class tLoadFromSave
	/// \brief 
	struct tLoadFromSave : public tBaseMessage
	{
		tDynamicBuffer mSaveGame;
		u32 mObjectiveSeed;

		tLoadFromSave( ) 
			: tBaseMessage( cMessageLoadFromSave ) 
		{

		}

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			tBaseMessage::fSaveLoad( archive );
			archive.fSaveLoad( mSaveGame );
			archive.fSaveLoad( mObjectiveSeed );
		}
	};

	///
	/// \class tReadyForInput
	/// \brief 
	struct tReadyForInput : public tBaseMessage
	{
		tPlatformUserId mUserId;

		tReadyForInput( ) 
		: tBaseMessage( cMessageReadyForInput )
		, mUserId( tUser::cInvalidUserId ){ }
		
		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			tBaseMessage::fSaveLoad( archive );
			archive.fSaveLoad( mUserId );
		}

	};

	struct tPlayerInfo : public tBaseMessage
	{
		tPlatformUserId mUserId;
		tDynamicArray<byte> mProfile;
		tDynamicArray<byte> mGameControllerConfig;

		tPlayerInfo( ) 
		: tBaseMessage( cMessagePlayerInfo )
		, mUserId( tUser::cInvalidUserId ){ }
		
		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			tBaseMessage::fSaveLoad( archive );
			archive.fSaveLoad( mUserId );
			archive.fSaveLoad( mProfile );
			archive.fSaveLoad( mGameControllerConfig );
		}
	};

	///
	/// \class tInputFrame
	/// \brief 
	struct tInputFrame : public tBaseMessage
	{
		tPlatformUserId mUserId;
		u32 mFrameId;
		u32 mErrorCode; // possible error code, ~0 for no error
		Input::tGamepad::tStateData mStateDataGamepad;
		Input::tKeyboard::tStateData mStateDataKeyboard;
		Input::tMouse::tStateData mStateDataMouse;

		tInputFrame( )
			: tBaseMessage( cMessageInputFrame )
			, mUserId( tUser::cInvalidUserId )
			, mFrameId( ~0 ) 
			, mErrorCode( ~0 ){ }

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			tBaseMessage::fSaveLoad( archive );
			archive.fSaveLoad( mUserId );
			archive.fSaveLoad( mFrameId );
			archive.fSaveLoad( mStateDataGamepad );
			archive.fSaveLoad( mStateDataKeyboard );
			archive.fSaveLoad( mStateDataMouse );
			archive.fSaveLoad( mErrorCode );
		}
	};

	///
	/// \class tSyncCheck
	/// \brief 
	struct tSyncCheck : public tBaseMessage
	{
		tSyncCheck( ) : tBaseMessage( cMessageSyncCheck ){ }

		u32 mHash;
		u32 mInputFrame;
		tDynamicBuffer mBuffer;
		tDynamicBuffer mCallstackBuffer; // Not serialized

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			tBaseMessage::fSaveLoad( archive );
			archive.fSaveLoad( mHash );
			archive.fSaveLoad( mInputFrame );
			archive.fSaveLoad( mBuffer );
		}
	};

	///
	/// \class tDesyncDetected
	/// \brief 
	struct tDesyncDetected : public tBaseMessage
	{
		tDesyncDetected( ) : tBaseMessage( cMessageDesyncDetected ) { }

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			tBaseMessage::fSaveLoad( archive );
		}
	};

	///
	/// \class tClientStateChange
	/// \brief Sent when the client player's state changes in the lobby
	struct tClientStateChange : public tBaseMessage
	{
		tClientStateChange( ) : tBaseMessage( cMessageClientState ) { }

		enum { cDropped, cReadyRequest, cReady, cUnreadyRequest, cAllowUnready, cMenuStateRequest, cQuit };
		u32 mState;

		void fSet( u32 state )
		{
			mState = state;
		}

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			tBaseMessage::fSaveLoad( archive );
			archive.fSaveLoad( mState );
		}

		static void fExportScriptInterface( tScriptVm& vm )
		{
			Sqrat::Class<tClientStateChange, Sqrat::DefaultAllocator<tClientStateChange> > classDesc( vm.fSq( ) );
			classDesc
				.Func( _SC( "Set" ),	&tClientStateChange::fSet )
				.Var(  _SC( "State" ),	&tClientStateChange::mState )
				;
			vm.fRootTable( ).Bind(_SC("ClientStateChange"), classDesc);

			vm.fConstTable( ).Const( _SC("CLIENTSTATE_DROPPED"), cDropped );
			vm.fConstTable( ).Const( _SC("CLIENTSTATE_READYREQUEST"), cReadyRequest );
			vm.fConstTable( ).Const( _SC("CLIENTSTATE_READY"), cReady );
			vm.fConstTable( ).Const( _SC("CLIENTSTATE_UNREADYREQUEST"), cUnreadyRequest );
			vm.fConstTable( ).Const( _SC("CLIENTSTATE_ALLOWUNREADY"), cAllowUnready );
			vm.fConstTable( ).Const( _SC("CLIENTSTATE_MENUSTATEREQUEST"), cMenuStateRequest );
			vm.fConstTable( ).Const( _SC("CLIENTSTATE_QUIT"), cQuit );
		}
	};

	///
	/// \class tSurvivalLevelSelectStatus
	/// \brief Status of the survival level select screen
	struct tLevelSelectStatus : public tBaseMessage
	{
		tLevelSelectStatus( ) : tBaseMessage( cMessageLevelSelectStatus ) { }

		u32 mSelectedLevel;
		b32 mSelectingMode;
		u32 mSelectedMode;
		u32 mCurrentDlc;
		u32 mAvailableDlcMask;

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			tBaseMessage::fSaveLoad( archive );
			archive.fSaveLoad( mSelectedLevel );
			archive.fSaveLoad( mSelectingMode );
			archive.fSaveLoad( mSelectedMode );
			archive.fSaveLoad( mCurrentDlc );
			archive.fSaveLoad( mAvailableDlcMask );
		}

		void fSet( u32 level, b32 selectingMode, u32 mode, u32 dlc, u32 availableDlc )
		{
			mSelectedLevel = level;
			mSelectingMode = selectingMode;
			mSelectedMode = mode;
			mCurrentDlc = dlc;
			mAvailableDlcMask = availableDlc;
		}

		static void fExportScriptInterface( tScriptVm& vm )
		{
			Sqrat::Class<tLevelSelectStatus, Sqrat::DefaultAllocator<tLevelSelectStatus> > classDesc( vm.fSq( ) );
			classDesc
				.Func( _SC( "Set" ),			&tLevelSelectStatus::fSet )
				.Var( _SC( "SelectedLevel" ),	&tLevelSelectStatus::mSelectedLevel )
				.Var( _SC( "SelectingMode" ),	&tLevelSelectStatus::mSelectingMode )
				.Var( _SC( "SelectedMode" ),	&tLevelSelectStatus::mSelectedMode )
				.Var( _SC( "CurrentDlc" ),		&tLevelSelectStatus::mCurrentDlc )
				.Var( _SC( "AvailableDlc" ),	&tLevelSelectStatus::mAvailableDlcMask )
				;
			vm.fRootTable( ).Bind(_SC("LevelSelectStatus"), classDesc);
		}
	};

}}


#endif//__GameNetMessages__
