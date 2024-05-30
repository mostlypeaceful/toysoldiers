#include "BasePch.hpp"
#include "tBlendAnimTrack.hpp"

namespace Sig
{

	tBlendAnimTrack::tBlendAnimTrack( 
		tTrackList& tracksToBlend,
		f32 blendIn, 
		f32 blendOut, 
		f32 timeScale, 
		f32 blendScale,
		f32 minTime, 
		f32 maxTime, 
		u32 flags )
		: tAnimTrack( blendIn, blendOut, timeScale, blendScale, minTime, maxTime < 0.f ? Math::cInfinity : maxTime, 0.f, flags )
	{
		mSubTracks.fSwap( tracksToBlend );
	}

	void tBlendAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		Math::tPRSXformf subRefFrameDelta = refFrameDelta;
		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
		{
			tAnimTrack& track = *mSubTracks[ i ];

			const f32 savedBlendScale = track.fBlendScale( );
			const f32 savedTimeScale = track.fTimeScale( );
			track.fSetBlendScale( savedBlendScale * fBlendScale( ) );
			track.fSetTimeScale( savedTimeScale * fTimeScale( ) );
			track.fStep( subRefFrameDelta, animSkel, false, dt );
			track.fSetBlendScale( savedBlendScale );
			track.fSetTimeScale( savedTimeScale );
		}

		if( forceFullBlend )	refFrameDelta = subRefFrameDelta;
		else					refFrameDelta.fBlendLerp( subRefFrameDelta, fBlendStrength( ) );
	}

	void tBlendAnimTrack::fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend )
	{
		fEvaluateInternal( result, animSkel, forceFullBlend, false );
	}

	void tBlendAnimTrack::fEvaluateAdditive( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend )
	{
		fEvaluateInternal( result, animSkel, forceFullBlend, true );
	}

	void tBlendAnimTrack::fEvaluateInternal( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend, b32 additive )
	{
		const f32 blendScale = forceFullBlend ? 1.f : fBlendStrength( );
		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
		{
			tAnimTrack& track = *mSubTracks[ i ];
			track.fSetAdditive( additive );

			const f32 savedBlendScale = track.fBlendScale( );
			const f32 savedTimeScale = track.fTimeScale( );
			track.fSetBlendScale( savedBlendScale * blendScale );
			track.fSetTimeScale( savedTimeScale * fTimeScale( ) );
			track.fEvaluate( result, animSkel, false );
			track.fSetBlendScale( savedBlendScale );
			track.fSetTimeScale( savedTimeScale );
		}
	}

	//------------------------------------------------------------------------------
	void tBlendAnimTrack::fOnPushed( tAnimatedSkeleton & skeleton )
	{
		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
			mSubTracks[ i ]->fOnPushed( skeleton );
	}

#ifdef sig_devmenu
	void tBlendAnimTrack::fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const
	{
		tAnimTrack::fDebugTrackData( ss, indentDepth );

		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
		{
			mSubTracks[ i ]->fDebugTrackName( ss, indentDepth + 1 );
			mSubTracks[ i ]->fDebugTrackData( ss, indentDepth + 1 );
		}
	}
#endif//sig_devmenu

}


