#ifndef __tAudioLogic__
#define __tAudioLogic__
#include "tLogic.hpp"
#include "Audio/tSource.hpp"
#include "Audio/tListener.hpp"
#include "tShapeEntity.hpp"

namespace Sig
{

	class tAudioLogicEvent;
	typedef tRefCounterPtr<tAudioLogicEvent> tAudioLogicEventPtr;

	class tAudioLogicEvent : public tRefCounter
	{
		debug_watch( tAudioLogicEvent );
	public:
		enum tSource { cLocalSource, cMasterSource, cGlobalBus };

		// if not null, will be fired on the audio object
		tStringPtr mWiseEventString;

		tStringPtr mSwitchToSet;
		tStringPtr mSwitchValue;

		tStringPtr mStateToSet;
		tStringPtr mStateValue;

		tStringPtr	mRTPCToSet;
		f32			mRTPCValue;
		f32			mRTPCChangeTotalTime;

		u32			mSource;

		b8			mSnapValues;		// rpc's are not set over time
		b8			pad0;
		b8			pad1;
		b8			pad2;


		//runtime values
		f32 mRTPCCurrentValue;
		f32 mRTPCCurrentTime;

		tGrowableArray<tAudioLogicEventPtr> mAdditionalEvents;

		tAudioLogicEvent( );

		void	fFire( const Audio::tSourcePtr& source );		
		b32		fUpdate( const Audio::tSourcePtr& source, f32 dt ); //return true if finished, used to step RTPC values
		tAudioLogicEvent* fAddEvent( );

		// this make the scripting easier on the eyes
		void fSetRTPC( const tStringPtr& rtpc, f32 value, f32 time );
		void fSetState( const tStringPtr& state, const tStringPtr& value );
		void fSetMasterRTPC( const tStringPtr& rtpc, f32 value, f32 time );
		void fSetGlobalRTPC( const tStringPtr& rtpc, f32 value, f32 time );
	};


	class tAudioLogic : public tLogic
	{
		debug_watch( tAudioLogic );
		define_dynamic_cast( tAudioLogic, tLogic );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tAudioLogic( );
		virtual ~tAudioLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );

		// This volume is now the top most volume in a listeners volume stack.
		void fBackOnTop( );

		const Audio::tSourcePtr& fSource( ) const { return mSource; }

	protected:
		virtual void fActST( f32 dt );

		void fEnterVolume( );
		void fExitVolume( );

		enum tEventTypes
		{
			cEventOnSpawn,
			cEventOnDelete,
			cEventOnVolumeEntered,
			cEventOnVolumeExited,
			cEventTypeCount
		};

		Audio::tSourcePtr	mSource;
		tFixedArray<tAudioLogicEventPtr, cEventTypeCount> mEvents;

		struct tStateEvents : public tRefCounter
		{
			tAudioLogicEventPtr mOnEnter;
			tAudioLogicEventPtr mOnExit;

			tAudioLogicEvent* fOnEnteredEventForScript( ) { if( !mOnEnter ) mOnEnter.fReset( NEW tAudioLogicEvent( ) ); return mOnEnter.fGetRawPtr( ); }
			tAudioLogicEvent* fOnExitedEventForScript( ) { if( !mOnExit ) mOnExit.fReset( NEW tAudioLogicEvent( ) ); return mOnExit.fGetRawPtr( ); }
		};
		typedef tRefCounterPtr< tStateEvents > tStateEventsPtr;
		tGrowableArray<tStateEventsPtr> mStates;

		tShapeEntity* mShape;

		b8 mInVolume;
		b8 mWasInVolume;
		b8 mAlwaysInVolume;
		b8 pad0;

		u16 mCurrentState;
		u16 pad1;

		tAudioLogicEvent* fOnSpawnEventForScript( ) { if( !mEvents[ cEventOnSpawn ] ) mEvents[ cEventOnSpawn ].fReset( NEW tAudioLogicEvent( ) ); return mEvents[ cEventOnSpawn ].fGetRawPtr( ); }
		tAudioLogicEvent* fOnDeleteEventForScript( ) { if( !mEvents[ cEventOnDelete ] ) mEvents[ cEventOnDelete ].fReset( NEW tAudioLogicEvent( ) ); return mEvents[ cEventOnDelete ].fGetRawPtr( ); }
		tAudioLogicEvent* fOnVolumeEnteredEventForScript( ) { if( !mEvents[ cEventOnVolumeEntered ] ) mEvents[ cEventOnVolumeEntered ].fReset( NEW tAudioLogicEvent( ) ); return mEvents[ cEventOnVolumeEntered ].fGetRawPtr( ); }
		tAudioLogicEvent* fOnVolumeExitedEventForScript( ) { if( !mEvents[ cEventOnVolumeExited ] ) mEvents[ cEventOnVolumeExited ].fReset( NEW tAudioLogicEvent( ) ); return mEvents[ cEventOnVolumeExited ].fGetRawPtr( ); }
		tStateEvents* fStateEventsForScript( u32 state );
		tStateEvents* fStateEvent( u32 state ) { if( mStates.fCount( ) > state ) return mStates[ state ].fGetRawPtr( ); else return NULL; }
		
		void fSetAlwaysInVolume( b32 value ) { mAlwaysInVolume = value; }
		b32 fAlwaysInVolume( ) const { return mAlwaysInVolume; }

		virtual void fStateMaskEnable( u32 index, u32 mask );

	};

	typedef tRefCounterPtr<tAudioLogic> tAudioLogicPtr;

}

#endif//__tAudioLogic__
