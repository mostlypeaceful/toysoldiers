#include "GameAppPch.hpp"
#include "tAirborneAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tAirborneLogic.hpp"

namespace Sig
{
	namespace 
	{
		enum tTracks
		{
			cTrackFastLeft,
			cTrackFastRight,
			cTrackSlowLeft,
			cTrackSlowRight,
			cTrackCount
		};
	}

	devvar( f32, Gameplay_Vehicle_Airborne_Anim_RollBlend, 0.2f );


	tAirborneAnimTrack::tAirborneAnimTrack( 
		tBlendAnimTrack::tTrackList& tracksToBlend,
		const tAirborneAnimDesc& desc )
		: tBlendAnimTrack( tracksToBlend, desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, 0.f, -1.f, tracksToBlend.fFront( )->fFlags( ) )
		, mAirborne( desc.mAirborne )
	{
	}

	void tAirborneAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == cTrackCount );
		sigassert( mAirborne );

		const f32 speedBlend = mAirborne->fPhysics( ).fSpeedBlend( );
		f32 rollBlend = mAirborne->fPhysics( ).fRollBlend( );
		rollBlend *= 0.5f;
		rollBlend += 0.5f;

		mRollDamp.fSetBlends( Gameplay_Vehicle_Airborne_Anim_RollBlend );
		mRollDamp.fStep( rollBlend, dt );

		f32 slow = 1.0f - speedBlend;
		f32 right = mRollDamp.fValue( );
		f32 left = 1.f - right;

		mSubTracks[ cTrackFastLeft ]->fSetBlendScale( left * speedBlend );
		mSubTracks[ cTrackFastRight ]->fSetBlendScale( right * speedBlend );
		mSubTracks[ cTrackSlowLeft ]->fSetBlendScale( left * slow );
		mSubTracks[ cTrackSlowRight ]->fSetBlendScale( right * slow );
		
		tBlendAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tAirborneAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			tBlendAnimTrack::tTrackList trackList;
			trackList.fResize( cTrackCount );
			trackList[ cTrackFastLeft ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mFastLeftAnim, 0.f, 0.f ) ) );
			trackList[ cTrackFastRight ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mFastRightAnim, 0.f, 0.f ) ) );
			trackList[ cTrackSlowLeft ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mSlowLeftAnim, 0.f, 0.f ) ) );
			trackList[ cTrackSlowRight ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mSlowRightAnim, 0.f, 0.f ) ) );
			stack->fPushTrack( tAnimTrackPtr( NEW tAirborneAnimTrack( trackList, *desc ) ) );
		}
	}
	void tAirborneAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tAirborneAnimDesc,Sqrat::DefaultAllocator<tAirborneAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("FastLeftAnim"), &tAirborneAnimDesc::mFastLeftAnim)
			.Var(_SC("FastRightAnim"), &tAirborneAnimDesc::mFastRightAnim)
			.Var(_SC("SlowLeftAnim"), &tAirborneAnimDesc::mSlowLeftAnim)
			.Var(_SC("SlowRightAnim"), &tAirborneAnimDesc::mSlowRightAnim)
			.Var(_SC("BlendIn"), &tAirborneAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tAirborneAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tAirborneAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tAirborneAnimDesc::mBlendScale)
			.Var(_SC("Airborne"), &tAirborneAnimDesc::mAirborne )
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("AirborneAnimTrack"), classDesc );
	}
}
