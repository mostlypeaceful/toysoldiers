#include "GameAppPch.hpp"
#include "tTankTreadAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tWheeledVehicleLogic.hpp"

namespace Sig
{
	namespace 
	{
		enum tTracks
		{
			cTrackLeft,
			cTrackRight,
			cTrackCount
		};
	}


	tTankTreadAnimTrack::tTankTreadAnimTrack( 
		tBlendAnimTrack::tTrackList& tracksToBlend,
		const tTankTreadAnimDesc& desc )
		: tBlendAnimTrack( tracksToBlend, desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, 0.f, -1.f, desc.mFlags | tAnimTrack::cFlagPartial )
		, mVehicle( desc.mVehicle )
	{
	}

	void tTankTreadAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == cTrackCount );
		sigassert( mVehicle );

		const f32 rpsToTS = 1.f / Math::c2Pi;

		mSubTracks[ cTrackLeft ]->fSetBlendScale( 1.f );
		mSubTracks[ cTrackRight ]->fSetBlendScale( 1.f );
		mSubTracks[ cTrackLeft ]->fSetTimeScale( mVehicle->fPhysics( ).fWheelStates( )[ 0 ].mAngVel * rpsToTS );
		mSubTracks[ cTrackRight ]->fSetTimeScale( mVehicle->fPhysics( ).fWheelStates( )[ 1 ].mAngVel * rpsToTS );
		
		tBlendAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tTankTreadAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			tBlendAnimTrack::tTrackList trackList;
			trackList.fResize( cTrackCount );
			trackList[ cTrackLeft ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mLeftAnim, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, tKeyFrameAnimTrack::fCurrentTimeOfTrack( desc->mLeftAnim, stack ), desc->mFlags ) ) );
			trackList[ cTrackRight ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRightAnim, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, tKeyFrameAnimTrack::fCurrentTimeOfTrack( desc->mRightAnim, stack ), desc->mFlags ) ) );
			stack->fPushTrack( tAnimTrackPtr( NEW tTankTreadAnimTrack( trackList, *desc ) ) );
		}
	}
	void tTankTreadAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tTankTreadAnimDesc,Sqrat::DefaultAllocator<tTankTreadAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("LeftAnim"),	&tTankTreadAnimDesc::mLeftAnim)
			.Var(_SC("RightAnim"),	&tTankTreadAnimDesc::mRightAnim)
			.Var(_SC("BlendIn"),	&tTankTreadAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"),	&tTankTreadAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"),	&tTankTreadAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tTankTreadAnimDesc::mBlendScale)
			.Var(_SC("Vehicle"),	&tTankTreadAnimDesc::mVehicle )
			.Var(_SC("Flags"),		&tTankTreadAnimDesc::mFlags )
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("TankTreadAnimTrack"), classDesc );
	}
}
