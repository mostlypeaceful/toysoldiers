#include "GameAppPch.hpp"
#include "tVehicleLogic.hpp"
#include "tGameApp.hpp"
#include "tSceneGraph.hpp"
#include "tLevelLogic.hpp"
#include "AI/tSigAIGoal.hpp"
#include "Input/tRumbleManager.hpp"
#include "tPlayer.hpp"
#include "tCharacterLogic.hpp"
#include "tBreakableLogic.hpp"
#include "Math/tIntersectionObbObb.hpp"
#include "Math/tIntersectionSphereObb.hpp"
#include "tRtsCamera.hpp"
#include "tUberBreakableLogic.hpp"
#include "tGameEffects.hpp"
#include "tReferenceFrameEntity.hpp"
#include "tWheeledVehicleLogic.hpp"
#include "tGoalBoxLogic.hpp"
#include "tOutOfBoundsIndicator.hpp"
#include "tSync.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;
using namespace Sig::Physics;

namespace Sig
{

	devvar( bool, AAACheats_DontDrainBattery, false );

	devvar( bool, Perf_VehicleProxyQuery, true );
	devvar( bool, Debug_Vehicle_DrawCollisionDetection, false );
	devvar( bool, Debug_Vehicle_Weapons_RenderTargets, false );
	devvar( bool, Gameplay_Vehicle_Pathing_AvoidCharacters, false );
	devvar( bool, Gameplay_Vehicle_ApplyTable, false ); //set true to constantly read values from table for tweaking

	devvar_clamp( f32, Gameplay_Vehicle_Weapons_TurretLowerLimit, -10.0f, -90.0f, 90.0f, 1 ); 
	devvar( f32, Gameplay_Vehicle_Weapons_TurretAngleDamp, 0.2f );
	devvar( f32, Gameplay_Vehicle_Passengers_CharacterEjectHorz, 4.0f );
	devvar( f32, Gameplay_Vehicle_Passengers_CharacterEjectVert, 4.0f ); 
	devvar( bool, Gameplay_Vehicle_ApplyControllerVibe, true ); 
	devvar( f32, Gameplay_Vehicle_ExpireTime, 15.0f ); 
	devvar( bool, Gameplay_Vehicle_ShowStats, false );
	devvar( f32, Gameplay_Vehicle_CargoCarrierDamageMod, 0.2f );


	devvar( f32, Gameplay_Vehicle_Audio_RPMLerp, 0.2f );
	devvar( f32, Gameplay_Vehicle_Audio_DriveLerp, 0.2f );
	devvar( f32, Gameplay_Vehicle_BoostTime, 1.0f );

	devvar( f32, Gameplay_Vehicle_TurretAcc, 2.0f );
	devvar( bool, Gameplay_Vehicle_TurretSrc, true );
	devvar( bool, AAACheats_Vehicle_SkipWaitTimer, false );
	devvar( f32, AAACheats_Vehicle_ShutDownGroundHeight, 2.5f );

	// Extra stuff
	devvar( f32, Gameplay_Vehicle_EnterDist, 1.5f );
	devvar( f32, Gameplay_Vehicle_DoorOpenAcc, 12.5f );

	f32 tVehicleLogic::fAudioRPMBlend( ) { return Gameplay_Vehicle_Audio_RPMLerp; }
	f32 tVehicleLogic::fAudioDriveBlend( ) { return Gameplay_Vehicle_Audio_DriveLerp; }


	namespace 
	{ 

		static const tStringPtr cBouncy( "bouncy" ); 			
		static const tStringPtr cBouncyDriver( "bouncyDriver" ); 
		static const tStringPtr cDriverLeftHand( "driverLeftHand" ); 		
		static const tStringPtr cDriverRightHand( "driverRightHand" ); 
		
		
		static const tStringPtr cHasExpiredLocKey( "Vehicle_Drained" );
		static const tStringPtr cExpiresLocKey( "Vehicle_Expires" );
		static const tStringPtr cWaitLocKey( "Vehicle_Wait" );
		static const tStringPtr cCargoDropName( "cargoDrop" );
		static const tStringPtr cExhaustSourceName( "exhaustSource" );

		static const tStringPtr cBossSmash( "BossSmash" );

		// Extra Stuff
		static const tStringPtr cEnterPoint( "enterPoint" );
		static const tStringPtr cSeatPoint( "seatPoint" );
		static const tStringPtr cDoor( "door" );
		static const tStringPtr cDoorHinge( "doorHinge" );
	}

	tVehicleLogic::tVehicleLogic( )
		: mFallingThrough( false )
		, mDeleteAfterFallThrough( false )
		, mDeleteMyEntity( false )
		, mExitedTheMap( false )
		, mEjectUser( false )
		, mStartingUp( false )
		, mDoCollisionTest( false )
		, mOversizeCollisionTest( false )
		, mDroppingEnabled( false )
		, mWaitingToLaunchCargo( false )
		, mFull3DPhysics( false )
		, mAICheckForCollisions( false )
		, mLockedToPathStart( false )
		, mHasDoneSpecialEntrance( false )
		, mDontInstaDestroyOutsideOfFlyBox( false )
		, mSlaveLinkTurrentChildren( false )
		, mDoMovementFX( false )
		, mDontRespawn( false )
		, mHasEverDroppedCargo( false )
		, mConstrainYaw( false )
		, mDropTimer( 0.f )
		, mSpeedWeHit( 0.0f )
		, mMass( 10.f )
		, mThrottleScale( 1.f )
		, mThrottleScaleTarget( 1.f )
		, mThrottleScaleTargetOverride( -1.f )
		, mThrottleBlend( 0.2f )
		, mPowerLevel( 1.f )
		, mBoostTimer( -1.f )
		, mVehicleControllerLogic( NULL )
		, mSlaveVehicleLogic( NULL )
	{
		mAnimatable.fSetLogic( this );
		fSetLogicType( GameFlags::cLOGIC_TYPE_VEHICLE );

		mUnitPath.fReset( NEW tUnitPath( ) );
		mUnitPath->fSetDesiredWaypointCnt( 2 );
	}

	b32 tVehicleLogic::fShowStats( ) const
	{
		return Gameplay_Vehicle_ShowStats;
	}

	void tVehicleLogic::fOnSpawn( )
	{

		log_assert( !fOwnerEntity( )->fObjectToWorld( ).fIsNan( ), "Nan transform!" );

		fOnPause( false );

		tUnitLogic::fOnSpawn( );

		mUnitPath->fSetOwnerEntity( fOwnerEntity( ) );

		fInitPassengers();

		mInitialTransform = fOwnerEntity( )->fObjectToWorld( );
		log_assert( !mInitialTransform.fIsNan( ), "Nan transform!" );

		fSetupVehicle( );
		fComputeCollisionShapeIfItDoesntExist( );

		// random debug color
		mColor = tRandom::fSubjectiveRand( ).fColor( 0.75f );

		fSetUseDefaultEndTransition( true );

		b32 hasSelectableFlag = fHasSelectionFlag( );
		if( hasSelectableFlag )
		{
			fEnableRTSCursorPulse( );
			tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
			levelLogic->fRegisterUseableUnit( fOwnerEntity( ) );
			fOwnerEntity( )->fReparent( *levelLogic->fOwnerEntity( ) );
		}
		else
			fAddRandomPickup( );

		// Collect turrets
		for( u32 s = 0; s < fWeaponStationCount( ); ++s )
		{
			for( u32 b = 0; b < fWeaponStation( s )->fBankCount( ); ++b )
			{
				for( u32 w = 0; w < fWeaponStation( s )->fBank( b )->fWeaponCount( ); ++w )
				{
					tWeaponPtr& weap = fWeaponStation( s )->fBank( b )->fWeapon( w );

					if( weap->fTurretEntity( ) )
					{
						tTurretData* data = mTurrets.fFind( weap->fTurretEntity( ) );
						if( !data )
						{
							mTurrets.fPushBack( tTurretData( ) );
							data = &mTurrets.fBack( );

							data->mEntity.fReset( weap->fTurretEntity( ) );
							data->mOriginalTurretTransform = data->mEntity->fParentRelative( );
						}

						data->mWeapons.fPushBack( weap );
					}
				}
			}
		}

		if( !gExtraMode ) mSeats.fSetCount( cVehicleSeatCount );

		tEntity* exhaustSourceParent = NULL;
		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			const tEntityPtr& child = fOwnerEntity( )->fChild( i );
			tEntity* childP = child.fGetRawPtr( );
			
			if( child->fName( ) == cCargoDropName )
				fAddCargoPoint( childP );
			else if( child->fName( ) == cExhaustSourceName )
				exhaustSourceParent = childP;
			else if( child->fName( ) == cEnterPoint )
				fValidateSeatArray( childP ).mEnterPoint = child;
			else if( child->fName( ) == cSeatPoint )
				fValidateSeatArray( childP ).mSeatPoint = child;
			else if( child->fName( ) == cDoor )
				fValidateSeatArray( childP ).mDoor = child;
			else if( child->fName( ) == cDoorHinge )
				fValidateSeatArray( childP ).mDoorHinge = child;
			else
			{
				// Need to do special check because some fx could be sigmls.
				tEntity* child = tReferenceFrameEntity::fSkipOverRefFrameEnts( fOwnerEntity( )->fChild( i ).fGetRawPtr( ) );
				u32 fx = child->fQueryEnumValue( GameFlags::cENUM_VEHICLE_MOTION_FX );
				if( fx != ~0 )
				{
					mDoMovementFX = true;
					FX::tAddPausedFxSystem cb( mMovementEffects[ fx ] );
					cb( *child );
					child->fForEachDescendent( cb );
				}
			}
		}

		for( u32 i = 0; i < mMovementEffects.fCount( ); ++i )
			mMovementEffects[ i ].fPause( false );

		if( mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_RUNNING ].fSystemCount( ) )
			mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_RUNNING ].fSetEmissionPercent( 0.f );

		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			tVehicleSeat& seat = mSeats[ i ];
			if( seat.mDoor && seat.mDoorHinge )
			{
				seat.mHinge = Physics::tOneWayHingePtr( NEW Physics::tOneWayHinge( seat.mDoorHinge->fParentRelative( ), seat.mDoor->fParentRelative( ) ) );
				seat.mHinge->fAutoLatch( 1.0f );
			}
		}

		if( exhaustSourceParent )
		{
			mExhaustSource.fReset( NEW Audio::tSource( "Vehicle Exhaust" ) );
			mExhaustSource->fSpawn( *exhaustSourceParent );
			mExhaustSource->fSetSwitch( tGameApp::cUnitIDSwitchGroup, fAudioID( ) );
			mAudio->fAddSwitchAndParamsLinkedChild( mExhaustSource.fGetRawPtr( ) );
		}
		else
		{
			mExhaustSource = mAudio;
		}

		fAddHealthBar( );
		
		fAddToMiniMap( );

		// mAnimatable.fOnSpawn( ); called from logic script

		if( hasSelectableFlag && !tGameApp::fInstance( ).fCurrentLevelLoadInfo( ).fChallengeVehicles( ) )
		{
			// kinda silly to create it just to destroy it but this is probably the only safe way
			fOwnerEntity( )->fDelete( );
		}
		else
		{
			if( mPowerLevel == 0.f )
				fShowWaitTimer( );	
		}

		if( fConstrainYaw( ) )
		{
			mConstrainYaw = true;
			fUpdateYawConstraint( true );
		}
	}
	void tVehicleLogic::fOnDelete( )
	{
		mAudio->fHandleEvent( AK::EVENTS::STOP_ENGINE );
		mExhaustSource->fHandleEvent( AK::EVENTS::STOP_EXHAUST );

		fReleaseTrenchedCargo( );

		// do important stuff first, like ditch goals
		tUnitLogic::fOnDelete( );
		mAnimatable.fOnDelete( );

		// Now deref pointers to things used by those
		mUnitPath.fRelease( );
		mPassengers.fResize( 0 );
		
		mCargo.fSetCount( 0 );
		mDroppingCargo = tVehicleCargo( );
		mCargoDropPts.fSetCount( 0 );
		mSeats.fSetCount( 0 );

		mIgnoreCollisionFrom.fSetCount( 0 );
		mOffenders.fSetCount( 0 );
		mAdditionalBreakablePtrs.fSetCount( 0 );

		mBoostEffects.fSafeCleanup( );
		for( u32 i = 0; i < mMovementEffects.fCount( ); ++i )
			mMovementEffects[ i ].fSafeCleanup( );

		mTurrets.fSetCount( 0 );

		mExpireTimer.fRelease( );
		mWaitTimer.fRelease( );
		mExhaustSource.fRelease( );

		mVehicleController.fRelease( );
		mVehicleControllerLogic = NULL;
		mSlaveVehicleLogic = NULL;

		mCargoSmokes.fSetCount( 0 );
		mPersistentEffect.fRelease( );

		mExplicitTargets.fSetCount( 0 );
		mPurchasedBy.fRelease( );
		mEnteredEnemyGoalBox.fRelease( );

		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		if( levelLogic )
			levelLogic->fUnregisterUseableUnit( fOwnerEntity( ) );

	}
	void tVehicleLogic::fOnSkeletonPropagated( )
	{
		tUnitLogic::fOnSkeletonPropagated( );
		mAnimatable.fListenForAnimEvents( *this );
	}
	tVehicleSeat& tVehicleLogic::fValidateSeatArray( tEntity* newData )
	{
		u32 index = newData->fQueryEnumValue( GameFlags::cENUM_EXTRA_SEAT_INDEX );
		sigassert( index != ~0 && "Vehicle seat needs an index!" );
		if( index >= mSeats.fCount( ) ) mSeats.fSetCount( index + 1 );
		return mSeats[ index ];
	}
	Physics::tOneWayHinge* tVehicleLogic::fDoorHingeForScript( u32 index ) const
	{
		if( index < mSeats.fCount( ) ) 
			return mSeats[ index ].mHinge.fGetRawPtr( );
		else 
			return NULL;
	}
	Gui::tRadialMenuPtr tVehicleLogic::fCreateSelectionRadialMenu( tPlayer& player )
	{
		if( !fCanBeUsed( ) ) 
			return Gui::tRadialMenuPtr( );

		Gui::tRadialMenuPtr radialMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptVehicleOptions ), player.fUser( ) ) );

		Sqrat::Object params( Sqrat::Table( ).SetValue( "Unit", this ).SetValue( "Player", &player ) );
		Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "DefaultSetup" ).Execute( params );

		return radialMenu;
	}

	void tVehicleLogic::fUpdateYawConstraint( b32 fullUpdate )
	{
		tVec3f axis = fOwnerEntity( )->fObjectToWorld( ).fZAxis( );

		//apply this constraint to weapons.
		for( u32 s = 0; s < fWeaponStationCount( ); ++s )
		{
			const tWeaponStationPtr& station = fWeaponStation( 0 );
			for( u32 b = 0; b < station->fBankCount( ); ++b )
			{
				const tWeaponBankPtr& bank = station->fBank( b );
				for( u32 w = 0; w < bank->fWeaponCount( ); ++w )
				{
					const tWeaponPtr& weap = bank->fWeapon( w );
					if( !fullUpdate )
						weap->fSetYawConstraint( axis );
					else
						weap->fSetYawConstraint( axis, fUnitAttributeQuadrantConstraintAngle( ) );
				}
			}
		}
	}

	void tVehicleLogic::fEjectAllPassengers( const Math::tVec3f& spawnVel )
	{
		for( u32 i = 0; i < mPassengers.fCount( ); ++i )
		{
			//mPassengers[ i ]->fReparent( *fOwnerEntity( )->fParent( ) );

			tVec3f randVel( 
				sync_rand( fFloatInRange( -Gameplay_Vehicle_Passengers_CharacterEjectHorz, Gameplay_Vehicle_Passengers_CharacterEjectHorz ) ), 
				sync_rand( fFloatInRange( 0, Gameplay_Vehicle_Passengers_CharacterEjectVert ) ),
				sync_rand( fFloatInRange( -Gameplay_Vehicle_Passengers_CharacterEjectHorz, Gameplay_Vehicle_Passengers_CharacterEjectHorz ) ) );
			fGetPassengerLogic( i )->fEject( randVel + spawnVel );
		}

		mPassengers.fResize( 0 );
	}

	u32 tVehicleLogic::fFindEmptySeat( const Math::tVec3f& myPos ) const
	{
		f32 minDist = Gameplay_Vehicle_EnterDist * Gameplay_Vehicle_EnterDist;
		u32 index = ~0;

		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			if( !mSeats[ i ].mOccupied )
			{
				sigassert( mSeats[ i ].mEnterPoint );
				f32 dist = (mSeats[ i ].mEnterPoint->fObjectToWorld( ).fGetTranslation( ) - myPos).fLengthSquared( );
				if( dist < minDist )
				{
					minDist = dist;
					index = i;
				}
			}
		}

		return index;
	}

	tVehicleSeat::tVehicleSeat( ) 
		: mOccupied( false ) 
	{ 
	}

	void tVehicleSeat::fStep( f32 dt )
	{
		if( mHinge )
		{
			const f32 cThresh = 0.05f;
			f32 direction = mHinge->fLowerLimit( ) < 0.f ? -1.f : 1.f;
			
			if( mOpeningDoor )
			{
				mHinge->fUnLatch( );
				mHinge->fSetAngleVel( mHinge->fAngleVel( ) + direction * Gameplay_Vehicle_DoorOpenAcc * dt );
				if( fAbs( mHinge->fAngle( ) ) > fAbs( mHinge->fLowerLimit( ) - mHinge->fUpperLimit( ) ) - cThresh )
				{
					// at extent
					mOpeningDoor = false;
				}
			}
			else if( mClosingDoor )
			{
				mHinge->fSetAngleVel( mHinge->fAngleVel( ) + direction * -Gameplay_Vehicle_DoorOpenAcc * dt );
				if( fAbs( mHinge->fAngle( ) ) < cThresh )
				{
					// closed
					mClosingDoor = false;
					mHinge->fAutoLatch( 4.f );
				}
			}
		}
	}

	void tVehicleSeat::fOpenCloseDoor( b32 open )
	{
		if( open )
		{
			mOpeningDoor = true;
			mClosingDoor = false;
		}
		else
		{
			mClosingDoor = true;
			mOpeningDoor = false;
		}
	}

	b32 tVehicleLogic::fTryToUse( tPlayer* player, u32 seat )
	{
		if( !fSeatOccupied( seat ) )
		{
			if( fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_CONTROL_BEGIN ) ) )
			{
				fOccupySeat( player, seat );
				return true;
			}
		}

		return false;
	}



	Sig::b32 tVehicleLogic::fCanBeUsed( )
	{
		if( (!AAACheats_Vehicle_SkipWaitTimer && mWaitTimer && !mWaitTimer->fFinished( )) || fIsDestroyed( ) ) 
			return false;

		return !fSeatOccupied( cVehicleDriverSeat );

		// no longer co-op able
		//for( u32 i = 0; i < mSeats.fCount( ); ++i )
		//{
		//	if( !fSeatOccupied( i ) )
		//		return true;
		//}
		//
		// return false;
	}

	b32 tVehicleLogic::fTryToUse( tPlayer* player )
	{
		if( !fCanBeUsed( ) )
			return false;

		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			if( !fSeatOccupied( i ) )
			{
				if( fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_CONTROL_BEGIN ) ) )
				{
					fOccupySeat( player, i );
					return true;
				}
			}
		}

		return false;
	}

	b32 tVehicleLogic::fEndUse( tPlayer& player )
	{
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			if( fGetPlayer( i ) == &player )
			{
				fVacateSeat( (tVehicleSeatName) i );
				
				if( !tGameApp::fExtraDemoMode( ) && i == cVehicleDriverSeat && fSeatOccupied( cVehicleGunnerSeat ) )
				{
					fSwitchSeats( cVehicleGunnerSeat, cVehicleDriverSeat );
				}

				b32 noUsers = !fUnderUserControl( );
				return noUsers;
			}
		}

		sigassert( 0 ); // should not get here!
		return false;
	}

	u32 tVehicleLogic::fSeatIndex( const tPlayer* player ) const
	{
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
			if( fGetPlayer( i ) == player )
				return i;

		return ~0;
	}

	void tVehicleLogic::fSwitchSeats( u32 fromSeat, u32 toSeat )
	{
		// switch gunner to main player
		tPlayer* player = fGetPlayer( fromSeat );
		sigassert( player );
		sigassert( !fSeatOccupied( toSeat ) );
		fVacateSeat( fromSeat );
		fOccupySeat( player, toSeat );
	}

	void tVehicleLogic::fVacateAllPlayers( )
	{
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			if( fSeatOccupied( i ) && fEndUse( *fGetPlayer( i ) ) )
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_USER_CONTROL_END ) );
		}
	}

	void tVehicleLogic::fOccupySeat( tPlayer* player, u32 seat )
	{
		mSeats[ seat ].mOccupied = true;
		mSeats[ seat ].mPlayer.fReset( player );
		fCheckAndSetSelectionEnable( );

		if( seat == cVehicleDriverSeat )
		{
			// activate all weapons
			for( u32 i = 0; i < mSeats.fCount( ); ++i )
				if( fHasWeaponStation( i ) )
					fWeaponStation( i )->fBeginUserControl( player );

			fSetTeamPlayer( player );
		}
		else if( seat == cVehicleGunnerSeat && fHasWeaponStation( cVehicleGunnerSeat ) )
		{
			// switch first station over to second user
			fWeaponStation( cVehicleGunnerSeat )->fEndUserControl( );
			fWeaponStation( cVehicleGunnerSeat )->fBeginUserControl( player );
		}

		// Set player, and then camera
		sigassert( player );
		fPushCamera( player, seat );

		if( mExpireTimer )
		{
			mExpireTimer->fDelete( );
			mExpireTimer.fRelease( );
		}

		if( !player->fCurrentBarrage( ).fIsNull( ) )
			player->fCurrentBarrage( ).fCodeObject( )->fEnteredBarrageUnit( this, true, player );
	}

	void tVehicleLogic::fVacateSeat( u32 seat )
	{
		mSeats[ seat ].mOccupied = false;
		tPlayerPtr& player = mSeats[ seat ].mPlayer;
		sigassert( player );

		if( !player->fCurrentBarrage( ).fIsNull( ) )
			player->fCurrentBarrage( ).fCodeObject( )->fEnteredBarrageUnit( this, false, player.fGetRawPtr( ) );

		if( player->fBToExitIndicator( ) )
			player->fBToExitIndicator( )->fShow( false );

		if( seat == cVehicleDriverSeat )
		{
			// deactivate all weapons
			for( u32 i = 0; i < mSeats.fCount( ); ++i )
				if( fHasWeaponStation( i ) )
					fWeaponStation( i )->fEndUserControl( );

			if( !mExpireTimer && !mWaitTimer && !mDontInstaDestroyOutsideOfFlyBox && !fIsDestroyed( ) )
			{
				mExpireTimer.fReset( NEW Gui::tHoverTimer( cExpiresLocKey, Gameplay_Vehicle_ExpireTime, false, fTeam( ) ) ); 
				mExpireTimer->fSpawn( *fOwnerEntity( ) );
			}

			fSetTeamPlayer( NULL );
		}
		else if( seat == cVehicleGunnerSeat && fHasWeaponStation( cVehicleGunnerSeat ) )
		{
			// switch second station back to first user, if he's still around
			fWeaponStation( cVehicleGunnerSeat )->fEndUserControl( );

			if( fSeatOccupied( cVehicleDriverSeat ) )
				fWeaponStation( cVehicleGunnerSeat )->fBeginUserControl( fGetPlayer( cVehicleDriverSeat ) );
		}

		// Set player, and then camera
		fResetTurretBlendData( );
		fPopCamera( player.fGetRawPtr( ) );
		player.fRelease( );

		fCheckAndSetSelectionEnable( );
	}

	b32 tVehicleLogic::fChargeIfNotCharging( )
	{
		if( !fUnderUserControl( ) && !mWaitTimer && mPowerLevel < 0.99f )
		{
			fShowWaitTimer( );
			return true;
		}

		return false;
	}

	void tVehicleLogic::fResetTurretBlendData( )
	{
		for( u32 i = 0; i < mTurrets.fCount( ); ++i )
		{
			mTurrets[ i ].mTurretYawBlend.fSetValue( mTurrets[ i ].mLastSetYaw );
			mTurrets[ i ].mTurretYawBlend.fSetVelocity( 0 );
		}
	}

	void tVehicleLogic::fShowInUseIndicator( Gui::tInUseIndicator* indicator )
	{
		if( indicator ) 
		{
			if( fSeatOccupied( cVehicleDriverSeat ) && fSeatOccupied( cVehicleGunnerSeat ) )
				indicator->fShow( );
			else if( fSeatOccupied( cVehicleDriverSeat ) )
				indicator->fShow( fGetPlayer( cVehicleDriverSeat )->fUser( ) );
			else if( fSeatOccupied( cVehicleGunnerSeat ) )
				indicator->fShow( fGetPlayer( cVehicleGunnerSeat )->fUser( ) );
			else
				indicator->fHide( );
		}
	}

	void tVehicleLogic::fAddRumbleEvent( const Input::tRumbleEvent& event )
	{
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			tPlayer* player = fGetPlayer( i );
			if( player ) player->fGamepad( ).fRumble( ).fAddEvent( event );
		}
	}

	void tVehicleLogic::fSetExplicitRumble( f32 rumble )
	{
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			tPlayer* player = fGetPlayer( i );
			if( player ) player->fGamepad( ).fRumble( ).fSetExplicitRumble( rumble );
		}
	}

	void tVehicleLogic::fCheckAndSetSelectionEnable( )
	{
		b32 hasOccupants = false;
		b32 vacancies = false;

		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			if( fSeatOccupied( i ) )
				hasOccupants = true;
			else 
				vacancies = true;
		}

		// no longer co-opable
		//fEnableSelection( vacancies );
		fEnableSelection( !hasOccupants );
		fSetUnderUserControl( hasOccupants );	
	}

	// index = GameFlags::cCARGO_INDEX_COUNT is where the ones with no index go.
	void tVehicleLogic::fAddCargoPoint( tEntity* e )
	{
		u32 index = e->fQueryEnumValue( GameFlags::cENUM_CARGO_INDEX, GameFlags::cCARGO_INDEX_COUNT );

		if( index >= mCargoDropPts.fCount( ) )
			mCargoDropPts.fSetCount( GameFlags::cCARGO_INDEX_COUNT + 1 );

		mCargoDropPts[ index ].fPushBack( tEntityPtr( e ) );
	}

	tGrowableArray<tEntityPtr>* tVehicleLogic::fCargoPoints( u32 index )
	{
		if( index < mCargoDropPts.fCount( ) && mCargoDropPts[ index ].fCount( ) > 0 )
			return &mCargoDropPts[ index ];
		else if( GameFlags::cCARGO_INDEX_COUNT < mCargoDropPts.fCount( ) && mCargoDropPts[ GameFlags::cCARGO_INDEX_COUNT ].fCount( ) > 0 )
			return &mCargoDropPts[ GameFlags::cCARGO_INDEX_COUNT ];
		else
		{
			log_warning( 0, "No cargo point found index: " << index );
			return NULL;
		}
	}

	void tVehicleLogic::fComputeUserInput( f32 dt )
	{
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			if( fSeatOccupied( i ) && fHasWeaponStation( i ) )
			{
				tWeaponStationPtr& station = fWeaponStation( i );

				if( station->fWantsAdvanceTargeting( ) )
					station->fProcessAdvancedTargetting( );
				station->fPitchTowardsDesiredAngle( dt );
				station->fProcessUserInput( );
			}
		}

		// Debug vehicle respawning
		//tPlayer* driver = fGetPlayer( cVehicleDriverSeat );
		//if( driver )
		//{			
		//	if( driver->fGamepad( ).fButtonHeld( Input::tGamepad::cButtonB ) ) 
		//		mRespawn = true; // reset
		//}

		fCommonInput( );
	}

	void tVehicleLogic::fComputeAIInput( f32 dt )
	{
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			if( fHasWeaponStation( i ) )
			{
				tWeaponStationPtr& station = fWeaponStation( i );
				if( station->fShouldAcquire( ) )
				{
					for( u32 b = 0; b < station->fBankCount( ); ++b )
					{
						tWeaponBankPtr& bank = station->fBank( b );
						if( bank->fFiring( ) )
						{
							if( !bank->fShouldFire( ) )
								bank->fEndFire( );

							if( bank->fNeedsReload( ) )
							{
								bank->fEndFire( );
								bank->fReloadAfterTimer( );
							}
						}
						else
						{
							if( bank->fNeedsReload( ) )
								bank->fReloadAfterTimer( );

							if( bank->fShouldFire( ) )
							{
								bank->fFire( );

								if( bank->fWantsLocks( ) )
								{
									//if we want locks and have targets, add a lock
									const tRingBuffer<tEntityPtr>& lockTargs = bank->fWeapon( 0 )->fAdditionalLockTargets( );

									for( u32 i = 0; i < lockTargs.fNumItems( ) && bank->fAcquireLocks( ); ++i )
									{
										const tEntityPtr& target = lockTargs[ i ];
										sigassert( target );
										bank->fAddLock( tWeaponTargetPtr( NEW tWeaponTarget( (u32)target, target, NULL, GameFlags::cUNIT_TYPE_NONE, tVec2f::cZeroVector ) ) );
									}

									bank->fEndFire( );
								}
							}
						}
					}
				}
			}
		}

		fCommonInput( );
	}

	void tVehicleLogic::fCommonInput( )
	{
		tFireEventList events;
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			if( fHasWeaponStation( i ) )
				fWeaponStation( i )->fAccumulateFireEvents( events );
		}

		if( events.fCount( ) )
		{
			if( events[ 0 ].mWeapon->fDesc( ).mBarrageWeapon )
			{
				tDynamicArray<tPlayerPtr>& players = tGameApp::fInstance( ).fPlayers( );
				for( u32 i = 0; i < players.fCount( ); ++i )
				{
					if( players[ i ]->fBarrageController( )->fBarrageActive( ) )
					{
						const tBarragePtr& b = players[ i ]->fCurrentBarrage( );
						sigassert( !b.fIsNull( ) );
						b.fCodeObject( )->fUsedBarrageUnit( this, events.fFront( ).mProjectile.fGetRawPtr( ), players[ i ].fGetRawPtr( ) );
					}
				}

				if( fUnitType( ) == GameFlags::cUNIT_TYPE_AIR && tPlayer::fIsBomber( fUnitID( ) ) && mWave && !mWave->fPrompted( ) )
				{
					mWave->fPrompt( this );
				}
			}

			for( u32 e = 0; e < events.fCount( ); ++e )
				fReactToWeaponFire( events[ e ] );

			if( Gameplay_Vehicle_ApplyControllerVibe )
			{
				for( u32 i = 0; i < mSeats.fCount( ); ++i )
				{
					tPlayer* player = fGetPlayer( i );
					if( player )
					{
						for( u32 e = 0; e < events.fCount( ); ++e )
							tWeaponStation::fApplyRumbleEvent( events[ e ], *player );
					}
				}
			}
		}
	}

	b32 tVehicleLogic::fBoostInput( b32 skipBoostFx )
	{
		if( fSeatOccupied( cVehicleDriverSeat ) && fGetPlayer( cVehicleDriverSeat )->fGamepad( ).fButtonDown( Input::tGamepad::cButtonA ) && mBoostEffect.fExists( ) )
		{
			mAudio->fHandleEvent( AK::EVENTS::EXTRA_BOOST );

			mBoostTimer = Gameplay_Vehicle_BoostTime;
			b32 skipEffect = mBoostEffects.fSystemCount( ) > 0 || skipBoostFx;

			tEffectArgs args;
			args.mDontSpawnEffect = skipEffect;
			tEntity* effect = tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), mBoostEffect, args );

			if( skipEffect )
				mBoostEffects.fReset( );
			else if( effect )
			{
				effect->fForEachDescendent( FX::tAddPausedFxSystem( mBoostEffects ) );
				mBoostEffects.fPause( false );
			}

			return true;
		}

		return false;
	}

	tVehiclePassengerLogic* tVehicleLogic::fGetPassengerLogic( u32 i )
	{
		return mPassengers[ i ]->fLogicDerivedStaticCast<tVehiclePassengerLogic>( );
	}

	void tVehicleLogic::fInitPassengers( ) 
	{
		// find bouncy attachements first
		tGrowableArray<tVec3f> bouncies;
		s32 driveIndex = -1;

		tEntity* leftHand = NULL;
		tEntity* rightHand = NULL;

		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			tEntity* child = fOwnerEntity( )->fChild( i ).fGetRawPtr( );
			if( child->fName( ) == cBouncyDriver )
			{
				driveIndex = bouncies.fCount( );
				bouncies.fPushBack( child->fParentRelative( ).fGetTranslation( ) );
			}
			else if( child->fName( ) == cBouncy )
				bouncies.fPushBack( child->fParentRelative( ).fGetTranslation( ) );
			else if( child->fName( ) == cDriverLeftHand )
				leftHand = child;
			else if( child->fName( ) == cDriverRightHand )
				rightHand = child;
		}


		//find passengers
		tGrowableArray<tEntityPtr> passengers;
		fCollectSoldiers( passengers );

		for( u32 p = 0; p < passengers.fCount( ); ++p )
		{
			// find closest bouncy
			s32 bID = -1; 
			f32 dist = cInfinity;
			for( u32 b = 0; b < bouncies.fCount( ); ++b )
			{
				f32 tDist = (bouncies[b] - passengers[p]->fParentRelative( ).fGetTranslation( )).fLengthSquared( );
				if( tDist < dist ) 
				{ 
					dist = tDist;
					bID = b;
				}
			}

			tVec3f closestB = bID > -1 ? bouncies[bID] : tVec3f::cZeroVector;

			tVehiclePassengerLogic* pl = passengers[ p ]->fLogicDerived< tVehiclePassengerLogic >( );
			if( pl )
			{
				pl->fGenerateRandomAnimEvents( true );
				pl->fSetup( closestB, passengers[p]->fParentRelative( ), fOwnerEntity( )->fObjectToWorld( ) );
				if( bID == driveIndex )
					pl->fSetTargets( leftHand, rightHand );

				mPassengers.fPushBack( passengers[ p ] );
				if( tGameApp::fExtraDemoMode( ) )
					pl->fSprungMass( ).fSetVerticalRange( 0.4f, 0.4f );
			}
		}
	}

	namespace
	{
		enum tCargoTableColumns
		{
			cCargoTableSigml,
			cCargoTableCount,
			cCargoTableSpawnRate,
			cCargoTableStopWhileSpawning,
			cCargoTableRemoveAfterSpawn,
			cCargoTableParentRelative,
			cCargoTableParachuting,
			cCargoTableDisablePhysics,
			cCargoTableFocusPrompt
		};
	}

	void tVehicleLogic::fAddCargo( const tStringPtr& cargoName )
	{
		const tStringHashDataTable& dt = tGameApp::fInstance( ).fCargoTable( );
		u32 rowIndex = dt.fRowIndex( cargoName );
		if( rowIndex != ~0 )
		{
			tStringPtr path =				dt.fIndexByRowCol<tStringPtr>( rowIndex, cCargoTableSigml );
			u32 count =	(u32)				dt.fIndexByRowCol<f32>( rowIndex, cCargoTableCount );
			f32 spawnRate =					dt.fIndexByRowCol<f32>( rowIndex, cCargoTableSpawnRate );
			b32 stopWhileSpawning = (b32)	dt.fIndexByRowCol<f32>( rowIndex, cCargoTableStopWhileSpawning );
			b32 removeCargoAfterDropping = (b32)dt.fIndexByRowCol<f32>( rowIndex, cCargoTableRemoveAfterSpawn );
			b32 parentRelative			 = (b32)dt.fIndexByRowCol<f32>( rowIndex, cCargoTableParentRelative );
			b32 parachuting				 = (b32)dt.fIndexByRowCol<f32>( rowIndex, cCargoTableParachuting );
			b32 disablePhysics			 = (b32)dt.fIndexByRowCol<f32>( rowIndex, cCargoTableDisablePhysics );
			tStringPtr focusPrompt		 = dt.fIndexByRowCol<tStringPtr>( rowIndex, cCargoTableFocusPrompt );

			mCargo.fPushBack( tVehicleCargo( tFilePathPtr( path ), count, spawnRate, stopWhileSpawning, removeCargoAfterDropping, parentRelative, parachuting, disablePhysics, focusPrompt ) );
		}

		if( fUnitType( ) != GameFlags::cUNIT_TYPE_BOSS )
			mDamageModifier = Gameplay_Vehicle_CargoCarrierDamageMod;
	}

	void tVehicleLogic::fAddSimpleCargo( u32 unitID, u32 count, f32 rate )
	{
		mCargo.fPushBack( tVehicleCargo( tGameApp::fInstance( ).fUnitResourcePath( unitID, fCountry( ) ), count, rate, false, true, false, false, false, tStringPtr::cNullPtr ) );
	}

	void tVehicleLogic::fDropCargo( u32 index )
	{
		//default to first cargo
		if( index >= mCargo.fCount( ) ) 
			index = 0;

		if( mCargo.fCount( ) > index )
		{
			mHasEverDroppedCargo = true;
			mDroppingCargo = mCargo[ index ];
			mDroppingCargo.mIndex = index;

			if( mDroppingCargo.mCount > 0 )
			{
				tGrowableArray<tEntityPtr>* list = fCargoPoints( index );
				if( list )
				{
					fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CARGO_DROP_BEGIN ) );
					
					tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
					if( level && tPlayer::fIsAPC( fUnitID( ) ) )
						level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_APC_BEGIN_UNLOAD ) );

					for( u32 i = 0; i < list->fCount( ); ++i )
					{
						tEntityPtr& dropPt = (*list)[ i ];
						sigassert( dropPt );
						if( dropPt->fLogic( ) )
							dropPt->fLogic( )->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CARGO_DROP_BEGIN ) );
						

						if( mDroppingCargo.mRemoveCargoAfterDropped )
							mCargo[ index ].mCount = 0; //this will invalidate it but keep its index position

						mDroppingCargo.mTotalCount = mDroppingCargo.mCount;
						mDroppingCargo.mTrenchedUnits.fSetCount( 0 );
						mDropTimer = mDroppingCargo.mSpawnRate;
						mWaitingToLaunchCargo = false;
					}
				}
			}
		}
	}

	void tVehicleLogic::fCancelCargoDrop( )
	{
		fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CARGO_DROP_END ) );
		tGrowableArray<tEntityPtr>* list = fCargoPoints( mDroppingCargo.mIndex );
		if( list )
		{
			for( u32 i = 0; i < list->fCount( ); ++i )
			{
				tEntityPtr& dropPt = (*list)[ i ];
				if( dropPt && dropPt->fLogic( ) )
					dropPt->fLogic( )->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CARGO_DROP_END ) );
			}
		}

		mDroppingCargo.mCount = 0;
		mDroppingEnabled = false;

		if( mDroppingCargo.mPathPt )
		{
			for( u32 i = 0; i < mCargoSmokes.fCount( ); ++i )
				if( mCargoSmokes[ i ]->mFrom == mDroppingCargo.mPathPt )
				{
					mCargoSmokes.fErase( i );
					break;
				}

			mDroppingCargo.mPathPt = NULL;
		}

		fReleaseTrenchedCargo( );

		mDamageModifier = 1.f;
	}

	void tVehicleLogic::fReleaseTrenchedCargo( )
	{
		for( u32 i = 0; i < mDroppingCargo.mTrenchedUnits.fCount( ); ++i )
		{
			tUnitLogic* ul = mDroppingCargo.mTrenchedUnits[ i ]->fLogicDerived<tUnitLogic>( );
			if( ul && ul->fUnitPath( ) )
			{
				ul->fUnitPath( )->fClearTargetOverride( );
				ul->fUnitPath( )->fAdvancePathSequence( );
			}
		}
		mDroppingCargo.mTrenchedUnits.fSetCount( 0 );
		mWaitingToLaunchCargo = false;
	}
	
	b32 tVehicleLogic::fPathPointReached( tPathEntity& point, const Logic::tEventContextPtr& context )
	{
		if( mVehicleControllerLogic && mVehicleControllerLogic->fPathPointReached( point, context ) )
			return true;

		if( point.fHasGameTagsAny( tEntityTagMask( GameFlags::cFLAG_DROP_CARGO ) ) )
		{
			fDropCargo( point.fQueryEnumValue( GameFlags::cENUM_CARGO_INDEX, GameFlags::cCARGO_INDEX_COUNT ) );
			mDroppingCargo.mPathPt = &point;
		}

		if( point.fHasGameTagsAny( tEntityTagMask( GameFlags::cFLAG_FOCAL_PROMPT ) ) )
		{
			if( mWave )
				mWave->fPrompt( this );
		}

		if( point.fHasGameTagsAny( tEntityTagMask( GameFlags::cFLAG_PARK_VEHICLE ) ) )
		{
			if( fHasSelectionFlag( ) )
				fEnableSelection( true );
			mInitialTransform = fOwnerEntity( )->fObjectToWorld( );
			mUnitPath->fClearPathStarts( );
			mUnitPath->fClearPath( );
			mUnitPath->fPausePathSequence( );
		}

		return fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_WAYPOINT_REACHED, context ) );
	}

	void tVehicleLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListAnimateMT );
			fRunListRemove( cRunListCollideMT );
			fRunListRemove( cRunListPhysicsMT );
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListThinkST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListAnimateMT );
			fRunListInsert( cRunListCollideMT );
			fRunListInsert( cRunListPhysicsMT );
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListThinkST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}

	void tVehicleLogic::fActST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicActST );

		tUnitLogic::fActST( dt );

		if( Gameplay_Vehicle_ApplyTable )
			fReapplyTable( );

		fClearContacts( );

		mBreakablesWeHit.fSetCount( 0 );
		mAdditionalBreakablePtrs.fSetCount( 0 );

		if( mExitedTheMap )
		{
			mExitedTheMap = false;
			mFallingThrough = false;

			if( mDeleteAfterFallThrough || !fHasSelectionFlag( ) )
				mDeleteMyEntity = true;
			else
			{
				if( tGameApp::fExtraDemoMode( ) )
					fRespawn( mInitialTransform );
				else
				{
					log_warning( 0, GameFlags::fLOGIC_TYPEEnumToString( fLogicType( ) ) << " vehicle exited the map. " << (mDestroyedByPlayer ? GameFlags::fTEAMEnumToValueString( mDestroyedByPlayer->fTeam( ) ) : tStringPtr( "Deleting" ) ) );

					if( mLevelEvents )
						mLevelEvents->fFire( GameFlags::cLEVEL_EVENT_UNIT_DESTROYED, this );

					if( mDestroyedByPlayer )
						fDestroy( mDestroyedByPlayer, true );
					else
						fDestroy( true );
				}
			}
		}
	}
	
	void tVehicleLogic::fCollideMT( f32 dt )
	{
		fStepTintStack( dt );

		profile( cProfilePerfVehicleLogicCollision );

		// keep vehicle inside arena
		tVec3f pos = fCurrentTransformMT( ).fGetTranslation( );
		const tAabbf &bounds = tGameApp::fInstance( ).fCurrentLevel( )->fLevelBounds( );

		bool leftMap = false;

		if( pos.x > bounds.mMax.x ) { pos.x -= bounds.mMax.x; leftMap = true; }
		else if( pos.x < bounds.mMin.x ) { pos.x -= bounds.mMin.x; leftMap = true; }

		if( pos.z > bounds.mMax.z ) { pos.z -= bounds.mMax.z; leftMap = true; }
		else if( pos.z < bounds.mMin.z ) { pos.z -= bounds.mMin.z; leftMap = true; }

		if( pos.y < bounds.mMin.y ) { pos.y -= bounds.mMin.y; leftMap = true; }

		mExitedTheMap = leftMap;

		// regular collision detection
		if( mDoCollisionTest ) 
			fFindCollisionMT( dt );
	}

	void tVehicleLogic::fFindCollisionMT( f32 dt )
	{
		const tMat3f& transform = fCurrentTransformMT( );
		tMat3f transformInv = transform.fInverse( );

		const tVec3f& vel = fQueryPhysicalDerived<Physics::tStandardPhysics>( )->fVelocity( );
		const tVec3f localVel = transformInv.fXformVector( vel );
		f32 speed = localVel.fLength( );

		const tVec3f localDisplacement = localVel * dt;
		tAabbf futureBounds = mCollisionBounds.fTranslate( localDisplacement );
		tObbf myOBB( futureBounds, transform );

		// only look for shape entities
		tProximity proximity;

		if( mAICheckForCollisions || fUnderUserControl( ) || gExtraMode )
		{
			profile( cProfilePerfVehicleLogicCollisionQuery );
			tDynamicArray<u32> spatialSetIndices( 1 );
			spatialSetIndices[ 0 ] = tShapeEntity::cSpatialSetIndex;
			proximity.fSetSpatialSetIndices( spatialSetIndices );
			proximity.fFilter( ).fAddTag( GameFlags::cFLAG_COLLISION );

			if( Perf_VehicleProxyQuery )
			{
				// this primitive is used for colliding with object, such as character
				tAabbf tightBounds = futureBounds.fInflate( 0.25f ); //little buffer space
				proximity.fAddObb( tightBounds );

				//fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tObbf( tightBounds, transform ), tVec4f(0,1,0,0.5f) );

				// this primitive is used for slowdown and avoidance

				if( mOversizeCollisionTest && !fEqual( speed, 0.0f ) && !fUnderUserControl( ) )
				{
					f32 radius = fMax( fAbs(speed), 10.0f );
					tSpheref broad( tVec3f(0,0, mCollisionBounds.mMax.z + radius * 0.25f), radius );
					proximity.fAddSphere( broad );

					if( Debug_Vehicle_DrawCollisionDetection )
						fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tSpheref(transform.fXformPoint( broad.mCenter), broad.mRadius ), tVec4f(1,0,0,0.5f) );
				}
			}
			//else
			//{
			//	// old way

			//	// add sphere corresponding to max range
			//	proximity.fAddSphere( mDetectionPrimitive );
			//}

			// run proximity query
			proximity.fRefreshMT( 0.f, *fOwnerEntity( ) );
		}

		// enumerate collision
		mOffenders.fSetCount( 0 );
		mPeepsWeHit.fSetCount( 0 );
		mBreakablesWeHit.fSetCount( 0 );

		tGrowableArray<Physics::tContactPoint> contactPoints;

		for( u32 i = 0; i < proximity.fEntityCount( ); ++i )
		{
			tEntity* e = proximity.fGetEntity( i );

			if( e->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE ) == GameFlags::cUNIT_TYPE_VEHICLE )
				continue;

			tShapeEntity *sE = e->fDynamicCast<tShapeEntity>( );
			if( sE )
			{	
				tGrowableArray<tIObbObbContact> collideResults;

				if( sE->fShapeType( ) == tShapeEntityDef::cShapeTypeBox )
				{
					const tObbf& theirObb = sE->fBox( );
					tIntersectionObbObbWithContact intersect( myOBB, theirObb );
					if( intersect.fIntersects( ) )
						intersect.fResults( ).fDisown( collideResults );
				}
				else if( sE->fShapeType( ) == tShapeEntityDef::cShapeTypeSphere )
				{
					const tSpheref& theirSphere = sE->fSphere( );
					tIntersectionSphereObbWithContact<f32> intersect( myOBB, theirSphere );
					if( intersect.fIntersects( ) )
					{
						collideResults.fPushBack( tIObbObbContact( intersect.fContactPt( ), intersect.fSphereNormal( ), intersect.fPenetration( ) ) );
					}
				}
				else
					sigassert( !"Unsupported shape!" );
				
				
				tUberBreakablePiece* uberPiece = sE->fDynamicCast<tUberBreakablePiece>( );
				if( uberPiece ) 
				{
					if( collideResults.fCount( ) )
					{
						uberPiece->fOnHit( vel );
						if( speed > 2.0f )
							continue; //going fast, go right through it
					}
				}

				// store vehicle offenders for ai, regardless if we actually hit them
				tVehicleLogic *vL = NULL;
				tCharacterLogic *cL = NULL;
				tBreakableLogic *bL = NULL;
				Physics::tStandardPhysics* phys = NULL; 
				tUnitLogic* uL = NULL;

				tEntity *logicEntity = e->fFirstAncestorWithLogic( );
				if( logicEntity )
				{
					uL = logicEntity->fLogicDerived<tUnitLogic>( );
					vL = logicEntity->fLogicDerived<tVehicleLogic>( );
					if( vL )
						phys = vL->fQueryPhysicalDerived<Physics::tStandardPhysics>( );
					else
					{
						cL = logicEntity->fLogicDerived<tCharacterLogic>( );
						bL = logicEntity->fLogicDerived<tBreakableLogic>( );
					}
				}

				b32 skip = false;
				for( u32 i = 0; i < mIgnoreCollisionFrom.fCount( ); ++i )
				{
					if( mIgnoreCollisionFrom[ i ] == logicEntity )
					{
						skip = true;
						break;
					}
				}
				if( skip ) 
					continue;

				if( bL && bL->fNextStateIsDelete( ) )
				{
					//a breakable that's about ready to explode
					//RAM IT!
				}
				else
				{
					// dont avoid anything! yay!
					////avoid it
					//if( !cL || Gameplay_Vehicle_Pathing_AvoidCharacters )
					//	mOffenders.fPushBack( tVehicleOffender(theirObb, phys ? phys->fVelocity( ) : tVec3f::cZeroVector) );
				}

				if( collideResults.fCount( ) )
				{
					Physics::tRigidBody* hitRigidBody = logicEntity ? logicEntity->fLogic( )->fQueryPhysicalDerived<Physics::tRigidBody>( ) : NULL;
					mSpeedWeHit = speed;

					for( u32 p = 0; p < collideResults.fCount( ); ++p )
					{
						tVec3f point = collideResults[p].mPoint;
						tVec3f normal = collideResults[p].mNormal;

						Physics::tContactPoint cp( point, (u32)e, normal, collideResults[p].mDepth );
						cp.mShape.fReset( sE );
						cp.mRigidBody = hitRigidBody;

						if( !mFull3DPhysics )
						{
							//fudge the normal to be on either front/back or side faces
							tVec3f localPt = myOBB.fPointToLocalVector( point ) / myOBB.fExtents( );
							if( fAbs(localPt.z / localPt.x) > 0.95f )
								cp.mNormal = transform.fZAxis( ) * fSign(localPt.z);
							else 
								cp.mNormal = transform.fXAxis( ) * fSign(localPt.x);
						}

						contactPoints.fPushBack( cp );

						if( cL )
						{
							// hit a character 
							mPeepsWeHit.fPushBack( cL );
						}
						else
						{
							b32 plowThrough = false;

							// hit something significant
							if( !vL )
							{
								if( uL )
								{
									//this might be dangerous but assume we're just going to break it
									if( uL->fUnitType( ) == GameFlags::cUNIT_TYPE_PICKUP
										|| (uL->fNextStateIsDelete( ) && speed > 2.f) ) //mps, hackwut 
										plowThrough = true;

									//might be a breakable
									mBreakablesWeHit.fPushBack( uL );
								}
							}

							// response stuff for our own vehicle and potentially another vehicles
							f32 theirMass = vL ? vL->fMass( ) : fMass( ); //default to our mass for collision with static enviornment
							b32 ignore = false;

							tVec3f response = fComputeContactResponse( uL, cp, theirMass, ignore );

							if( !ignore )
							{
								// Can't cross talk in MT threads!
								//// pass the response to the other guy too, if he's a vehicle
								//if( vL && !fEqual( response.fLengthSquared( ), 0.0f ) )
								//	vL->fAddCollisionResponse( response, fMass( ) );

								if( !plowThrough && (fUnderUserControl( ) || sE->fHasGameTagsAny( GameFlags::cFLAG_STOP_AI )) )
									fAddContactPt( dt, cp );
							}
						}
					}
	
				}			
			}
		}

		if( Debug_Vehicle_DrawCollisionDetection )
		{
			fSceneGraph( )->fDebugGeometry( ).fRenderOnce( myOBB, mColor );

			for( u32 i = 0; i < contactPoints.fCount( ); ++i )
			{
				fSceneGraph( )->fDebugGeometry( ).fRenderOnce( contactPoints[i].mPoint, contactPoints[i].mPoint + tVec3f(0,10,0), tVec4f(1,0,0,1) );
				fSceneGraph( )->fDebugGeometry( ).fRenderOnce( contactPoints[i].mPoint, contactPoints[i].mPoint + contactPoints[i].mNormal * 5, mColor );
			}
		}
	}

	void tVehicleLogic::fProcessMovementFx( f32 dt )
	{
		if( mDoMovementFX )
		{
			tVec3f localV = fOwnerEntity( )->fWorldToObject( ).fXformVector( fLinearVelocity( tVec3f::cZeroVector ) );
			localV /= fMaxSpeed( );

			f32 xEmissionFactor = 1.f;
			f32 yEmissionFactor = 1.f;
			f32 xEmissionBonus = 0.f;
			f32 yEmissionBonus = 0.f;
			if( fUnitID( ) == GameFlags::cUNIT_ID_BOSS_SUB )
			{
				//log_line( 0, "Sub height - [" << fOwnerEntity( )->fWorldToObject( ).fGetTranslation( ).y << "]" );
				if( fOwnerEntity( )->fWorldToObject( ).fGetTranslation( ).y > 55.f )		//too deep! too deep!
					yEmissionFactor = 0.f;
				else
					yEmissionFactor *= 1.5f;
			}

			if( mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_PLUS_Z ].fSystemCount( ) )
			{
				f32 zFac = fClamp( localV.z, 0.f, 1.f );
				mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_PLUS_Z ].fSetEmissionPercent( xEmissionBonus + zFac * xEmissionFactor );
			}
			if( mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_MINUS_Z ].fSystemCount( ) )
			{
				f32 zFac = fClamp( -localV.z, 0.f, 1.f );
				mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_MINUS_Z ].fSetEmissionPercent( xEmissionBonus + zFac * xEmissionFactor );
			}			
			if( mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_PLUS_Y ].fSystemCount( ) )
			{
				f32 yFac = fClamp( localV.y, 0.f, 1.f );
				mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_PLUS_Y ].fSetEmissionPercent( yEmissionBonus + yFac * yEmissionFactor );
			}
			if( mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_MINUS_Y ].fSystemCount( ) )
			{
				f32 yFac = fClamp( -localV.y, 0.f, 1.f );
				mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_MINUS_Y ].fSetEmissionPercent( yEmissionBonus + yFac * yEmissionFactor );
			}
			
		}
	}

	void tVehicleLogic::fPhysicsMT( f32 dt )
	{
		fProcessMovementFx( dt );	

		if( fUnderUserControl( ) && tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) && tGameApp::fInstance( ).fCurrentLevel( ) )
		{
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevelDemand( );
			for( u32 i = 0; i < level->fGoalBoxCount( ); ++i )
			{
				tEntity* gb = level->fGoalBox( i );
				if( gb->fQueryEnumValue( GameFlags::cENUM_COUNTRY ) != fCountry( ) )
				{
					tGoalBoxLogic* goal = gb->fLogicDerivedStaticCast<tGoalBoxLogic>( );
					if( goal->fCheckInBounds( this ) )
					{
						mEnteredEnemyGoalBox.fReset( gb );
					}
				}
			}
		}

		tUnitLogic::fPhysicsMT( dt );
	}

	namespace
	{
		struct tTargetAngle
		{
			tEntity* mEnt;
			f32 mYaw;

			tTargetAngle( tEntity* ent = NULL, f32 yaw = 0.f )
				: mEnt( ent ), mYaw( yaw )
			{
			}

			b32 operator == (const tEntity* e ) const { return mEnt == e; }
		};
	}

	void tVehicleLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicBaseMoveST );

		const tMat3f& transform = fCurrentTransformMT( );

		for( u32 i = 0; i < mTurrets.fCount( ); ++i )
		{
			tTurretData& data = mTurrets[ i ];
			f32 userBlendInOut = 0.f;

			for( s32 w = data.mWeapons.fCount( ) - 1; w >= 0; --w )
			{
				b32 pointAtTarget = fUnderUserControl( ) ? true : (data.mWeapons[ w ]->fHasTarget( ) && data.mWeapons[ w ]->fShouldAcquire( ));

				if( pointAtTarget && fSceneGraph( ) )
				{
					tVec3f fireVec = data.mWeapons[ w ]->fComputeIdealLaunchVector( );

					sigassert( data.mEntity );
					sigassert( data.mEntity->fParent( ) );
					fireVec = data.mEntity->fParent( )->fObjectToWorld( ).fInverseXformVector( fireVec );
					fireVec.fNormalizeSafe( tVec3f::cZAxis );

					data.mTargetYaw = atan2f( fireVec.x, fireVec.z );

					if( fUnderUserControl( ) && Gameplay_Vehicle_TurretSrc )
					{
						tVec3f viewDir = fGetPlayer( 0 )->fUser( )->fViewport( )->fLogicCamera( ).fLocalToWorld( ).fZAxis( );
						viewDir = data.mEntity->fParent( )->fObjectToWorld( ).fInverseXformVector( viewDir );
						data.mTargetYaw = atan2f( viewDir.x, viewDir.z );
						userBlendInOut = 1.f;
					}
					else
						data.mBlendBlend = 0.f; //reset immediately to blend controller

					break;
				}
				else
				{
					data.mTargetYaw = 0;
					data.mBlendBlend = 0.f;
				}
			}

			// rectify the value so we end up going the shortest way.
			f32 shortestDelta = fShortestWayAround( data.mTurretYawBlend.fValue( ), data.mTargetYaw );
			data.mTurretYawBlend.fSetValue( data.mTargetYaw - shortestDelta );
			data.mTurretYawBlend.fSetAcceleration( Gameplay_Vehicle_TurretAcc );
			data.mTurretYawBlend.fStep( data.mTargetYaw, dt );

			data.mBlendBlend = fLerp( data.mBlendBlend, userBlendInOut, 0.2f );
			data.mLastSetYaw = fLerp( data.mTurretYawBlend.fValue( ), data.mTargetYaw, data.mBlendBlend );

			const tQuatf quatY( tAxisAnglef( tVec3f::cYAxis, data.mLastSetYaw ) );
			tMat3f result = data.mOriginalTurretTransform * tMat3f( quatY );
			data.mEntity->fSetParentRelativeXform( result );
		}

		const tVec3f cHingeGravity( 0, -10.f, 0 );
		for( u32 i = 0; i < mSeats.fCount( ); ++i )
		{
			mSeats[ i ].fStep( dt );

			if( mSeats[ i ].mHinge )
			{
				tVec3f point = fOwnerEntity( )->fObjectToWorld( ).fXformPoint( mSeats[ i ].mHinge->fHingePoint( ) );
				mSeats[ i ].mHinge->fStepMT( fPointVelocityMT( point ), cHingeGravity, fOwnerEntity( )->fObjectToWorld( ), dt );
				mSeats[ i ].mDoor->fSetParentRelativeXform( mSeats[ i ].mHinge->fBRelativeToA( ) );
			}
		}

		if( mEnteredEnemyGoalBox && !mDeleteMyEntity )
		{
			mEnteredEnemyGoalBox->fLogicDerivedStaticCast<tGoalBoxLogic>( )->fSomeoneEntered( );
			fReachedEnemyGoal( );
			if( fGetPlayer( 0 ) )
				fGetPlayer( 0 )->fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_UI_SCORE );
			mDeleteMyEntity = true;
		}
	}

	void tVehicleLogic::fThinkST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicBaseThinkST );

		if( mDeleteMyEntity )
		{
			mEjectUser = true;
			fOwnerEntity( )->fDelete( );
		}

		if( mEjectUser )
		{
			mEjectUser = false;
			if( fUnderUserControl( ) )
			{
				//return rts camera to base position
				const tMat3f* teamOrient = tGameApp::fInstance( ).fTeamOrientation( fTeam( ) );
				if( teamOrient )
				{
					tRtsCamera* rtsCam = fGetPlayer( cVehicleDriverSeat )->fCameraStack( ).fFindCameraOfType<tRtsCamera>( );
					sigassert( rtsCam );
					rtsCam->fAcquirePosition( *teamOrient );
					rtsCam->fSetPreventPositionAcquisition( true );
				}

				fVacateAllPlayers( );
			}
		}

		// Battery recharge
		if( mWaitTimer )
		{
			mWaitTimer->fThinkST( dt );
			if( mWaitTimer->fFinished( ) && mWaitTimer->fFadeOut( ) )
			{
				mWaitTimer->fDelete( );
				mWaitTimer.fRelease( );
				mPowerLevel = 1.f;
				mTakesDamage = true;
			}
			else
				fAddPowerLevel( dt / fUnitAttributeBatteryRechargeTime( ) );
		}

		if( fUnderUserControl( ) )
		{
			if( !tGameApp::fExtraDemoMode( ) )
			{
				// Battery use timer
				if( fUnitAttributeOverChargeDuration( ) > 0.f )
				{
					b32 inputLocked = tGameApp::fInstance( ).fCurrentLevelDemand( )->fDisableVehicleInput( );
					if( !AAACheats_DontDrainBattery && !inputLocked )
						mPowerLevel -= 1.0f / fUnitAttributeOverChargeDuration( ) * dt;
					mPowerLevel = fMin( mPowerLevel, 1.f );
					if( mPowerLevel < 0.f )
					{
						mPowerLevel = 1.f; //so this doesnt trigger twice.
						tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
						if( level )
						{
							tPlayer* p = fGetPlayer( 0 );
							sigassert( p && "we're under user control right?" );
							level->fImpactText( cHasExpiredLocKey, *p );

							level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_UNIT_DESTROYED, fUnitID( ), fUnitID( ) ) );
						}

						// kill ourselves
						fDestroy( true );
					}
				}
			}

			//// This is from when it used to be an out of bounds indicator.
			//tPlayer* player = fGetPlayer( 0 );
			//if( player->fBToExitIndicator( ) )
			//	player->fBToExitIndicator( )->fShow( !fInCameraBox( ) );
		}
		else 
		{
			// not under user control
			if( fHasSelectionFlag( ) && !fUnitPath( )->fHasWaypoints( ) && !mDontInstaDestroyOutsideOfFlyBox && !fInCameraBox( ) )
			{
				fDestroy( true );
			}
		}

		if( mExpireTimer )
		{
			mExpireTimer->fThinkST( dt );
			
			if( mExpireTimer->fFinished( ) )
			{
				// kill ourselves
				fDestroy( true );
			}
		}

		mUnitPath->fUpdate( dt );

		{
			profile( cProfilePerfVehicleLogicBaseThinkSTWeapons );
			for( u32 ws = 0; ws < mWeaponStations.fCount( ); ++ws )
			{
				mWeaponStations[ ws ]->fSetParentVelocityMT( this );
				mWeaponStations[ ws ]->fProcessST( dt );
			}
		}

		if( mWaitingToLaunchCargo )
		{
			b32 ready = true;
			for( u32 i = 0; i < mDroppingCargo.mTrenchedUnits.fCount( ); ++i )
			{
				tUnitLogic* ul = mDroppingCargo.mTrenchedUnits[ i ]->fLogicDerived<tUnitLogic>( );
				sigassert( ul );
				if( ul->fIsValidTarget( ) && ul->fUnitPath( )->fPathMode( ) == tUnitPath::cPathModeFollow )
				{
					ready = false;
					break;
				}
			}

			if( ready )
				fCancelCargoDrop( );
		}
		else if( mDroppingCargo.mCount > 0 && mDroppingEnabled )
		{
			mDropTimer += dt;
			while( mDropTimer >= mDroppingCargo.mSpawnRate )
			{

				tGrowableArray<tEntityPtr>* list = fCargoPoints( mDroppingCargo.mIndex );
				if( list )
				{
					for( u32 i = 0; i < list->fCount( ); ++i )
					{
						tEntityPtr& dropPtr = (*list)[ i ];
						sigassert( dropPtr );
						tUnitLogic* unit = fDropCargoItem( dropPtr );

						fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CARGO_DROP_SPAWN, new Logic::tObjectEventContext( Sqrat::Table( ).SetValue( "Index", mDroppingCargo.mTotalCount - mDroppingCargo.mCount ).SetValue( "Unit", unit ) ) ) );

					}
				}

				mDropTimer -= mDroppingCargo.mSpawnRate;
				--mDroppingCargo.mCount;

				if( mDroppingCargo.mCount == 0 )
					mWaitingToLaunchCargo = true;
			}
		}

		mBoostTimer -= dt;

		fDestroyBreakablesWeHit( );
	}

	void tVehicleLogic::fHideShowBToExit( b32 show )
	{
		if( fUnderUserControl( ) )
		{
			tPlayer* player = fGetPlayer( 0 );
			if( player && player->fBToExitIndicator( ) )
				player->fBToExitIndicator( )->fShow( show );
		}
	}

	void tVehicleLogic::fDestroyBreakablesWeHit( )
	{
		//tDamageContext dc;
		//dc.fSetAttacker( this );
		//dc.fSetWeapon( GameFlags::cDAMAGE_TYPE_IMPACT, (effect > 0.25f) ? 10.0f : 0.0f );
		//dc.mWorldPosition = pos;
		//dc.mWorldEffectorVector = transform.fZAxis( ) * mSpeedWeHit * 0.5f;

		for( u32 b = 0; b < mBreakablesWeHit.fCount( ); ++b )
		{
			//mBreakablesWeHit[ b ]->fDealDamage( dc );

			if( !mBreakablesWeHit[ b ]->fIsDestroyed( ) )
			{
				u32 type = mBreakablesWeHit[ b ]->fUnitType( );
				b32 force = ( fUnitType( ) == GameFlags::cUNIT_TYPE_BOSS || type == GameFlags::cUNIT_TYPE_PICKUP );
				mBreakablesWeHit[ b ]->fDestroy( this, force, fGetPlayer( cVehicleDriverSeat ) );
			}
		}
	}
	
	tUnitLogic* tVehicleLogic::fDropCargoItem( const tEntityPtr& dropPtr )
	{
		if( dropPtr->fLogic( ) )
			dropPtr->fLogic( )->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_CARGO_DROP_SPAWN ) );

		u32 specialEntrance = dropPtr->fQueryEnumValue( GameFlags::cENUM_SPECIAL_ENTRANCE );
		b32 relativeToDropPt = mDroppingCargo.mRelativeToDropPt;
		b32 parachuting = mDroppingCargo.mParachute;

		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		tEntity* spawnParent = relativeToDropPt ? dropPtr.fGetRawPtr( ) : level->fOwnerEntity( );
		tEntity* ent = spawnParent->fSpawnChild( mDroppingCargo.mPath );
		if( ent ) 
		{
			if( !relativeToDropPt )
			{
				tMat3f xform = dropPtr->fObjectToWorld( );
				xform.fOrientYAxis( tVec3f::cYAxis, xform.fXAxis( ) );
				ent->fMoveTo( xform );
			}

			if( specialEntrance != ~0 )
				ent->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_SPECIAL_ENTRANCE, specialEntrance ) );

			tUnitLogic* uL = ent->fLogicDerived<tUnitLogic>( );
			sigassert( uL );
			uL->fQueryEnums( );
			uL->fSetCreationType( tUnitLogic::cCreationTypeFromGenerator );
			uL->fSetHitPointModifier( mHitPointsModifier );
			uL->fSetTimeScale( mTimeScale );

			if( mWave )
			{
				// set wave before fAddToAliveList
				mWave->fAddCargoUnit( uL );
				uL->fSetWave( mWave.fGetRawPtr( ) );
			}

			uL->fAddToAliveList( );
			
			if( !mDroppingCargo.mFocusShown && mDroppingCargo.mFocusPrompt.fExists( ) )
			{
				mDroppingCargo.mFocusShown = true;
				uL->fShowFocalPrompt( mDroppingCargo.mFocusPrompt );
			}

			tCharacterLogic* character = ent->fLogicDerived<tCharacterLogic>( );
			if( character )
			{
				if( relativeToDropPt )
					character->fSetParentRelativeUntilLand( mDroppingCargo.mDisablePhysics );
				if( parachuting ) 
					character->fSetParachuteing( true );
			}

			if( parachuting ) 
			{
				tWheeledVehicleLogic* vehicle = ent->fLogicDerived<tWheeledVehicleLogic>( );
				if( vehicle )
					vehicle->fSetParachuteing( true );
			}

			fGiveAGoalPathByUnitType( uL, dropPtr->fObjectToWorld( ).fGetTranslation( ) );

			return uL;
		}
		else
			log_warning( 0, "Could not spawn entity in cargo drop." );

		return NULL;
	}

	f32 tVehicleLogic::fBoostProgress( ) const
	{
		return fClamp( mBoostTimer / Gameplay_Vehicle_BoostTime, 0.f, 1.f );
	}

	b32 tVehicleLogic::fGiveAGoalPathByUnitType( tUnitLogic* unit, Math::tVec3f searchPt )
	{
		// Search for the closest path with the same unit type enum tag.
		if( unit )
		{
			tEntity* ent = unit->fOwnerEntity( );
			tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

			unit->fQueryEnums( );
			u32 unitType = unit->fUnitType( );

			tUnitPath* up = unit->fUnitPath( );
			up->fSetOwnerEntity( ent );

			const tStringPtr& pathName = mUnitPath->fLastStartedPathName( );

			if( unitType == GameFlags::cUNIT_TYPE_INFANTRY && pathName.fExists( ) )
			{
				s32 exitGen = tUnitPath::fFindClosestStartPoint( searchPt, level->fExitGenStarts( ), pathName, &unitType );
				if( exitGen > -1 ) 
				{
					up->fAddPathStartEntry( tUnitPath::tPathStartEntry( level->fExitGenStarts( )[ exitGen ] ) );
				}
			
				// trenches
				s32 trenchStart = tUnitPath::fFindClosestStartPoint( searchPt, level->fTrenchStarts( ), pathName, &unitType );
				if( trenchStart > -1 ) 
				{
					f32 trenchLen = level->fTrenchLength( trenchStart );
					const tPathEntityPtr& path = level->fTrenchStarts( )[ trenchStart ];

					f32 percentageOfPath = mDroppingCargo.mCount / (f32)mDroppingCargo.mTotalCount;
					Math::tVec3f trenchPos;
					path->fTraversePath( trenchLen * percentageOfPath, trenchPos );
					searchPt = trenchPos;

					tUnitPath::tPathStartEntry trenchSeq( path, percentageOfPath * trenchLen, true );
					trenchSeq.mUseSpecificPt = true;
					trenchSeq.mSpecificPt = trenchPos + ( sync_rand( fVecNorm<tVec3f>( ) ) * 2.f ).fProjectToXZ( );

					up->fAddPathStartEntry( trenchSeq );
					mDroppingCargo.mTrenchedUnits.fPushBack( tEntityPtr( ent ) );
				}
			}

			s32 pathStart = tUnitPath::fFindClosestStartPoint( searchPt, level->fPathStarts( ), pathName, &unitType );
			if( pathStart > -1 )
			{
				// Actual path
				up->fAddPathStartEntry( tUnitPath::tPathStartEntry( level->fPathStarts( )[ pathStart ] ) );
				up->fStartPathSequence( );

				return true;
			}
			else
				log_warning( 0, "No path starts with name (or missing unit_type tag): " << pathName );
		}

		return false;
	}

	void tVehicleLogic::fCoRenderMT( f32 dt )
	{
		tEntity* targetPt = mUnitPath->fWaypoint( );
		if( targetPt )
		{
			u32 throttle = targetPt->fQueryEnumValue( GameFlags::cENUM_THROTTLE, GameFlags::cTHROTTLE_COUNT );
			if( throttle < GameFlags::cTHROTTLE_COUNT )
				mThrottleScaleTarget = throttle / (f32)(GameFlags::cTHROTTLE_COUNT-1);
		}

		f32 target = mThrottleScaleTarget;
		if( mThrottleScaleTargetOverride >= 0.f )
			target = mThrottleScaleTargetOverride;

		mThrottleScale = fLerp( mThrottleScale, target, mThrottleBlend );

		if( fConstrainYaw( ) )
			fUpdateYawConstraint( false );

		for( u32 ws = 0; ws < mWeaponStations.fCount( ); ++ws )
			mWeaponStations[ ws ]->fProcessMT( dt );
	}

	void tVehicleLogic::fUpdateCharactersMT( f32 dt )
	{
		const tMat3f& transform = fCurrentTransformMT( );

		for( u32 i = 0; i < mPassengers.fCount( ); ++i )
		{
			fGetPassengerLogic( i )->fDependentPhysicsMT( dt, transform, *this );
		}
	}

	b32 tVehicleLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		if( mVehicleControllerLogic && mVehicleControllerLogic->fHandleLogicEvent( e ) )
			return true;

		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_UNIT_DESTROYED:
			{
				mEjectUser = true;
				mFallingThrough = mUseDefaultEndTransition;
				mPersistentEffect.fRelease( );

				if( mUnitID == GameFlags::cUNIT_ID_BOSS_FLYINGTANK )
					tGameApp::fInstance( ).fAwardAchievementToAllPlayers( GameFlags::cACHIEVEMENTS_SHOCKING_RESULTS );
				else if( mUnitID == GameFlags::cUNIT_ID_BOSS_SUB )
					tGameApp::fInstance( ).fAwardAchievementToAllPlayers( GameFlags::cACHIEVEMENTS_SUNK );
				else if( mUnitID == GameFlags::cUNIT_ID_BOSS_TANK )
					tGameApp::fInstance( ).fAwardAchievementToAllPlayers( GameFlags::cACHIEVEMENTS_A_FEW_LOOSE_SCREWS );
				else if( mUnitID == GameFlags::cUNIT_ID_BOSS_SUPERTANK )
				{
					for( u32 i = 0; i < tGameApp::fInstance( ).fPlayerCount( ); ++i )
					{
						tPlayer* player = tGameApp::fInstance( ).fGetPlayer( i );
						sigassert( player );
						if( player->fHasDLC( GameFlags::cDLC_EVIL_EMPIRE ) )
							player->fAwardAchievement( GameFlags::cACHIEVEMENTS_DLC1_DESTROY_SUPERTANK );
					}
				}
				else if( mUnitID == GameFlags::cUNIT_ID_BOSS_MI12 )
				{
					for( u32 i = 0; i < tGameApp::fInstance( ).fPlayerCount( ); ++i )
					{
						tPlayer* player = tGameApp::fInstance( ).fGetPlayer( i );
						sigassert( player );
						if( player->fHasDLC( GameFlags::cDLC_EVIL_EMPIRE ) )
							player->fAwardAchievement( GameFlags::cACHIEVEMENTS_DLC2_DESTROY_HOMER );
					}
				}
				else if( mDestroyedByPlayer && !mHasEverDroppedCargo && !fHasSelectionFlag( ) && tPlayer::fIsIFV( fUnitID( ) ) )
				{
					mDestroyedByPlayer->fAwardAchievement( GameFlags::cACHIEVEMENTS_CONCENTRATED_FIRE );
				}

				if( fHasSelectionFlag( ) )
				{
					tPlayer* player = NULL;

					if( mPurchasedBy )
						player = mPurchasedBy.fGetRawPtr( );
					else if( tGameApp::fInstance( ).fGameMode( ).fIsVersus( ) || !tGameApp::fInstance( ).fGameMode( ).fIsMultiPlayer( ) )
						player = tGameApp::fInstance( ).fGetPlayerByTeam( fTeam( ) );

					if( player )
						player->fStats( ).fIncStat( GameFlags::cSESSION_STATS_VEHICLES_LOST, 1.f );
				}

				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
				if( fHasSelectionFlag( ) && level && !level->fDisableVehicleRespawn( ) && !mDontRespawn )
				{
					tFilePathPtr unitPath = tGameApp::fInstance( ).fUnitResourcePath( fUnitID( ), fCountry( ) );
					tEntity* unit = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( unitPath );
					if( unit )
					{
						// this prevents double spawning
						fOwnerEntity( )->fRemoveGameTags( GameFlags::cFLAG_SELECTABLE );

						unit->fAddGameTagsRecursive( GameFlags::cFLAG_SELECTABLE );
						if( fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_MINIGAME_UNIT ) ) 
							unit->fAddGameTagsRecursive( GameFlags::cFLAG_MINIGAME_UNIT );

						tUnitLogic* ul = unit->fLogicDerived<tUnitLogic>( );
						ul->fCopyLevelEvents( this );

						unit->fSetName( fOwnerEntity( )->fName( ) );
						unit->fMoveTo( mInitialTransform );
						tVehicleLogic* veh = unit->fLogicDerived<tVehicleLogic>( );
						if( veh	)
							veh->fSetPowerLevel( 0.f );
					}
					else
						log_warning( 0, "Could not spawn unit: " << GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) << " path: " << unitPath );
				}
				else
					mDeleteAfterFallThrough = true;

				fReleaseTrenchedCargo( );
				fCancelAllWeaponFire( );
			}
			break;
		case GameFlags::cEVENT_CARGO_DROP_READY:
			{
				mDroppingEnabled = true;
				return true;
			}
			break;
		case GameFlags::cEVENT_WAYPOINT_REACHED:
			{
				const tPathContext* pc = e.fContext<tPathContext>( );
				if( pc )
				{
					u32 targetIndex = pc->fWaypoint( )->fQueryEnumValue( GameFlags::cENUM_TARGET_INDEX );
					u32 weaponAction = pc->fWaypoint( )->fQueryEnumValue( GameFlags::cENUM_WEAPON_ACTION );
					u32 weaponIndex = pc->fWaypoint( )->fQueryEnumValue( GameFlags::cENUM_WEAPON_INDEX );

					fSetWeaponTargetIndex( weaponIndex, targetIndex );
					fHandleWeaponAction( weaponAction, weaponIndex );

					if( pc->mWaypoint->fName( ) == cBossSmash )
						fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_DO_SPECIAL_MOVE, NEW Logic::tIntEventContext( 0 ) ) );
				}
			}
			break;
		case GameFlags::cEVENT_GAME_EFFECT:
			{
				const tEffectLogicEventContext* context = e.fContext<tEffectLogicEventContext>( );
				if( context )
				{
					mAudio->fHandleEvent( context->mAudioEvent );
					context->mAudio.fRelease( );

					if( context->mPersistent )
						mPersistentEffect = context->mPersistent;
				}
				return true;
			}
			break;
		}
		return tUnitLogic::fHandleLogicEvent( e );
	}

	void tVehicleLogic::fSetWeaponTargetIndex( u32 weaponIndex, u32 targetIndex )
	{
		if( targetIndex != ~0 && weaponIndex != ~0 )
		{
			tEntity* target = NULL;

			if( targetIndex == GameFlags::cTARGET_INDEX_RANDOM )
			{
				if( mExplicitTargets.fCount( ) )
					target = mExplicitTargets[ sync_rand( fIntInRange( 0, mExplicitTargets.fCount( )-1 ) ) ].fGetRawPtr( );
				else
				{
					log_warning( 0, "No explicit targets to random!" );
				}
			}
			else if( targetIndex == GameFlags::cTARGET_INDEX_NONE )
			{
				// nothing
			}
			else if( targetIndex - GameFlags::cTARGET_INDEX_1 < mExplicitTargets.fCount( ) )
				target = mExplicitTargets[ targetIndex - GameFlags::cTARGET_INDEX_1 ].fGetRawPtr( );
			else
			{
				log_warning( 0, "Target index out of range!" );
			}

			if( fWeaponStationCount( ) > 0 )
			{
				tWeaponStationPtr& station = fWeaponStation( 0 );
				if( station->fBankCount( ) > 0 )
				{
					if( weaponIndex >= station->fBankCount( ) ) weaponIndex = 0;
					tWeaponBankPtr& bank = station->fBank( weaponIndex );
					for( u32 i = 0; i < bank->fWeaponCount( ); ++i )
						bank->fWeapon( i )->fSetAITargetOverride( target );
				}
			}
		}
	}

	void tVehicleLogic::fHandleWeaponAction( u32 weaponAction, u32 weaponIndex )
	{
		if( weaponAction != ~0 )
		{
			if( weaponAction == GameFlags::cWEAPON_ACTION_FIRE_INTERNAL || weaponAction == GameFlags::cWEAPON_ACTION_END_FIRE_INTERNAL )
			{
				if( fWeaponStationCount( ) > 0 )
				{
					tWeaponStationPtr& station = fWeaponStation( 0 );
					if( station->fBankCount( ) > 0 )
					{
						if( weaponIndex >= station->fBankCount( ) ) weaponIndex = 0;
						tWeaponBankPtr& bank = station->fBank( weaponIndex );
						bank->fSetAIFireOverride( weaponAction == GameFlags::cWEAPON_ACTION_FIRE_INTERNAL );
					}
				}
			}
			else
			{
				// call to children weapons
				if( mHitPointLinkedChildren.fCount( ) > 0 )
				{
					sigassert( weaponIndex == ~0 || weaponIndex < mHitPointLinkedChildren.fCount( ) );
					u32 start = weaponIndex;
					u32 end = weaponIndex;

					if( weaponIndex == ~0 )
					{
						start = 0;
						end = mHitPointLinkedChildren.fCount( ) - 1;
					}	

					Logic::tEvent newE( GameFlags::cEVENT_WEAPON_ACTION, NEW Logic::tIntEventContext( weaponAction ) );

					for( u32 i = start; i <= end; ++i )
					{
						for( u32 c = 0; c < mHitPointLinkedChildren[ i ].fCount( ); ++c )
						{
							mHitPointLinkedChildren[ i ][ c ]->fLogic( )->fHandleLogicEvent( newE );
						}
					}
				}
			}
		}
	}

	void tVehicleLogic::fApplyMotionStateToArtillerySoldiers( const char* motionState )
	{
		Sqrat::Table table;
		Sqrat::Object obj( table );

		for( u32 i = 0; i < mPassengers.fCount( ); ++i )
		{
			tVehiclePassengerLogic* soldierLogic = fGetPassengerLogic( i );
			sigassert( soldierLogic );
			Logic::tAnimatable* anim = soldierLogic->fQueryAnimatable( );
			sigassert( anim );

			anim->fExecuteMotionState( motionState, obj );
		}
	}

	void tVehicleLogic::fApplyMotionStateToTurrets( const char* motionState )
	{
		for( u32 i = 0; i < mTurrets.fCount( ); ++i )
		{
			tTurretData& data = mTurrets[ i ];
			sigassert( data.mEntity );
			sigassert( data.mEntity->fLogic( ) );
			Logic::tAnimatable* anim = data.mEntity->fLogic( )->fQueryAnimatable( );
			sigassert( anim );
			sigassert( data.mWeapons.fCount( ) );

			Sqrat::Table table;
			table.SetValue(_SC("Weapon"), data.mWeapons.fFront( ).fGetRawPtr( ) );
			anim->fExecuteMotionState( motionState, Sqrat::Object( table ) );
		}
	}

	void tVehicleLogic::fShowWaitTimer( )
	{
		if( fOwnerEntity( ) )
		{
			fQueryEnums( );
			if( !fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_MINIGAME_UNIT ) && fUnitAttributeBatteryRechargeTime( ) > 0 )
			{
				if( mExpireTimer )
				{
					mExpireTimer->fDelete( );
					mExpireTimer.fRelease( );
				}

				mWaitTimer.fReset( NEW Gui::tHoverTimer( cWaitLocKey, fUnitAttributeBatteryRechargeTime( ) * (1.f - mPowerLevel), true, fTeam( ) ) );
				mWaitTimer->fSpawn( *fOwnerEntity( ) );
				mTakesDamage = false;
			}
			else
				mPowerLevel = 1.f;
		}
	}


	void tVehicleLogic::fStartup( b32 startup )
	{
		mStartingUp = startup;
		if( startup ) 
		{
			// stupid hack dlc shit
			switch( mUnitID )
			{
			case GameFlags::cUNIT_ID_HELO_SUPER_HORMONE:
				mAudio->fHandleEvent( AK::EVENTS::PLAY_VEHICLE_HORMONE_DLC );
				break;
			case GameFlags::cUNIT_ID_JET_PACK:
				mAudio->fHandleEvent( AK::EVENTS::PLAY_VEHICLE_JETPACK_DLC );
				break;
			default:
				mAudio->fHandleEvent( AK::EVENTS::PLAY_ENGINE );
				mExhaustSource->fHandleEvent( AK::EVENTS::PLAY_EXHAUST );
			}

			if( mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_RUNNING ].fSystemCount( ) )
				mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_RUNNING ].fSetEmissionPercent( 1.f );
		}
	}

	void tVehicleLogic::fShutDown( b32 shutDown )
	{
		if( shutDown ) 
		{
			// stupid hack dlc shit
			switch( mUnitID )
			{
			case GameFlags::cUNIT_ID_HELO_SUPER_HORMONE:
				mAudio->fHandleEvent( AK::EVENTS::STOP_VEHICLE_HORMONE_DLC );
				break;
			case GameFlags::cUNIT_ID_JET_PACK:
				mAudio->fHandleEvent( AK::EVENTS::STOP_VEHICLE_JETPACK_DLC );
				break;
			default:
				mAudio->fHandleEvent( AK::EVENTS::STOP_ENGINE );
				mExhaustSource->fHandleEvent( AK::EVENTS::STOP_EXHAUST );
			}

			if( mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_RUNNING ].fSystemCount( ) )
				mMovementEffects[ GameFlags::cVEHICLE_MOTION_FX_RUNNING ].fSetEmissionPercent( 0.f );
		}
	}
	
	void tVehicleLogic::fAddCargoDropSmokePtr( tRefCounterPtr<tSmokeDestroyer>& ptr ) 
	{ 
		mCargoSmokes.fPushBack( ptr ); 
	}

	void tVehicleLogic::fStopToDropCargo( )
	{
		if( mDroppingCargo.mStopWhileSpawning )
		{
			tUnitPath* path = mSlaveVehicleLogic ? mSlaveVehicleLogic->fUnitPath( ) : mUnitPath.fGetRawPtr( );
			
			path->fPausePathSequence( );
			path->fAdvanceWaypoint( ); //so we dont process the same event again
		}
	}
	void tVehicleLogic::fResumeFromDropCargo( )
	{
		if( mDroppingCargo.mStopWhileSpawning )
		{
			tUnitPath* path = mSlaveVehicleLogic ? mSlaveVehicleLogic->fUnitPath( ) : mUnitPath.fGetRawPtr( );

			mUnitPath->fResumePathSequence( );
		}
	}
	void tVehicleLogic::fSetLockedToPathStart( b32 locked )
	{
		mLockedToPathStart = locked;

		if( mLockedToPathStart )
		{
			const tPathEntityPtr& path = fUnitPath( )->fLastStartedPathPt( );
			if( path )
				fOwnerEntity( )->fMoveTo( path->fObjectToWorld( ) );
			else
				log_warning( 0, "No path start point for fSetLockedToPathStart" );
		}
	}



}

namespace Sig
{
	typedef AI::tDerivedLogicGoalHelper<tVehicleLogic> tVehicleGoalHelper;

	class tVehicleWaitForUserInput : public AI::tSigAIGoal, public tVehicleGoalHelper
	{
		define_dynamic_cast(tVehicleWaitForUserInput, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fPersist( );
			fAcquireDerivedLogic( logic );
			fCheckContinue( );
			AI::tSigAIGoal::fOnActivate( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
			fCheckContinue( );
		}
		void fCheckContinue( )
		{
			if( !tGameApp::fInstance( ).fCurrentLevel( )->fDisableVehicleInput( ) )
				fMarkAsComplete( );
		}
	};

	class tVehicleWaitingForShutdown : public AI::tSigAIGoal, public tVehicleGoalHelper
	{
		define_dynamic_cast(tVehicleWaitingForShutdown, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fPersist( );
			fAcquireDerivedLogic( logic );
			fCheckHeight( );
			AI::tSigAIGoal::fOnActivate( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
			fCheckHeight( );
		}
		void fCheckHeight( )
		{
			if( fLogic( )->fGroundHeight( ) < AAACheats_Vehicle_ShutDownGroundHeight )
				fMarkAsComplete( );
		}
	};

	class tVehicleUnderUserControlGoal : public AI::tSigAIGoal, public tVehicleGoalHelper
	{
		define_dynamic_cast(tVehicleUnderUserControlGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			fLogic( )->fVacateAllPlayers( );
			AI::tSigAIGoal::fOnSuspend( logic );
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
			fLogic( )->fComputeUserInput( dt );
		}
	};

	class tVehicleUnderAiControlGoal : public AI::tSigAIGoal, public tVehicleGoalHelper
	{
		define_dynamic_cast(tVehicleUnderAiControlGoal, AI::tSigAIGoal);
	public:
		virtual void fOnActivate( tLogic* logic )
		{
			fAcquireDerivedLogic( logic );
			AI::tSigAIGoal::fOnActivate( logic );

			if( fLogic( )->fCreationType( ) != tUnitLogic::cCreationTypeFromLevel 
				&& !fLogic( )->fWaveDisabledAIFire( ) )
			{
				for( u32 i = 0; i < fLogic( )->fWeaponStationCount( ); ++i )
					fLogic( )->fWeaponStation( i )->fSetAcquireTargets( true );
			}
		}
		virtual void fOnSuspend( tLogic* logic )
		{
			AI::tSigAIGoal::fOnSuspend( logic );

			if( fLogic( )->fCreationType( ) != tUnitLogic::cCreationTypeFromLevel )
			{
				for( u32 i = 0; i < fLogic( )->fWeaponStationCount( ); ++i )
				{
					fLogic( )->fWeaponStation( i )->fSetAcquireTargets( false );
					fLogic( )->fWeaponStation( i )->fEndFire( );
				}
			}
		}
		virtual void fOnProcess( AI::tGoalPtr& goalPtr, tLogic* logic, f32 dt )
		{
			fLogic( )->fComputeAIInput( dt );
			AI::tSigAIGoal::fOnProcess( goalPtr, logic, dt );
		}
	};

	static void fExportVehicleGoalsScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tVehicleWaitForUserInput, AI::tSigAIGoal, Sqrat::NoCopy<tVehicleWaitForUserInput> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("VehicleWaitForUserInput"), classDesc);
		}
		{
			Sqrat::DerivedClass<tVehicleWaitingForShutdown, AI::tSigAIGoal, Sqrat::NoCopy<tVehicleWaitingForShutdown> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("VehicleWaitingForShutdown"), classDesc);
		}
		{
			Sqrat::DerivedClass<tVehicleUnderAiControlGoal, AI::tSigAIGoal, Sqrat::NoCopy<tVehicleUnderAiControlGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("VehicleUnderAiControlGoal"), classDesc);
		}
		{
			Sqrat::DerivedClass<tVehicleUnderUserControlGoal, AI::tSigAIGoal, Sqrat::NoCopy<tVehicleUnderUserControlGoal> > classDesc( vm.fSq( ) );
			vm.fRootTable( ).Bind(_SC("VehicleUnderUserControlGoal"), classDesc);
		}
	}
}



namespace Sig
{
	namespace
	{
		u32 fPointerToInt( tUnitLogic* ul )
		{
			return (u32)ul;
		}
	}

	void tVehicleLogic::fExportScriptInterface( tScriptVm& vm )
	{
		if( Gameplay_Vehicle_ApplyTable )
			log_warning( 0, "Performance warning: Vehicles are applying their table every frame." );

		{
			Sqrat::Class<tVehicleCargo, Sqrat::NoCopy<tVehicleCargo> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("Index"),		&tVehicleCargo::mIndex)
				;

			vm.fRootTable( ).Bind(_SC("VehicleCargo"), classDesc);
		}
		{
			Sqrat::DerivedClass<tVehicleLogic, tUnitLogic, Sqrat::NoCopy<tVehicleLogic> > classDesc( vm.fSq( ) );
			classDesc
				.Func<b32 (tVehicleLogic::*)(tPlayer*)>(_SC("TryToUse"), &tVehicleLogic::fTryToUse)
				.Func(_SC("Startup"),						&tVehicleLogic::fStartup)
				.Func(_SC("ShutDown"),						&tVehicleLogic::fShutDown)
				.Func(_SC("AddCargo"),						&tVehicleLogic::fAddCargo)
				.Func(_SC("ClearCargo"),					&tVehicleLogic::fClearCargo)
				.Func(_SC("AddSimpleCargo"),				&tVehicleLogic::fAddSimpleCargo)
				.Func(_SC("DropCargo"),						&tVehicleLogic::fDropCargo)
				.Prop(_SC("IsDroppingCargo"),				&tVehicleLogic::fIsDroppingCargo)
				.Func(_SC("StopToDropCargo"),				&tVehicleLogic::fStopToDropCargo)
				.Func(_SC("ResumeFromDropCargo"),			&tVehicleLogic::fResumeFromDropCargo)
				.Func(_SC("ApplyMotionStateToSoldiers"),	&tVehicleLogic::fApplyMotionStateToArtillerySoldiers)
				.Func(_SC("ApplyMotionStateToTurrets"),		&tVehicleLogic::fApplyMotionStateToTurrets)
				.Func(_SC("CancelCargoDrop"),				&tVehicleLogic::fCancelCargoDrop)
				.Func(_SC("HandleWeaponAction"),			&tVehicleLogic::fHandleWeaponAction)
				.Func(_SC("SetWeaponTargetIndex"),			&tVehicleLogic::fSetWeaponTargetIndex)
				.Func(_SC("AddExplicitTarget"),				&tVehicleLogic::fAddExplicitTarget)
				.Func(_SC("StartSpecialMove"),				&tVehicleLogic::fStartSpecialMove)
				.Func(_SC("StopSpecialMove"),				&tVehicleLogic::fStopSpecialMove)
				.Func(_SC("DoorHinge"),						&tVehicleLogic::fDoorHingeForScript)
				.Prop(_SC("AICheckForCollisions"),			&tVehicleLogic::fAICheckForCollisions, &tVehicleLogic::fSetAICheckForCollisions)
				.Prop(_SC("LockedToPathStart"),				&tVehicleLogic::fLockedToPathStart, &tVehicleLogic::fSetLockedToPathStart)
				.Prop(_SC("HasDoneSpecialEntrance"),		&tVehicleLogic::fHasDoneSpecialEntrance, &tVehicleLogic::fSetHasDoneSpecialEntrance)
				.Prop(_SC("DontInstaDestroyOFB"),			&tVehicleLogic::fDontInstaDestroyOFB, &tVehicleLogic::fSetDontInstaDestroyOFB)
				.Prop(_SC("SlaveLinkTurrentChildren"),		&tVehicleLogic::fSlaveLinkTurrentChildren, &tVehicleLogic::fSetSlaveLinkTurrentChildren)
				.Prop(_SC("CurrentCargo"),					&tVehicleLogic::fCurrentCargo)
				.Prop(_SC("DroppingEnabled"),				&tVehicleLogic::fDroppingEnabled, &tVehicleLogic::fSetDroppingEnabled)
				.Prop(_SC("HasEverDroppedCargo"),			&tVehicleLogic::fHasEverDroppedCargo)
				.StaticFunc(_SC("PtrToInt"), &fPointerToInt)
				;

			vm.fRootTable( ).Bind(_SC("VehicleLogic"), classDesc);
		}
		
		fExportVehicleGoalsScriptInterface( vm );
	}
}

