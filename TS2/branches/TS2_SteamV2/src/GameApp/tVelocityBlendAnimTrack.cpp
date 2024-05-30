#include "GameAppPch.hpp"
#include "tVelocityBlendAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tTurretLogic.hpp"

namespace Sig
{
	tVelocityBlendAnimTrack::tVelocityBlendAnimTrack( 
		tBlendAnimTrack::tTrackList& tracksToBlend,
		f32 blendIn, 
		f32 blendOut, 
		f32 timeScale, 
		f32 blendScale,
		tTurretLogic* turret )
		: tBlendAnimTrack( tracksToBlend, blendIn, blendOut, timeScale, blendScale, 0.f, -1.f, tracksToBlend.fFront( )->fFlags( ) ), mTurret( turret )
	{
	}

	void tVelocityBlendAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == 2 );

		const f32 pitch = mTurret->fSpeedBlendValue( );

		// The first anim is low and the second is high
		mSubTracks[ 0 ]->fSetBlendScale( 1 );
		mSubTracks[ 1 ]->fSetBlendScale( pitch );
		
		tBlendAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tVelocityBlendAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			tBlendAnimTrack::tTrackList trackList;
			trackList.fPushBack( tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mLowAnim, 0.f, 0.f ) ) ) );
			trackList.fPushBack( tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mHighAnim, 0.f, 0.f ) ) ) );
			stack->fPushTrack( tAnimTrackPtr( NEW tVelocityBlendAnimTrack( trackList, desc->mBlendIn, desc->mBlendOut, desc->mTimeScale, desc->mBlendScale, desc->mTurret ) ) );
		}
	}
	void tVelocityBlendAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tVelocityBlendAnimDesc,Sqrat::DefaultAllocator<tVelocityBlendAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("LowAnim"), &tVelocityBlendAnimDesc::mLowAnim)
			.Var(_SC("HighAnim"), &tVelocityBlendAnimDesc::mHighAnim)
			.Var(_SC("BlendIn"), &tVelocityBlendAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tVelocityBlendAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tVelocityBlendAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tVelocityBlendAnimDesc::mBlendScale)
			.Var(_SC("Turret"), &tVelocityBlendAnimDesc::mTurret )
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("VelocityBlendTrack"), classDesc );
	}
}
