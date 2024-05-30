#include "GameAppPch.hpp"
#include "tShellLogic.hpp"
#include "tLevelLogic.hpp"
#include "tGameApp.hpp"
#include "tSync.hpp"

using namespace Sig::Math;

namespace Sig
{
	devvar( f32, Gameplay_Weapon_ShellCam_ShellSteerRate, 0.5f );

	namespace
	{
		const tStringPtr cBurstAudio( "Play_Weapon_Shell_BreakApart" );
	}

	tShellLogic::tShellLogic( )
		: mShellCamSteerRate( Gameplay_Weapon_ShellCam_ShellSteerRate )
		, mRotation( 0.f )
		, mRotationRate( 0.f )
		, mOriginalX( 0.f )
		, mBurstCount( 0 )
		, mBurstDamageMod( 1.f )
		, mBurstCountdown( -1.f )
		, mBurstOffset( 0.5f )
		, mBurstSpread( 2.f )
	{
	}
	void tShellLogic::fMoveST( f32 dt )
	{
		tProjectileLogic::fMoveST( dt );

		if( mBurstCount && mBurstCountdown > 0 )
		{
			mBurstCountdown -= dt;

			if( mBurstCountdown < 0.f )
				fBurst( dt );
		}
	}

	void tShellLogic::fInitPhysics( )
	{
		mOriginalX = fOwnerEntity( )->fObjectToWorld( ).fXAxis( );
		mPhysics.mPosition = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		mPhysics.mVelocity = mLaunchVector;

		if( mDamageID.mDesc )
		{
			mPhysics.mGravity.y = mDamageID.mDesc->fShellGravity( );
			mRotationRate = mDamageID.mDesc->mProjectileSpin;
		}
	}

	void tShellLogic::fComputeNewPosition( f32 dt )
	{
		dt *= mTimeMultiplier;
		tVec3f newP = mPhysics.fStep( dt );

		tVec3f dir = newP - fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		dir.fNormalizeSafe( tVec3f::cZAxis );
		mNextPos.fOrientXAxisZAxis( mOriginalX, dir );
		mNextPos.fSetTranslation( newP );

		mRotation += mRotationRate * dt;
		mNextPos = mNextPos * tMat3f( tQuatf( tAxisAnglef( tVec3f::cZAxis, mRotation ) ) );
		sigassert( !mNextPos.fIsNan( ) );

		fCheckLevelBounds( );
	}

	f32 tShellLogic::fUpdateShellCam( tPlayer& player, f32 dt, u32 inputFilter )
	{
		const tMat3f& transform = fOwnerEntity( )->fObjectToWorld( );
		const tVec2f aimStick = player.fGameController( )->fAimStick( tUserProfile::cProfileShellCam, inputFilter );
		const tVec2f moveStick = player.fGameController( )->fMoveStick( tUserProfile::cProfileShellCam, inputFilter );

		fStepTimeMultiplier( moveStick.y, dt );

		tMat3f newTransform;

		// steering stuff
		f32 steer = -aimStick.x * mShellCamSteerRate;

		newTransform = tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, steer * dt ) ) );

		tVec3f& vel = fCurrentVelocity( );
		sigassert( !vel.fIsNan( ) );
		vel = newTransform.fXformVector( vel );

		//rotate in place "cheaply"
		mOriginalX = newTransform.fXformVector( mOriginalX );
		mOriginalX.fNormalizeSafe( tVec3f::cXAxis );
		newTransform = newTransform * transform;
		newTransform.fSetTranslation( transform.fGetTranslation( ) );

		fOwnerEntity( )->fMoveTo( newTransform );

		return steer;
	}

	void tShellLogic::fInherit( tProjectileLogic& from, f32 dt )
	{
		tProjectileLogic::fInherit( from, dt );

		tShellLogic* other = from.fDynamicCast<tShellLogic>( );
		if( other )
		{
			mPhysics = other->mPhysics;
			mShellCamSteerRate = other->mShellCamSteerRate;
			fSetDamageMod( other->mBurstDamageMod );
			
			mBurstOffset = other->mBurstOffset;
			mBurstSpread = other->mBurstSpread;
			mDamageMod = other->mBurstDamageMod;

			Math::tVec3f offset = sync_rand( fVecNorm<Math::tVec3f>( ) );	
			offset.z = fAbs( offset.z );

			Math::tMat3f xform = from.fOwnerEntity( )->fObjectToWorld( );
			offset = xform.fXformVector( offset );

			mPhysics.mPosition += offset * fBurstOffset( );
			mPhysics.mVelocity += offset * fBurstSpread( );
			mOriginalX = other->mOriginalX;

			xform.fSetTranslation( mPhysics.mPosition );
			fOwnerEntity( )->fMoveTo( xform );
		}

		fComputeNewPosition( dt );
	}

	void tShellLogic::fBurst( f32 dt )
	{
		if( fBurstCount( ) < 1 || !fSceneGraph( ) )
			return;

		tEntity* parent = tGameApp::fInstance( ).fCurrentLevelDemand( )->fRootEntity( );
		tShellCameraPtr cam = fShellCam( );

		if( fBurstEffect( ).fExists( ) )
			tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), fBurstEffect( ) );

		const tFilePathPtr& path = ( fBurstPath( ).fExists( ) )
			? fBurstPath( )
			: fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( )->fSgResource( )->fGetPath( );

		if( mSoundSource )
			mSoundSource->fHandleEvent( cBurstAudio );

		u32 bCount = fBurstCount( );
		for( u32 i = 0; i < bCount; ++i )
		{
			tEntity* newEnt = parent->fSpawnChild( path );
			if( newEnt )
			{
				tProjectileLogic* proj = newEnt->fLogicDerived<tProjectileLogic>( );
				sigassert( proj );
				proj->fInherit( *this, dt );

				if( cam )
				{
					cam->fAddProjectile( proj );
					proj->fSetShellCam( cam );
				}
				else
				{
					if( mDamageID.mWeapon )
						mDamageID.mWeapon->fAddProjectileToLastFiredList( proj );
				}
			}
		}

		if( cam )
			cam->fBurst( );

		fProjectileDelete( );
	}
}


namespace Sig
{
	namespace
	{
		// This projectile logic is specific to the ussr scud missile so far

		class tScudMissile : public tShellLogic
		{
			define_dynamic_cast( tScudMissile, tShellLogic );

			f32 mLaunchHesitationTimer;
			
		public:
			tScudMissile( )
				: tShellLogic( )
				, mLaunchHesitationTimer( 1.5f )
			{
			
			}
			
			virtual void fComputeNewPosition( f32 dt )
			{
				f32 timeMultiplier = fTimeMultiplier( );
				if( mLaunchHesitationTimer > 0.f )
				{
					mLaunchHesitationTimer -= dt;
					f32 delta = mLaunchHesitationTimer / 1.5f;
					timeMultiplier = 1.f - delta;
					fSetTimeMultiplier( timeMultiplier*timeMultiplier*timeMultiplier );
					tShellLogic::fComputeNewPosition( dt );
					if( mLaunchHesitationTimer < 0.f )
						fSetTimeMultiplier( 1.f );
				}
				if( mLaunchHesitationTimer < 0.f )
				{
					tShellLogic::fComputeNewPosition( dt * timeMultiplier );
				}
			}
			
			virtual void fHitSomething( const tEntityPtr& ent )
			{
				if( mLaunchHesitationTimer > 0.f )
					fIgnoreHit( );
				else
					tShellLogic::fHitSomething( ent );
			}
		};




		class tSubMissile : public tShellLogic
		{
			define_dynamic_cast( tSubMissile, tShellLogic );

			f32 mLaunchHesitationTimer;
			Math::tVec3f mGroundTargetPos;
			Math::tVec3f mSpotAtLastStageChange;
			Math::tVec3f mCurrentHeadingIs;
			Math::tVec3f mCurrentHeadingShouldBe;
			Math::tVec3f mOffsetVector;
			
			f32 mInitialVelocity;
			f32 mAcceleration;
			f32 mTurnRate;
			s32 mCurrentStage;
		public:
			tSubMissile( )
				: tShellLogic( )
				, mLaunchHesitationTimer( 2.f )
				, mGroundTargetPos( Math::tVec3f::cZeroVector )
				, mCurrentHeadingIs( Math::tVec3f::cYAxis )
				, mCurrentHeadingShouldBe( mCurrentHeadingIs )
				, mInitialVelocity( -1.f )
				, mAcceleration( 10.f )
				, mTurnRate( 0.f )
				, mCurrentStage( -1 )	//-1 launch, 0 go up, 1 go wide, 2 come closer, 3 go home.
			{
				mOffsetVector.x = sync_rand( fFloatInRange( 350.f, 500.f ) );
				mOffsetVector.z = sync_rand( fFloatInRange( 10.f, 50.f ) );
				mOffsetVector.y = sync_rand( fFloatInRange( 50.f, 100.f ) );
				if( sync_rand( fBool( ) ) )
					mOffsetVector.x *= -1.f;
			}
			virtual void fSetTarget( tEntityPtr& target, const Math::tVec3f& targetPt )
			{
				mGroundTargetPos = targetPt;
			}	
			virtual void fComputeNewPosition( f32 dt )
			{
				f32 timeMultiplier = fTimeMultiplier( );
				if( mLaunchHesitationTimer > 0.f )
				{
					mLaunchHesitationTimer -= dt;
					timeMultiplier *= ( 2.f - mLaunchHesitationTimer );
				}
				else
				{
					mInitialVelocity += mAcceleration * dt;
					mTurnRate += ( 1.f * dt );
				}
				dt *= timeMultiplier;

				tVec3f newP = fPhysics( )->fStep( dt, 1.5f );
				
				if( mInitialVelocity < 0.f )
				{
					mInitialVelocity = fPhysics( )->mVelocity.fLength( );
					mCurrentHeadingIs = fOwnerEntity( )->fObjectToWorld( ).fZAxis( );
				}

				if( mCurrentStage == -1 )	//launching, don't do much
				{
					if( mLaunchHesitationTimer < 0.f )
					{
						mSpotAtLastStageChange = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
						mCurrentStage = 0;
					}
				}
				if( mCurrentStage == 0 )	//okay zoom straight up, well not straight up, but mostly...
				{
					f32 distanceSinceLastChange = ( newP - mSpotAtLastStageChange ).fLength( );
					if( distanceSinceLastChange > 5.f )
					{
						mSpotAtLastStageChange = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
						mOffsetVector += mSpotAtLastStageChange;
						mCurrentStage = 1;
						mTurnRate = 0.f;
					}
				}
				
				if( mCurrentStage == 1 )	//turn to the left or right
				{
					mCurrentHeadingShouldBe = mOffsetVector - newP;
					mCurrentHeadingShouldBe.fNormalize( );

					f32 distanceSinceLastChange = ( newP - mSpotAtLastStageChange ).fProjectToXZ( ).fLength( );
					if( distanceSinceLastChange > 100.f )
					{
						mSpotAtLastStageChange = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
						mCurrentStage = 2;
						mTurnRate = 1.f;
					}
				}
				if( mCurrentStage == 2 )	//come closer
				{
					Math::tVec3f closerPos = mGroundTargetPos;
					closerPos.y = newP.y - 5.f;
					mCurrentHeadingShouldBe = closerPos - newP;
					mCurrentHeadingShouldBe.fNormalize( );
					f32 howClose = ( newP - closerPos ).fLength( );
					if( howClose < 80.f )
					{
						mSpotAtLastStageChange = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
						mCurrentStage = 3;
						mTurnRate = 2.f;
					}
				}
				
				if( mCurrentStage == 3  )	//take it home
				{
					mTurnRate += ( 1.f * dt );
					mCurrentHeadingShouldBe = mGroundTargetPos - newP;
					mCurrentHeadingShouldBe.fNormalize( );
				}
				
				
				mCurrentHeadingIs = Math::fLerp( mCurrentHeadingIs, mCurrentHeadingShouldBe, mTurnRate * dt );
				mCurrentHeadingIs.fNormalizeSafe( tVec3f::cZAxis );
				fPhysics( )->mVelocity = mCurrentHeadingIs * mInitialVelocity;

				mNextPos.fOrientZAxis( mCurrentHeadingIs, fOwnerEntity( )->fObjectToWorld( ).fYAxis( ) );
				mNextPos.fSetTranslation( newP );

				f32 rotation = fRotation( ) + ( fRotationRate( ) * dt );
				fSetRotation( rotation );
				mNextPos = mNextPos * tMat3f( tQuatf( tAxisAnglef( tVec3f::cZAxis, fRotation( ) ) ) );
				sigassert( !mNextPos.fIsNan( ) );
			}
			virtual void fMoveST( f32 dt )
			{
				tShellLogic::fMoveST( dt );
			}
			virtual void fHitSomething( const tEntityPtr& ent )
			{
				if( mLaunchHesitationTimer > 0.f )
					fIgnoreHit( );
				else
					tShellLogic::fHitSomething( ent );
			}
		};
	}


	void tShellLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tShellLogic, tProjectileLogic, Sqrat::NoCopy<tShellLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("ShellCamSteerRate"), &tShellLogic::mShellCamSteerRate)
			.Var(_SC("BurstCount"), &tShellLogic::mBurstCount)
			.Var(_SC("BurstDamageMod"), &tShellLogic::mBurstDamageMod)
			.Var(_SC("BurstCountdown"), &tShellLogic::mBurstCountdown)
			.Var(_SC("BurstPath"), &tShellLogic::mBurstPath)
			.Var(_SC("BurstEffect"), &tShellLogic::mBurstEffect)
			.Var(_SC("BurstOffset"), &tShellLogic::mBurstOffset)
			.Var(_SC("BurstSpread"), &tShellLogic::mBurstSpread)
			;
		vm.fRootTable( ).Bind(_SC("ShellLogic"), classDesc);


		{
			Sqrat::DerivedClass<tScudMissile, tShellLogic, Sqrat::NoCopy<tScudMissile> > classDesc( vm.fSq( ) );
			classDesc
				;
			vm.fRootTable( ).Bind(_SC("ScudMissile"), classDesc);
		}
		{
			Sqrat::DerivedClass<tSubMissile, tShellLogic, Sqrat::NoCopy<tSubMissile> > classDesc( vm.fSq( ) );
			classDesc
				;
			vm.fRootTable( ).Bind(_SC("SubMissile"), classDesc);
		}
	}
}
