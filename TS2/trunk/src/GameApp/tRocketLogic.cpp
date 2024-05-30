#include "GameAppPch.hpp"
#include "tRocketLogic.hpp"
#include "tExplosionLogic.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tSync.hpp"

//for hallway rocket:
#include "tUnitLogic.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( f32, Gameplay_Weapon_RocketMinDistToDetonate, 15.f ); //Rocket wont fuse detonate unless it's gotten within 15m of its target.
	devvar_clamp( f32, Gameplay_Weapon_ShellCam_ExtraRocketSteerRate, 0.0f, -2.f, 2.f, 2 );
	devvar( bool, Gameplay_Weapon_ShellCam_RocketPitchInvert, false );

	devvar( f32, Gameplay_Weapon_ShellCam_Rocket_BurstOffset, 0.5f );
	devvar( f32, Gameplay_Weapon_ShellCam_Rocket_BurstSpread, 2.0f );
	devvar( f32, Gameplay_Weapon_ShellCam_RocketHall_PointsRate, 0.05f );
	devvar( f32, Gameplay_Weapon_ShellCam_RocketHall_PointsValue, 100.0f );
	devvar( f32, Gameplay_Weapon_ShellCam_RocketHall_TimeScalePoints, 1.0f );

	devvar( f32, Gameplay_Weapon_ShellCam_RocketHall_VibeStrength, 1.0f );
	devvar( f32, Gameplay_Weapon_ShellCam_RocketHall_VibeDist, 120.0f );
	devvar( f32, Gameplay_Weapon_ShellCam_RocketHall_TargetSlowdown, 0.03f );
	devvar( f32, Gameplay_Weapon_ShellCam_RocketHall_TargetSlowdownTime, 1.0f );



	tRocketLogic::tRocketLogic( )
		: mGuidanceMode( cStraightFire )
		, mNextGuidanceMode( mGuidanceMode )
		, mGuidanceKickinTimer( -1.f )
		, mThrustDelayTimer( -1.f )
		, mSmoothThrust( false )
		, mTargetPoint( tVec3f::cZeroVector )
		, mTargetSet( false )
		, mTargetLogic( NULL )
		, mDistanceToTargetSqrd( 0.f )
		, mInitialDistanceToTarget( 1.f )
		, mTargetDropVelocity( 0.f )
		, mTargetDrop( 0.f )
		, mShellCamSteerRate( 0.5f )
		, mRotation( 0.f )
		, mRotationRate( 10.f )
	{
	}

	void tRocketLogic::fOnSpawn( )
	{
		tProjectileLogic::fOnSpawn( );
	}

	void tRocketLogic::fOnDelete( )
	{
		mTargetEntity.fRelease( );
		tProjectileLogic::fOnDelete( );
	}

	void tRocketLogic::fMoveST( f32 dt )
	{
		// need to move the base projectile before doing any position dependent computation
		tProjectileLogic::fMoveST( dt );

		if( mGuidanceKickinTimer > 0.f )
		{
			mGuidanceKickinTimer -= dt;
			if( mGuidanceKickinTimer <= 0.f )
				mGuidanceMode = mNextGuidanceMode;
		}

		if( !mDeleteMe && (mGuidanceMode != cStraightFire) )
		{
			f32 newDist = (mPhysics.mTransform.fGetTranslation( ) - mTargetPoint).fLengthSquared( );
			if( newDist > mDistanceToTargetSqrd && newDist < fSquare( (f32)Gameplay_Weapon_RocketMinDistToDetonate ) )
				fHitSomething( tEntityPtr( ) );
			else
				mDistanceToTargetSqrd = newDist;
		}
	}
	
	void tRocketLogic::fCoRenderMT( f32 dt )
	{
		if( mTargetSet )
		{
			switch( mGuidanceMode )
			{
			case  cStraightFire:
				{
					mPhysics.mTransform.fTranslateGlobal( tVec3f( 0, mTargetDropVelocity * dt, 0 ) );
				}
				break;
			case cFullLock:
				{
					if( mTargetEntity )
					{
						mTargetPoint = mTargetEntity->fObjectToWorld( ).fGetTranslation( );
						sigassert( mTargetPoint == mTargetPoint );
						if( mTargetLogic )
						{
							const f32 leadMult = 2.f; //extra lead time
							mTargetPoint += mTargetLogic->fLinearVelocity( tVec3f::cZeroVector ) * (dt * leadMult);
						}
						sigassert( mTargetPoint == mTargetPoint );

						if( !mTargetEntity->fSceneGraph( ) )
							mGuidanceMode = cLazyLock;
					}
				}
					// Falls into lazy lock
			case cLazyLock:
				{
					sigassert( mTargetPoint == mTargetPoint );
					sigassert( !mPhysics.mTransform.fIsNan( ) );
					tVec3f newDirection = mTargetPoint + tVec3f( 0, mTargetDrop, 0 ) - mPhysics.mTransform.fGetTranslation( );
					sigassert( newDirection == newDirection );
					newDirection.fNormalizeSafe( tVec3f::cZAxis );
					sigassert( newDirection == newDirection );
					tVec3f current = mPhysics.mTransform.fZAxis( );
					sigassert( current == current );
					current.fNormalizeSafe( tVec3f::cZAxis );
					sigassert( current == current );

					tAxisAnglef delta( current, newDirection );
					sigassert( delta.mAngle == delta.mAngle && delta.mAxis.x == delta.mAxis.x );
					delta.mAngle = fMin( delta.mAngle, fToRadians( mTurnRate ) );
					
					Math::tQuatf deltaQ( delta );
					newDirection = deltaQ.fRotate( current );
					mPhysics.mTransform.fOrientZAxis( newDirection );

					mPhysics.mVelocity = deltaQ.fRotate( mPhysics.mVelocity );
				}
				break;
			}
		}


		if( mPhysics.mTransform.fIsNan( ) )
		{
			log_warning( 0, "last ditch!" );
			mPhysics.mTransform = tMat3f::cIdentity;
			mPhysics.mVelocity = tVec3f::cZAxis;
			mTargetPoint = tVec3f( 0,2,0 );
			mTargetDropVelocity = 0.f;
			mTargetDrop = 0.f;
			mNextPos = mPhysics.mTransform;
			mOutsideLevel = true;
		}

		tProjectileLogic::fCoRenderMT( dt );
	}

	void tRocketLogic::fInitPhysics( )
	{
		sigassert( mDamageID.mDesc && "Not necessarily needed." );
		mPhysics.fSetup( fOwnerEntity( )->fObjectToWorld( ), mLaunchVector, mDamageID.mDesc->mProjectileAcceleration, mThrustDelayTimer, mSmoothThrust );
		sigassert( !mPhysics.mTransform.fIsNan( ) );
		mTurnRate = mDamageID.mDesc->mTurnRate;
		mRotationRate = mDamageID.mDesc->mProjectileSpin;

		mDistanceToTargetSqrd = (mPhysics.mTransform.fGetTranslation( ) - mTargetPoint).fLengthSquared( );
		mDistanceToTargetSqrd = fMax( mDistanceToTargetSqrd, 0.1f );
		mInitialDistanceToTarget = fSqrt( mDistanceToTargetSqrd );
	}

	void tRocketLogic::fComputeNewPosition( f32 dt )
	{
		dt *= mTimeMultiplier;

		if( mDamageID.mDesc )
		{
			mTargetDropVelocity += mDamageID.mDesc->mProjectileGravity * dt;
			mTargetDrop += mTargetDropVelocity * dt;
		}

		mNextPos = mPhysics.fStep( dt );

		mRotation += mRotationRate * dt;

		tMat3f spinMat( tQuatf( tAxisAnglef( tVec3f::cZAxis, mRotation ) ) );
		sigassert( !spinMat.fIsNan( ) );

		mNextPos = mNextPos * spinMat;
		sigassert( !mNextPos.fIsNan( ) );
		sync_event_v_c( mNextPos, tSync::cSCProjectile );

		fCheckLevelBounds( );
	}

	void tRocketLogic::fInherit( tProjectileLogic& from, f32 dt )
	{
		tProjectileLogic::fInherit( from, dt );

		tRocketLogic* other = from.fDynamicCast<tRocketLogic>( );
		if( other )
		{
			mPhysics = other->mPhysics;
			mTargetEntity = other->mTargetEntity;
			mTargetLogic = other->mTargetLogic;
			mTargetPoint = other->mTargetPoint;
			mGuidanceMode = other->mGuidanceMode;
			mNextGuidanceMode = other->mNextGuidanceMode;
			mGuidanceKickinTimer = other->mGuidanceKickinTimer;
			mThrustDelayTimer = other->mThrustDelayTimer;
			mTargetSet = other->mTargetSet;
			mTurnRate = other->mTurnRate;
			mDistanceToTargetSqrd = other->mDistanceToTargetSqrd;
			mInitialDistanceToTarget = other->mInitialDistanceToTarget;
			mTargetDropVelocity = other->mTargetDropVelocity;
			mTargetDrop = other->mTargetDrop;
			mShellCamSteerRate = other->mShellCamSteerRate;

			Math::tVec3f offset = sync_rand( fVecNorm<Math::tVec3f>( ) );
			offset.z = fAbs( offset.z );

			mPhysics.mTransform.fTranslateLocal( offset * Gameplay_Weapon_ShellCam_Rocket_BurstOffset );
			mPhysics.mVelocity += mPhysics.mTransform.fXformVector( offset * Gameplay_Weapon_ShellCam_Rocket_BurstSpread );
		}

		fComputeNewPosition( dt );
	}

	void tRocketLogic::fSetTarget( tEntityPtr& target, const Math::tVec3f& targetPt )
	{
		mTargetEntity = target;
		mTargetLogic = mTargetEntity ? mTargetEntity->fLogicDerived< tUnitLogic >( ) : NULL;
		mTargetPoint = targetPt;
		mTargetSet = true;
	}

	f32 tRocketLogic::fUpdateShellCam( tPlayer& player, f32 dt )
	{
		mGuidanceMode = cStraightFire;
		mInitialDistanceToTarget = 0.f;

		const tVec2f aimStick = Input::tGamepad::fMapStickCircleToRectangle( player.fAimStick( tUserProfile::cProfileShellCam ) );
		const tVec2f speedStick = Input::tGamepad::fMapStickCircleToRectangle( player.fMoveStick( tUserProfile::cProfileShellCam ) );

		fStepTimeMultiplier( speedStick.y, dt );

		tMat3f newTransform;

		// steering stuff
		f32 rate = mShellCamSteerRate + Gameplay_Weapon_ShellCam_ExtraRocketSteerRate;
		f32 steer = -aimStick.x * rate;
		f32 pitch = aimStick.y * rate;

		if( Gameplay_Weapon_ShellCam_RocketPitchInvert )
			pitch *= -1.f;

		tVec3f zAxis = mPhysics.mTransform.fZAxis( );
		f32 dot = zAxis.fDot( tVec3f::cYAxis );
		if( dot > 0.9 && pitch > 0 ) pitch = 0.f;
		if( dot < -0.9 && pitch < 0 ) pitch = 0.f;

		tVec3f xAxis = zAxis.fCross( tVec3f::cYAxis ).fNormalizeSafe( tVec3f::cXAxis );
		newTransform = tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, steer * dt ) ) * tQuatf( tAxisAnglef( xAxis, pitch * dt ) ) );

		tVec3f& vel = mPhysics.mVelocity;
		sigassert( !vel.fIsNan( ) );
		vel = newTransform.fXformVector( vel );
		
		tVec3f dir = vel;
		dir.fNormalizeSafe( mPhysics.mTransform.fZAxis( ) );

		mPhysics.mTransform.fOrientZAxis( dir );

		return steer;
	}

	void tRocketLogic::fHitSomething( const tEntityPtr& ent )
	{
		if( mDamageID.mPlayer && mShellCamera )
		{
			f32 bonus = 0.f;
			f32 topRange = mMaxTimeMultiplier - mUserTimeMultiplier;
			if( topRange > 0.01f && mTimeMultiplier > mUserTimeMultiplier )
			{
				bonus = (mTimeMultiplier - mUserTimeMultiplier) / topRange;
				mDamageID.mSpeedBonus = bonus; // 0 to 1 ratio of bonus
			}
		}

		tProjectileLogic::fHitSomething( ent );
	}

}

namespace Sig
{
	namespace
	{
		// This projectile logic is specific to the missile down the hallway level.

		b32 fZSort( const Math::tVec3f& a, const Math::tVec3f& b )
		{
			return a.z < b.z; 
		}

		static const tStringPtr cPassedTargetEffect( "hallway_passed_target" );

		class tHallwayRocket : public tRocketLogic
		{
			define_dynamic_cast( tHallwayRocket, tRocketLogic );
		public:
			tHallwayRocket( )
				: tRocketLogic( )
				, mWaitingToPass( 0 )
				, mAccumulator( 0 )
				, mCombo( 1 )
				, mSlowdownTimer( 0.f )
				, mSegmentLength( 1.f )
				, mSegmentDist( cInfinity )
				, mTargetIth( 0 )
			{
				fGetData( );
			}

			virtual void fComputeNewPosition( f32 dt )
			{
				tRocketLogic::fComputeNewPosition( dt );

				if( mOutsideLevel && mNextPos.fGetTranslation( ).z > 0 )
					mOutsideLevel = false;
			}

			virtual void fMoveST( f32 dt )
			{
				tRocketLogic::fMoveST( dt );

				// NO SLOWDOWN
				//mSlowdownTimer -= dt;
				//if( mSlowdownTimer > 0.f )
				//	mTimeMultiplier = Gameplay_Weapon_ShellCam_RocketHall_TargetSlowdown;

				if( mNextPos.fGetTranslation( ).z > mSegmentDist )
				{
					mSegmentDist += mSegmentLength;
					mSpawnSegment.Execute( );
					fMoreTargets( );
				}

				if( mWaitingToPass < mTargets.fCount( ) && !mEntityHitWithLogic )
				{
					// NO CONTINUOUS POINTS
					//mAccumulator += dt * mTimeMultiplier;
					//const f32 cRate = Gameplay_Weapon_ShellCam_RocketHall_PointsRate;
					//if( mAccumulator > cRate )
					//{
					//	mAccumulator -= cRate;
					//	f32 timeScale = fMax( mTimeMultiplier, mUserTimeMultiplier ) * Gameplay_Weapon_ShellCam_RocketHall_TimeScalePoints;
					//	mPoints.Execute( Gameplay_Weapon_ShellCam_RocketHall_PointsValue * mCombo * timeScale );
					//}

					const f32 cThresh = 0.f;
					f32 targetZ = mTargets[ mWaitingToPass ] + cThresh;
					f32 dist = targetZ - mNextPos.fGetTranslation( ).z;

					if( dist < 0.f )
					{
						++mWaitingToPass;

						mPoints.Execute( Gameplay_Weapon_ShellCam_RocketHall_PointsValue * mCombo );
						fIncCombo( );
						
						tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cPassedTargetEffect );
					}
					else if( mShellCamera && dist < Gameplay_Weapon_ShellCam_RocketHall_VibeDist )
					{
						f32 scale = 1.f - dist / Gameplay_Weapon_ShellCam_RocketHall_VibeDist;
						mShellCamera->fPlayer( ).fGamepad( ).fRumble( ).fSetExplicitRumble( Gameplay_Weapon_ShellCam_RocketHall_VibeStrength * scale );
					}
				}
			}

			void fMoreTargets( )
			{
				u32 start = mTargets.fCount( );
				mTargets.fJoin( mTargetsOrig );

				for( u32 i = start; i < mTargets.fCount( ); ++i )
					mTargets[ i ] += mTargetIth * mSegmentLength;

				++mTargetIth;
			}

			void fGetData( )
			{
				Sqrat::Function zeroF( tGameApp::fInstance( ).fCurrentLevelForScript( ), "GetSegmentZero" );
				Sqrat::Function lenF( tGameApp::fInstance( ).fCurrentLevelForScript( ), "GetSegmentLength" );
				mSpawnSegment = Sqrat::Function( tGameApp::fInstance( ).fCurrentLevelForScript( ), "SpawnSegment" );
				mPoints = Sqrat::Function( tGameApp::fInstance( ).fCurrentLevelForScript( ), "ExtraMiniGamePoints" );

				sigassert( !mPoints.IsNull( ) && "Need 'ExtraMiniGamePoints' level function!" );
				sigassert( !zeroF.IsNull( ) && "Need 'GetSegmentZero' level function!" );
				sigassert( !lenF.IsNull( ) && "Need 'GetSegmentLength' level function!" );
				sigassert( !mSpawnSegment.IsNull( ) && "Need 'SpawnSegment' level function!" );
				mSegmentLength = lenF.Evaluate<f32>( );
				mSegmentDist = zeroF.Evaluate<f32>( );

				fGetTargets( );
				fMoreTargets( );
			}

			void fGetTargets( )
			{
				Sqrat::Function countF( tGameApp::fInstance( ).fCurrentLevelForScript( ), "GetTargetCount" );
				Sqrat::Function getF( tGameApp::fInstance( ).fCurrentLevelForScript( ), "GetTarget" );

				//if( countF.IsNull( ) )
				//{
				//	log_warning( 0, "wtf hall rocket?" );
				//	return;
				//}

				sigassert( !countF.IsNull( ) && "Need 'GetTargetCount' level function!" );
				sigassert( !getF.IsNull( ) && "Need 'GetTarget' level function!" );


				u32 count = countF.Evaluate<u32>( );
				mTargetsOrig.fSetCapacity( count );

				for( u32 i = 0; i < count; ++i )
				{
					tEntity* e = getF.Evaluate<tEntity*>( i );
					sigassert( e );
					mTargetsOrig.fPushBack( e->fObjectToWorld( ).fGetTranslation( ).z );
				}

				std::sort( mTargetsOrig.fBegin( ), mTargetsOrig.fEnd( ) );
			}

			virtual void fHitSomething( const tEntityPtr& ent )
			{
				if( ent && ( ent->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE ) == GameFlags::cUNIT_TYPE_PICKUP ) )
				{
					tUnitLogic* ul = ent->fLogicDerivedStaticCast<tUnitLogic>( );
					ul->fDestroy( mDamageID.mUnitLogic, true, mDamageID.mPlayer );

					fIgnoreHit( );
					//fIncCombo( );

					mSlowdownTimer = Gameplay_Weapon_ShellCam_RocketHall_TargetSlowdownTime;
				}
				else
				{
					tRocketLogic::fHitSomething( ent );
					tGameApp::fInstance( ).fCurrentLevelDemand( )->fHandleTutorialEventScript( GameFlags::cTUTORIAL_EVENT_MINIGAME_DEFEAT );
				}
			}

			void fIncCombo( )
			{
				++mCombo;
				
				// Dont tell them we're comboing
				//if( mDamageID.mPlayer )
				//	mDamageID.mPlayer->fStats( ).fSpawnComboText( (u32)mCombo, fOwnerEntity( ), tVec4f( 1.0, 0.69f, 0.12f, 0.0f ) );
			}

			Sqrat::Function mPoints;
			Sqrat::Function mSpawnSegment;
			tGrowableArray<f32> mTargetsOrig;
			tGrowableArray<f32> mTargets;
			u32 mWaitingToPass;
			f32 mAccumulator;
			f32 mCombo;
			f32 mSlowdownTimer;
			f32 mSegmentLength;
			f32 mSegmentDist;
			u32 mTargetIth;
		};
	}

	void tRocketLogic::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tRocketLogic, tProjectileLogic, Sqrat::NoCopy<tRocketLogic> > classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("GuidanceMode"), &tRocketLogic::fGuidanceMode, &tRocketLogic::fSetGuidanceMode )
				.Prop(_SC("NextGuidanceMode"), &tRocketLogic::fNextGuidanceMode, &tRocketLogic::fSetNextGuidanceMode )
				.Var(_SC("ShellCamSteerRate"), &tRocketLogic::mShellCamSteerRate)
				.Var(_SC("GuidanceKickinTimer"), &tRocketLogic::mGuidanceKickinTimer)
				.Var(_SC("ThrustDelayTimer"), &tRocketLogic::mThrustDelayTimer)
				.Var(_SC("SmoothThrust"), &tRocketLogic::mSmoothThrust)
				;

			vm.fRootTable( ).Bind(_SC("RocketLogic"), classDesc);
			vm.fConstTable( ).Const(_SC("GUIDANCE_MODE_STRAIGHT_FIRE"),	(s32)cStraightFire );
			vm.fConstTable( ).Const(_SC("GUIDANCE_MODE_FULL_LOCK"),		(s32)cFullLock );
			vm.fConstTable( ).Const(_SC("GUIDANCE_MODE_LAZY_LOCK"),		(s32)cLazyLock );
		}

		{
			Sqrat::DerivedClass<tHallwayRocket, tRocketLogic, Sqrat::NoCopy<tHallwayRocket> > classDesc( vm.fSq( ) );
			classDesc
				;
			vm.fRootTable( ).Bind(_SC("HallwayRocket"), classDesc);
		}
	}
}