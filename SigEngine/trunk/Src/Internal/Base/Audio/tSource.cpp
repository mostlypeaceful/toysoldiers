#include "BasePch.hpp"
#include "tSource.hpp"
#include "tSystem.hpp"

#ifdef sig_use_wwise
#	include <AK/SoundEngine/Common/AkSoundEngine.h>
#else
	enum { AK_INVALID_GAME_OBJECT = ~0 };
#endif

namespace Sig { namespace Audio
{
	devvar( bool, Debug_Audio_Warnings_EventNotRecognized,			true );
	devvar( bool, Debug_Audio_Warnings_TriggerNotFound,				true );
	devvar( bool, Debug_Audio_Warnings_ResetSwitchNotRecognized,	true );
	devvar( bool, Debug_Audio_Warnings_SwitchNotRecognized,			true );
	devvar( bool, Debug_Audio_Warnings_ParamNotRecognized,			true );

	void tRTPCSmoother::fStep( f32 dt )
	{
		f32 diff = mTargetValue - mCurrentValue;
		diff = fClamp( diff, -mMaxChange * dt, mMaxChange * dt );
		mCurrentValue += diff;
	}



	tSource::tSource( const char *debugName )
		: mObjectID( AK_INVALID_GAME_OBJECT )
		, mLockedToListener( false )
		, mEnabled( true )
	{
		mObjectID = (u32)this;

		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		
		if_wwise( AKRESULT result = AK::SoundEngine::RegisterGameObj( mObjectID, debugName ) );
		if_wwise( sigassert( result == AK_Success ) );

		sigassert( tSystem::fGetDefaultSystem( ) );
		fSetListenerMask( tSystem::fGetDefaultSystem( )->fListenerMask( ) );
	}

	tSource::tSource( b32 globalValues )
		: mObjectID( AK_INVALID_GAME_OBJECT )
		, mLockedToListener( false )
		, mEnabled( true )
	{
	}

	tSource::~tSource( )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )

		if_wwise( if( mObjectID != AK_INVALID_GAME_OBJECT ) AK::SoundEngine::UnregisterGameObj( mObjectID ) );
		mObjectID = AK_INVALID_GAME_OBJECT ;
	}

	void tSource::fOnDelete( )
	{
		tEntity::fOnDelete( );

		mEventLinkedChildren.fSetCount( 0 );
		mSwitchAndParamsLinkedChildren.fSetCount( 0 );
	}

	void tSource::fOnSpawn( )
	{
		tEntity::fOnSpawn( );

		fOnPause( false );
		fUpdatePosition( );
	}

	void tSource::fOnPause( b32 paused )
	{
		if( paused )
			fRunListRemove( cRunListThinkST );
		else
			fRunListInsert( cRunListThinkST );
	}

	void tSource::fSetListenerMask( u32 mask )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		if_wwise( AK::SoundEngine::SetActiveListeners( mObjectID, mask ) );
	}

	void tSource::fHandleEvent( u32 e )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if_wwise( if( ( AK::SoundEngine::PostEvent( e, mObjectID ) == AK_INVALID_PLAYING_ID ) && Debug_Audio_Warnings_EventNotRecognized ) log_warning( "Audio event not recognized: Event: " << e ) );
		}

		for( u32 i = 0; i < mEventLinkedChildren.fCount( ); ++i )
			mEventLinkedChildren[ i ]->fHandleEvent( e );
	}

	void tSource::fHandleEvent( const tStringPtr& e )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if_wwise( if( ( AK::SoundEngine::PostEvent( e.fCStr( ), mObjectID ) == AK_INVALID_PLAYING_ID ) && Debug_Audio_Warnings_EventNotRecognized ) log_warning( "Audio event not recognized: Event: " << e ) );
		}

		for( u32 i = 0; i < mEventLinkedChildren.fCount( ); ++i )
			mEventLinkedChildren[ i ]->fHandleEvent( e );
	}

	void tSource::fHandleEvent( const char* e )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )		
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if_wwise( if( ( AK::SoundEngine::PostEvent( e, mObjectID ) == AK_INVALID_PLAYING_ID ) && Debug_Audio_Warnings_EventNotRecognized ) log_warning( "Audio event not recognized: Event: " << e ) );
		}

		for( u32 i = 0; i < mEventLinkedChildren.fCount( ); ++i )
			mEventLinkedChildren[ i ]->fHandleEvent( e );
	}


	void tSource::fSetSwitch( u32 group, u32 value )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if_wwise( if( ( AK::SoundEngine::SetSwitch( group, value, mObjectID ) != AK_Success ) && Debug_Audio_Warnings_SwitchNotRecognized ) log_warning( "Audio switch not recognized: Group: " << group << " Value: " << value ) );
			log_output( Log::cFlagAudio, "0x" << this << " " << group << " set to " << value << std::endl );
		}

		for( u32 i = 0; i < mSwitchAndParamsLinkedChildren.fCount( ); ++i )
			mSwitchAndParamsLinkedChildren[ i ]->fSetSwitch( group, value );
	}

	void tSource::fSetSwitch( const char* group, const char* value )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if( !value )
			{
				fResetSwitch( group );
				return;
			}

			if_wwise( if( ( AK::SoundEngine::SetSwitch( group, value, mObjectID ) != AK_Success ) && Debug_Audio_Warnings_SwitchNotRecognized ) log_warning( "Audio switch not recognized. Group[" << group << "] Value[" << value << "]" ) );
			log_output( Log::cFlagAudio, "0x" << this << " " << group << " set to " << value << std::endl );
		}

		for( u32 i = 0; i < mSwitchAndParamsLinkedChildren.fCount( ); ++i )
			mSwitchAndParamsLinkedChildren[ i ]->fSetSwitch( group, value );
	}

	void tSource::fSetSwitch( const tStringPtr& group, const tStringPtr& value )
	{
		fSetSwitch( group.fCStr( ), value.fCStr( ) );
	}

	void tSource::fResetSwitch( u32 group )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if_wwise( if( ( AK::SoundEngine::SetSwitch( (AkSwitchStateID)group, (AkSwitchStateID)AK_DEFAULT_SWITCH_STATE, mObjectID ) != AK_Success ) && Debug_Audio_Warnings_ResetSwitchNotRecognized ) log_warning( "Audio reset switch not recognized! Group: " << group ) );
			log_output( Log::cFlagAudio, "0x" << this << " " << group << " reset" << std::endl );
		}

		for( u32 i = 0; i < mSwitchAndParamsLinkedChildren.fCount( ); ++i )
			mSwitchAndParamsLinkedChildren[ i ]->fResetSwitch( group );
	}

	void tSource::fResetSwitch( const char* group )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if_wwise( if( ( AK::SoundEngine::SetSwitch( (AkSwitchStateID)AK::SoundEngine::GetIDFromString( group ), (AkSwitchStateID)AK_DEFAULT_SWITCH_STATE, mObjectID ) != AK_Success ) && Debug_Audio_Warnings_ResetSwitchNotRecognized ) log_warning( "Audio reset switch not recognized! Group [" << group << "]" ) );
			log_output( Log::cFlagAudio, "0x" << this << " " << group << " reset" << std::endl );
		}

		for( u32 i = 0; i < mSwitchAndParamsLinkedChildren.fCount( ); ++i )
			mSwitchAndParamsLinkedChildren[ i ]->fResetSwitch( group );
	}

	void tSource::fResetSwitch( const tStringPtr& group )
	{
		fResetSwitch( group.fCStr( ) );
	}

	void tSource::fSetGameParam( u32 param, f32 value )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if_wwise( if( ( AK::SoundEngine::SetRTPCValue( param, value, mObjectID ) != AK_Success ) && Debug_Audio_Warnings_ParamNotRecognized ) log_warning( "Audio param not recognized: Param: " << param ) );
		}

		for( u32 i = 0; i < mSwitchAndParamsLinkedChildren.fCount( ); ++i )
			mSwitchAndParamsLinkedChildren[ i ]->fSetGameParam( param, value );
	}

	void tSource::fSetGameParam( const char* param, f32 value )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		profile( cProfilePerfAudioEvent );

		if( mEnabled )
		{
			if_wwise( if( ( AK::SoundEngine::SetRTPCValue( param, value, mObjectID ) != AK_Success ) && Debug_Audio_Warnings_ParamNotRecognized ) log_warning( "Audio param not recognized: Param: " << param ) );
		}

		for( u32 i = 0; i < mSwitchAndParamsLinkedChildren.fCount( ); ++i )
			mSwitchAndParamsLinkedChildren[ i ]->fSetGameParam( param, value );
	}

	void tSource::fSetGameParam( const tStringPtr& param, f32 value )
	{
		fSetGameParam( param.fCStr( ), value );
	}

    void tSource::fPostTrigger( const u32 t )
    {
        if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
        profile( cProfilePerfAudioEvent );

        // Always returns AK_SUCCESS
        if_wwise( if( mEnabled ) AK::SoundEngine::PostTrigger( (AkTriggerID)t, mObjectID ) );
    }

    void tSource::fPostTrigger( const tStringPtr& t )
    {
        if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
        profile( cProfilePerfAudioEvent );

        if( mEnabled )
        {
            if_wwise( if( ( AK::SoundEngine::PostTrigger( t.fCStr( ), mObjectID ) != AK_Success ) && Debug_Audio_Warnings_TriggerNotFound ) log_warning( "Audio post trigger failed. Trigger not found! Trigger: " << t ) );
            log_output( Log::cFlagAudio, "0x" << this << " " << t << " triggered" << std::endl );
        }
    }

    void tSource::fPostTrigger( const char* t )
    {
        if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
        profile( cProfilePerfAudioEvent );

        if( mEnabled )
        {
            if_wwise( if( ( AK::SoundEngine::PostTrigger( t, mObjectID ) != AK_Success ) && Debug_Audio_Warnings_TriggerNotFound ) log_warning( "Audio post trigger failed. Trigger not found! Trigger: " << t ) );
            log_output( Log::cFlagAudio, "0x" << this << " " << t << " triggered" << std::endl );
        }
    }

	void tSource::fThinkST( f32 dt )
	{
		fUpdatePosition( );
		fStepSmoothers( dt );
	}

	void tSource::fStepSmoothers( f32 dt )
	{
		for( u32 i = 0; i < mSmoothers.fCount( ); ++i )
		{
			tRTPCSmoother& smoother = mSmoothers[ i ];

			smoother.fStep( dt );

			if( smoother.mRTPCNameID != ~0 )
				fSetGameParam( smoother.mRTPCNameID, smoother.mCurrentValue );
			else
				fSetGameParam( smoother.mRTPCName, smoother.mCurrentValue );
		}
	}

	void tSource::fUpdatePosition( )
	{
		if( !mLockedToListener )
		{
			if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )

			if_wwise( AKRESULT result = AK::SoundEngine::SetPosition( mObjectID, fSigToAKPosition( mObjectToWorld.fGetTranslation( ), mObjectToWorld.fZAxis( ) ) ) );
			if_wwise( sigassert( result == AK_Success ) );
		}
	}

	void tSource::fLockToListener( u32 index )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )
		if( index != ~0 )
		{
			mLockedToListener = true;
			if_wwise( AKRESULT result = AK::SoundEngine::SetPosition( mObjectID, AkSoundPosition(), index ) );
			if_wwise( sigassert( result == AK_Success ) );
		}
		else
		{
			mLockedToListener = false;
			fUpdatePosition( );
		}
	}

	void tSource::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tSource, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			.Overload<void (tSource::*)(const char*)>(_SC("AudioEvent"),			&tSource::fHandleEvent)
			.Overload<void (tSource::*)(u32, u32)>(_SC("SetSwitch"),				&tSource::fSetSwitch)
			.Overload<void (tSource::*)(const char*, f32)>(_SC("SetAudioParam"),	&tSource::fSetGameParam)
			;

		vm.fRootTable( ).Bind( _SC("AudioSource"), classDesc );
	}
}}


