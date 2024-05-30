#include "BasePch.hpp"
#include "tBlendAnimTrack.hpp"

namespace Sig { namespace Anim
{

	tBlendAnimTrack::tBlendAnimTrack( tTrackList& tracksToBlend, const tAnimTrackDesc& desc )
		: tAnimTrack( desc )
	{
		mSubTracks.fSwap( tracksToBlend );
	}

	void tBlendAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		Math::tPRSXformf subRefFrameDelta = refFrameDelta;
		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
		{
			tAnimTrack& track = *mSubTracks[ i ];

			// Dont override the child track's timescale, dt has already been scaled here by the base tAnimTrack::fStep
			const f32 savedBlendScale = track.fBlendScale( );
			track.fSetBlendScale( savedBlendScale * fBlendScale( ) );
			track.fStep( subRefFrameDelta, animSkel, false, dt );
			track.fSetBlendScale( savedBlendScale );
		}

		if( forceFullBlend )	refFrameDelta = subRefFrameDelta;
		else					refFrameDelta.fBlendLerp( subRefFrameDelta, fBlendStrength( ) );
	}

	void tBlendAnimTrack::fEvaluateLerp( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend )
	{
		fEvaluateInternal( result, animSkel, forceFullBlend, false );
	}

	void tBlendAnimTrack::fEvaluateInternal( tAnimEvaluationResult& result, tAnimatedSkeleton& animSkel, b32 forceFullBlend, b32 additive )
	{
		const f32 blendScale = forceFullBlend ? 1.f : fBlendStrength( );

		// accumulate complete subtrack influence before combining it with total result.
		tAnimEvaluationResult tempResult = result;

		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
		{
			tAnimTrack& track = *mSubTracks[ i ];
			track.fEvaluate( tempResult, animSkel, false );
		}

		result.fBlendWith( tempResult, blendScale );
	}

	//------------------------------------------------------------------------------
	void tBlendAnimTrack::fOnPushed( tAnimatedSkeleton & skeleton )
	{
		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
			mSubTracks[ i ]->fOnPushed( skeleton );
	}

	void tBlendAnimTrack::fRestart( )
	{
		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
			mSubTracks[ i ]->fRestart( );
	}

#ifdef sig_devmenu
	b32 tBlendAnimTrack::fVisible( ) const
	{
		if( !tAnimTrack::fVisible( ) )
			return false;

		for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
			if( mSubTracks[ i ]->fVisible( ) )
				return true;

		return false;
	}

	void tBlendAnimTrack::fDebugTrackName( std::stringstream& ss, u32 indentDepth ) const
	{
		if( fVisible( ) )
		{
			tAnimTrack::fDebugTrackName( ss, indentDepth );
		}
	}

	void tBlendAnimTrack::fDebugTrackData( std::stringstream& ss, u32 indentDepth ) const
	{
		if( fVisible( ) )
		{
			tAnimTrack::fDebugTrackData( ss, indentDepth );

			for( u32 i = 0; i < mSubTracks.fCount( ); ++i )
			{
				mSubTracks[ i ]->fDebugTrackName( ss, indentDepth + 1 );
				mSubTracks[ i ]->fDebugTrackData( ss, indentDepth + 1 );
			}
		}
	}
#endif

} }


