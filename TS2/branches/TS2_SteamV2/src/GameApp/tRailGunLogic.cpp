#include "GameAppPch.hpp"
#include "tWaveList.hpp"
#include "tWaveManager.hpp"
#include "tGeneratorLogic.hpp"
#include "tLevelLogic.hpp"
#include "tRailGunLogic.hpp"
#include "Wwise_IDs.h"
#include "AI/tSigAIGoal.hpp"

using namespace Sig::Math;
using namespace Sig::Physics;

namespace Sig
{

	tRailGunLogic::tRailGunLogic( )
		: mCurrentState( RailGunState::cDormant )
		, mStateTimer( 0.0f )
		, mSpeed( 0.0f )
		, mComingOutCount( 0 )
	//	, mSpawnTransportCount( 0 )
		, mGunPitch( 0.0f )
		, mGunYaw( 0.0f )
		, mGunTargetPitch( 0.0f )
		, mGunTargetYaw( 0.0f )
		, mGunMomentum( 0.0f )
		, mGunReloadTime( 0.0f )
		, mFireCount( 0 )
		, mReachedGoal( false )
		, mWhistleAudioTimer( 10.0f )
	{
		mTransportWaveList = NULL;
		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		if( levelLogic )
		{
			mTransportWaveList = levelLogic->fWaveManager()->fWaveList( tStringPtr( "TransportWaves" ) );
		}
		fEnterStateDormant( );
	}

	void tRailGunLogic::fActST( f32 dt )
	{
		tWheeledVehicleLogic::fActST( dt );

		switch( mCurrentState )
		{
		case RailGunState::cDormant:
			fUpdateStateDormant( dt );
			break;
		case RailGunState::cComingOut:
			fUpdateStateComingOut( dt );
			break;
		case RailGunState::cFiringRandomly:
			fUpdateStateFiringRandomly( dt );
			break;
		case RailGunState::cGoingIn:
			fUpdateStateGoingIn( dt );
			break;
		case RailGunState::cChargingGoal:
			fUpdateStateChargingGoal( dt );
			break;
		case RailGunState::cSpawnTransports:
			fUpdateStateSpawnTransports( dt );
			break;
		case RailGunState::cDead:
			fUpdateStateDead( dt );
			break;
		default:
			log_warning( 0, "Invalid RailGunState: " + mCurrentState );
			break;
		}
	}

	void tRailGunLogic::fEnterStateDormant( )
	{
		mStateTimer = 5.0f;
		mCurrentState = RailGunState::cDormant;
	}

	void tRailGunLogic::fUpdateStateDormant( f32 dt )
	{
		// Check if enough time has elapsed to go to the coming out state.
		mStateTimer -= dt;
		if( mStateTimer <= 0.0f )
		{
			fEnterStateComingOut( );
			return;
		}
	}

	void tRailGunLogic::fEnterStateComingOut( )
	{
		mCurrentState = RailGunState::cComingOut;
		mAudio->fHandleEvent( AK::EVENTS::PLAY_RAILGUN_MUSIC );
		mAudio->fHandleEvent( AK::EVENTS::PLAY_AUDIO_EVENT_RAILGUN_COME_OUT );
		mSpeed = 0.0f;
		mComingOutCount++;
		// See if we should just be charging the goal after coming out several times.
		if( mComingOutCount >= 4 )
		{
			fEnterStateChargingGoal( );
			return;
		}
	}

	void tRailGunLogic::fUpdateStateComingOut( f32 dt )
	{
		// Update our speed for this frame.
		mSpeed += 5.0f * dt;
		mSpeed = fMin( mSpeed, 14.0f );

		// We're traveling along the negative X axis.
		// Slow down after we reach x=+180.0, down to 0 at x=+155.0
		tMat3f xform = fOwnerEntity( )->fObjectToWorld( );
		tVec3f pos = xform.fGetTranslation( );
		f32 throttle = 1.0f;
		if( pos.x < 180.0f )
		{
			throttle = ( pos.x - 155.0f ) / ( 180.0f - 155.0f );

			// Check if we've come out far enough and it's time to fire.
			if( throttle <= 0.1f )
			{
				mAudio->fHandleEvent( AK::EVENTS::STOP_AUDIO_EVENT_RAILGUN_COME_OUT );
				fEnterStateFiringRandomly( );
				return;
			}
		}

		// Translate the entity along its forward vector.  Multiply in another
		// time delta here to make it behave more like constant acceleration.
		fOwnerEntity( )->fTranslate( xform.fZAxis( ) * mSpeed * throttle * dt );
	}

	void tRailGunLogic::fEnterStateFiringRandomly( )
	{
		mCurrentState = RailGunState::cFiringRandomly;
		mAudio->fHandleEvent( AK::EVENTS::PLAY_AUDIO_EVENT_RAILGUN_IDLE );

		// Set our initial aim target and current aim.
		mGunPitch = 0.0f;
		mGunYaw = 0.0f;
		mGunTargetPitch = tRandom::fSubjectiveRand( ).fFloatInRange( 15.0f, 35.0f );
		mGunTargetYaw = tRandom::fSubjectiveRand( ).fFloatInRange( -1.0f, -0.3f );
		mGunMomentum = 0.0f;
		mGunReloadTime = 0.0f;
		// Set the number of times we'll fire.
		mFireCount = 7;
		// Sets the amount of time we'll pause before rotating toward target aim.
		mStateTimer = 0.0f;
	}

	void tRailGunLogic::fUpdateStateFiringRandomly( f32 dt )
	{
		mGunReloadTime -= dt;
		mStateTimer -= dt;

		// Check if we're aiming the the right direction and we've waiting for reload.
		if( fEqual( mGunYaw, mGunTargetYaw, 0.5f ) && fEqual( mGunPitch, mGunTargetPitch, 0.5f ) && mGunReloadTime <= 0.0f )
		{
			// Play fire animation.
			//Anim.PushOneShot( entity, data.mainAnimPack[ "fire" ], 0.0 )

			// Adjust the projectile velocity to add to the randomness of targets.
			fWeaponStation( 0 )->fBank( 0 )->fWeapon( 0 )->fSetProjectileSpeed( tRandom::fSubjectiveRand( ).fFloatInRange( 80.0f, 100.0f ) );

			mAudio->fHandleEvent( AK::EVENTS::PLAY_AUDIO_EVENT_RAILGUN_FIRE );

			// Fire the weapon.
			fWeaponStation( 0 )->fBank( 0 )->fSetAIFireOverride( true );
			fWeaponStation( 0 )->fBank( 0 )->fFire( );
			fWeaponStation( 0 )->fBank( 0 )->fEndFire( );
			fWeaponStation( 0 )->fBank( 0 )->fSetAIFireOverride( false );
			fWeaponStation( 0 )->fBank( 0 )->fReload( );

			// Check if we've fired enough and it's time to go in.
			mFireCount--;
			if( mFireCount == 0 )
			{
				mAudio->fHandleEvent( AK::EVENTS::STOP_AUDIO_EVENT_RAILGUN_IDLE );
				fEnterStateGoingIn( );
				return;
			}

			// Pick a new target aim for the gun.
			mGunTargetPitch = tRandom::fSubjectiveRand( ).fFloatInRange( 15.0f, 35.0f );
			if( tRandom::fSubjectiveRand( ).fFloatZeroToOne( ) < 0.5 )
			{
				mGunTargetYaw = tRandom::fSubjectiveRand( ).fFloatInRange( 0.3f, 0.6f );
			}
			else
			{
				mGunTargetYaw = tRandom::fSubjectiveRand( ).fFloatInRange( -1.0f, -0.3f );
			}

			// Pause a little bit before we start moving towards new aim.
			mStateTimer = 0.33f;
			
			// Reset our momentum for the next aiming.
			mGunMomentum = 0.0f;

			// Reset our reload time.
			mGunReloadTime = tRandom::fSubjectiveRand( ).fFloatInRange( 1.0f, 2.0f );
		}

		// Check if we can start aiming.
		if( mStateTimer <= 0.0f )
		{
			// Add in time for our momentum value.
			mGunMomentum += dt;

			// Move the gun pitch.  We'll move faster the farther away from the target we are.
			f32 pitchDiff = mGunTargetPitch - mGunPitch;
			mGunPitch += pitchDiff * mGunMomentum * dt;
			
			// Apply gun pitch to anim blend.
			// Anim.SetBlendedAnimTrackBlend( entity, "blend_up", mGunPitch / 90.0f )

			// Move the gun yaw.  We'll move faster the farther away from the target we are.
			f32 yawDiff = mGunTargetYaw - mGunYaw;
			mGunYaw += yawDiff * mGunMomentum * dt;
			// Apply gun yaw to anim blend.
			if( mGunYaw < 0.0f )
			{
				// Anim.SetBlendedAnimTrackBlend( entity, "aim_left", Math.Abs( mGunYaw ) );
				// Anim.SetBlendedAnimTrackBlend( entity, "aim_right", 0.0 )
			}
			else
			{
				// Anim.SetBlendedAnimTrackBlend( entity, "aim_right", mGunYaw );
				// Anim.SetBlendedAnimTrackBlend( entity, "aim_left", 0.0 )
			}
		}
	}

	void tRailGunLogic::fEnterStateGoingIn( )
	{
		mCurrentState = RailGunState::cGoingIn;
		mAudio->fHandleEvent( AK::EVENTS::PLAY_AUDIO_EVENT_RAILGUN_HIDE );
		mSpeed = 0.0f;
	}

	void tRailGunLogic::fUpdateStateGoingIn( f32 dt )
	{
		// Move the pitch of the gun to its rest state.
		mGunPitch -= 0.33f * dt;
		mGunPitch = fMax( mGunPitch, 0.0f );
		// Apply gun pitch to anim blend.
		//Anim.SetBlendedAnimTrackBlend( entity, "blend_up", mGunPitch / 90.0f )


		// Move the yaw of the gun to its rest state.
		if( mGunYaw > 0.0f )
		{
			mGunYaw -= 0.33f * dt;
			mGunYaw = fMax( mGunYaw, 0.0f );

			//Anim.SetBlendedAnimTrackBlend( entity, "aim_right", mGunYaw )
			//Anim.SetBlendedAnimTrackBlend( entity, "aim_left", 0.0 )
		}
		else
		{
			mGunYaw += 0.33f * dt;
			mGunYaw = fMin( mGunYaw, 0.0f );

			//Anim.SetBlendedAnimTrackBlend( entity, "aim_left", Math.Abs( mGunYaw ) )
			//Anim.SetBlendedAnimTrackBlend( entity, "aim_right", 0.0 )
		}

		// Update our speed for this frame.
		mSpeed += 3.0f * dt;
		mSpeed = fMin( mSpeed, 15.0f );

		// We're traveling along the negative X axis.
		// Once we're in the tunnel, switch to spawning transports.
		tMat3f xform = fOwnerEntity( )->fObjectToWorld( );
		tVec3f pos = xform.fGetTranslation( );
		if( pos.x > 356.0f )
		{
			mAudio->fHandleEvent( AK::EVENTS::STOP_AUDIO_EVENT_RAILGUN_HIDE );
			fEnterStateSpawnTransports( );
			return;
		}

		// Translate the entity along its forward vector.  Multiply in another
		// time delta here to make it behave more like constant acceleration.
		// We're going backwards, so go in opposite dir of our zaxis.
		fOwnerEntity( )->fTranslate( -1.0f * xform.fZAxis( ) * mSpeed * dt );
	}

	void tRailGunLogic::fEnterStateSpawnTransports( )
	{
		mStateTimer = 0.0f;
		if( mTransportWaveList != NULL )
		{
			mTransportWaveList->fReset( );
			mTransportWaveList->fActivate( );
		}
		mCurrentState = RailGunState::cSpawnTransports;
	}

	void tRailGunLogic::fUpdateStateSpawnTransports( f32 dt )
	{
		//  If we don't have a valid transport list or we are finished launching, move to the next state
		if( ( mTransportWaveList == NULL ) || ( mTransportWaveList->fFinishedLaunching( ) ) )
		{
			fEnterStateDormant( );
		}
		else
		{
			mTransportWaveList->fUpdate( dt );
		}
	}

	void tRailGunLogic::fEnterStateChargingGoal( )
	{
		mCurrentState = RailGunState::cChargingGoal;
		mSpeed = 0.0f;
		mReachedGoal = false;
	}

	void tRailGunLogic::fUpdateStateChargingGoal( f32 dt )
	{
		// We're traveling along the negative X axis.
		// Slow down after we reach x=+180.0, down to 0 at x=+155.0
		tMat3f xform = fOwnerEntity( )->fObjectToWorld( );
		tVec3f pos = xform.fGetTranslation( );

		// Check if we're still not at the goal box.
		if( pos.x > -177.0f )
		{
			// Update our speed for this frame.
			mSpeed += 5.0f * dt;
			mSpeed = fMin( mSpeed, 34.0f );
		}
		// Reached goalbox.
		else
		{
			mAudio->fHandleEvent( AK::EVENTS::STOP_AUDIO_EVENT_RAILGUN_COME_OUT );
			// Reached the goal.
			if( !mReachedGoal )
			{
				// Call the level script function for the boss reaching the goal.
				Sqrat::Function f( tGameApp::fInstance( ).fCurrentLevelForScript( ), "OnBossReachedGoal" );
				if( !f.IsNull( ) )
					f.Execute( );

				mReachedGoal = true;
			}

			// Slow down.
			if( mSpeed > 0.0f )
			{
				mSpeed -= 15.0f * dt;
				mSpeed = fMax( mSpeed, 0.0f );
			}
		}
		mWhistleAudioTimer -= dt;
		if ( mWhistleAudioTimer < 0 )
		{
			mWhistleAudioTimer = tRandom::fSubjectiveRand( ).fFloatInRange( 1.0f, 3.0f );
			mAudio->fHandleEvent( AK::EVENTS::PLAY_AUDIO_EVENT_RAILGUNBOSS_WHISTLE_BLOW );
		}
		// Translate the entity along its forward vector.  Multiply in another
		// time delta here to make it behave more like constant acceleration.
		fOwnerEntity( )->fTranslate( xform.fZAxis( ) * mSpeed * dt );
	}

	void tRailGunLogic::fEnterStateDead( )
	{
		mCurrentState = RailGunState::cDead;
		mAudio->fHandleEvent( AK::EVENTS::STOP_AUDIO_EVENT_RAILGUN_COME_OUT );
		mAudio->fHandleEvent( AK::EVENTS::STOP_AUDIO_EVENT_RAILGUN_IDLE );
		mAudio->fHandleEvent( AK::EVENTS::STOP_AUDIO_EVENT_RAILGUN_HIDE );
		mAudio->fHandleEvent( AK::EVENTS::PLAY_AUDIO_EVENT_RAILGUN_DESTROYED );
	}

	void tRailGunLogic::fUpdateStateDead( f32 dt )
	{
		//  See Railgun.goaml for death logic
	}

	b32 tRailGunLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		if ( e.fEventId( ) == GameFlags::cEVENT_UNIT_DESTROYED )
			fEnterStateDead( );
		return tVehicleLogic::fHandleLogicEvent( e );
	}

	typedef AI::tDerivedLogicGoalHelper<tRailGunLogic> tRailgGunGoalHelper;

	class tRailGunGoal : public AI::tSigAIGoal, public tRailgGunGoalHelper
	{
		define_dynamic_cast(tRailGunGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fPersist( );
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
	};

	void tRailGunLogic::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tRailGunLogic, tWheeledVehicleLogic, Sqrat::NoCopy<tRailGunLogic> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("RailGunLogic"), classDesc);
		}

		{
			Sqrat::DerivedClass<tRailGunGoal, AI::tSigAIGoal, Sqrat::NoCopy<tRailGunGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("RailGunGoal"), classDesc);
		}
	}
}

