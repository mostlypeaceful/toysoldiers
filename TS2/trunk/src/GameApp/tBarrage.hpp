#ifndef __tBarrage__
#define __tBarrage__


namespace Sig
{
	class tPlayer;
	class tUnitLogic;

	class tBarrage : public tRefCounter
	{
	public:
		tBarrage( ) 
			: mSkippedInto( false )
			, mForTutorial( false ) 
			, mUsingStatToIncrement( ~0 )
			, mKillStatToIncrement( ~0 )
			, mProgress( 0.f )
		{ }

		virtual ~tBarrage( ) { }

		tStringPtr		mDevName;
		tStringPtr		mName;
		tFilePathPtr	mIconPath;
		f32				mDuration;
		tStringPtr		mAudioEventSelected;
		tStringPtr		mAudioEventBegin;
		tStringPtr		mAudioEventEnd;
		tStringPtr		mAudioEventKill;
		tStringPtr		mBarrageFactionSwitchValue;

		b32				mSkippedInto;
		b32				mForTutorial;
		u32				mUsingStatToIncrement;
		u32				mKillStatToIncrement;
		f32				mProgress;

		void fForTutorial( ) { mForTutorial = true; }
		virtual void fSelected( tPlayer* player );
		virtual void fSetTarget( tPlayer* player, tEntity* target ) { }
		virtual void fSkipInto( tPlayer* player ) { mSkippedInto = true; }
		virtual void fForceUse( tPlayer* player ) { }

		virtual void fBegin( tPlayer* player );
		virtual void fReset( tPlayer* player );
		virtual f32 fProcessST( tPlayer* player, f32 dt ) { return 1.f; /*done*/ }
		virtual b32 fBarrageUsable( ) const { return true; }
		virtual b32 fEnteredBarrageUnit( tUnitLogic* logic, b32 enter, tPlayer* player ) { return false; }
		virtual b32 fUsedBarrageUnit( tUnitLogic* logic, tEntity* ent, tPlayer* player ) { return false; }
		virtual f32 fProgress( ) const { return mProgress; }

		// this function may get called after fReset!
		b32 fKilledWithBarrageUnit( tPlayer* player, tUnitLogic* killer, tUnitLogic* killed );

		enum tAudioState
		{
			cCallIn,
			cRolling, //timer has started
			cEnd
		};
		void fAudioEvent( const tStringPtr& event, tPlayer* player ) const;
		void fAudioState( tAudioState state, tPlayer* player ) const;

		static void fExportScriptInterface( tScriptVm& vm );
	};

	class tBarragePtr : public tScriptOrCodeObjectPtr<tBarrage>
	{
	public:
		tBarragePtr( ) { }
		explicit tBarragePtr( const Sqrat::Object& o ) 
			: tScriptOrCodeObjectPtr<tBarrage>( o ) { }
		explicit tBarragePtr( tBarrage* o ) 
			: tScriptOrCodeObjectPtr<tBarrage>( o ) { }

		void fBegin( tPlayer* player );
		void fReset( tPlayer* player );
		f32 fProcessST( tPlayer* player, f32 dt );
	};

}

#endif//__tBarrage__
