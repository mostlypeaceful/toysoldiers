#include "GameAppPch.hpp"
#include "tPitchBlendMuzzleAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tWeapon.hpp"

namespace Sig
{
	namespace 
	{
		enum tTracks
		{
			cTrackLow,
			cTrackHigh,
			cTrackCount
		};
	}


	tPitchBlendMuzzleAnimTrack::tPitchBlendMuzzleAnimTrack( 
		tBlendAnimTrack::tTrackList& tracksToBlend,
		const tPitchBlendMuzzleAnimDesc& desc )
		: tBlendAnimTrack( tracksToBlend, desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, 0.f, -1.f, desc.mFlags )
		, mWeapon( desc.mWeapon )
		, mLowAngle( desc.mLowAngle )
		, mHighAngle( desc.mHighAngle )
	{
	}

	void tPitchBlendMuzzleAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == cTrackCount );
		sigassert( mWeapon );

		const f32 pitch = mWeapon->fOutputPitchAngle( );

		const f32 range = mHighAngle - mLowAngle;
		f32 blend = (pitch - mLowAngle) / range;
		blend = fClamp( blend, 0.f, 1.f );

		mSubTracks[ cTrackLow ]->fSetBlendScale( 1 );
		mSubTracks[ cTrackHigh ]->fSetBlendScale( blend );
		
		tBlendAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tPitchBlendMuzzleAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			tBlendAnimTrack::tTrackList trackList;
			trackList.fResize( cTrackCount );
			trackList[ cTrackLow ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mLowAnim, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, 0.f, desc->mFlags ) ) );
			trackList[ cTrackHigh ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mHighAnim, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, 0.f, desc->mFlags ) ) );
			stack->fPushTrack( tAnimTrackPtr( NEW tPitchBlendMuzzleAnimTrack( trackList, *desc ) ) );
		}
	}
	void tPitchBlendMuzzleAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tPitchBlendMuzzleAnimDesc,Sqrat::DefaultAllocator<tPitchBlendMuzzleAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("LowAnim"), &tPitchBlendMuzzleAnimDesc::mLowAnim)
			.Var(_SC("HighAnim"), &tPitchBlendMuzzleAnimDesc::mHighAnim)
			.Var(_SC("LowAngle"), &tPitchBlendMuzzleAnimDesc::mLowAngle)
			.Var(_SC("HighAngle"), &tPitchBlendMuzzleAnimDesc::mHighAngle)
			.Var(_SC("BlendIn"), &tPitchBlendMuzzleAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tPitchBlendMuzzleAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tPitchBlendMuzzleAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tPitchBlendMuzzleAnimDesc::mBlendScale)
			.Var(_SC("Weapon"), &tPitchBlendMuzzleAnimDesc::mWeapon )
			.Var(_SC("Flags"), &tPitchBlendMuzzleAnimDesc::mFlags )
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("PitchBlendMuzzleTrack"), classDesc );
	}
}
