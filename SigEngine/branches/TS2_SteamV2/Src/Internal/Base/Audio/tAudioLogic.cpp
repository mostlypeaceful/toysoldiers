#include "BasePch.hpp"
#include "tAudioLogic.hpp"
#include "tGameAppBase.hpp"

namespace Sig
{

	tAudioLogicEvent::tAudioLogicEvent( )
		: mRTPCValue( 0 )
		, mRTPCChangeTotalTime( 0 )
		, mRTPCCurrentValue( 0 )
		, mRTPCCurrentTime( 0 )
		, mSource( cLocalSource )
		, mSnapValues( false )
	{	 			 	
	}

	void tAudioLogicEvent::fFire( const Audio::tSourcePtr& source )
	{
		const Audio::tSourcePtr& realSource = (mSource == cLocalSource) ? source : (mSource == cMasterSource) ? tGameAppBase::fInstance( ).fSoundSystem( )->fMasterSource( ) : tGameAppBase::fInstance( ).fSoundSystem( )->fGlobalValues( );
		sigassert( realSource );

		// These are called in this order so that all parameters have been set before the event fires.

		//todo set rtpc over time
		if( mRTPCToSet.fExists( ) ) realSource->fSetGameParam( mRTPCToSet, mRTPCValue );
		if( mStateToSet.fExists( ) ) tGameAppBase::fInstance( ).fSoundSystem( )->fSetState( mStateToSet, mStateValue );
		
		if( mSwitchToSet.fExists( ) ) realSource->fSetSwitch( mSwitchToSet, mSwitchValue );
		if( mWiseEventString.fExists( ) ) realSource->fHandleEvent( mWiseEventString );

		for( u32 i = 0; i < mAdditionalEvents.fCount( ); ++i )
			mAdditionalEvents[ i ]->fFire( source );
	}

	b32 tAudioLogicEvent::fUpdate( const Audio::tSourcePtr& source, f32 dt )
	{
		//Audio::tSourcePtr realSource = mUseMasterSource ? tGameAppBase::fInstance( ).fSoundSystem( )->fMasterSource( ) : source;
		//sigassert( realSource );

		//todo set rtpc over time

		for( u32 i = 0; i < mAdditionalEvents.fCount( ); ++i )
			mAdditionalEvents[ i ]->fUpdate( source, dt );

		// need to utilize this return type to indicate that "i'm done changing" to reduce processing cost:
		return true;
	}

	void tAudioLogicEvent::fSetRTPC( const tStringPtr& rtpc, f32 value, f32 time )
	{
		mRTPCToSet = rtpc;
		mRTPCValue = value;
		mRTPCChangeTotalTime = time;

		if( mRTPCChangeTotalTime <= 0.f ) mSnapValues = true;
	}

	void tAudioLogicEvent::fSetState( const tStringPtr& state, const tStringPtr& value )
	{
		mStateToSet = state;
		mStateValue = value;
	}

	void tAudioLogicEvent::fSetMasterRTPC( const tStringPtr& rtpc, f32 value, f32 time )
	{
		fSetRTPC( rtpc, value, time );
		mSource = cMasterSource;
	}

	void tAudioLogicEvent::fSetGlobalRTPC( const tStringPtr& rtpc, f32 value, f32 time )
	{
		fSetRTPC( rtpc, value, time );
		mSource = cGlobalBus;
	}

	tAudioLogicEvent* tAudioLogicEvent::fAddEvent( ) 
	{ 
		mAdditionalEvents.fPushBack( tAudioLogicEventPtr( NEW tAudioLogicEvent( ) ) ); 
		return mAdditionalEvents.fBack( ).fGetRawPtr( ); 
	}

	tAudioLogic::tAudioLogic( )
		: mInVolume( false )
		, mWasInVolume( false )
		, mAlwaysInVolume( false )
		, mCurrentState( tSpatialEntity::cMaxStateMaskValue )
	{
	}
	tAudioLogic::~tAudioLogic( )
	{
	}
	void tAudioLogic::fOnSpawn( )
	{
		tLogic::fOnSpawn( );

		mSource.fReset( NEW Audio::tSource( "AudioLogic" ) );
		mSource->fSpawn( *fOwnerEntity( ) );

		mShape = fOwnerEntity( )->fDynamicCast<tShapeEntity>( );

		if( mEvents[ cEventOnSpawn ] )
			mEvents[ cEventOnSpawn ]->fFire( mSource );

		fOnPause( false );

		fStateMaskEnable( 0, tSpatialEntity::cMaxStateMaskValue );
	}
	void tAudioLogic::fOnDelete( )
	{
		if( mInVolume )
			fExitVolume( );

		if( mEvents[ cEventOnDelete ] ) 
			mEvents[ cEventOnDelete ]->fFire( mSource );

		mSource.fRelease( );
		mEvents.fFill( tAudioLogicEventPtr( ) );
		mStates.fSetCount( 0 );

		tLogic::fOnDelete( );
	}
	void tAudioLogic::fOnPause( b32 paused )
	{
		if( mShape )
		{
			if( paused )
			{
				fRunListRemove( cRunListActST );
			}
			else
			{
				fRunListInsert( cRunListActST );
			}
		}
	}
	void tAudioLogic::fEnterVolume( )
	{
		if( mEvents[ cEventOnVolumeEntered ] ) 
			mEvents[ cEventOnVolumeEntered ]->fFire( mSource );

		const tGrowableArray<Audio::tListenerPtr>& listeners = tGameAppBase::fInstance( ).fSoundSystem( )->fListeners( );
		for( u32 i = 0; i < listeners.fCount( ); ++i )
			listeners[ i ]->fPushVolume( *this );
	}
	void tAudioLogic::fExitVolume( )
	{
		if( mEvents[ cEventOnVolumeExited ] ) 
			mEvents[ cEventOnVolumeExited ]->fFire( mSource );

		const tGrowableArray<Audio::tListenerPtr>& listeners = tGameAppBase::fInstance( ).fSoundSystem( )->fListeners( );
		for( u32 i = 0; i < listeners.fCount( ); ++i )
			listeners[ i ]->fPopVolume( *this );
	}
	void tAudioLogic::fBackOnTop( )
	{
		if( mEvents[ cEventOnVolumeEntered ] ) 
			mEvents[ cEventOnVolumeEntered ]->fFire( mSource );
		mInVolume = true;
		mWasInVolume = true;
	}
	void tAudioLogic::fStateMaskEnable( u32 index, u32 mask )
	{
		if( mCurrentState != tStateableEntity::cMaxStateMaskValue )
		{
			tStateEvents* prevState = fStateEvent( mCurrentState );
			if( prevState && prevState->mOnExit )
				prevState->mOnExit->fFire( mSource );
		}

		mCurrentState = index;

		tStateEvents* nextState = fStateEvent( mCurrentState );
		if( nextState && nextState->mOnEnter )
			nextState->mOnEnter->fFire( mSource );
	}
	tAudioLogic::tStateEvents* tAudioLogic::fStateEventsForScript( u32 state )
	{
		if( state >= mStates.fCount( ) )
			mStates.fSetCount( state + 1 );
		
		if( !mStates[ state ] )
			mStates[ state ].fReset( NEW tStateEvents( ) );

		return mStates[ state ].fGetRawPtr( );
	}
	void tAudioLogic::fActST( f32 dt )
	{
		sigassert( mShape );

		mInVolume = mAlwaysInVolume;

		const tGrowableArray<Audio::tListenerPtr>& listeners = tGameAppBase::fInstance( ).fSoundSystem( )->fListeners( );
		for( u32 i = 0; !mInVolume && i < listeners.fCount( ); ++i )
		{
			Math::tVec3f pos = listeners[ i ]->fLastXForm( ).fGetTranslation( );
			if( mShape->fContains( pos ) ) 
				mInVolume = true;
		}

		if( mInVolume && !mWasInVolume ) fEnterVolume( );
		else if( !mInVolume && mWasInVolume ) fExitVolume( );
		mWasInVolume = mInVolume;

		for( u32 i = 0; i < cEventTypeCount; ++i ) if( mEvents[ i ] ) mEvents[ i ]->fUpdate( mSource, dt );
	}
}


namespace Sig
{

	void tAudioLogic::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tAudioLogicEvent, Sqrat::DefaultAllocator<tAudioLogicEvent> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("Event"),			&tAudioLogicEvent::mWiseEventString)
				.Var(_SC("SwitchToSet"),	&tAudioLogicEvent::mSwitchToSet)
				.Var(_SC("SwitchValue"),	&tAudioLogicEvent::mSwitchValue)
				.Var(_SC("StateToSet"),		&tAudioLogicEvent::mStateToSet)
				.Var(_SC("StateValue"),		&tAudioLogicEvent::mStateValue)
				.Var(_SC("RTPCToSet"),		&tAudioLogicEvent::mRTPCToSet)
				.Var(_SC("RTPCValue"),		&tAudioLogicEvent::mRTPCValue)
				.Var(_SC("RTPCChangeTotalTime"), &tAudioLogicEvent::mRTPCChangeTotalTime)
				.Var(_SC("Source"),			&tAudioLogicEvent::mSource)
				.Var(_SC("SnapValues"),		&tAudioLogicEvent::mSnapValues)
				.Func(_SC("AddEvent"),		&tAudioLogicEvent::fAddEvent)
				.Func(_SC("SetRTPC"),		&tAudioLogicEvent::fSetRTPC)
				.Func(_SC("SetState"),		&tAudioLogicEvent::fSetState)
				.Func(_SC("SetMasterRTPC"), &tAudioLogicEvent::fSetMasterRTPC)
				.Func(_SC("SetGlobalRTPC"), &tAudioLogicEvent::fSetGlobalRTPC)
				;
			vm.fRootTable( ).Bind(_SC("AudioLogicEvent"), classDesc);
			vm.fConstTable( ).Const(_SC("AUDIO_SOURCE_LOCAL"), tAudioLogicEvent::cLocalSource);
			vm.fConstTable( ).Const(_SC("AUDIO_SOURCE_MASTER"), tAudioLogicEvent::cMasterSource);
			vm.fConstTable( ).Const(_SC("AUDIO_SOURCE_GLOBAL"), tAudioLogicEvent::cGlobalBus);
		}
		{
			Sqrat::Class<tAudioLogic::tStateEvents, Sqrat::NoCopy<tAudioLogic::tStateEvents> > classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("OnEnter"),	&tAudioLogic::tStateEvents::fOnEnteredEventForScript)
				.Prop(_SC("OnExit"),	&tAudioLogic::tStateEvents::fOnExitedEventForScript)
				;
			vm.fRootTable( ).Bind(_SC("StateEvents"), classDesc);
		}
		{
			Sqrat::DerivedClass<tAudioLogic, tLogic, Sqrat::NoCopy<tAudioLogic> > classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("AlwaysInVolume"),		&tAudioLogic::fAlwaysInVolume, &tAudioLogic::fSetAlwaysInVolume)
				.Prop(_SC("OnSpawnEvent"),			&tAudioLogic::fOnSpawnEventForScript)
				.Prop(_SC("OnDeleteEvent"),			&tAudioLogic::fOnDeleteEventForScript)
				.Prop(_SC("OnVolumeEnteredEvent"),	&tAudioLogic::fOnVolumeEnteredEventForScript)
				.Prop(_SC("OnVolumeExitedEvent"),	&tAudioLogic::fOnVolumeExitedEventForScript)
				.Func(_SC("StateEvents"),			&tAudioLogic::fStateEventsForScript)
				;
			vm.fRootTable( ).Bind(_SC("AudioLogic"), classDesc);
		}
	}
}

