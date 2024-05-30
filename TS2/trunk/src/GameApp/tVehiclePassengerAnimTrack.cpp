#include "GameAppPch.hpp"
#include "tVehiclePassengerAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tVehiclePassengerLogic.hpp"

namespace Sig
{
	namespace 
	{
		enum tVehiclePassengerTracks
		{
			cVehiclePassFrontLeftAnim = 0,
			cVehiclePassFrontRightAnim,
			cVehiclePassBackLeftAnim,
			cVehiclePassBackRightAnim,
			cVehiclePassAnimCount
		};
	}


	devvar( bool, Gameplay_Vehicle_CharacterLeanOverride, false );
	devvar_clamp( f32, Gameplay_Vehicle_CharacterFrontBack, 0.0f, -1.0f, 1.0f, 2 ); 
	devvar_clamp( f32, Gameplay_Vehicle_CharacterLeftRight, 0.0f, -1.0f, 1.0f, 2 );  

	tVehiclePassengerAnimTrack::tVehiclePassengerAnimTrack( 
		tBlendAnimTrack::tTrackList& tracksToBlend,
		f32 blendIn, 
		f32 blendOut, 
		f32 timeScale, 
		f32 blendScale,
		tSprungMassRef acc )
		: tBlendAnimTrack( tracksToBlend, blendIn, blendOut, timeScale, blendScale, 0.f, -1.f, tAnimTrack::cFlagPartial )
		, mAcc( acc )
	{
	}

	void tVehiclePassengerAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == cVehiclePassAnimCount );

		f32 leftRight = -0.0f;
		f32 frontBack = -0.0f;

		if( mAcc.mAcc )
		{
			Math::tVec2f horizontalAcc = mAcc.mAcc->fXZ( );
			leftRight = horizontalAcc.x;
			frontBack = horizontalAcc.y;
		}

		if( Gameplay_Vehicle_CharacterLeanOverride )
		{
			leftRight = Gameplay_Vehicle_CharacterLeftRight;
			frontBack = Gameplay_Vehicle_CharacterFrontBack;
		}

		f32 l = fClamp( leftRight / 2.0f + 0.5f, 0.f, 1.f );
		f32 r = 1.0f - l;
		f32 f = fClamp( frontBack / 2.0f + 0.5f, 0.f, 1.f );
		f32 b = 1.0f - f;

		mSubTracks[ cVehiclePassFrontLeftAnim ]->fSetBlendScale( l * f );
		mSubTracks[ cVehiclePassFrontRightAnim ]->fSetBlendScale( r * f );
		mSubTracks[ cVehiclePassBackLeftAnim ]->fSetBlendScale( l * b );
		mSubTracks[ cVehiclePassBackRightAnim ]->fSetBlendScale( r * b );
		
		tBlendAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
	}
}


namespace Sig
{
	namespace
	{
		static tVehiclePassengerAnimTrack* fMakeAnim( const tVehiclePassengerAnimDesc* desc )
		{
			tBlendAnimTrack::tTrackList trackList;
			trackList.fNewArray( cVehiclePassAnimCount );

			trackList[ cVehiclePassFrontLeftAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mFrontLeftAnim, 0.f, 0.f ) ) );
			trackList[ cVehiclePassFrontRightAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mFrontRightAnim, 0.f, 0.f ) ) );
			trackList[ cVehiclePassBackLeftAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mBackLeftAnim, 0.f, 0.f ) ) );
			trackList[ cVehiclePassBackRightAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mBackRightAnim, 0.f, 0.f ) ) );

			return NEW tVehiclePassengerAnimTrack( trackList, desc->mBlendIn, desc->mBlendOut, desc->mTimeScale, desc->mBlendScale, desc->mAcc );
		}

		static void fPushAnim( const tVehiclePassengerAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fPushTrack( tAnimTrackPtr( fMakeAnim( desc ) ) );
		}

		static void fPushBeforeTag( const tVehiclePassengerAnimDesc* desc, tAnimatedSkeleton* stack, tStringPtr& tag )
		{
			stack->fPushTrackBeforeTag( tAnimTrackPtr( fMakeAnim( desc ) ), tag );
		}
	}
	void tVehiclePassengerAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class< tVehiclePassengerAnimDesc,Sqrat::DefaultAllocator<tVehiclePassengerAnimDesc> > classDesc( vm.fSq( ) );
			classDesc
				.GlobalFunc(_SC("Push"), &fPushAnim)
				.GlobalFunc(_SC("PushBeforeTag"), &fPushBeforeTag)
				.Var(_SC("FrontLeftAnim"), &tVehiclePassengerAnimDesc::mFrontLeftAnim)
				.Var(_SC("FrontRightAnim"), &tVehiclePassengerAnimDesc::mFrontRightAnim)
				.Var(_SC("BackLeftAnim"), &tVehiclePassengerAnimDesc::mBackLeftAnim)
				.Var(_SC("BackRightAnim"), &tVehiclePassengerAnimDesc::mBackRightAnim)

				.Var(_SC("BlendIn"), &tVehiclePassengerAnimDesc::mBlendIn)
				.Var(_SC("BlendOut"), &tVehiclePassengerAnimDesc::mBlendOut)
				.Var(_SC("TimeScale"), &tVehiclePassengerAnimDesc::mTimeScale)
				.Var(_SC("BlendScale"), &tVehiclePassengerAnimDesc::mBlendScale)
				.Var(_SC("Acc"), &tVehiclePassengerAnimDesc::mAcc )
				;

			vm.fNamespace(_SC("Anim")).Bind( _SC("VehiclePassengerTrack"), classDesc );
		}
		{
			Sqrat::Class< tSprungMassRef,Sqrat::DefaultAllocator<tSprungMassRef> > classDesc( vm.fSq( ) );
			classDesc
				;

			vm.fNamespace(_SC("Anim")).Bind( _SC("SprungMassRef"), classDesc );
		}
	}
}
