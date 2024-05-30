#include "BasePch.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "Logic/tAnimatable.hpp"

namespace Sig
{

	b32 tAnimTrack::gEnableEndOfTrackEvent = false;

	tAnimTrack::tAnimTrack( f32 blendIn, f32 blendOut, f32 timeScale, f32 blendScale, f32 minTime, f32 maxTime, f32 startTime, u32 flags )
		: mFlags( flags )
		, mCurrentTime( startTime )
		, mTimeScale( timeScale )
		, mBlendScale( blendScale )
		, mMinTime( minTime )
		, mMaxTime( maxTime < 0.f ? Math::cInfinity : maxTime )
		, mBlendIn( blendIn <= 0.f ? 1.f : 0.f )
		, mBlendInDelta( blendIn <= 0.f ? 0.f : ( 1.f / blendIn ) )
		, mBlendOut( 1.f )
		, mBlendOutDelta( 0.f )
		, mBlendStrength( blendIn <= 0.f ? 1.f : 0.f )
	{
		fClampTime( );

		if( blendOut > 0.f )
		{
			const f32 startBlendOutAt = ( mTimeScale > 0.f ) ? ( mMaxTime - blendOut ) : ( mMinTime + blendOut );
			const f32 timeTilStart = fAbs( startBlendOutAt - mCurrentTime );
			mBlendOutDelta = -1.f / blendOut;
			mBlendOut = 1.f - mBlendOutDelta * timeTilStart;
		}
	}

	void tAnimTrack::fStep( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt )
	{
		// before we step, store the current time (this becomes our "previous" time)
		mPrevTime = fCurrentTime( );

		// increment current time (ensuring we scale dt first), and wrap/clamp
		const f32 wrapSign = fApplyTime( dt * mTimeScale, animSkel );

		// N.B.! we don't apply mTimeScale to blending in/out
		const f32 absDt = fAbs( dt );
		mBlendIn = fMin( 1.f, mBlendIn + mBlendInDelta * absDt );
		mBlendOut = fMax( 0.f, mBlendOut + mBlendOutDelta * absDt );
		mBlendStrength = fMin( mBlendIn, mBlendOut );

		// call the derived step method (making sure to scale dt)
		fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt * mTimeScale, wrapSign );
	}

	void tAnimTrack::fEvaluate( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend )
	{
		// Max told me to remove the call to fEvaluateAdditive
		const b32 additive = fFlags( ) & cFlagAdditive;
		sigassert( !additive );
		//if( additive )
		//	fEvaluateAdditive( result, animSkel, forceFullBlend );
		//else
		fEvaluateLerp( result, animSkel, forceFullBlend );
	}

	void tAnimTrack::fBeginBlendingOut( f32 blendOutOverNSeconds )
	{
		if( blendOutOverNSeconds <= 0.f )
		{
			// blend out immediately
			mBlendOut = 0.f;
			mBlendOutDelta = -1.f;
		}
		else
		{
			// blend out over time
			mBlendOutDelta = -1.f / blendOutOverNSeconds;
			mBlendOut = fMin( 1.f, mBlendOut );
		}
	}

	void tAnimTrack::fSetAdditive( b32 isAdditive )
	{
		if( isAdditive )
			mFlags = fSetBits( mFlags, cFlagAdditive );
		else
			mFlags = fClearBits( mFlags, cFlagAdditive );
	}

	void tAnimTrack::fSetEnableEndOfTrackEvent( b32 enabled )
	{
		gEnableEndOfTrackEvent = enabled;
	}

	void tAnimTrack::fClampTime( )
	{
		const f32 maxTimeFudge = 0.9999f;

		if( mMaxTime < mMinTime )
			return;
		else if( mMaxTime == mMinTime )
			mCurrentTime = mMinTime;
		else if( mFlags & cFlagClampTime )
			mCurrentTime = fClamp( mCurrentTime, mMinTime, maxTimeFudge*mMaxTime );
		else
			mCurrentTime = fWrap( mCurrentTime, mMinTime, maxTimeFudge*mMaxTime );
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
		else if( gEnableEndOfTrackEvent )
		{
			if( !fTestBits( mFlags, cFlagIgnoreEndEvent ) 
				&&		fTestBits( mFlags, cFlagClampTime ) 
				&&		dt > 0.f
				&&		( ( fLooping( ) && stepSign == 0.f ) || mBlendOut < 1.f ) ) // reached the end of a one shot animation
			{
				animSkel.fQueueEvent( tKeyFrameEvent( mCurrentTime, fBlendStrength( ), tKeyFrameAnimTrack::fEventKeyFrameID( ), Logic::AnimationEvent::cEventReachedEndOneShot, tStringPtr( ) ) );
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
		ss << std::fixed << std::setprecision( 3 ) << " (time = " << fCurrentTime( ) << ") (blend = " << fBlendStrength( ) << ")" << std::endl;
	}

	void tAnimTrack::fDebugIndent( std::stringstream& ss, u32 indentDepth ) const
	{
		const char* tab = "    ";
		for( u32 i = 0; i < indentDepth; ++i )
			ss << tab;		
	}
#endif//sig_devmenu
}


