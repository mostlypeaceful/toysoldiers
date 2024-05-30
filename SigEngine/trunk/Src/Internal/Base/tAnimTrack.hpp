#ifndef __tAnimTrack__
#define __tAnimTrack__

#include "tAnimTrackInputs.hpp"

namespace Sig { namespace Anim
{
	class tAnimatedSkeleton;

	struct base_export tAnimEvaluationResult
	{
		tDynamicArray< Math::tPRSXformf > mBoneResults;

		// Combines other into this with lerp percentage.
		void fBlendWith( const tAnimEvaluationResult& other, f32 lerp );
	};

	struct base_export tAnimTrackDesc
	{
		f32 mBlendIn;
		f32 mBlendOut;
		f32 mTimeScale;
		f32 mBlendScale;
		f32 mMinTime;
		f32 mMaxTime;
		f32 mStartTime;
		u32 mFlags;
		b32 mApplyRefFrame;
		tStringPtr mTag;

		tAnimTrackDesc( f32 blendIn = 0.2f
			, f32 blendOut = 0.0f
			, f32 timeScale = 1.f
			, f32 blendScale = 1.f
			, f32 minTime = 0.f
			, f32 maxTime = -1.f
			, f32 startTime = 0.f
			, u32 flags = 0
			, b32 applyRefFrame = true )

			: mBlendIn( blendIn )
			, mBlendOut( blendOut )
			, mTimeScale( timeScale )
			, mBlendScale( blendScale )
			, mMinTime( minTime )
			, mMaxTime( maxTime )
			, mStartTime( startTime )
			, mFlags( flags )
			, mApplyRefFrame( applyRefFrame )
		{ }

		tAnimTrackDesc( const tAnimTrackDesc& desc
			, b32 prepareForTrack = false )
		{
			*this = desc;
			if( prepareForTrack )
			{
				mMaxTime = mMaxTime < 0.f ? Math::cInfinity : mMaxTime;
			}
		}

		virtual ~tAnimTrackDesc( ) { }

		// Inline modifiers
		tAnimTrackDesc fSetStartTime( f32 startTime ) const { tAnimTrackDesc desc = *this; desc.mStartTime = startTime; return desc; }
		tAnimTrackDesc fSetTag( const tStringPtr& tag ) const { tAnimTrackDesc desc = *this; desc.mTag = tag; return desc; }

		// The tools will utilize these to preview the track's behavior. They're sort of optional and not used by the game.
		virtual const tTrackInputDesc*  fInputDesc( u32 index ) const { return NULL; }
		virtual u32						fInputDescCount( ) const { return 0; }
		virtual tTrackInput*			fInput( u32 index ) const { return NULL; }

		inline b32 operator==(const tAnimTrackDesc& rhs ) const { return fMemCmp( this, &rhs, sizeof( *this ) ) == 0; }

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			archive.fSaveLoad( mBlendIn );
			archive.fSaveLoad( mBlendOut );
			archive.fSaveLoad( mTimeScale );
			archive.fSaveLoad( mBlendScale );
			archive.fSaveLoad( mMinTime );
			archive.fSaveLoad( mMaxTime );
			archive.fSaveLoad( mStartTime );
			archive.fSaveLoad( mFlags );
			archive.fSaveLoad( mApplyRefFrame );
			archive.fSaveLoad( mTag );
		}

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
	};

	struct base_export tAnimTrackData
	{
		f32 mBlendIn;
		f32 mBlendOut;

		f32 mTimeScale;
		f32 mBlendScale;

		tAnimTrackData( )
			: mBlendIn( 0.f )
			, mBlendOut( 0.f )
			, mTimeScale( 0.f )
			, mBlendScale( 0.f )
		{ }

		tAnimTrackData( const tAnimTrackDesc& desc )
			: mBlendIn( desc.mBlendIn )
			, mBlendOut( desc.mBlendOut )
			, mTimeScale( desc.mTimeScale )
			, mBlendScale( desc.mBlendScale )
		{ }

		inline b32 operator==(const tAnimTrackData& rhs ) const { return fMemCmp( this, &rhs, sizeof( *this ) ) == 0; }

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			archive.fSaveLoad( mBlendIn );
			archive.fSaveLoad( mBlendOut );
			archive.fSaveLoad( mTimeScale );
			archive.fSaveLoad( mBlendScale );
		}
	};

	class base_export tAnimTrack : public tRefCounter
	{
		debug_watch( tAnimTrack );
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

	protected:
		f32 mCurrentTime;
		f32 mPrevTime;

		f32 mBlendInDelta;
		f32 mBlendOutDelta;
		f32 mBlendStrength;

		b32 mEnded; // at least once

		// Most of the core parameters are here.
		const tAnimTrackDesc mDesc;

		// This contains parameters that can be modified after the track is created
		tAnimTrackData mData;

	public:
		tAnimTrack( const tAnimTrackDesc& desc );
		virtual ~tAnimTrack( ) { }

		void fStep( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt );
		void fEvaluate( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend );
		virtual void fPostAnimEvaluate( tAnimatedSkeleton& animSkel ) { }
		void fBeginBlendingOut( f32 blendOutOverNSeconds );

		// This method is only called for post tracks.
		virtual void fStepST( f32 dt, tAnimatedSkeleton& animSkel ) { }


		void fSetTimeScale( f32 newTimeScale ) { mData.mTimeScale = newTimeScale; }
		void fSetBlendScale( f32 newBlendScale ) { mData.mBlendScale = newBlendScale; }
		void fSetCurrentTime( f32 time ) { mCurrentTime = time; }
		void fSetData( const tAnimTrackData& data ) { mData = data; }
		static void fSetEnableEndOfTrackEvent( b32 enabled );

	public:
		virtual void fOnPushed( tAnimatedSkeleton& skeleton ) { }

	public:
		inline const tAnimTrackDesc& fDesc( ) const				{ return mDesc; }
		inline const tStringPtr& fTag( ) const					{ return mDesc.mTag; }
		inline const tAnimTrackData& fData( ) const				{ return mData; }

		inline f32 fCurrentTime( ) const	{ return mCurrentTime; }
		inline f32 fNormalizedTime( ) const { return mCurrentTime / ( mDesc.mMaxTime - mDesc.mMinTime ); }
		inline f32 fPrevTime( ) const		{ return mPrevTime; }
		inline f32 fTimeScale( ) const		{ return mData.mTimeScale; }
		inline f32 fMaxTime( ) const		{ return mDesc.mMaxTime; }
		
		inline f32 fBlendScale( ) const		{ return mData.mBlendScale; }
		inline f32 fBlendStrength( ) const	{ return mBlendStrength * mData.mBlendScale; }
		inline f32 fBlendIn( ) const		{ return mData.mBlendIn; }
		inline f32 fBlendOut( ) const		{ return mData.mBlendOut; }

		inline u32 fFlags( ) const	 { return mDesc.mFlags; }
		inline b32 fPartial( ) const { return mDesc.mFlags & cFlagPartial; }
		inline f32 fLooping( ) const { return mBlendOutDelta==0.f; }

		// This is expiremental stuff for tSigAnimMoMap
		inline b32 fHasEnded( ) const { return mEnded; }
		virtual void fRestart( );

	public: // debuggish
		static const f32 cVisibleThresh;

		if_devmenu( virtual b32 fVisible( ) const { return (fBlendScale( ) > cVisibleThresh); } );
		if_devmenu( virtual void fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const );
		if_devmenu( virtual void fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const );
		if_devmenu( void fDebugIndent( std::stringstream& ss, u32 indentDepth ) const );
	protected:
		virtual void fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign ) { fZeroOut( refFrameDelta ); }
		virtual void fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend ) { }
	private:
		void fClampTime( );
		f32  fApplyTime( f32 dt, tAnimatedSkeleton& animSkel );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tAnimTrack );
} }


#endif//__tAnimTrack__

