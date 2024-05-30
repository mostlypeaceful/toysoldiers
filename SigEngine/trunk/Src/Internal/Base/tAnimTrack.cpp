#include "BasePch.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "Logic/tAnimatable.hpp"

namespace Sig { namespace Anim
{

	b32 tAnimTrack::gEnableEndOfTrackEvent = false;
	const f32 tAnimTrack::cVisibleThresh = 0.1f;


	void tAnimEvaluationResult::fBlendWith( const tAnimEvaluationResult& other, f32 lerp )
	{
		sigassert( mBoneResults.fCount( ) == other.mBoneResults.fCount( ) );
		for( u32 i = 0; i < mBoneResults.fCount( ); ++i )
			mBoneResults[ i ].fBlendNLerp( other.mBoneResults[ i ], lerp );
	}

	tAnimTrack::tAnimTrack( const tAnimTrackDesc& desc )
		: mDesc( desc, true )
		, mData( desc )
		, mCurrentTime( desc.mStartTime )
		, mBlendInDelta( desc.mBlendIn <= 0.f ? 0.f : ( 1.f / desc.mBlendIn ) )
		, mBlendOutDelta( 0.f )
		, mBlendStrength( desc.mBlendIn <= 0.f ? 1.f : 0.f )
		, mEnded( false )
	{
		fRestart( );
	}

	void tAnimTrack::fRestart( )
	{
		mEnded = false;
		mCurrentTime = mDesc.mStartTime;
		mData.mBlendIn = (mDesc.mBlendIn <= 0.f) ? 1.f : 0.f;

		fClampTime( );

		if( mDesc.mBlendOut > 0.f )
		{
			const f32 startBlendOutAt = ( mDesc.mTimeScale > 0.f ) ? ( mDesc.mMaxTime - mDesc.mBlendOut ) : ( mDesc.mMinTime + mDesc.mBlendOut );
			const f32 timeTilStart = fAbs( startBlendOutAt - mCurrentTime );
			mBlendOutDelta = -1.f / mDesc.mBlendOut;
			mData.mBlendOut = 1.f - mBlendOutDelta * timeTilStart;
		}
		else
		{
			//this is for the blend strength lerp in fStep. ghetto but is necessary. -mk @ max
			mData.mBlendOut = 1.f;
		}
	}

	void tAnimTrack::fStep( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt )
	{
		// before we step, store the current time (this becomes our "previous" time)
		mPrevTime = fCurrentTime( );

		// increment current time (ensuring we scale dt first), and wrap/clamp
		const f32 wrapSign = fApplyTime( dt * mData.mTimeScale, animSkel );

		const f32 absDt = fAbs( dt * mData.mTimeScale );
		mData.mBlendIn = fMin( 1.f, mData.mBlendIn + mBlendInDelta * absDt );
		mData.mBlendOut = fMax( 0.f, mData.mBlendOut + mBlendOutDelta * absDt );
		mBlendStrength = fMin( mData.mBlendIn, mData.mBlendOut );

		// call the derived step method (making sure to scale dt)
		Math::tPRSXformf temp = Math::tPRSXformf::cIdentity;
		fStepInternal( mDesc.mApplyRefFrame ? refFrameDelta : temp, animSkel, forceFullBlend, dt * mData.mTimeScale, wrapSign );
	}

	void tAnimTrack::fEvaluate( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend )
	{
		fEvaluateLerp( result, animSkel, forceFullBlend );
	}

	void tAnimTrack::fBeginBlendingOut( f32 blendOutOverNSeconds )
	{
		if( blendOutOverNSeconds <= 0.f )
		{
			// blend out immediately
			mData.mBlendOut = 0.f;
			mBlendOutDelta = -1.f;
		}
		else
		{
			// blend out over time
			mBlendOutDelta = -1.f / blendOutOverNSeconds;
			mData.mBlendOut = fMin( 1.f, mData.mBlendOut );
		}
	}

	void tAnimTrack::fSetEnableEndOfTrackEvent( b32 enabled )
	{
		gEnableEndOfTrackEvent = enabled;
	}

	void tAnimTrack::fClampTime( )
	{
		const f32 maxTimeFudge = 0.9999f;

		if( mDesc.mMaxTime < mDesc.mMinTime )
			return;
		else if( mDesc.mMaxTime == mDesc.mMinTime )
			mCurrentTime = mDesc.mMinTime;
		else if( mDesc.mFlags & cFlagClampTime )
			mCurrentTime = fClamp( mCurrentTime, mDesc.mMinTime, maxTimeFudge*mDesc.mMaxTime );
		else
			mCurrentTime = fWrap( mCurrentTime, mDesc.mMinTime, maxTimeFudge*mDesc.mMaxTime );
	}

	f32 tAnimTrack::fApplyTime( f32 dt, tAnimatedSkeleton& animSkel )
	{
		const f32 t0 = mCurrentTime;
		mCurrentTime += dt;
		fClampTime( );
		const f32 t1 = mCurrentTime;

		const f32 stepSign = t1 - t0;

		f32 o = 0.f;

		if( stepSign < 0.f && dt > 0.f ) // looped forward
			o = +1.f;
		else if( stepSign > 0.f && dt < 0.f ) // looped backward
			o = -1.f;
		else 
		{
			if( !fEqual( dt, 0.f ) && mData.mBlendOut < 1.f ) // reached the end of a one shot animation
			{
				mEnded = true;

				if( gEnableEndOfTrackEvent && !fTestBits( mDesc.mFlags, cFlagIgnoreEndEvent ) ) 
				{
					animSkel.fQueueEvent( tKeyFrameEvent( mCurrentTime, fBlendStrength( ), tKeyFrameAnimTrack::fEventKeyFrameID( ), Logic::AnimationEvent::cEventReachedEndOneShot, tStringPtr( ) ) );
				}
			}
		}

		return o;
	}

#ifdef sig_devmenu
	void tAnimTrack::fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const
	{
		fDebugIndent( ss, indentDepth );
		ss << fDebugTypeName( );
		if( fTag( ).fExists( ) )
			ss << " - " << fTag( ).fCStr( );
	}

	void tAnimTrack::fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const
	{
		ss << std::fixed << std::setprecision( 3 ) << " (time = " << fCurrentTime( ) << ") (blend = " << fBlendStrength( ) << ") (ratio = " << fNormalizedTime( ) << ")" << std::endl;
	}

	void tAnimTrack::fDebugIndent( std::stringstream& ss, u32 indentDepth ) const
	{
		const char* tab = "    ";
		for( u32 i = 0; i < indentDepth; ++i )
			ss << tab;		
	}
#endif
} }

namespace Sig { namespace Anim
{

	void tAnimTrackDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tAnimTrackDesc, Sqrat::DefaultAllocator<tAnimTrackDesc> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("BlendIn"),		&tAnimTrackDesc::mBlendIn)
			.Var(_SC("BlendOut"),		&tAnimTrackDesc::mBlendOut)
			.Var(_SC("TimeScale"),		&tAnimTrackDesc::mTimeScale)
			.Var(_SC("BlendScale"),		&tAnimTrackDesc::mBlendScale)
			.Var(_SC("MinTime"),		&tAnimTrackDesc::mMinTime)
			.Var(_SC("MaxTime"),		&tAnimTrackDesc::mMaxTime)
			.Var(_SC("StartTime"),		&tAnimTrackDesc::mStartTime)
			.Var(_SC("Flags"),			&tAnimTrackDesc::mFlags)
			.Var(_SC("ApplyRefFrame"),	&tAnimTrackDesc::mApplyRefFrame)
			.Var(_SC("Tag"),			&tAnimTrackDesc::mTag)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("AnimTrackDesc"), classDesc );
	}
} }


