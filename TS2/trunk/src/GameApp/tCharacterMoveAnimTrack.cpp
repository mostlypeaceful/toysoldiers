#include "GameAppPch.hpp"
#include "tCharacterMoveAnimTrack.hpp"
#include "tUserControllableCharacterLogic.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"
#include "Logic/tPhysical.hpp"

namespace Sig
{
	devvar( f32, Gameplay_Character_UserControl_IdleBlendSpring, 0.2f ); 
	devvar( f32, Gameplay_Character_UserControl_WalkBlendSpring, 0.1f );
	devvar( f32, Gameplay_Character_UserControl_RunThreshold, 0.75f );
	devvar( f32, Gameplay_Character_UserControl_WalkThreshold, 0.01f );

	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;


		enum tCharacterMoveTracks
		{
			cCharacterMoveWalkAnim = 0,
			cCharacterMoveRunAnim,
			cCharacterMoveIdleAnim,
			cCharacterMoveAnimCount
		};
	}

	tCharacterMoveAnimTrack::tCharacterMoveAnimTrack( 
		tBlendAnimTrack::tTrackList& tracksToBlend,
		const tCharacterMoveAnimDesc& desc )
		: tBlendAnimTrack( tracksToBlend, desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, 0.f, -1.f/*, tAnimTrack::cFlagPartial*/ )
		, mCharacter( desc.mCharacter )
		, mIdleBlend( 1.0f, 0.2f, 0.2f, 1.0f )
		, mWalkBlend( 1.0f, 0.2f, 0.2f, 1.0f )
		, mFirstTick( true )
	{
		mWalkToRunTimeRatio = desc.mWalk->mLengthOneShot / desc.mRun->mLengthOneShot;
		mCharacter->fSetDoAdvancedMovement( true );
	}

	void tCharacterMoveAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == cCharacterMoveAnimCount );

		const f32 speed = mCharacter->fGetSpeed( );
		const b32 idle = speed <= Gameplay_Character_UserControl_WalkThreshold;
		const b32 walking = speed <= Gameplay_Character_UserControl_RunThreshold;
		const f32 idleBlendTarget = idle ? 1.0f : 0.0f;
		const f32 walkBlendTarget = walking ? 1.0f : 0.0f;

		mIdleBlend.fSetBlends( Gameplay_Character_UserControl_IdleBlendSpring );
		mWalkBlend.fSetBlends( Gameplay_Character_UserControl_WalkBlendSpring );

		//if( !mFirstTick )
		{
			mIdleBlend.fStep( idleBlendTarget, dt );
			mWalkBlend.fStep( walkBlendTarget, dt );
		} 
		//else 
		//{
		//	mIdleBlend.fSetValue( idleBlendTarget );
		//	mWalkBlend.fSetValue( walkBlendTarget );
		//	mFirstTick = false;
		//}
		
		const f32 idleBlend = mIdleBlend.fValue( );
		const f32 moveBlend = 1.0f - idleBlend;
		const f32 walkWeight = mWalkBlend.fValue( );
		const f32 runBlend = 1.0f - walkWeight;

		const f32 runSpeed = idle ? 0.f : speed;
		const f32 walkSpeed = runSpeed * mWalkToRunTimeRatio;

		if( idle && mIdleBlend.fValue( ) > 0.98f )
		{
			mSubTracks[ cCharacterMoveWalkAnim ]->fSetCurrentTime( 0.f );
			mSubTracks[ cCharacterMoveRunAnim ]->fSetCurrentTime( 0.f );
		}

		mSubTracks[ cCharacterMoveWalkAnim ]->fSetTimeScale( walkSpeed );
		mSubTracks[ cCharacterMoveWalkAnim ]->fSetBlendScale( walkWeight * moveBlend );
		mSubTracks[ cCharacterMoveRunAnim ]->fSetTimeScale( runSpeed );
		mSubTracks[ cCharacterMoveRunAnim ]->fSetBlendScale( runBlend * moveBlend );
		mSubTracks[ cCharacterMoveIdleAnim ]->fSetBlendScale( idleBlend );

		tBlendAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
	}
}


namespace Sig
{
	namespace
	{
		tAnimTrackPtr fMakeTrack( const tCharacterMoveAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			sigassert( desc->mWalk && desc->mRun && desc->mIdle );
			f32 startTime = 0.f;

			//find any tracks with the name walk anim and sync time.
			for( s32 i = stack->fTrackCount( ) - 1; i >= 0 ; --i )
			{
				tCharacterMoveAnimTrack *track = stack->fTrack( i ).fDynamicCast< tCharacterMoveAnimTrack >( );
				if( track )
				{
					tKeyFrameAnimTrack *animTrack = track->fSubTracks( )[ cCharacterMoveWalkAnim ]->fDynamicCast< tKeyFrameAnimTrack >( );
					if( &animTrack->fAnim( ) == desc->mWalk )
					{
						startTime = animTrack->fCurrentTime( );
						break;
					}
				}
			}

			f32 walkToRunTimeRatio = desc->mWalk->mLengthOneShot / desc->mRun->mLengthOneShot;

			tBlendAnimTrack::tTrackList trackList;
			trackList.fNewArray( cCharacterMoveAnimCount );

			trackList[ cCharacterMoveWalkAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mWalk, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, startTime ) ) );
			trackList[ cCharacterMoveRunAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mRun, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, startTime / walkToRunTimeRatio ) ) );
			trackList[ cCharacterMoveIdleAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mIdle, 0.f, 0.f, 1.f, 1.f, 0.f, -1.f, startTime ) ) );


			return tAnimTrackPtr( NEW tCharacterMoveAnimTrack( trackList, *desc ) );
		}

		static void fPushAnim( const tCharacterMoveAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fPushTrack( fMakeTrack( desc, stack ) );
		}

		static void fPushAnimBeforeTag( const tCharacterMoveAnimDesc* desc, tAnimatedSkeleton* stack, const tStringPtr& tag )
		{
			stack->fPushTrackBeforeTag( fMakeTrack( desc, stack ), tag );
		}
	}
	void tCharacterMoveAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tCharacterMoveAnimDesc,Sqrat::DefaultAllocator<tCharacterMoveAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.GlobalFunc(_SC("PushBeforeTag"), &fPushAnimBeforeTag)
			.Var(_SC("WalkAnim"), &tCharacterMoveAnimDesc::mWalk)
			.Var(_SC("RunAnim"), &tCharacterMoveAnimDesc::mRun)
			.Var(_SC("IdleAnim"), &tCharacterMoveAnimDesc::mIdle)

			.Var(_SC("BlendIn"), &tCharacterMoveAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tCharacterMoveAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tCharacterMoveAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tCharacterMoveAnimDesc::mBlendScale)
			.Var(_SC("Character"), &tCharacterMoveAnimDesc::mCharacter)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("CharacterMoveTrack"), classDesc );
	}
}
