#ifndef __tAnimTrack__
#define __tAnimTrack__

namespace Sig
{
	class tAnimatedSkeleton;

	struct base_export tAnimEvaluationResult
	{
		tDynamicArray< Math::tPRSXformf > mBoneResults;
	};

	class base_export tAnimTrack : public tRefCounter
	{
		define_dynamic_cast_base( tAnimTrack );
	public:
		enum tFlags
		{
			cFlagClampTime		= ( 1 << 0 ), // if not set, then time is wrapped (default behavior)
			cFlagPartial		= ( 1 << 1 ),
			cFlagAdditive		= ( 1 << 2 ),
			cFlagPost			= ( 1 << 3 ), // track is added to mPostAnim list and has it's fPostAnimEvaluate function called.
			cFlagResumeTime		= ( 1 << 4 ), // set to indicate this track should resume the same time as other tracks of its kind
			cFlagIgnoreEndEvent = ( 1 << 5 ),
		};

		static b32 gEnableEndOfTrackEvent; //set this true to fire Logic::AnimationEvent::cEventReachedEndOneShot

	private:
		tStringPtr mTag;
		u32 mFlags;
		f32 mCurrentTime;
		f32 mPrevTime;
		f32 mTimeScale;
		f32 mBlendScale;
		f32 mMinTime;
		f32 mMaxTime;
		f32 mBlendIn, mBlendInDelta;
		f32 mBlendOut, mBlendOutDelta;
		f32 mBlendStrength;
	public:
		tAnimTrack( f32 blendIn, f32 blendOut, f32 timeScale, f32 blendScale, f32 minTime, f32 maxTime, f32 startTime, u32 flags );
		virtual ~tAnimTrack( ) { }
		void fStep( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt );
		void fEvaluate( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );
		virtual void fPostAnimEvaluate( tAnimatedSkeleton& animSkel ) { }
		void fBeginBlendingOut( f32 blendOutOverNSeconds );
		void fSetTag( const tStringPtr& tag ) { mTag = tag; }
		void fSetTimeScale( f32 newTimeScale ) { mTimeScale = newTimeScale; }
		void fSetBlendScale( f32 newBlendScale ) { mBlendScale = newBlendScale; }
		void fSetAdditive( b32 isAdditive );
		void fSetCurrentTime( f32 time ) { mCurrentTime = time; }
		void fIgnoreEndEvent( b32 ignore ) { if( ignore ) mFlags = fSetBits( mFlags, cFlagIgnoreEndEvent ); else mFlags = fClearBits( mFlags, cFlagIgnoreEndEvent ); }
		static void fSetEnableEndOfTrackEvent( b32 enabled );

	public: // events

		virtual void fOnPushed( tAnimatedSkeleton& skeleton ) { }

	public: // accessors
		inline const tStringPtr& fTag( ) const { return mTag; }
		inline f32 fCurrentTime( ) const { return mCurrentTime; }
		inline f32 fNormalizedTime( ) const { return mCurrentTime / ( mMaxTime - mMinTime ); }
		inline f32 fPrevTime( ) const { return mPrevTime; }
		inline f32 fMinTime( ) const { return mMinTime; }
		inline f32 fMaxTime( ) const { return mMaxTime; }
		inline f32 fTimeScale( ) const { return mTimeScale; }
		inline f32 fBlendStrength( ) const { return mBlendStrength * mBlendScale; }
		inline f32 fBlendScale( ) const { return mBlendScale; }
		inline b32 fPartial( ) const { return mFlags & cFlagPartial; }
		inline f32 fLooping( ) const { return mBlendOutDelta==0.f; }
		inline u32 fFlags( ) const { return mFlags; }
		inline f32 fBlendIn( ) const { return mBlendIn; }
		inline f32 fBlendOut( ) const { return mBlendOut; }
	public: // debuggish
		if_devmenu( virtual void fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const );
		if_devmenu( virtual void fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const );
		if_devmenu( void fDebugIndent( std::stringstream& ss, u32 indentDepth ) const );
	protected:
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign ) { fZeroOut( refFrameDelta ); }
		virtual void fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend ) { }
		virtual void fEvaluateAdditive( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend ) { }
	private:
		void fClampTime( );
		f32  fApplyTime( f32 dt, tAnimatedSkeleton& animSkel );
	};


	define_smart_ptr( base_export, tRefCounterPtr, tAnimTrack );
}


#endif//__tAnimTrack__

