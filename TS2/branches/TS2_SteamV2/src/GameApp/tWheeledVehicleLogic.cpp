#include "GameAppPch.hpp"
#include "tWheeledVehicleLogic.hpp"
#include "tVehiclePathFollowing.hpp"
#include "tGameApp.hpp"
#include "tSceneGraph.hpp"
#include "tCharacterLogic.hpp"
#include "tUseVehicleCamera.hpp"
#include "tTankPalette.hpp"
#include "tGameEffects.hpp"
#include "tAirborneLogic.hpp"
#include "tLevelLogic.hpp"
#include "tReferenceFrameEntity.hpp"
#include "tSync.hpp"
#include "tTurretLogic.hpp"

#include "Wwise_IDs.h"

// Extra stuff
#include "tVehicleLagCamera.hpp"

using namespace Sig::Math;
using namespace Sig::Physics;

namespace Sig
{

	devvar( f32, Gameplay_Vehicle_CharacterLaunchHorz, 8.0f );
	devvar( f32, Gameplay_Vehicle_CharacterLaunchVert, 6.0f );
	devvar( f32, Gameplay_Vehicle_CharacterLaunchRand, 4.0f );
	devvar( f32, Gameplay_Vehicle_ReverseThreshold, 0.85f ); 

	devvar( f32, Gameplay_Vehicle_Boost, 3.0f );
	devvar( f32, Gameplay_Vehicle_ImpactDoorPopThresh, 1.177f );
	devvar( f32, Gameplay_Vehicle_BrakesLightsDormant, 0.6f );
	devvar( f32, Gameplay_Vehicle_BrakesLightsActive, 2.0f );
	devvar( f32, Gameplay_Vehicle_BurnoutFadeTime, 2.0f );
	devvar( bool, Debug_Vehicle_ShowWheelBounds, false );
	devvar( bool, Debug_Vehicle_ShowStats, false );

	devvar( f32, Gameplay_Vehicle_Parachute_Speed, 10.0f );
	devvar( f32, Gameplay_Vehicle_Parachute_DecelerateLerp, 0.1f );

	
	namespace 
	{ 

		enum tGearIndexes
		{
			cGearIndex1,
			cGearIndex2,
			cGearIndex3,
			cGearIndex4,
			cGearIndex5,
			cGearIndex6,
			cGearIndexReverse,
			cGearIndexCount,
		};

		const u32 cGearShifts[ cGearIndexCount ] = 
		{ 
			AK::SWITCHES::TRANSMISSION_GEAR::SWITCH::GEAR1,
			AK::SWITCHES::TRANSMISSION_GEAR::SWITCH::GEAR2,
			AK::SWITCHES::TRANSMISSION_GEAR::SWITCH::GEAR3,
			AK::SWITCHES::TRANSMISSION_GEAR::SWITCH::GEAR4,
			AK::SWITCHES::TRANSMISSION_GEAR::SWITCH::GEAR5,
			AK::SWITCHES::TRANSMISSION_GEAR::SWITCH::GEAR6,
			AK::SWITCHES::TRANSMISSION_GEAR::SWITCH::GEARREV
		};

		static const tStringPtr cLFW( "LFW" ); 
		static const tStringPtr cRFW( "RFW" ); 
		static const tStringPtr cLRW( "LRW" ); 
		static const tStringPtr cRRW( "RRW" );
		static const tStringPtr cScrewWheel( "screwWheel" ); 

		// FENDERS
		static const tStringPtr cLFF( "LFF" ); 
		static const tStringPtr cRFF( "RFF" ); 
		static const tStringPtr cLRF( "LRF" ); 
		static const tStringPtr cRRF( "RRF" ); 

		static const tStringPtr cVehicleShape( "vehicleShape" ); 		
		static const tStringPtr cTurret( "turret" );
		static const tStringPtr cDeliveryBase( "deliveryBase" );
		static const tStringPtr cTrailer( "trailer" );
		static const tStringPtr cTrailerPivot( "trailerPivot" );
		static const tStringPtr cDummyWheel( "dummyWheel" );
		static const tStringPtr cBoostEffect( "wheeled_vehicle_boost" );
		static const tStringPtr cHitInfantryEffect( "drove_over_infantry" );
		static const tStringPtr cParachute( "parachute" );

		static const tStringPtr cStringLeftTread( "leftTread" );
		static const tStringPtr cStringRightTread( "rightTread" );
		static const tStringPtr cBrakeLights( "brake_lights" );

		//param table column names
		enum tVehicleTableParams
		{
			cVehicleTableParamMass,
			cVehicleTableParamInertiaScale,
			cVehicleTableParamAcceleration,
			cVehicleTableParamDeceleration,
			cVehicleTableParamMaxSpeed,
			cVehicleTableParamAiMaxThrottle,
			cVehicleTableParamAIThrottleBlend,
			cVehicleTableParamSpeedDamping,
			cVehicleTableParamSteerSpeedDamping,
			cVehicleTableParamAngDamping,
			cVehicleTableParamForceMixing,
			cVehicleTableParamDampMixingCompress,
			cVehicleTableParamDampMixingExtend,
			cVehicleTableParamOrientationLimit, //deprecated
			cVehicleTableParamOrientationSpring, //deprecated
			cVehicleTableParamSteerLock,
			cVehicleTableParamSteerLockMaxSpeed,
			cVehicleTableParamTurnRoll,
			cVehicleTableParamAccDive,
			cVehicleTableParamIsTank,
			cVehicleTableParamTreadScale,
			cVehicleTableParamCameraTurnIn,
			cVehicleTableParamCameraZoomOutDist,
			cVehicleTableParamCameraLerp,
			cVehicleTableParamCameraPitchRange,
			cVehicleTableParamImpactImpulse,
			cVehicleTableParamDamageImpulse,
			cVehicleTableParamImpulseLimit,
			cVehicleTableParamTransmissionGears,
			cVehicleTableParamTransmissionSpeedMult,
			cVehicleTableParamTransmissionIdle,
			cVehicleTableParamTransmissionUpShiftDelay,
			cVehicleTableParamTransmissionDownShiftDelay,
			cVehicleTableParamTransmissionDownShiftPoint,
			cVehicleTableParamGravity,
			cVehicleTableParamProbeLength,
			// The following are only for Full3D Mode
			cVehicleTableParamSpringMin,
			cVehicleTableParamSpringMax,
			cVehicleTableParamDampMin,
			cVehicleTableParamDampMax,
			cVehicleTableParamFull3DMode,
		};

		const tStringPtr cPhysicsTableName( "VEHICLE" );
		
	}


	tTransmission::tTransmission( )
		: mIdle( 0.1f )
		, mShiftDelay( 0 )
		, mUpShiftDelayTime( 0.75f )
		, mDownShiftDelayTime( 0.25f )
		, mDownShiftPoint( 0.75f )
		, mGear( 0 )
		, mRPM( 0.f )
		, mClutch( 0.f )
		, mRedLining( false )
		, mReversing( false )
		, mShiftHold( false )
	{
	}

	void tTransmission::fSetup( u32 gears, f32 transMaxSpeed )
	{
		sigassert( gears > 0 && "Need positive gear count" );
		sigassert( transMaxSpeed > 0 && "Need positive transMaxSpeed" );
		mGears.fSetCount( gears );

		// n gears evenly spaced works pretty well
		f32 delta = transMaxSpeed / gears;
		for( u32 i = 0; i < gears; ++i )
		{
			f32 ratio = fComputeRatio( (i+1) * delta );
			f32 torque = (transMaxSpeed - ratio * 0.75f) / transMaxSpeed; //leaves 25% torque in top gear
			mGears[ i ] = tGear( ratio, torque );
			//log_line( 0, "Ratio: " << ratio << " Torque: " << torque );
		}
	}

	void tTransmission::fSetupAdvanced( f32 idle, f32 upShiftDelay, f32 downShiftDelay, f32 downShiftPoint )
	{
		sigassert( fInBounds( idle, 0.f, 0.5f ) );
		mIdle = idle;
		mUpShiftDelayTime = upShiftDelay;
		mDownShiftDelayTime = downShiftDelay;
		mDownShiftPoint = downShiftPoint;
	}

	void tTransmission::fStep( f32 dt, f32 speed, tResults& output )
	{
		sigassert( mGears.fCount( ) > 0 && "Call fSetup on tTransmission" );
		b32 redLine = false;

		output.mUpShift = false;
		output.mDownShift = false;
		output.mChangedDirections = false;

		f32 absSpeed = fAbs( speed );
		f32 targetRPM = fMax( mIdle, absSpeed / fRatio( ) );

		if( absSpeed > fTopSpeed( ) )
		{
			if( mShiftHold || fTopGear( ) ) redLine = true;
			else
			{
				output.mUpShift = true;
				fUpShift( );
			}
		}

		if( absSpeed < fMinSpeed( ) )
		{
			if( !mShiftHold && !fBottomGear( ) ) 
			{ 
				fDownShift( );
				output.mDownShift = true;
			}
		}

		mShiftDelay -= dt;
		f32 shiftDelay = fShiftDelayPercentage( );
		if( shiftDelay > 0.f )
		{
			mClutch = 0.f;
			mRPM = fLerp( mRPM, targetRPM, (1.f - shiftDelay) );
		}
		else
		{
			mClutch = 1.f;
			mRPM = targetRPM;
		}

		// Handle state change info
		mRedLining = redLine;

		if( !mShiftHold )
		{
			if( !mReversing && speed < 0.f )
			{
				mReversing = true;
				output.mChangedDirections = true;
			}
			else if( mReversing && speed > 0.f )
			{
				mReversing = false;
				output.mChangedDirections = true;
			}
		}
	}

	//////////////////////////////////////////////////////////////

	tWheeledVehicleLogic::tWheeledVehicleLogic( )
		: mGroundHeightOffset( 0.f )
		, mFollowDistance( 0.f )
		, mAngDamping( 0.f )
		, mMaxSpeed( 0.f )
		, mWheelRumble(0.0f) 
		, mImpulseMeter( 0.f )
		, mNextSurfaceTypeMT( GameFlags::cSURFACE_TYPE_DIRT )
		, mDeliveryLogic( NULL )
		, mReverseTimer( 0.f )
		, mIsTrailer( false )
		, mBurnout( false )
		, mParachuting( false )
		, mChuteOpen( false )
		, mBurnoutTimer( 0.f )
		, mRestTimer( 0.f )
		, mRobotAttackTimer( 0.f )
		, mForceFireState( false )
		, mForceFireTier( 0 )
		, mForceFireTimerInterval( 60 )
		, mForceFireTimer( 0.f )
		, mFlyingBaseLogic( NULL )
		, mFlyingBaseInitialized( false )
		, mParachuteLogic( NULL )
		, mWheelDustSigmlPath( "Effects\\Entities\\Vehicles\\wheel_dust_small.sigml" )

	{
		mUnitPath->fSetDistanceTolerance( 2.f );
		mUnitPath->fSetFlatPath( true );
		mDoCollisionTest = true;
		mOversizeCollisionTest = true;
		mBoostEffect = cBoostEffect;
		mSurfaceTypesMT.fFill( GameFlags::cSURFACE_TYPE_DIRT );
		mThrottleScaleTarget = 0.5f;
	}

	void tWheeledVehicleLogic::fOnSpawn( )
	{
		tVehicleLogic::fOnSpawn( );

		if( mFlyingBasePath.fExists( ) )
			fAttachFlyingBase( true );
	}

	void tWheeledVehicleLogic::fOnDelete( )
	{
		mPhysics.fOnDelete( );
		mScrewWheels.fSetCount( 0 );
		mWheelEntities.fSetCount( 0 );
		mFenderEntities.fSetCount( 0 );
		mWheelDusts.fSetCount( 0 );
		mDeliveryBase.fRelease( );
		mTrailer.fRelease( );
		mTrailerConstraint.fRelease( );

		if( mFlyingBaseEntity )
			mFlyingBaseEntity->fDelete( );
		mFlyingBaseEntity.fRelease( );
		mFlyingBaseLogic = NULL;

		mAudio->fHandleEvent( AK::EVENTS::STOP_WHEEL );

		tVehicleLogic::fOnDelete( );
	}

	Math::tVec4f tWheeledVehicleLogic::fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const
	{
		if( varName == cStringLeftTread )
			return Math::tVec4f( mPhysics.fTankTreadPosition( )[ 0 ] );
		else if( varName == cStringRightTread )
			return Math::tVec4f( mPhysics.fTankTreadPosition( )[ 1 ] );
		else if( varName == cBrakeLights )
			return (mPhysics.fInput( ).mUserControl && (!fEqual( mPhysics.fInput( ).mDeccelerate, 0.f ) || mPhysics.fInput( ).mHandBrake) ) ? tVec4f( Gameplay_Vehicle_BrakesLightsActive ) : tVec4f( Gameplay_Vehicle_BrakesLightsDormant );
		else
			return tVehicleLogic::fQueryDynamicVec4Var( varName, viewportIndex );
	}

	void tWheeledVehicleLogic::fSetupVehicle( )
	{
		f32 frontWheelbase = 1.f;
		f32 frontTrack = 1.f;
		f32 rearWheelbase = 1.f;
		f32 rearTrack = 1.f;
		f32 wheelRadius = 1.f;
		f32 wheelHeight = 1.f;

		Physics::tVehicleProperties props;
		props.mGroundMask = GameFlags::cFLAG_GROUND | GameFlags::cFLAG_INSTA_DESTROY;
		props.mSurfaceTypeEnumID = GameFlags::cENUM_SURFACE_TYPE;
		props.mHalfExtents = fOwnerEntity( )->fCombinedObjectSpaceBox( ).mMax;
		props.mCallback = this;

		fApplyTableProperties( props );

		mFollowDistance = sync_rand( fFloatInRange( 2.0f, 10.0f ) );
		mDetectionPrimitive = tSpheref( tVec3f::cZeroVector, 200.0 );

		// find everything else
		props.mWheels.fResize( 4 );
		mWheelEntities.fSetCount( 4 );
		mFenderEntities.fSetCount( 4 );
		tEntityPtr shapeEntity;
		tEntityPtr collisionEntity;
		tEntityPtr trailerPivot;

		tGrowableArray<tEntityPtr> extraWheels;
		tGrowableArray<tEntityPtr> screwWheels;

		u32 foundCnt = 0;
		for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
		{
			tEntityPtr child = fOwnerEntity( )->fChild( i );
			
			if( child->fName( ) == cVehicleShape )
			{
				shapeEntity = child;
				props.mHalfExtents = child->fParentRelative( ).fGetScale( );
			}
			else if( child->fHasGameTagsAny( GameFlags::cFLAG_INWARD_COLLISION ) )
			{
				collisionEntity = child;
			}
			else if( child->fName( ) == cLFW )
			{
				tVec3f pos = child->fParentRelative( ).fGetTranslation( );
				frontWheelbase = pos.z;
				frontTrack = pos.x;
				wheelHeight = pos.y;

				Math::tAabbf wheelShape = child->fCombinedObjectSpaceBox( ); // tMeshEntity::fCombinedObjectSpaceBox( *child ); //
				wheelShape = wheelShape.fTransform( child->fParentRelative( ) );
				mDebugWheelShape = wheelShape; //debugging
				tVec3f diag = wheelShape.fComputeDiagonal( );
				wheelRadius = diag.y / 2.0f;
				props.mWheels[ GameFlags::cWHEEL_INDEX_LEFT_FRONT ] = tWheelProperties( child->fParentRelative( ), wheelRadius, props.mIsTank ? 0.0f : 1.0f, props.mProbeLength );
				mWheelEntities[ GameFlags::cWHEEL_INDEX_LEFT_FRONT ] = child;
				++foundCnt;
			}
			else if( child->fName( ) == cLRW )
			{
				tVec3f pos = child->fParentRelative( ).fGetTranslation( );
				rearWheelbase = pos.z;
				rearTrack = pos.x;
				props.mWheels[ GameFlags::cWHEEL_INDEX_LEFT_REAR ] = tWheelProperties( child->fParentRelative( ), wheelRadius, 0.f, props.mProbeLength );
				mWheelEntities[ GameFlags::cWHEEL_INDEX_LEFT_REAR ] = child;
				++foundCnt;
			}
			else if( child->fName( ) == cRFW )
			{
				props.mWheels[ GameFlags::cWHEEL_INDEX_RIGHT_FRONT ] = tWheelProperties( child->fParentRelative( ), wheelRadius, props.mIsTank ? 0.0f : 1.0f, props.mProbeLength );
				mWheelEntities[ GameFlags::cWHEEL_INDEX_RIGHT_FRONT ] = child;
				++foundCnt;
			}
			else if( child->fName( ) == cRRW )
			{
				props.mWheels[ GameFlags::cWHEEL_INDEX_RIGHT_REAR ] = tWheelProperties( child->fParentRelative( ), wheelRadius, 0.f, props.mProbeLength );
				mWheelEntities[ GameFlags::cWHEEL_INDEX_RIGHT_REAR ] = child;
				++foundCnt;
			}
			else if( child->fName( ) == cLFF )
			{
				mFenderEntities[ GameFlags::cWHEEL_INDEX_LEFT_FRONT ] = child;
				props.mHasFenders = true;
			}
			else if( child->fName( ) == cLRF )
			{
				mFenderEntities[ GameFlags::cWHEEL_INDEX_LEFT_REAR ] = child;
				props.mHasFenders = true;
			}
			else if( child->fName( ) == cRFF )
			{
				mFenderEntities[ GameFlags::cWHEEL_INDEX_RIGHT_FRONT ] = child;
				props.mHasFenders = true;
			}
			else if( child->fName( ) == cRRF )
			{
				mFenderEntities[ GameFlags::cWHEEL_INDEX_RIGHT_REAR ] = child;
				props.mHasFenders = true;
			}
			else if( child->fName( ) == cDeliveryBase )
			{
				mDeliveryBase = child;
				mDeliveryLogic = child->fLogicDerived<tTankPalette>( );
				if( !mDeliveryLogic ) mDeliveryBase.fRelease( );
			}	
			else if( child->fName( ) == cTrailer )
			{
				mTrailer = child;
			}	
			else if( child->fName( ) == cTrailerPivot )
			{
				trailerPivot = child;
			}
			else if( child->fName( ) == cDummyWheel  )
			{
				extraWheels.fPushBack( child );
			}
			else if( child->fName( ) == cScrewWheel  )
			{
				screwWheels.fPushBack( child );
			}
			else
			{
				// Need to do special check because some fx could be sigmls.
				tEntity* child = tReferenceFrameEntity::fSkipOverRefFrameEnts( fOwnerEntity( )->fChild( i ).fGetRawPtr( ) );
				if( mParachuting && child->fName( ) == cParachute )
				{
					mParachute.fReset( child );
					mParachuteLogic = mParachute->fLogic( );
					mParachuteLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FALL ) );
				}
			}
		}

		if( screwWheels.fCount( ) )
		{
			mScrewWheels.fSetCount( 2 );
			for( u32 i = 0; i < screwWheels.fCount( ); ++i )
			{
				if( screwWheels[ i ]->fParentRelative( ).fGetTranslation( ).x > 0 )
					mScrewWheels[ 0 ] = screwWheels[ i ];
				else
					mScrewWheels[ 1 ] = screwWheels[ i ];
			}
		}

		if( mWheelDustSigmlPath.fExists( ) )
		{
			for( u32 w = mWheelEntities.fCount( )/2; w < mWheelEntities.fCount( ); ++w )
			{
				tEntity* wheelDustFx = fOwnerEntity( )->fSpawnChild( mWheelDustSigmlPath );
				Math::tMat3f xform( Math::tMat3f::cIdentity );
				xform.fSetTranslation( mWheelEntities[ w ]->fParentRelative( ).fGetTranslation( ) );
				wheelDustFx->fMoveTo( xform );

				for( u32 i = 0; i < wheelDustFx->fChildCount( ); ++i )
				{
					const tEntityPtr& e = wheelDustFx->fChild( i );
					FX::tFxFileRefEntity *fx = e->fDynamicCast< FX::tFxFileRefEntity >( );
					if( fx )
						mWheelDusts.fPushBack( FX::tFxFileRefEntityPtr( fx ) );
				}
			}
		}

		sigassert( mWheelEntities[ 0 ] );

		f32 sagDist = 0.f;
		if( !mFull3DPhysics )
		{
			// compensate for spring sag, this equation is so jacked. but not changing it yet to support legacy content
			f32 frameRate = 30.0f;
			f32 springRate = props.mMass * props.mForceMixing * frameRate;
			f32 gravityForce = props.mMass * props.mGravity;
			f32 sagForce = gravityForce / props.mWheels.fCount( );

			f32 fudgeFact = 0.75f; //due to damping i assume
			sagDist = sagForce / springRate * fudgeFact * props.mWheels[0].mProbeLength;
			//log_line( 0, "SD: " << sagDist );
		}

		sigassert( sagDist == sagDist && "Nan sagDist!" );

		for( u32 i = 0; i < extraWheels.fCount( ); ++i )
		{
			tEntityPtr& e = extraWheels[ i ];
			u32 index = e->fQueryEnumValue( GameFlags::cENUM_WHEEL_INDEX );
			if( index != ~0 )
			{
				mWheelEntities.fPushBack( e );
				props.mWheels.fPushBack( tWheelProperties( e->fParentRelative( ), wheelRadius ) );
				props.mWheels.fBack( ).mFollowIndex = index;
			}
		}
		for( u32 i = 0; i < props.mWheels.fCount( ); ++i )
		{
			props.mWheels[i].mPosition.fTranslateGlobal( tVec3f( 0, sagDist, 0 ) );
			props.mWheels[i].mRadius = wheelRadius;
		}
		
		if( props.mWheels.fCount( ) )
		{
			sigassert( wheelRadius == wheelRadius && "Nan wheelRadius!" );
			mGroundHeightOffset = wheelRadius + -props.mWheels[0].mPosition.fGetTranslation( ).y;
		}

		if( props.mIsTank ) 
			mWheelEntities.fSetCount( 0 ); //ditch wheels
		else
		{
			for( u32 i = 0; i < mWheelEntities.fCount( ); ++i )
				if( mWheelEntities[ i ] )
					tUnitTintLogic::fAddTo( mWheelEntities[ i ] );
			for( u32 i = 0; i < mFenderEntities.fCount( ); ++i )
				if( mFenderEntities[ i ] )
					tUnitTintLogic::fAddTo( mFenderEntities[ i ] );
		}

		props.mFrontWheelbase = frontWheelbase;
		props.mRearWheelbase = rearWheelbase;
		props.mPivotPt = props.mIsTank ? 0.0f : rearWheelbase;
		props.mTrack = rearTrack;

		mPhysics.fSetProperties( props );

		if( collisionEntity )
			mCollisionBounds = tAabbf( collisionEntity->fParentRelative( ).fXformPoint( tVec3f(-1,-1,-1) ), collisionEntity->fParentRelative( ).fXformPoint( tVec3f(1,1,1) ) );
		else if( shapeEntity )
			mCollisionBounds = tAabbf( shapeEntity->fParentRelative( ).fXformPoint( tVec3f(-1,-1,-1) ), shapeEntity->fParentRelative( ).fXformPoint( tVec3f(1,1,1) ) );
		else
			mCollisionBounds = tAabbf( -props.mHalfExtents, props.mHalfExtents );

		sigassert( mGroundHeightOffset == mGroundHeightOffset && "Nan offset!" );
		fOwnerEntity( )->fTranslate( Math::tVec3f( 0, mGroundHeightOffset, 0 ) );
		mPhysics.fReset( fOwnerEntity( )->fObjectToWorld( ) );

		mMass = props.mMass;

		// Trailer stuff
		if( mTrailer && trailerPivot ) 
		{
			tWheeledVehicleLogic* trailer = mTrailer->fLogicDerived<tWheeledVehicleLogic>( );
			sigassert( trailer  );

			trailer->fSetIsTrailer( fOwnerEntity( ) );
			mTrailer->fSetLockedToParent( false );
			mTrailerConstraint.fReset( NEW Physics::tPinConstraint( fOwnerEntity( ), mTrailer.fGetRawPtr( ), trailerPivot->fParentRelative( ).fGetTranslation( ) ) );
			mIgnoreCollisionFrom.fPushBack( mTrailer );
		}

		fApplySurfaceSwitch( );

		if( !mSelectionShapes.fCount( ) && shapeEntity )
		{
			tShapeEntity* shape = shapeEntity->fStaticCast<tShapeEntity>( );
			mSelectionShapes.fPushBack( tShapeEntityPtr( shape ) );

			tSpheref sphere = shape->fParentRelativeSphere( );
			mSelectionRadius = sphere.mCenter.fLength( ) + sphere.mRadius;
		}
	}

	void tWheeledVehicleLogic::fApplyTableProperties( Physics::tVehicleProperties &props )
	{
		const tStringHashDataTable* params = tGameApp::fInstance( ).fUnitsPhysicsTable( mCountry ).fFindTable( cPhysicsTableName );
		log_assert( params, "No vehicle properties loaded!" );

		u32 row = params->fRowIndex( GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) );
		log_assert( row != ~0, "No Vehicle physics properties found for: " << GameFlags::fUNIT_IDEnumToValueString( fUnitID( ) ) );

		props.mMass				= params->fIndexByRowCol<f32>( row, cVehicleTableParamMass );
		props.mInertiaScale		= params->fIndexByRowCol<f32>( row, cVehicleTableParamInertiaScale );
		props.mAcceleration		= params->fIndexByRowCol<f32>( row, cVehicleTableParamAcceleration );
		props.mDecceleration	= params->fIndexByRowCol<f32>( row, cVehicleTableParamDeceleration );
		props.mSpeedDamping		= params->fIndexByRowCol<f32>( row, cVehicleTableParamSpeedDamping );
		props.mSteerSpeedDamping = params->fIndexByRowCol<f32>( row, cVehicleTableParamSteerSpeedDamping );
		props.mAngDamping		= params->fIndexByRowCol<f32>( row, cVehicleTableParamAngDamping );
		props.mForceMixing		= params->fIndexByRowCol<f32>( row, cVehicleTableParamForceMixing );
		props.mDampMixingCompress	= params->fIndexByRowCol<f32>( row, cVehicleTableParamDampMixingCompress);
		props.mDampMixingExtend		= params->fIndexByRowCol<f32>( row, cVehicleTableParamDampMixingExtend );
		props.mMaxSpeed			= params->fIndexByRowCol<f32>( row, cVehicleTableParamMaxSpeed );
		props.mMaxAIThrottle	= params->fIndexByRowCol<f32>( row, cVehicleTableParamAiMaxThrottle );
		props.mOrientationLimit		= fToRadians( params->fIndexByRowCol<f32>( row, cVehicleTableParamOrientationLimit ) );
		props.mOrientationSpring	= params->fIndexByRowCol<f32>( row, cVehicleTableParamOrientationSpring );
		props.mSteerLock		= fToRadians( params->fIndexByRowCol<f32>( row, cVehicleTableParamSteerLock ) );
		props.mSteerLockMaxSpeed = params->fIndexByRowCol<f32>( row, cVehicleTableParamSteerLockMaxSpeed );
		props.mTurnRoll			= params->fIndexByRowCol<f32>( row, cVehicleTableParamTurnRoll );
		props.mAccelerationDive = params->fIndexByRowCol<f32>( row, cVehicleTableParamAccDive );
		props.mIsTank			= params->fIndexByRowCol<f32>( row, cVehicleTableParamIsTank ) > 0 ? true : false;
		props.mTreadScalar		= params->fIndexByRowCol<f32>( row, cVehicleTableParamTreadScale );
		props.mImpactImpulse	= params->fIndexByRowCol<f32>( row, cVehicleTableParamImpactImpulse );
		props.mDamageImpulse	= params->fIndexByRowCol<f32>( row, cVehicleTableParamDamageImpulse );
		props.mImpulseLimit		= params->fIndexByRowCol<f32>( row, cVehicleTableParamImpulseLimit );
		props.mGravity			= params->fIndexByRowCol<f32>( row, cVehicleTableParamGravity );
		props.mProbeLength		= params->fIndexByRowCol<f32>( row, cVehicleTableParamProbeLength );


		props.mSpringMax		= params->fIndexByRowCol<f32>( row, cVehicleTableParamSpringMax );
		props.mSpringMin		= params->fIndexByRowCol<f32>( row, cVehicleTableParamSpringMin );
		props.mDampMax			= params->fIndexByRowCol<f32>( row, cVehicleTableParamDampMax );
		props.mDampMin			= params->fIndexByRowCol<f32>( row, cVehicleTableParamDampMin );

		mFull3DPhysics			= (b32)params->fIndexByRowCol<f32>( row, cVehicleTableParamFull3DMode );
		mPhysics.fSet3DSimulationMode( mFull3DPhysics );	
		
		mCameraData.mCameraTurnIn	= fToRadians( params->fIndexByRowCol<f32>( row, cVehicleTableParamCameraTurnIn ) );
		mCameraData.mCameraZoomOut	= params->fIndexByRowCol<f32>( row, cVehicleTableParamCameraZoomOutDist );
		mCameraData.mCameraLerp		= params->fIndexByRowCol<f32>( row, cVehicleTableParamCameraLerp );

		tVec4f cameraPitchRange		= params->fIndexByRowCol<tVec4f>( row, cVehicleTableParamCameraPitchRange );
		for( u32 i = 0; i < cVehicleSeatCount; ++i )
			mCameraMovement[ i ].fSetMinMaxPitch( fToRadians( cameraPitchRange.x ), fToRadians( cameraPitchRange.y ) );

		mAngDamping = props.mAngDamping;
		mMaxSpeed = props.mMaxSpeed;

		mThrottleBlend = params->fIndexByRowCol<f32>( row, cVehicleTableParamAIThrottleBlend );

		mTrans.fSetup( params->fIndexByRowCol<u32>( row, cVehicleTableParamTransmissionGears )
			, mMaxSpeed * params->fIndexByRowCol<f32>( row, cVehicleTableParamTransmissionSpeedMult ) );

		mTrans.fSetupAdvanced( params->fIndexByRowCol<f32>( row, cVehicleTableParamTransmissionIdle )
			, params->fIndexByRowCol<f32>( row, cVehicleTableParamTransmissionUpShiftDelay )
			, params->fIndexByRowCol<f32>( row, cVehicleTableParamTransmissionDownShiftDelay )
			, params->fIndexByRowCol<f32>( row, cVehicleTableParamTransmissionDownShiftPoint ) );
	}

	void tWheeledVehicleLogic::fReapplyTable( )
	{
		Physics::tVehicleProperties props = mPhysics.fProperties( );
		fApplyTableProperties( props );
		mPhysics.fSetProperties( props );
	}

	void tWheeledVehicleLogic::fApplySurfaceSwitch( )
	{
		if( mNextSurfaceTypeMT != ~0 )
		{
			mAudio->fSetSwitch( tGameApp::cSurfaceTypeSwitchGroup, GameFlags::fSURFACE_TYPEEnumToValueString( mNextSurfaceTypeMT ) );
			mNextSurfaceTypeMT = ~0;
		}
	}

	void tWheeledVehicleLogic::fPushCamera( tPlayer* player, u32 seat )
	{
		tVec3f lookDir = player->fCameraStackTop( )->fViewport( )->fRenderCamera( ).fLocalToWorld( ).fZAxis( ).fProjectToXZAndNormalize( );
		mCameraMovement[ seat ].fReset( lookDir );
		
		if( mFull3DPhysics )
			player->fPushCamera( Gfx::tCameraControllerPtr( NEW tVehicleLagCamera( *player, seat, *this ) ) );			
		else
			player->fPushCamera( Gfx::tCameraControllerPtr( NEW tUseVehicleCamera( *player, seat, *this ) ) );
	}

	void tWheeledVehicleLogic::fPopCamera( tPlayer* player )
	{
		if( mFull3DPhysics )
			player->fCameraStack( ).fPopCamerasOfType<tVehicleLagCamera>( );
		else
			player->fCameraStack( ).fPopCamerasOfType<tUseVehicleCamera>( );
	}
	
	void tWheeledVehicleLogic::fComputeUserInput( f32 dt )
	{
		profile( cProfilePerfVehicleLogicUserST );

		if( fSeatOccupied( cVehicleDriverSeat ) )
		{
			tPlayer* player = fGetPlayer( cVehicleDriverSeat );

			if( fIsTank( ) && (!fHasWeaponStation( cVehicleDriverSeat ) || !fWeaponStation( cVehicleDriverSeat )->fShellCaming( )) )
			{
				// Rotate camera
				tVec2f stick = player->fGameController( )->fAimStick( tUserProfile::cProfileVehicles );
				if( player->fGameController( )->fMode( ) == tGameController::KeyboardMouse )
				{
					stick *= 1.1f;
				}
				mCameraMovement[ cVehicleDriverSeat ].fSetSpeed( fUseCamRotSpeed( ) );
				mCameraMovement[ cVehicleDriverSeat ].fSetDamping( fUnitAttributeUseCamRotDamping( ).fXY( ) );
				mCameraMovement[ cVehicleDriverSeat ].fUpdate( dt, stick );
			}

			Physics::tVehicleInput input;
			input.fZero( );
			input.mUserControl = true;

			tVec2f moveStick = player->fGameController( )->fMoveStick( tUserProfile::cProfileVehicles );

			tVec2f stick = moveStick;
			if( !mFull3DPhysics )
				stick = Input::tGamepad::fMapStickCircleToRectangle( stick );

			const f32 stickMag = moveStick.fLength( );
			f32 intendedSpeed = 0.f;

			if( fIsTank( ) )
			{
				tVec3f stick3d( -stick.x, 0, stick.y );
				tVec3f worldDir = mCameraMovement[ cVehicleDriverSeat ].fFlatCamera( ).fXformVector( stick3d );
				const tVec3f leveledDir = mPhysics.fTransform( ).fZAxis( ).fProjectToXZAndNormalize( );

				f32 inputAngle = fAcos( worldDir.fDot( leveledDir ) );
				b32 driveBackwards = false;

				//if already going backwards, favor backwards
				const f32 speedThresh = 0.1f;
				if( mPhysics.fSpeed( ) < -speedThresh )		
				{
					f32 thresh = (1.f - Gameplay_Vehicle_ReverseThreshold);
					driveBackwards = inputAngle > cPi * thresh;
				}
				else		
				{
					f32 thresh = Gameplay_Vehicle_ReverseThreshold;
					driveBackwards = inputAngle > cPi * thresh;
				}
				

				if( driveBackwards ) 
					worldDir *= -1.f;
				input.mSteer = mPhysics.fComputeSteerInputForTankToReachHeading( worldDir ) * stickMag;

				intendedSpeed = stickMag - pow( input.mSteer, 2.f );
				intendedSpeed *= mPhysics.fMaxSpeed( );
				if( driveBackwards ) intendedSpeed *= -1.f;
			}
			else
			{
				input.mSteer = -fPow( stick.x, 3.f );
				intendedSpeed = stick.y * mPhysics.fMaxSpeed( );
			}

			b32 shellCaming = fHasWeaponStation( cVehicleDriverSeat ) && fWeaponStation( cVehicleDriverSeat )->fShellCaming( );
			b32 inputLocked = shellCaming || tGameApp::fInstance( ).fCurrentLevelDemand( )->fDisableVehicleInput( );

			if( inputLocked )
			{
				intendedSpeed = 0.f;
				if( fIsTank( ) )
					input.mSteer = 0.f;
			}

			if( mFull3DPhysics )
			{
				const f32 cMaxStickActivation = 0.5f;
				const f32 cMinSpeed = 0.1f;
				const f32 cReverseDelay = 0.5f;
				f32 velMag = fAbs( mPhysics.fSpeed( ) ); //mPhysics.fVelocity( ).fLength( );

				if( inputLocked )
				{
					input.mDeccelerate = 1.f;
					input.mAccelerate = player->fGameController( )->fGetAcceleration( tUserProfile::cProfileVehicles );
				}
				else
				{
					input.mAccelerate = player->fGameController( )->fGetAcceleration( tUserProfile::cProfileVehicles );
					input.mDeccelerate = player->fGameController( )->fGetDeceleration( tUserProfile::cProfileVehicles );
					input.mHandBrake = player->fGameController( )->fButtonHeld( tUserProfile::cProfileVehicles, GameFlags::cGAME_CONTROLS_HANDBRAKE );

					fBoostInput( );

					if( mBoostTimer > 0.f )
					{
						f32 taper = fBoostProgress( );
						input.mBoost = Gameplay_Vehicle_Boost * taper;
						input.mMaxSpeedMult += 0.5f * taper;
					}

					if( velMag < cMinSpeed )
					{
						if( player->fGameController( )->fGetAcceleration( tUserProfile::cProfileVehicles ) > cMaxStickActivation )
							mReverseTimer = fMax( mReverseTimer - dt, 0.f );
						else if( player->fGameController( )->fGetDeceleration( tUserProfile::cProfileVehicles ) > cMaxStickActivation )
							mReverseTimer = fMin( mReverseTimer + dt, cReverseDelay * 2.f );
					}

					if( mReverseTimer > cReverseDelay )
					{
						//reversing
						input.mAccelerate = -player->fGameController( )->fGetDeceleration( tUserProfile::cProfileVehicles );
						input.mDeccelerate = player->fGameController( )->fGetAcceleration( tUserProfile::cProfileVehicles );
					}
				}

				// can burn out even if input is locked
				if( input.mDeccelerate != 0.f )
				{
					if( velMag < cMinSpeed && player->fGameController( )->fButtonDown( tUserProfile::cProfileVehicles, GameFlags::cGAME_CONTROLS_ACCELERATE ) )
						mBurnout = true;

					if( !mBurnout )
						input.mAccelerate = 0.f;
				}
				else
					mBurnout = false;

				if( mBurnout )
					mBurnoutTimer = Gameplay_Vehicle_BurnoutFadeTime;
				else
					mBurnoutTimer -= dt;

				input.mBurnout = fMax( 0.f, mBurnoutTimer / Gameplay_Vehicle_BurnoutFadeTime );
			}
			else
			{
				mPhysics.fComputeAccelerationAndDeccelerationToReachSpeed( intendedSpeed, mPhysics.fSpeed( ), dt, input, stick.y );
			}

			input.mClutch *= mTrans.fClutch( );
			mPhysics.fSetInput( input );
			mTrans.fSetShiftHold( !mPhysics.fBackAxle( ).mStaticFriction );

		}

		if( fSeatOccupied( cVehicleGunnerSeat ) )
		{
			if( fIsTank( ) && (!fHasWeaponStation( cVehicleGunnerSeat ) || !fWeaponStation( cVehicleGunnerSeat )->fShellCaming( )) )
			{
				// Rotate camera
				tVec2f stick = fGetPlayer( cVehicleGunnerSeat )->fGameController( )->fAimStick( tUserProfile::cProfileVehicles );
				mCameraMovement[ cVehicleGunnerSeat ].fSetSpeed( fUseCamRotSpeed( ) );
				mCameraMovement[ cVehicleGunnerSeat ].fSetDamping( fUnitAttributeUseCamRotDamping( ).fXY( ) );
				mCameraMovement[ cVehicleGunnerSeat ].fUpdate( dt, stick );
			}
		}
		
		tVehicleLogic::fComputeUserInput( dt );
	}
	
	void tWheeledVehicleLogic::fComputeAIInput( f32 dt )
	{
		profile( cProfilePerfVehicleLogicAIST );

		tVehicleInput input;
		input.fZero( );
		input.mUserControl = false;

		if( !mIsTrailer )
		{
			if( mReverseTimer > 0 )
			{
				f32 intendedSpeed = -mPhysics.fMaxSpeed( );
				mPhysics.fComputeAccelerationAndDeccelerationToReachSpeed( intendedSpeed, mPhysics.fSpeed( ), dt, input );
				mReverseTimer -= 1.0f;
				if( mReverseTimer <= 0.0f )
				{
					mReverseTimer = 0.0f;
					fSetRestTimer( 150.0f );
				} 
			}
			else if( mUnitPath->fHasWaypoints( ) )
				mFollower.fComputeInput( input, *this, *mUnitPath.fGetRawPtr( ), mOffenders, dt, &fSceneGraph( )->fDebugGeometry( ) );
			else
			{
				f32 intendedSpeed = 0.0f;
				mPhysics.fComputeAccelerationAndDeccelerationToReachSpeed( intendedSpeed, mPhysics.fSpeed( ), dt, input );
			}
		}

		mPhysics.fSetInput( input );
		tVehicleLogic::fComputeAIInput( dt );
	}

	void tWheeledVehicleLogic::fRespawn( const Math::tMat3f& tm )
	{
		mPhysics.fReset( tm );
		fOwnerEntity( )->fMoveTo( tm );
		tVehicleLogic::fRespawn( tm );
	}

	void tWheeledVehicleLogic::fAnimateMT( f32 dt )
	{
		profile( cProfilePerfVehicleLogicAnimateMT );
		dt *= fTimeScale( );

		mAnimatable.fAnimateMT( dt );

		mPhysics.fSetTransform( fOwnerEntity( )->fObjectToWorld( ) );
	}

	void tWheeledVehicleLogic::fPhysicsMT( f32 dt )
	{
		profile( cProfilePerfVehicleLogicPhysicsMT );
		dt *= fTimeScale( );

		//physics

		if( mLockedToPathStart )
		{
			const tPRSXformf& delta = mAnimatable.fAnimatedSkeleton( )->fRefFrameDelta( );
			mPhysics.fSetTransform( mPhysics.fApplyRefFrameDelta( fOwnerEntity( )->fObjectToWorld( ), delta ) );
			mPhysics.fSetVelocity( delta.mP / dt );
		}
		else if( !fOnFlyingBase( ) )
		{
			mPhysics.fProperties( ).mAngDamping = mFallingThrough ? 1.0f : mAngDamping;

			mPhysics.fSetDoGroundCheck( !mFallingThrough );
			mPhysics.fPhysicsMT( this, dt );
		}

		fUpdateCharactersMT( dt );

		// surface audio stuff
		if( mPhysics.fWheelStates( ).fCount( ) >= 2 )
		{
			u32 newSurface = ~0;
			for( u32 i = 0; i < 2; ++i )
			{
				u32 st = mPhysics.fWheelStates( )[ i ].mSurfaceType;
				if( st != ~0 && st != mSurfaceTypesMT[ i ] )
				{
					mSurfaceTypesMT[ i ] = st;
					newSurface = st;
				}
			}

			if( newSurface != ~0 )
				mNextSurfaceTypeMT = newSurface;
		}

		if( mParachuteLogic && mChuteOpen )
		{
			if( mChuteOpen )
			{
				tVec3f vel = mPhysics.fVelocity( );
				f32 idealySpeed = fMax( vel.y, -(f32)Gameplay_Vehicle_Parachute_Speed );
				vel.y = fLerp( vel.y, idealySpeed, (f32)Gameplay_Vehicle_Parachute_DecelerateLerp );
				mPhysics.fSetVelocity( vel );
			}
		}

		tVehicleLogic::fPhysicsMT( dt );
	}

	void tWheeledVehicleLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicMoveST );
		dt *= fTimeScale( );
		mAnimatable.fMoveST( dt );

		const tMat3f &transform = mPhysics.fTransform( );
		
		if( !fOnFlyingBase( ) )
			fOwnerEntity( )->fMoveTo( transform );

		for( u32 i = 0; i < cVehicleSeatCount; ++i )
			mCameraMovement[ i ].fCameraBasis( ).fSetTranslation( transform.fGetTranslation( ) );

		u32 wCnt = mWheelEntities.fCount( );
		for( u32 w = 0; w < wCnt; ++w )
		{
			mWheelEntities[w]->fMoveTo( mPhysics.fWheelStates( )[w].mTransform );

			if( w < mFenderEntities.fCount( ) && mFenderEntities[ w ] )
				mFenderEntities[ w ]->fMoveTo( mPhysics.fWheelStates( )[w].mFenderTransform );

			if( Debug_Vehicle_ShowWheelBounds && w == 0 )
			{
				tMat3f x = transform;
				fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tObbf( mDebugWheelShape, x ), tVec4f(1,0,0,0.25f) ); 
			}
		}

		if( mScrewWheels.fCount( ) )
		{
			if( mScrewWheels[ 0 ] )
				mScrewWheels[ 0 ]->fMoveTo( mScrewWheels[ 0 ]->fObjectToWorld( ) * tMat3f( tQuatf( tAxisAnglef( tVec3f::cZAxis, mPhysics.fWheelStates( )[ 0 ].mAngVel * dt ) ) ) );
			if( mScrewWheels[ 1 ] )
				mScrewWheels[ 1 ]->fMoveTo( mScrewWheels[ 1 ]->fObjectToWorld( ) * tMat3f( tQuatf( tAxisAnglef( tVec3f::cZAxis, mPhysics.fWheelStates( )[ 1 ].mAngVel * dt ) ) ) );
		}

		f32 speedEffect = mPhysics.fSpeedPercent( );
		if( mPersistentEffect )
		{
			mPersistentEffect->mUserScale = speedEffect;
			if( mPhysics.fProperties( ).mIsTank )
				mPersistentEffect->mUserScale = fMin( 1.f, mPersistentEffect->mUserScale + fAbs( mPhysics.fSteering( ) * 0.75f ) );
		}

		for( u32 i = 0; i < mWheelDusts.fCount( ); ++i )
		{
			if( mFull3DPhysics ) 
				mWheelDusts[ i ]->fSetEmissionPercent( ( mPhysics.fAxles( )[ i / 2 ].mTemperature + mPhysics.fAxles( )[ i / 2 ].mSlipMagnitude) * 3.0f );
			else
				mWheelDusts[ i ]->fSetEmissionPercent( speedEffect );
		}

		tVehicleLogic::fMoveST( dt );
	}

	void tWheeledVehicleLogic::fThinkST( f32 dt )
	{
		profile( cProfilePerfVehicleLogicThinkST );
		dt *= fTimeScale( );

		if( mRestTimer > 0 )
		{
			mRestTimer -= 1;
			if( mRestTimer <= 0 )
			{
				mRestTimer = 0;
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_REST_END ) );
			}
		}
		if( mRobotAttackTimer > 0 )
		{
			mRobotAttackTimer -= 1;
			if( mRobotAttackTimer <= 0 )
			{
				mRobotAttackTimer = 0;
				fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_ROBOT_ATTACK ) );
			}
		}
		if( mForceFireState )
		{
			if( mForceFireTimer <= 0 )
			{
				mForceFireTimer = mForceFireTimerInterval;
			}
			mForceFireTimer -= 1;
			if( mForceFireTimer <= 0 )
			{
				if ( mForceFireTier == 2 )
					fForceFireSecondaryTurrets( );
				else if (mForceFireTier == 3 )
					fForceFireTertiaryTurrets( );
				else
					fForceFireTurrets( );
			}
		}
		mImpulseMeter -= dt * mPhysics.fProperties( ).mImpulseLimit;
		mImpulseMeter = fMax( mImpulseMeter, 0.f );

		mPhysics.fThinkST( this, dt );
		mPhysics.fDrawDebugInfo( this );

		const tMat3f &transform = fOwnerEntity( )->fObjectToWorld( );
		const tVec3f pos = transform.fGetTranslation( );

		tTransmission::tResults transmissionResults;
		mTrans.fStep( dt, mPhysics.fSpeed( ), transmissionResults );
		
		// Drive is how much load the user is requesting. putsing along or sitting stationary is zero load
		f32 audioSpeed = mPhysics.fSpeedPercent( );
		f32 audioDrive = fAbs( mPhysics.fInput( ).mAccelerate ) * mTrans.fClutch( );
		f32 skid = fAbs( mPhysics.fSteering( ) );
		if( mFull3DPhysics )
		{
			skid = mPhysics.fFrontAxle( ).mSlipMagnitude + mPhysics.fBackAxle( ).mSlipMagnitude;
			skid = fClamp( skid, 0.f, 1.f );
		}

		mAudio->fSetGameParam( AK::GAME_PARAMETERS::SPEED, audioSpeed );
		mAudio->fSetGameParam( AK::GAME_PARAMETERS::SKID_INTENSITY, skid );
		mAudio->fSetGameParamSmooth( AK::GAME_PARAMETERS::RPM, pow( mTrans.fRPMPercentage( ), 2.f ) * 0.5f, fAudioRPMBlend( ) );
		mAudio->fSetGameParamSmooth( AK::GAME_PARAMETERS::DRIVE, audioDrive, fAudioDriveBlend( ) );
		if( transmissionResults.fGearChanged( ) ) 
		{
			u32 gearIndex = fMin( mTrans.fGear( ), cGearIndexReverse - 1u );
			if( mTrans.fReverse( ) ) gearIndex = cGearIndexReverse;

			mAudio->fSetSwitch( AK::SWITCHES::TRANSMISSION_GEAR::GROUP, cGearShifts[ gearIndex ] );
			mAudio->fHandleEvent( AK::EVENTS::PLAY_GEAR_SHIFT );
		}

		fApplySurfaceSwitch( );

		//DEBUGGING
		if( fUnderUserControl( ) && fHasWeaponStation( 0 ) && (fShowStats( ) || (mFull3DPhysics && Debug_Vehicle_ShowStats)) )
		{
			const Gui::tWeaponUIPtr& ui = fWeaponStation( 0 )->fUI( );
			if( ui ) 
			{
				ui->fSetVehicleStats( audioSpeed, mTrans.fRPMPercentage( ), audioDrive * 0.5f );

				for( u32 i = 0; i < 2; ++i )
				{
					const Physics::tVehiclePhysics::tAxle& axle = mPhysics.fAxles( )[ i ];
					ui->fSetWheelStats( ).Execute( i, axle.fMaxFrictionGs( ), !axle.mStaticFriction, tVec3f(axle.mFrictionAcc,0.f), axle.mCompression/*axle.mSuspensionAcc*/, axle.mSlipRatio, axle.mTemperature );
				}
			}
		}

		if( fUnderUserControl( ) )
		{
			if( tPlayer::fIsMedTank( fUnitID( ) ) )
				fGetPlayer( cVehicleDriverSeat )->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_USING_MEDIUM_TANK, dt );
			else if( tPlayer::fIsHeavyTank( fUnitID( ) ) )
				fGetPlayer( cVehicleDriverSeat )->fStats( ).fIncQuick( GameFlags::cSESSION_STATS_TIME_USING_HEAVY_TANK, dt );
		}

		// handle damage and response for characters we hit
		f32 effect = fMax( 0.25f, mPhysics.fSpeedPercent( ) );

		tDamageContext killDamage;
		killDamage.fSetAttacker( tDamageID( this, fUnderUserControl( ) ? fGetPlayer( cVehicleDriverSeat ) : NULL, fTeam( ) ), GameFlags::cDAMAGE_TYPE_IMPACT );
		killDamage.fSetExplicit( 20000 );

		u32 hitCount = 0;
		for( u32 p = 0; p < mPeepsWeHit.fCount( ); ++p )
		{
			tCharacterLogic* peep = mPeepsWeHit[ p ];
			Logic::tPhysical *phys = peep->fQueryPhysical( );
			if( phys )
			{
				tCharacterPhysics *ch = phys->fDynamicCast<tCharacterPhysics>( );

				if( ch )
				{
					if( (fUnderUserControl( ) && !tGameApp::fExtraDemoMode( ))
						&& peep->fTeam( ) == fGetPlayer( 0 )->fEnemyTeam( )
						&& !peep->fIsCommando( ) )
					{
						// TS2 insta enemy kill
						peep->fSetDieImmediately( true );
						peep->fDealDamage( killDamage );
						++hitCount;
					}
					else if( tGameApp::fExtraDemoMode( ) || (fUnderUserControl( ) && mPeepsWeHit[ p ]->fIsValidInTermsOfUserControl( )) ) // dont "jump" unoccupied selectable characters
					{
						// TS2 friendly collision
						// Extra mode normal behavior

						//jump						
						tVec3f delta = ch->fTransform( ).fGetTranslation( ) - pos;
						delta.y = 0;
						delta.fNormalizeSafe( tVec3f::cYAxis );
						delta *= Gameplay_Vehicle_CharacterLaunchHorz;
						delta.y = Gameplay_Vehicle_CharacterLaunchVert;
						delta += sync_rand( fVecNorm<tVec3f>( ) ) * Gameplay_Vehicle_CharacterLaunchRand;

						delta *= effect;
						delta += mPhysics.fVelocity( );

						ch->fJump( delta );
						++hitCount;
					}
				}
			}
		}

		if( hitCount )
			tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cHitInfantryEffect );

		// collision rumble
		f32 collisionStrength = mPhysics.fGetFeedback( ).mCollisionDeltaV.fLength( ) / mPhysics.fMaxCollisionResponse( );

		if( !fEqual( collisionStrength, 0.0f ) )
			fAddRumbleEvent( Input::tRumbleEvent( fClamp(collisionStrength * 2.0f, 0.0f, 1.0f), 0.0f, 0.5f ) );

		// suspension rumble
		u32 wCnt = mPhysics.fWheelStates( ).fCount( );
		f32 wheelVib = 0.0f;
		b32 popDoors = false;
		for( u32 w = 0; w < wCnt; ++w )
		{
			f32 oneG = 10.0f;
			f32 wheelVel = fAbs(mPhysics.fWheelStates( )[w].mCompressionVel) / oneG;
			if( wheelVel > Gameplay_Vehicle_ImpactDoorPopThresh )
				popDoors = true;

			wheelVel = fMin( wheelVel, 1.0f );
			wheelVib = fMax( wheelVib, wheelVel );
		}

		if( popDoors )
		{
			for( u32 i = 0; i < mSeats.fCount( ); ++i )
			{
				if( mSeats[ i ].mHinge && sync_rand( fFloatZeroToOne( ) ) < 0.5f )
				{
					mSeats[ i ].mHinge->fUnLatch( );
					mSeats[ i ].mHinge->fAutoLatch( 2.0f );
				}
			}
		}

		wheelVib = fSqrt(wheelVib) * 0.5f;
		mWheelRumble = fMax( mWheelRumble, wheelVib );

		f32 decayTime = 0.5f;
		mWheelRumble -= (1.0f / decayTime) * dt;
		mWheelRumble = fMax( 0.0f, mWheelRumble );
		fSetExplicitRumble( mWheelRumble );

		if( mDeliveryLogic && mDeliveryLogic->fFinished( ) && mPhysics.fOnGround( ) )
		{
			mDeliveryLogic = NULL;
			mDeliveryBase->fDelete( );
			mDeliveryBase.fRelease( );
		}

		if( mParachuteLogic && mPhysics.fOnGround( ) )
		{
			mParachuteLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_LAND ) );
			mParachuteLogic = NULL;
			mParachute.fRelease( );
			mParachuting = false;
		}

		if( mTrailerConstraint )
			mTrailerConstraint->fStepST( dt );

		tVehicleLogic::fThinkST( dt );

		if( !mFlyingBaseInitialized && mFlyingBaseLogic && mFlyingBaseLogic->fSceneGraph( ) )
		{
			fInitializeFlyingBase( );
		}

	}

	void tWheeledVehicleLogic::fCoRenderMT( f32 dt )
	{
		profile( cProfilePerfVehicleLogicCoRenderMT );
		dt *= fTimeScale( );

		mPhysics.fCoRenderMT( this, dt );

		tVehicleLogic::fCoRenderMT( dt );
	}

	b32 tWheeledVehicleLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		switch( e.fEventId( ) )
		{
		case GameFlags::cEVENT_UNIT_DESTROYED:
			{
				mPhysics.fSetFullDynamicsTimer( 0.5f );
			}
			break;
		}
		return tVehicleLogic::fHandleLogicEvent( e );
	}
	
	void tWheeledVehicleLogic::fReactToDamage( const tDamageContext& dc, const tDamageResult& dr )
	{
		if( dc.fEffectDamageType( ) == GameFlags::cDAMAGE_TYPE_EXPLOSION && !mParachuteLogic )
		{
			// make sure the final reaction is "epic"
			f32 effect = dr.mEffect;
			if( dr.mDestroysYou )
				effect = fMax( effect, 0.25f ) * 3.0f;

			f32 scale = 1.f;
			if( dc.fWeaponDesc( ) )
				scale = dc.fWeaponDesc( )->mDamageImpulseScale;

			tVec3f myPos = mPhysics.fTransform( ).fGetTranslation( );
			tVec3f forcePt = myPos + dr.mAttackerDirection;

			f32 impulse = fMeterImpulse( effect * mPhysics.fProperties( ).mDamageImpulse * scale );
			tVec3f force( 0, impulse * mPhysics.fProperties( ).mMass * 30.0f, 0 ); //30 hz

			mPhysics.fAddForce( force, forcePt );
			//fSceneGraph( )->fDebugGeometry( ).fRenderOnce( forcePt, forcePt + force, tVec4f(1,0,0,1) );
		}

		if( dr.mDestroysYou ) 
			fEjectAllPassengers( mPhysics.fVelocity( ) + dr.mSpawnInfluence );

		tUnitLogic::fReactToDamage( dc, dr );
	}

	void tWheeledVehicleLogic::fReactToWeaponFire( const tFireEvent& event )
	{
		if( !mParachuteLogic )
		{
			// physical reaction			
			const tMat3f& trans = fOwnerEntity( )->fObjectToWorld( );
			const tVehicleProperties& props = mPhysics.fProperties( );

			tVec3f reactVec = event.mProjectilVel;
			reactVec.fSetLength( -fMeterImpulse( event.mWeapon->fDesc( ).mReactionImpulse ) );

			// project the reponse onto the wheel base so that it is reduced when fired on the narrow direction
			tVec3f projected = trans.fInverseXformVector( reactVec );
			projected *= tVec3f( props.mTrack, 1, props.mFrontWheelbase - props.mRearWheelbase );
			projected = trans.fXformVector( projected );

			const f32 frameRate = 30.0f;
			f32 impulser = props.mMass * frameRate;
			mPhysics.fAddForce( projected * impulser, trans.fXformPoint( event.mLocalMuzzlePt ) );
		}
	}

	f32 tWheeledVehicleLogic::fMeterImpulse( f32 desired )
	{
		f32 existing = mImpulseMeter;
		mImpulseMeter = fMin( mImpulseMeter + desired, mPhysics.fProperties( ).mImpulseLimit );

		f32 leftOver = mImpulseMeter - existing;
		return leftOver;
	}

	void tWheeledVehicleLogic::fSetIsTrailer( tEntity* parent )
	{
		mIsTrailer = true;
		mFull3DPhysics = true;
		mIgnoreCollisionFrom.fPushBack( tEntityPtr( parent ) );
	}

	void tWheeledVehicleLogic::fStartup( b32 startup )
	{
		if( startup )
			mAudio->fHandleEvent( AK::EVENTS::PLAY_WHEEL );

		tVehicleLogic::fStartup( startup );
	}
	
	void tWheeledVehicleLogic::fShutDown( b32 shutDown )
	{
		if( shutDown )
			mAudio->fHandleEvent( AK::EVENTS::STOP_WHEEL );

		tVehicleLogic::fShutDown( shutDown );
	}
	void tWheeledVehicleLogic::fSetRestTimer( f32 restTimer )
	{
		mRestTimer = restTimer;
		fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_REST_BEGIN ) );
	}
	void tWheeledVehicleLogic::fSetRobotAttackTimer( f32 robotAttackTimer )
	{
		mRobotAttackTimer = robotAttackTimer;
	}

	class tDeployChildren
	{
	public:
		b32 operator( ) ( tEntity& e ) const
		{
			if( tTurretLogic* turret = e.fLogicDerived<tTurretLogic>( ) )
			{

				if( turret->fUnitID( ) == GameFlags::cUNIT_ID_BOSS_SUB_SILOS )
				{
					turret->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_DEPLOY_BEGIN ) );
				}
			}
			return false;
		}
	};

	void tWheeledVehicleLogic::fDeployTurrets( )
	{
		fOwnerEntity( )->fForEachDescendent( tDeployChildren( ) );
	}

	class tForceFireChildren
	{
	public:
		b32 operator( ) ( tEntity& e ) const
		{
			if( tTurretLogic* turret = e.fLogicDerived<tTurretLogic>( ) )
			{
				if( ( ( turret->fUnitID( ) == GameFlags::cUNIT_ID_BOSS_SUB_SILOS ) && ( ( turret->fCountry( ) == GameFlags::cCOUNTRY_GERMAN ) || ( turret->fCountry( ) == GameFlags::cCOUNTRY_BRITISH ) ) ) )
				{
					turret->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FORCE_FIRE ) );
				}
			}
			return false;
		}
	};

	void tWheeledVehicleLogic::fForceFireTurrets( )
	{
		fOwnerEntity( )->fForEachDescendent( tForceFireChildren( ) );
	}

	class tForceFireSecondaryChildren
	{
	public:
		b32 operator( ) ( tEntity& e ) const
		{
			if( tTurretLogic* turret = e.fLogicDerived<tTurretLogic>( ) )
			{
				if( ( ( turret->fUnitID( ) == GameFlags::cUNIT_ID_BOSS_SUB_TOWER ) && ( turret->fCountry( ) == GameFlags::cCOUNTRY_BRITISH ) ) )
				{
					turret->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FORCE_FIRE ) );
				}
			}
			return false;
		}
	};

	void tWheeledVehicleLogic::fForceFireSecondaryTurrets( )
	{
		fOwnerEntity( )->fForEachDescendent( tForceFireSecondaryChildren( ) );
		tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), tStringPtr("PLAY_AUDIO_EVENT_DLC2_ROBOT_BOSS_CANNON_FIRE") );
	}

	class tForceFireTertiaryChildren
	{
	public:
		b32 operator( ) ( tEntity& e ) const
		{
			if( tTurretLogic* turret = e.fLogicDerived<tTurretLogic>( ) )
			{
				if( ( ( turret->fUnitID( ) == GameFlags::cUNIT_ID_BOSS_SUPERTANK ) && ( turret->fCountry( ) == GameFlags::cCOUNTRY_BRITISH ) ) )
				{
					turret->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FORCE_FIRE ) );
				}
			}
			return false;
		}
	};

	void tWheeledVehicleLogic::fForceFireTertiaryTurrets( )
	{
		fOwnerEntity( )->fForEachDescendent( tForceFireTertiaryChildren( ) );
		tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), tStringPtr("PLAY_AUDIO_EVENT_DLC2_ROBOT_BOSS_EYE_LASER_BEAM") );
	}

	void tWheeledVehicleLogic::fAttachFlyingBase( b32 attach )
	{
		if( attach )
		{
			if( mFlyingBaseEntity ) 
				fAttachFlyingBase( false );

			if( mFlyingBasePath.fExists( ) )
			{
				mFlyingBaseEntity.fReset( fOwnerEntity( )->fParent( )->fSpawnChild( mFlyingBasePath ) );
				if( mFlyingBaseEntity )
				{
					mFlyingBaseEntity->fMoveTo( fOwnerEntity( )->fObjectToWorld( ) );
					mFlyingBaseLogic = mFlyingBaseEntity->fLogicDerived<tAirborneLogic>( );
					if( !mFlyingBaseLogic )
					{
						log_warning( 0, "Invaid flying base logic type!" );
						mFlyingBaseEntity->fDelete( );
						mFlyingBaseEntity.fRelease( );
						return;
					}

					mSlaveVehicleLogic = mFlyingBaseLogic;
					mFlyingBaseLogic->fSetVehicleController( this );
					mFlyingBaseInitialized = false;
				}
			}
		}
		else
		{
			mFlyingBaseInitialized = false;
			mSlaveVehicleLogic = NULL;

			if( mFlyingBaseEntity )
			{
				fOwnerEntity( )->fReparentImmediate( *mFlyingBaseEntity->fParent( ) );

				mPhysics.fReset( mFlyingBaseEntity->fObjectToWorld( ) );
				mPhysics.fSetVelocity( mFlyingBaseLogic->fPhysics( ).fVelocity( ) );
				mFlyingBaseEntity->fDelete( );
				mFlyingBaseEntity.fRelease( );
				mFlyingBaseLogic = NULL;

				if( !fGiveAGoalPathByUnitType( this, fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) ) )
				{
					log_warning( 0, "No goal path found after detaching from flying base, for unit type: " << GameFlags::fUNIT_TYPEEnumToValueString( fUnitType( ) ) );
				}
			}
		}
	}

	void tWheeledVehicleLogic::fInitializeFlyingBase( )
	{
		mFlyingBaseInitialized = true;
		fOwnerEntity( )->fReparent( *mFlyingBaseEntity );
		fOwnerEntity( )->fMoveTo( tMat3f::cIdentity );
		
		if( mUnitPath->fCurrentPathSequence( ) > -1 )
		{
			tUnitPath* up = mFlyingBaseLogic->fUnitPath( );
			sigassert( up );

			up->fClearPathStarts( );
			up->fAddPathStartEntry( mUnitPath->fSegment( ) );
			up->fStartPathSequence( );

			mUnitPath->fPausePathSequence( );
			mUnitPath->fClearPathStarts( );
			mUnitPath->fClearPath( );
		}
	}

	b32 tWheeledVehicleLogic::fPathPointReached( tPathEntity& point, const Logic::tEventContextPtr& context )
	{
		if( point.fHasGameTagsAll( tEntityTagMask( GameFlags::cFLAG_DETACH_FLYING_BASE ) ) )
		{
			fAttachFlyingBase( 0 );
		}

		return tVehicleLogic::fPathPointReached( point, context );
	}

	b32 tWheeledVehicleLogic::fShouldSuspendWheels( tLogic* logic )
	{
		tUnitLogic* unitLogic = logic->fDynamicCast<tUnitLogic>( );

		// dont suspend on any valid target (unless it has a DONT INSTA DESTROY flag, or if it's impossible to destroy)		
		// also need to suspend on bosses and vehicles for cargo drops
		b32 suspend = !unitLogic 
			|| !unitLogic->fIsValidTarget( ) 
			|| unitLogic->fUnitType( ) == GameFlags::cUNIT_TYPE_BOSS
			|| unitLogic->fUnitType( ) == GameFlags::cUNIT_TYPE_VEHICLE
			|| (unitLogic->fUnitType( ) == GameFlags::cUNIT_TYPE_TURRET && fUnitType( ) == GameFlags::cUNIT_TYPE_INFANTRY ) //atvs should not blow up turrets
			|| (unitLogic->fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_DONT_INSTA_DESTROY ) && !unitLogic->fOwnerEntity( )->fHasGameTagsAny( GameFlags::cFLAG_INSTA_DESTROY ) );

		if( !suspend )
		{
			mAdditionalBreakablePtrs.fPushBack( tEntityPtr( logic->fOwnerEntity( ) ) );
			mBreakablesWeHit.fPushBack( unitLogic );
		}

		return suspend;
	}

}




namespace Sig
{
	void tWheeledVehicleLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tWheeledVehicleLogic, tVehicleLogic, Sqrat::NoCopy<tWheeledVehicleLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("DeployTurrets"),		&tWheeledVehicleLogic::fDeployTurrets)
			.Func(_SC("EnableForceFire"),	&tWheeledVehicleLogic::fEnableForceFire)
			.Func(_SC("SetForceFireTier"),	&tWheeledVehicleLogic::fSetForceFireTier)
			.Func(_SC("SetForceFireTimerInterval"),	&tWheeledVehicleLogic::fSetForceFireTimerInterval)
			.Func(_SC("SetReverseTimer"),	&tWheeledVehicleLogic::fSetReverseTimer)
			.Prop(_SC("RestTimer"),			&tWheeledVehicleLogic::fRestTimer, &tWheeledVehicleLogic::fSetRestTimer)
			.Prop(_SC("RobotAttackTimer"),	&tWheeledVehicleLogic::fRobotAttackTimer, &tWheeledVehicleLogic::fSetRobotAttackTimer)
			.Func(_SC("SetFlyingBase"),		&tWheeledVehicleLogic::fSetFlyingBase)
			.Func(_SC("AttachFlyingBase"),	&tWheeledVehicleLogic::fAttachFlyingBase)
			.Prop(_SC("FlyingBase"),		&tWheeledVehicleLogic::fFlyingBase)
			.Prop(_SC("ChuteOpen"),			&tWheeledVehicleLogic::fChuteOpen, &tWheeledVehicleLogic::fSetChuteOpen)
			.Prop(_SC("Parachuting"),		&tWheeledVehicleLogic::fParachuting, &tWheeledVehicleLogic::fSetParachuteing)
			.Func(_SC("SetWheelDustSigmlPath"),	&tWheeledVehicleLogic::fSetWheelDustSigmlPath)
			;

		vm.fRootTable( ).Bind(_SC("WheeledVehicleLogic"), classDesc);
	}
}

