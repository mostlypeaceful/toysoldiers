#include "GameAppPch.hpp"
#include "tCharacterMoveAnimTrackFPS.hpp"
#include "tUserControllableCharacterLogic.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"
#include "Logic/tPhysical.hpp"

using namespace Sig::Math;

namespace Sig
{
	devvar( f32, Gameplay_Character_UserControlFPS_IdleBlendSpring, 0.2f ); 
	devvar( f32, Gameplay_Character_UserControlFPS_SprintBlendSpring, 0.8f ); 

	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;


		enum tCharacterMoveTracks
		{
			cCharacterMoveSprintAnim,
			cCharacterMoveRunForwardAnim,
			cCharacterMoveRunBackwardAnim,
			cCharacterMoveRunLeftAnim,
			cCharacterMoveRunRightAnim,
			cCharacterMoveRunLeftBackAnim,
			cCharacterMoveRunRightBackAnim,
			cCharacterMoveIdleAnim,
			cCharacterMoveIdleAimUpAnim,
			cCharacterMoveIdleAimDownAnim,
			cCharacterMoveRunAimUpAnim,
			cCharacterMoveRunAimDownAnim,
			cCharacterMoveAnimCount
		};
	}

	tCharacterMoveAnimTrackFPS::tCharacterMoveAnimTrackFPS( 
		tCharacterMoveAnimTrackFPS* prev,
		tBlendAnimTrack::tTrackList& tracksToBlend,
		f32 blendIn, 
		f32 blendOut, 
		f32 timeScale, 
		f32 blendScale,
		tUserControllableCharacterLogic* character )
		: tBlendAnimTrack( tracksToBlend, blendIn, blendOut, timeScale, blendScale, 0.f, -1.f/*, tAnimTrack::cFlagPartial*/ )
		, mCharacter( character )
		, mIdleBlend( 0.0f, 0.2f, 0.2f, 1.0f )
		, mCurrentRevereseBlend( 0.f )
		, mTargetReverseBlend( 0.f )
	{
		mSprintBlend.fSetValue( mCharacter->fSprinting( ) ? 1.0f : 0.0f );

		if( prev )
		{
			mIdleBlend				= prev->mIdleBlend;
			mSprintBlend			= prev->mSprintBlend;
			mCurrentRevereseBlend	= prev->mCurrentRevereseBlend;
			mTargetReverseBlend		= prev->mTargetReverseBlend;
		}
	}

	void tCharacterMoveAnimTrackFPS::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == cCharacterMoveAnimCount );

		// Idle run mixing
		f32 speed = fAbs( mCharacter->fGetSpeed( ) );

		mIdleBlend.fSetBlends( Gameplay_Character_UserControlFPS_IdleBlendSpring );
		mIdleBlend.fStep( fAbs(speed) > 0.0f ? 1.0f : 0.0f, dt );

		mSprintBlend.fSetBlends( Gameplay_Character_UserControlFPS_SprintBlendSpring );
		mSprintBlend.fStep( mCharacter->fSprinting( ) ? 1.0f : 0.0f, dt );
		
		f32 sprint = mSprintBlend.fValue( );
		f32 move = 1.f - sprint;
		f32 run = move * mIdleBlend.fValue( );
		f32 idle = move * (1.0f - run);

		// run direction mixing, this only works for running forwards
		Math::tVec3f localVel = mCharacter->fGetMoveVec( );
		localVel.y = 0;
		localVel.fNormalizeSafe( Math::tVec3f::cXAxis );

		f32 f = fMax( localVel.z, 0.0f );
		f32 b = fMax( -localVel.z, 0.0f );
		f32 l = fMax( localVel.x, 0.0f );
		f32 r = fMax( -localVel.x, 0.0f );

		// Incorporate reversing, to allow for running backwards.
		//  This transition needs to be smoothed.
		mTargetReverseBlend = ( b > 0.f ) ? 1.f : 0.f; //going backwards, reverse
		mCurrentRevereseBlend = fLerp( mCurrentRevereseBlend, mTargetReverseBlend, 0.2f );

		f32 forwardBlend = 1.0f - mCurrentRevereseBlend;
		f32 backBlend = mCurrentRevereseBlend;

		// aiming
		f32 aimBlend = mCharacter->fGetAimBlendMain( );
		f32 aimHigh = 1.0f;
		f32 aimLow = mCharacter->fGetAimBlendDownUp( ) / 2.0f + 0.5f;

		mSubTracks[ cCharacterMoveSprintAnim ]->fSetBlendScale( sprint );
		mSubTracks[ cCharacterMoveIdleAimUpAnim ]->fSetBlendScale( idle * aimBlend * aimHigh );
		mSubTracks[ cCharacterMoveIdleAimDownAnim ]->fSetBlendScale( idle * aimBlend * aimLow );
		mSubTracks[ cCharacterMoveRunAimUpAnim ]->fSetBlendScale( run * aimBlend * aimHigh );
		mSubTracks[ cCharacterMoveRunAimDownAnim ]->fSetBlendScale( run * aimBlend * aimLow );

		// Set output blends and speeds
		mSubTracks[ cCharacterMoveIdleAnim ]->fSetBlendScale( idle );
		mSubTracks[ cCharacterMoveRunForwardAnim ]->fSetTimeScale( speed );
		mSubTracks[ cCharacterMoveRunBackwardAnim ]->fSetTimeScale( speed );
		mSubTracks[ cCharacterMoveRunLeftAnim ]->fSetTimeScale( speed );
		mSubTracks[ cCharacterMoveRunRightAnim ]->fSetTimeScale( speed );

		mSubTracks[ cCharacterMoveRunForwardAnim ]->fSetBlendScale( f * run );
		mSubTracks[ cCharacterMoveRunBackwardAnim ]->fSetBlendScale( b * run );
		mSubTracks[ cCharacterMoveRunLeftAnim ]->fSetBlendScale( forwardBlend * l * run );
		mSubTracks[ cCharacterMoveRunRightAnim ]->fSetBlendScale( forwardBlend * r * run );
		mSubTracks[ cCharacterMoveRunLeftBackAnim ]->fSetBlendScale( backBlend * l * run );
		mSubTracks[ cCharacterMoveRunRightBackAnim ]->fSetBlendScale( backBlend * r * run );


		tBlendAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
	}
}


namespace Sig
{
	namespace
	{
		tAnimTrackPtr fMakeTrack( const tCharacterMoveFPSAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			tBlendAnimTrack::tTrackList trackList;
			trackList.fNewArray( cCharacterMoveAnimCount );


			f32 runStartTime = 0.f;
			f32 idleStartTime = 0.f;

			tCharacterMoveAnimTrackFPS *prevTrack = NULL;

			//find any tracks with the name walk anim and sync time.
			for( s32 i = stack->fTrackCount( ) - 1; i >= 0 ; --i )
			{
				prevTrack = stack->fTrack( i ).fDynamicCast< tCharacterMoveAnimTrackFPS >( );
				if( prevTrack )
				{
					runStartTime = prevTrack->fSubTracks( )[ cCharacterMoveRunForwardAnim ]->fDynamicCast< tKeyFrameAnimTrack >( )->fCurrentTime( );
					idleStartTime = prevTrack->fSubTracks( )[ cCharacterMoveIdleAnim ]->fDynamicCast< tKeyFrameAnimTrack >( )->fCurrentTime( );
					break;
				}
			}

			trackList[ cCharacterMoveSprintAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mSprint, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, 0.f ) ) );
			trackList[ cCharacterMoveRunForwardAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRunForward, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, runStartTime ) ) );
			trackList[ cCharacterMoveRunBackwardAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRunBackward, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, runStartTime ) ) );
			trackList[ cCharacterMoveRunLeftAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRunLeft, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, runStartTime ) ) );
			trackList[ cCharacterMoveRunRightAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRunRight, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, runStartTime ) ) );
			trackList[ cCharacterMoveRunLeftBackAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRunLeftBack, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, runStartTime ) ) );
			trackList[ cCharacterMoveRunRightBackAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRunRightBack, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, runStartTime ) ) );
			trackList[ cCharacterMoveRunAimUpAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRunAimUp, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, runStartTime ) ) );
			trackList[ cCharacterMoveRunAimDownAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRunAimDown, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, runStartTime ) ) );
			trackList[ cCharacterMoveIdleAimUpAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mIdleAimUp, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, idleStartTime ) ) );
			trackList[ cCharacterMoveIdleAimDownAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mIdleAimDown, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, idleStartTime ) ) );
			trackList[ cCharacterMoveIdleAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mIdle, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, idleStartTime ) ) );

			return tAnimTrackPtr( NEW tCharacterMoveAnimTrackFPS( prevTrack, trackList, desc->mBlendIn, desc->mBlendOut, desc->mTimeScale, desc->mBlendScale, desc->mCharacter ) );
		}

		static void fPushAnim( const tCharacterMoveFPSAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fPushTrack( fMakeTrack( desc, stack ) );
		}

		static void fPushAnimBeforeTag( const tCharacterMoveFPSAnimDesc* desc, tAnimatedSkeleton* stack, const tStringPtr& tag )
		{
			stack->fPushTrackBeforeTag( fMakeTrack( desc, stack ), tag );
		}
	}
	void tCharacterMoveFPSAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tCharacterMoveFPSAnimDesc,Sqrat::DefaultAllocator<tCharacterMoveFPSAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.GlobalFunc(_SC("PushBeforeTag"), &fPushAnimBeforeTag)
			.Var(_SC("RunForwardAnim"), &tCharacterMoveFPSAnimDesc::mRunForward)
			.Var(_SC("RunBackwardAnim"), &tCharacterMoveFPSAnimDesc::mRunBackward)
			.Var(_SC("RunLeftAnim"), &tCharacterMoveFPSAnimDesc::mRunLeft)
			.Var(_SC("RunRightAnim"), &tCharacterMoveFPSAnimDesc::mRunRight)
			.Var(_SC("RunLeftBackAnim"), &tCharacterMoveFPSAnimDesc::mRunLeftBack)
			.Var(_SC("RunRightBackAnim"), &tCharacterMoveFPSAnimDesc::mRunRightBack)
			.Var(_SC("IdleAimUpAnim"), &tCharacterMoveFPSAnimDesc::mIdleAimUp)
			.Var(_SC("IdleAimDownAnim"), &tCharacterMoveFPSAnimDesc::mIdleAimDown)
			.Var(_SC("RunAimUpAnim"), &tCharacterMoveFPSAnimDesc::mRunAimUp)
			.Var(_SC("RunAimDownAnim"), &tCharacterMoveFPSAnimDesc::mRunAimDown)
			.Var(_SC("IdleAnim"), &tCharacterMoveFPSAnimDesc::mIdle)
			.Var(_SC("SprintAnim"), &tCharacterMoveFPSAnimDesc::mSprint)

			.Var(_SC("BlendIn"), &tCharacterMoveFPSAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tCharacterMoveFPSAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tCharacterMoveFPSAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tCharacterMoveFPSAnimDesc::mBlendScale)
			.Var(_SC("Character"), &tCharacterMoveFPSAnimDesc::mCharacter)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("CharacterMoveTrackFPS"), classDesc );
	}
}
