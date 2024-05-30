#include "GameAppPch.hpp"
#include "tPitchBlendAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tTurretLogic.hpp"

namespace Sig
{
	tPitchBlendAnimTrack::tPitchBlendAnimTrack( 
		tBlendAnimTrack::tTrackList& tracksToBlend,
		f32 blendIn, 
		f32 blendOut, 
		f32 timeScale, 
		f32 blendScale,
		tTurretLogic* turret )
		: tBlendAnimTrack( tracksToBlend, blendIn, blendOut, timeScale, blendScale, 0.f, -1.f, tracksToBlend.fFront( )->fFlags( ) ), mTurret( turret )
	{
	}

	void tPitchBlendAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == 2 );

		log_warning_nospam( 0, "Dont use this PitchBlendAnimTrack! Call UseMuzzleTrack in your momap! UnitID: " << GameFlags::fUNIT_IDEnumToValueString( mTurret->fUnitID( ) ) );

		const f32 pitch = mTurret->fPitchBlendValue( );

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
		static void fPushAnim( const tPitchBlendAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			tBlendAnimTrack::tTrackList trackList;
			trackList.fPushBack( tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mLowAnim, 0.f, 0.f ) ) ) );
			trackList.fPushBack( tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mHighAnim, 0.f, 0.f ) ) ) );
			stack->fPushTrack( tAnimTrackPtr( NEW tPitchBlendAnimTrack( trackList, desc->mBlendIn, desc->mBlendOut, desc->mTimeScale, desc->mBlendScale, desc->mTurret ) ) );
		}
	}
	void tPitchBlendAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tPitchBlendAnimDesc,Sqrat::DefaultAllocator<tPitchBlendAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("LowAnim"), &tPitchBlendAnimDesc::mLowAnim)
			.Var(_SC("HighAnim"), &tPitchBlendAnimDesc::mHighAnim)
			.Var(_SC("BlendIn"), &tPitchBlendAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tPitchBlendAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tPitchBlendAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tPitchBlendAnimDesc::mBlendScale)
			.Var(_SC("Turret"), &tPitchBlendAnimDesc::mTurret )
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("PitchBlendTrack"), classDesc );
	}
}
