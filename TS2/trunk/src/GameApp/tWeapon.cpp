#include "GameAppPch.hpp"
#include "tWeapon.hpp"
#include "tUnitLogic.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tProjectileLogic.hpp"
#include "Input/tGamepad.hpp"
#include "tShellCamera.hpp"
#include "tShellLogic.hpp"
#include "Logic/tAnimatable.hpp"
#include "tDebrisLogic.hpp"
#include "tLightEffectLogic.hpp"
#include "Wwise_IDs.h"
#include "Math/ProjectileUtility.hpp"
#include "tGameEffects.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tSync.hpp"

using namespace Sig::Math;

namespace Sig
{
	devvar( bool, AAACheats_InfiniteAmmo, false );
	devvar_clamp( f32, Anim_PitchRotSpeed, 5.f, 0.1f, 60.f, 1 );
	
	devvar( bool, Debug_Weapons_RenderTargets, false );
	devvar( bool, Debug_Weapons_DisableTargetting, false );
	devvar( bool, Debug_Weapons_RenderGunObstructions, false );
	devvar( bool, Debug_Weapons_AlwaysSpawnTracer, false );
	devvar( bool, Debug_Weapons_UseEntityList, true );
	devvar( f32, Gameplay_Weapon_AcquireCountWeight, 20.f );
	devrgba( Debug_Paths_TargetColor, tVec4f( 1.f, 0.f, 0.f, 0.5f ) );
	devrgba( Debug_Paths_DirectionColor, tVec4f( 1.f, 0.f, 0.f, .5f ) );

	devvar_clamp( f32, Gameplay_Weapon_MinimumLevelHeight, -20.0f, -200.0f, 999999.0f, 0 );
	devvar( f32, Gameplay_Weapon_TargetCooloff, 2.0f );
	devvar( f32, Gameplay_Weapon_PitchSmoothLerp, 0.65f );
	devvar( f32, Gameplay_Weapon_ShellCam_IneligibilityTimer, 0.25f );
	devvar( bool, Perf_Weapons_ParentedTargetting, true );

	devvar( u32, Perf_Weapons_PlayerCastCount, 3 );
	devvar( bool, Perf_Weapons_AlwaysPlayerCast, false );
	devvar( bool, Perf_Audio_DisableBarrelSpin, false );
	//devvar( bool, Perf_Audio_DisableWeaponLoop, false );
	//devvar( bool, Perf_Audio_DisableWeaponFire, false );


	namespace
	{
		struct tLineOfSightRayCastCallback
		{
			mutable tRayCastHit		mHit;
			mutable tEntity*		mHitEntity;
			tEntity*				mIgnore;

			explicit tLineOfSightRayCastCallback( tEntity* ignore = 0 ) 
				: mHitEntity( 0 ), mIgnore( ignore )
			{
			}

			inline void fRayCastAgainstSpatial( const tRayf& ray, tSpatialEntity* spatial ) const
			{
				if( spatial->fToSpatialSetObject( )->fQuickRejectByBox( ray ) )
					return;

				tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( !hit.fHit( ) )
					return;

				if( hit.mT < mHit.mT )
				{
					mHit = hit;
					mHitEntity = spatial;
				}					
			}

			inline void operator( )( const tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;

				tSpatialEntity* spatial = static_cast<tSpatialEntity*>( i );
				if( spatial->fHasGameTagsAny( GameFlags::cFLAG_DUMMY ) )
					return;
				if( mIgnore && (mIgnore == spatial || spatial->fIsAncestorOfMine( *mIgnore )) )
					return;

				fRayCastAgainstSpatial( ray, spatial );
			}
		};

	}

	b32 tWeaponInstData::fUnlimitedAmmo( ) const
	{
		return mOverCharged || AAACheats_InfiniteAmmo;
	}

	const tStringPtr tWeapon::cWeaponAttachName( "weapon" );

	tWeapon::tWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst )
		: mInst( inst )
		, mDesc( desc )
		, mPlayer( NULL )
		, mParentVelocity( tVec3f::cZeroVector )
		, mBulletExplosionLightLogic( NULL )
		, mPredictedTargetPosition( tVec3f::cZeroVector )
		, mAITargetCooloff( 0.f )
		, mReticalTargetPosition( tVec3f::cZeroVector )
		, mWorldSpaceArcTargetPosition( tVec3f::cZeroVector )
		, mReticalTargetLogic( NULL )
		, mYawConstraintAxis( tVec3f::cZAxis )
		, mYawConstaintAngle( 0.f )
		, mIdealFacingDirection( tVec3f::cZAxis )
		, mIdealPitchAngle( fClamp( 0.f, desc.mMinPitch, desc.mMaxPitch ) )
		, mCurrentPitchAngle( fClamp( 0.f, desc.mMinPitch, desc.mMaxPitch ) )
		, mIdealYawAngle( 0.0f )
		, mCurrentYawAngle( 0.0f )
		, mIdealProjectileSpeed( mDesc.mProjectileSpeed )
		, mCurrentProjectileSpeed( mDesc.mProjectileSpeed )
		, mMinimumProjectileSpeed( 0.f )
		, mCurrentSpread( 0.f )
		, mFireTimer( mDesc.mFireRate )
		, mSpinUpPercentage( 1.f )
		, mHasTarget( false )
		, mWithinReach( false )
		, mUseRayCastedTarget( false )
		, mRayCastCounter( 0 )
		, mUseArcTarget( false )
		, mFiring( false )
		, mShellCaming( false )
		, mShellCamDeath ( false )
		, mShellCamPushed( false )
		, mReticalOverTarget( false )
		, mIsHoldAndReleaseFire( false )
		, mYawConstraintSet( false )
		, mInvertPitch( false )
		, mAcquireTargets( false )
		, mReloadTimerActive( false )
		, mReloadOverride( false )
		, mAIFireOverride( false )
		, mEnable( true )
		, mRendering( false )
		, mLoopingAudioPushed( false )
		, mBehaveContinuously( desc.mIsContinuousFire && !desc.mAnimationDriven )
		, mShellCamInitiateTimer( 0.0f )
		, mShellCamDeathTimer( 0.0f )
		, mReloadTimer( 0.f )
		, mShotsFired( 0 )
		, mNextMuzzleIndex( 0 )
		, mTracerCounter( 0 )
		, mAdditionalLockTargets( desc.mMaxLocks )
		, mAdditionalLockTargetsMT( desc.mMaxLocks )
		, mWorldSpacePitch( 0.f )
	{
		fResetAmmoCount( );

		if( mDesc.mPreSpawnProjectiles )
			fPreSpawnProjectiles( );

		mLastFireEvent.mWeapon = this;
		fConfigureAudio( );
	}

	tWeapon::~tWeapon( ) 
	{
	}

	tDamageID tWeapon::fBuildID( )
	{
		tDamageID id( mInst.mOwnerUnit, mPlayer, mInst.mOwnerUnit->fTeam( ) );
		id.mWeapon.fReset( this );
		id.mDesc = &mDesc;
		id.mOverCharged = mInst.mOverCharged;
		if( mPlayer && mPlayer->fCurrentUnit( ) == mInst.mOwnerUnit )
			id.mUserControlled = true;
		id.mUserDamageMultiplier = mInst.mUserDamageMultiplier;
		sigassert( mInst.mStation );
		if( mInst.mStation->fUI( ) )
			id.mNightVision = mInst.mStation->fUI( )->fUsingNightVision( );
		return id;
	}

	/*static*/ b32 tWeapon::fRenderTargets( )
	{
		return Debug_Weapons_RenderTargets;
	}

	void tWeapon::fOnSpawn( )
	{
		if( mInst.mAnimated )
		{
			sigassert( mInst.mAnimated->fLogic( ) && "This should not have been initialized without a logic" );
			Logic::tAnimatable* skel = mInst.mAnimated->fLogic( )->fQueryAnimatable( );
			if( skel && !skel->fMotionMap( ).fIsNull( ) )
				mRecoilMotionState = skel->fMotionMap( ).fMapState( "Recoil" );
		}
	}

	void tWeapon::fOnDelete( )
	{
		mInst.fOnDelete( );
		mBulletExplosionLightEntity.fRelease( );
		if( mBulletExplosionLightLogic ) 
			mBulletExplosionLightLogic->fSetPersist( false );
		mBulletExplosionLightLogic = NULL;

		mAITarget.fRelease( );
		mAITargetMT.fRelease( );
		mAITargetOverride.fRelease( );
		mAdditionalLockTargets.fReset( );
		mAdditionalLockTargetsMT.fReset( );
		mReticalTarget.fRelease( );	
		mReticalTargetLogic = NULL;

		mRecoilMotionState = Sqrat::Function( );

		for( u32 i = 0; i < mFireParticleSystems.fCount( ); ++i )
			mFireParticleSystems[ i ].fSafeCleanup( );
		for( u32 i = 0; i < mFireParticleSystemsOverCharged.fCount( ); ++i )
			mFireParticleSystemsOverCharged[ i ].fSafeCleanup( );
		mFireParticleSystems.fSetCount( 0 );
		mFireParticleSystemsOverCharged.fSetCount( 0 );

		mPreSpawnedProjectiles.fSetCount( 0 );
	}

	void tWeapon::fConfigureAudio( )
	{
		if( !mDesc.mUseBankAudio )
		{
			sigassert( mInst.mMuzzles.fCount( ) > 0 );

			Audio::tSourcePtr source( NEW Audio::tSource( "Turret Muzzle" ) );
			source->fSpawn( *mInst.mMuzzles[ 0 ].mProjectileSpawn );
			mInst.mAudioSources.fPushBack( source );
			
			//source->fSetSwitch( AK::SWITCHES::WEAPON_TYPE::GROUP, AK::SWITCHES::GUNS::SWITCH::MACHINEGUN );
			source->fSetSwitch( tGameApp::cWeaponTypeSwitchGroup, mDesc.mAudioAlias );

			fSetAudioRTPC( tGameApp::cPlayerControlRTPC, 0.f );
		}
	}

	void tWeapon::fSpinUp( b32 up )
	{
		if( !Perf_Audio_DisableBarrelSpin )
		{
			if( up )
				fHandleAudioEvent( AK::EVENTS::PLAY_WEAPON_BARRELSPIN );
			else
				fHandleAudioEvent( AK::EVENTS::STOP_WEAPON_BARRELSPIN );
		}
	}

	b32 tWeapon::fTargetCooled( ) const
	{
		return mAITargetCooloff <= 0.f;
	}

	b32 tWeapon::fShouldFire( ) const 
	{ 
		// Fire if we "can", and we alreayd are or we're pointing at the target. being under user control, or ai with a target
		return fCanFire( ) && 
			(fUnderUserControl( ) // user says so
			||  (mAIFireOverride  // ai god says so
				|| ( fShouldAcquire( ) && fHasTarget( ) && fTargetCooled( ) && (fFiring( ) || fIsAimingNearTarget( )) /*&& tWeapon::fFireAtTarget( mAITarget.fGetRawPtr( ), mInst.mOwnerUnit->fOwnerEntity( ) )*/ ) // legit ai targetting
				)
			)
			;
	}

	b32 tWeapon::fFire( const tWeaponTarget* target )
	{
		mFiring = true;

		if( !mLoopingAudioPushed && mDesc.mAreaDamage )
			fAudioBegin( );

		if( mBehaveContinuously )
		{
			sigassert( fCanFire( ) );
			mFireTimer = fMin( mFireTimer, mDesc.mFireRate );

			if( mDesc.mAreaDamage )
				for( u32 i = 0; i < mInst.mMuzzles.fCount( ); ++i )
					fBeginAreaEffect( i );

			return true;
		}
		else
		{
			if( fCurrentAmmo( ) > 0 )
				fSpawnProjectiles( target );

			return false;
		}
	}
	void tWeapon::fEndFire( )
	{
		if( mFiring )
			fAudioEnd( );

		mFiring = false;

		if( mDesc.mAreaDamage )
			for( u32 i = 0; i < mInst.mMuzzles.fCount( ); ++i )
				fEndAreaEffect( i );
	}
	b32 tWeapon::fShouldShellCam( ) const
	{
		return mPlayer ? (mDesc.mFireMode == cWeaponFireModeOneShotMinigameMode || mPlayer->fGamepad( ).fButtonHeld( mInst.mTriggerButton )) : false;
	}
	void tWeapon::fHandleAudioEvent( u32 eventID )
	{
		if( mDesc.mUseBankAudio )
			mInst.mStation->fBank( mInst.mBankID )->fHandleBankAudioEvent( eventID );
		else
		{
			for( u32 i = 0; i < mInst.mAudioSources.fCount( ); ++i )
				mInst.mAudioSources[ i ]->fHandleEvent( eventID );
		}
	}
	void tWeapon::fSetAudioRTPC( const tStringPtr& rtpc, f32 value )
	{
		if( mDesc.mUseBankAudio )
			mInst.mStation->fBank( mInst.mBankID )->fBankAudio( )->fSetGameParam( rtpc, value );
		else
		{
			for( u32 i = 0; i < mInst.mAudioSources.fCount( ); ++i )
				mInst.mAudioSources[ i ]->fSetGameParam( rtpc, value );
		}
	}
	b32 tWeapon::fPotentialTargetInRange( const tVec3f& targetPosition ) const
	{
		tVec3f toTarget = targetPosition - fTargetingRefFrame( ).fGetTranslation( );
		toTarget.y = 0;

		f32 lenSqrd = toTarget.fLengthSquared( );

		if( !fInBounds( lenSqrd, fSquare( mDesc.mMinRange ), fSquare( mDesc.mMaxRange ) ) )
			return false;
		
		if( mYawConstraintSet )
		{
			f32 len = fSqrt( lenSqrd );
			if( len < 0.01f ) 
				return false;

			toTarget /= len;
			return fAcos( mYawConstraintAxis.fDot( toTarget ) ) < mYawConstaintAngle;
		}

		return true;
	}

	b32 tWeapon::fReleaseTargetIfNoLongerValid( tEntityPtr& target )
	{
		if( !target )
			return false;

		tUnitLogic* unit = target->fLogicDerived< tUnitLogic >( );
		if( !unit || unit->fIsDestroyed( ) || !unit->fIsValidTarget( )
			|| (unit->fCreationType( ) == tUnitLogic::cCreationTypeFromLevel && !unit->fUnderUserControl( ))
			|| !fPotentialTargetInRange( target->fObjectToWorld( ).fGetTranslation( ) ) )
		{
			target.fRelease( );
			return true;
		}

		return false;
	}
	void tWeapon::fAcquireTarget( const tEntityPtr& target )
	{
		if( mAITarget != target )
		{
			if( mAITarget )
				fReleaseTarget( );

			mAITarget = target;
			if( mAITarget )
			{
				tUnitLogic* ul = mAITarget->fLogicDerived<tUnitLogic>( );
				if( ul )
					mAITargetCooloff = ul->fAcquireTarget( );
				else
					mAITargetCooloff = 0.f;
			}	
		}
	}

	void tWeapon::fReleaseTarget( )
	{
		mHasTarget = false;
		mAITarget.fRelease( );
	}
	void tWeapon::fUpdateTargetRelatedData( )
	{
		mWithinReach = false;
		mReticalOverTarget = false;

		tEntityPtr* target = NULL;

		if( fUnderUserControl( ) )
		{
			if( mUseRayCastedTarget && mReticalTarget ) 
			{
				target = &mReticalTarget;

				if( mReticalTargetLogic 
					&& (mReticalTargetLogic->fTeam( ) != mPlayer->fTeam( ))
					&& (mDesc.fTargetPriority( mReticalTargetLogic->fUnitType( ) ) != ~0) )
					mReticalOverTarget = true;
			}
		}
		else if( mAITarget )
		{
			target = &mAITarget;
		}

		if( target )
		{
			if( !fUnderUserControl( ) )
				mPredictedTargetPosition = fPredictTargetPosition( *target );
			else if( mUseRayCastedTarget ) 
				mPredictedTargetPosition = mReticalTargetPosition;
			else if( mUseArcTarget ) 
				mPredictedTargetPosition = mWorldSpaceArcTargetPosition;

			mHasTarget = true;	
			const tVec3f anchorPoint = mInst.mMuzzles[ 0 ].mProjectileSpawn->fObjectToWorld( ).fGetTranslation( );
			mIdealFacingDirection = ( mPredictedTargetPosition - anchorPoint ).fNormalizeSafe( tVec3f::cZAxis );

			fComputeIdealAngle( );
			fComputeIdealYawAngle( );
			fComputeIdealVelocity( );
		}
		else
		{
			mHasTarget = false;
			mIdealFacingDirection = tVec3f::cZAxis;
		}

		sigassert( mIdealFacingDirection == mIdealFacingDirection );
		sigassert( mPredictedTargetPosition == mPredictedTargetPosition );
	}

	void tWeapon::fRayCastAndAdjustTarget( )
	{
		if( !fShellCaming( ) )
		{
			if( fReadyToRaycast( Perf_Weapons_PlayerCastCount ) || Perf_Weapons_AlwaysPlayerCast )
			{
				tEntityPtr newRetTarget;
				if( fStation( ).fComputeRealTargetThroughRetical( fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( ), mReticalTargetPosition, &newRetTarget )  )
				{
					if( mReticalTarget != newRetTarget )
					{
						mReticalTarget = newRetTarget;
						tEntity* logicEnt = mReticalTarget->fFirstAncestorWithLogic( );
						if( logicEnt ) 
							mReticalTargetLogic = logicEnt->fLogicDerived< tUnitLogic >( );
					}

					mUseRayCastedTarget = true;
				}
				else
				{
					mReticalTargetLogic = false;
					mUseRayCastedTarget = false;
				}
			}
		}
		else
		{
			mReticalTargetLogic = false;
			mUseRayCastedTarget = false;
		}
	}

	tVec3f tWeapon::ComputeLeveledAnchorZ( u32 anchorIndex ) const
	{
		const tMat3f &xform = mInst.mMuzzles[ anchorIndex ].mProjectileSpawn->fObjectToWorld( );
		tVec3f zAxis = xform.fZAxis( );
		zAxis.y = 0.f;
		zAxis.fNormalizeSafe( tVec3f::cZAxis );
		return zAxis;
	}

	void tWeapon::fPreSpawnProjectiles( )
	{
		mPreSpawnedProjectiles.fSetCount( mInst.mMuzzles.fCount( ) );
		for( u32 i = 0; i < mPreSpawnedProjectiles.fCount( ); ++i )
			fPreSpawnProjectiles( i );
	}

	void tWeapon::fPreSpawnProjectiles( u32 index )
	{
		if( !mPreSpawnedProjectiles[ index ] )
		{
			tEntity* proj = mInst.mMuzzles[ index ].mProjectileSpawn->fSpawnChild( mDesc.mProjectilePath );
			log_assert( proj, "Could not spawn projectile: " << mDesc.mProjectilePath );

			mPreSpawnedProjectiles[ index ].fReset( proj );
		}
	}

	void tWeapon::fSpawnProjectiles( const tWeaponTarget* target )
	{
		if( !tGameApp::fInstance( ).fCurrentLevel( ) )
		{
			//the game must have ended
			return;
		}

		for( u32 shot = 0; shot < mDesc.mShotCount; ++shot )
		{
			b32 spawnedOne = false;

			if( mDesc.mPreSpawnProjectiles )
			{
				for( u32 i = 0; i < mPreSpawnedProjectiles.fCount( ); ++i )
				{
					tEntityPtr& ent = mPreSpawnedProjectiles[ i ];
					if( ent )
					{
						sigassert( tGameApp::fInstance( ).fCurrentLevel( ) );
						sigassert( tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );

						// Need valid parent relative transform immediately
						ent->fReparentImmediate( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
						fSpawnProjectile( ent.fGetRawPtr( ), i, ent->fObjectToWorld( ), target, 0 ); //always "zero" for prespawned, so they get full effects
						ent.fRelease( );
						spawnedOne = true;
						break;
					}
				}
			}
			
			if( !spawnedOne )
			{
				if( target && target->mMuzzleIndex != ~0 )
				{
					// someone may have passed us in a defunct target just for the muzzle index. dont propogate it if so.
					fSpawnNewProjectileFromMuzzle( target->mMuzzleIndex, target->mEntity ? target : NULL, shot );
				}
				else
				{
					switch( mDesc.mFireMode )
					{
					case cWeaponFireModeAlternate:
						{	
							u32 mCnt = mInst.mMuzzles.fCount( );
							if( mCnt > 0 )
							{
								mNextMuzzleIndex = fModulus( mNextMuzzleIndex + 1, mCnt );
								fSpawnNewProjectileFromMuzzle( mNextMuzzleIndex, target, shot );
							}
						}
						break;
					default:
						for( u32 a = 0; a < mInst.mMuzzles.fCount( ); ++a )
							fSpawnNewProjectileFromMuzzle( a, target, shot );
						break;
					}
				}
			}
		}
	}

	void tWeapon::fSpawnNewProjectileFromMuzzle( u32 attachIndex, const tWeaponTarget* target, u32 shotIndex )
	{
			// Create a new projectile
		tEntity* proj = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( mDesc.mProjectilePath );
		log_assert( proj, "Could not spawn projectile: " << mDesc.mProjectilePath );

		tVec3f pos = mInst.mMuzzles[ attachIndex ].mProjectileSpawn->fObjectToWorld( ).fGetTranslation( );

		sigassert( mInst.mOwnerUnit );
		pos += mParentVelocity * mInst.mOwnerUnit->fSceneGraph( )->fFrameDeltaTime( );

		tVec3f zAxis;
		f32 extraPitch = 0;
		f32 extraYaw = 0;

		if( mUseRayCastedTarget )
			zAxis = ( mReticalTargetPosition - pos ).fNormalizeSafe( tVec3f::cZAxis );
		else
		{
			zAxis = ComputeLeveledAnchorZ( attachIndex );
			extraPitch = fToRadians( fOutputPitchAngle( ) );
			extraYaw = fToRadians( mCurrentYawAngle );
		}

		// apply spread
		{
			f32 sAmount = (f32)mCurrentSpread;
			if( mDesc.mShotCount > 1 )
				sAmount = mDesc.mMaxSpread; //full spread for shotgun

			tVec2f spread = sync_rand( fVec<tVec2f>( ) ) * fToRadians( sAmount );

			// Rotate about y-axis to incorporate the offset
			const tVec3f side = zAxis.fCross( tVec3f::cYAxis ).fNormalizeSafe( tVec3f::cZAxis );
			const tQuatf pitchShift( tAxisAnglef( side, extraPitch + spread.y ) );
			const tQuatf yawShift( tAxisAnglef( tVec3f::cYAxis, extraYaw + spread.x ) );
			zAxis = pitchShift.fRotate( zAxis );
			zAxis = yawShift.fRotate( zAxis );
		}

		log_assert( fEqual( zAxis.fLength( ), 1.f, cVectorZeroEpsilon ), "You scaled a sigml with a weapon attachment point. Dont do that." );
		tMat3f xform = tMat3f::cIdentity;
		xform.fSetTranslation( pos );
		xform.fOrientZAxis( zAxis );

		fSpawnProjectile( proj, attachIndex, xform, target, shotIndex );
	}

	void tWeapon::fSpawnProjectile( tEntity* projectile, u32 attachIndex, const tMat3f& xForm, const tWeaponTarget* target, u32 shotIndex )
	{
		tEntityPtr targetEnt;
		tVec3f targetPt;

		if( target ) 
		{
			targetPt = target->mEntity->fObjectToWorld( ).fGetTranslation( );
			targetEnt = target->mEntity;
		}
		else if( mUseRayCastedTarget )
			targetPt = mReticalTargetPosition;
		else if( mUseArcTarget )
			targetPt = mWorldSpaceArcTargetPosition;
		else
		{
			targetPt = mPredictedTargetPosition;
			targetEnt = mAITarget;
		}

		// Move the projectile forward a frame
		const f32 dt = 1.f / 30.f;
		tMat3f xF = xForm;
		xF.fTranslateGlobal( mParentVelocity * dt );
		projectile->fMoveTo( xForm );

		mLastFireEvent.mProjectilVel = tVec3f::cZeroVector;
		mLastFireEvent.mLocalMuzzlePt = mInst.mOwnerUnit->fOwnerEntity( )->fObjectToWorld().fInverse( ).fXformPoint( xForm.fGetTranslation( ) );
		
		tProjectileLogic* proj = projectile->fLogicDerived< tProjectileLogic >( );
		sigassert( proj );

		if( !mBehaveContinuously || !mShellCamEligible )
			mLastProjectiles.fSetCount( 0 );
		mLastProjectiles.fPushBack( proj );

		mLastFireEvent.mProjectile.fReset( projectile );
		mLastFireEvent.mProjectilVel = mParentVelocity * mDesc.mParentVelScale + xForm.fZAxis( ) * mCurrentProjectileSpeed;
		proj->fSetSpeed( mCurrentProjectileSpeed );
		proj->fSetLaunchVector( mLastFireEvent.mProjectilVel );
		proj->fSetFiredBy( fBuildID( ) );
		proj->fSetTarget( targetEnt, targetPt );
		proj->fInitPhysics( );
		if( proj->fSceneGraph( ) ) 
			proj->fPreSpawnedSpawn( );

		sigassert( mInst.mIgnoreParent );
		proj->fSetIgnoreParent( mInst.mIgnoreParent->fOwnerEntity( ) );

		if( mDesc.mAreaDamage ) 
			fBeginAreaEffect( attachIndex );
		else
			fSpawnFireEffect( proj, attachIndex );

		if( shotIndex == 0 )
		{
			if( !mInst.fUnlimitedAmmo( ) && fCurrentAmmo( ) > 0 )
				++mShotsFired;

			fSpawnCasing( attachIndex );
			if( !mLoopingAudioPushed )
				fAudioBegin( );
		}
		
		mShellCamInitiateTimer = 0.0f;
		mShellCamEligible = true;
		mLastFireEvent.mSet = true;

		if( mPlayer )
		{
			mPlayer->fStats( ).fIncStat( GameFlags::cSESSION_STATS_AMMO_EXPENDED, 1.f );
			if( mDesc.mUseStatToInc < GameFlags::cSESSION_STATS_COUNT )
				mPlayer->fStats( ).fIncStat( mDesc.mUseStatToInc, 1.f );
		}
	}
	void tWeapon::fAudioBegin( )
	{
		if( mDesc.mAudioType == tWeaponDesc::cAudioTypeLooping && (!fWantsSpinUp( ) || !fSpinningUp( )) )
		{
			mLoopingAudioPushed = true;
			fHandleAudioEvent( AK::EVENTS::PLAY_WEAPON_FIRE_LP );
		}
		else if( mDesc.mAudioType == tWeaponDesc::cAudioTypeSingleShot || fWantsSpinUp( ) )
			fHandleAudioEvent( AK::EVENTS::PLAY_WEAPON_FIRE );
	}
	void tWeapon::fAudioEnd( )
	{
		if( mLoopingAudioPushed )
		{
			mLoopingAudioPushed = false;
			fHandleAudioEvent( AK::EVENTS::STOP_WEAPON_FIRE_LP );
		}
	}
	void tWeapon::fInitParticles( )
	{
		if( mBehaveContinuously )
		{
			if( mDesc.mFireEffectPath.fExists( ) )
			{
				mFireParticleSystems.fSetCount( mInst.mMuzzles.fCount( ) );
				for( u32 i = 0; i < mInst.mMuzzles.fCount( ); ++i )
				{
					tEntity* parent = mInst.mMuzzles[ i ].mProjectileSpawn.fGetRawPtr( );
					sigassert( parent );

					if( mDesc.mAreaDamage ) 
					{
						tEntity* effectParent = tAreaEffectParent::fCreate( tWeaponPtr( this ) );
						effectParent->fSpawn( *parent );
						parent = effectParent;
					}

					tEntity* ps = parent->fSpawnChild( mDesc.mFireEffectPath );
					if( ps )
						ps->fForEachDescendent( FX::tAddPausedFxSystem( mFireParticleSystems[ i ], GameFlags::cFLAG_DONT_INHERIT_STATE_CHANGE ) );
					else
						log_warning( 0, "Could not create particle effect: " << mDesc.mFireEffectPath );

					if( mDesc.mAreaDamage ) 
					{
						mFireParticleSystems[ i ].fPause( false );
						mFireParticleSystems[ i ].fSetEmissionPercent( 0.f );
					}
				}
			}
			if( mDesc.mFireEffectPathOverCharged.fExists( ) )
			{
				mFireParticleSystemsOverCharged.fSetCount( mInst.mMuzzles.fCount( ) );
				for( u32 i = 0; i < mInst.mMuzzles.fCount( ); ++i )
				{
					tEntity* parent = mInst.mMuzzles[ i ].mProjectileSpawn.fGetRawPtr( );

					if( mDesc.mAreaDamage ) 
					{
						tEntity* effectParent = tAreaEffectParent::fCreate( tWeaponPtr( this ) );
						effectParent->fSpawn( *parent );
						parent = effectParent;
					}

					tEntity* ps = parent->fSpawnChild( mDesc.mFireEffectPathOverCharged );
					if( ps )
						ps->fForEachDescendent( FX::tAddPausedFxSystem( mFireParticleSystemsOverCharged[ i ], GameFlags::cFLAG_DONT_INHERIT_STATE_CHANGE ) );
					else
						log_warning( 0, "Could not create particle effect: " << mDesc.mFireEffectPathOverCharged );

					if( mDesc.mAreaDamage ) 
					{
						mFireParticleSystemsOverCharged[ i ].fPause( false );
						mFireParticleSystemsOverCharged[ i ].fSetEmissionPercent( 0.f );
					}
				}
			}
		}

		if( mDesc.mMuzzleFlashLightSize > 0.f )
		{
			for( u32 i = 0; i < mInst.mMuzzles.fCount( ); ++i )
			{
				tLightEffectLogic *le = NEW tLightEffectLogic( mDesc.mMuzzleFlashLightSize, 0.f, mDesc.mMuzzleFlashLightLife, true );
				Gfx::tLightEntity* ent = tLightEffectLogic::fSpawnLightEffect( tMat3f::cIdentity, le, *mInst.mMuzzles[ i ].mProjectileSpawn, mDesc.mMuzzleFlashLightColor );

				if( ent )
				{
					le->fSetActive( false );
					mInst.mMuzzles[ i ].mLightEntity.fReset( ent );
					mInst.mMuzzles[ i ].mLightEntityLogic = le;
				}
			}
		}
	}
	tEntity* tWeapon::fSetTurretEntityNamed( const tStringPtr& name )
	{
		sigassert( mInst.mOwnerUnit );
		mInst.mTurretEntity.fReset( mInst.mOwnerUnit->fOwnerEntity( )->fFirstDescendentWithName( name ) );
		if( mInst.mTurretEntity )
		{
			if( mInst.mTurretEntity->fLogic( ) )
				mInst.mAnimated = mInst.mTurretEntity;
		}
		else
			log_warning( 0, "Could not find turrent entity named: " << name );

		return mInst.mTurretEntity.fGetRawPtr( );
	}
	tEntity* tWeapon::fSetTurretEntity( tEntity* ent )
	{
		sigassert( ent );
		mInst.mTurretEntity.fReset( ent );
		if( mInst.mTurretEntity->fLogic( ) )
			mInst.mAnimated = mInst.mTurretEntity;
		return mInst.mTurretEntity.fGetRawPtr( );
	}
	tEntity* tWeapon::fSetAnimated( tEntity* ent )
	{
		sigassert( ent && ent->fLogic( ) );
		mInst.mAnimated.fReset( ent );
		return ent;
	}
	void tWeapon::fSpawnFireEffect( tProjectileLogic* projectile, u32 anchorIndex )
	{
		projectile->fSpawnTrailEffects( );

		fPlayRecoilMotion( anchorIndex );

		if( mDesc.mFireEffectPath.fExists( ) )
		{
			if( mBehaveContinuously )
			{
				if( mInst.mOverCharged && mDesc.mFireEffectPathOverCharged.fExists( ) )
				{
					mFireParticleSystemsOverCharged[ anchorIndex ].fPause( false );
					mFireParticleSystemsOverCharged[ anchorIndex ].fReset( );
				}
				else
				{
					mFireParticleSystems[ anchorIndex ].fPause( false );
					mFireParticleSystems[ anchorIndex ].fReset( );
				}
			}
			else
			{
				const tFilePathPtr& effectPath = (mInst.mOverCharged && mDesc.mFireEffectPathOverCharged.fExists( )) ? mDesc.mFireEffectPathOverCharged : mDesc.mFireEffectPath;
				mInst.mMuzzles[ anchorIndex ].mProjectileSpawn->fSpawnChild( effectPath );

				if( mDesc.mAfterFireEffectPath.fExists( ) )
					mInst.mMuzzles[ anchorIndex ].mProjectileSpawn->fSpawnChild( mDesc.mAfterFireEffectPath );
			}
		}

		if( mInst.mMuzzles[ anchorIndex ].mLightEntityLogic )
			mInst.mMuzzles[ anchorIndex ].mLightEntityLogic->fRestart( );
	}
	void tWeapon::fPlayRecoilMotion( u32 anchorIndex )
	{
		if( !mRecoilMotionState.IsNull( ) )
		{
			Sqrat::Table table;
			table.SetValue( "MuzzleID", anchorIndex );
			table.SetValue( "BankID", (u32)mInst.mBankID );
			mRecoilMotionState.Execute( Sqrat::Object( table ) );
		}
	}
	void tWeapon::fSpawnCasing( u32 attachIndex )
	{
		if( mDesc.mShellCasingPath.fExists( ) )
		{
			if( mInst.mMuzzles[ attachIndex ].mShellCasingSpawn )
			{
				tEntity* pos = mInst.mMuzzles[ attachIndex ].mShellCasingSpawn.fGetRawPtr( );
				tEntity* parent = tGameApp::fInstance( ).fCurrentLevel( )->fRootEntity( );
				tEntity* ent = parent->fSpawnChild( mDesc.mShellCasingPath );

				if( ent )
				{
					const tDebrisLogicDef& def = tGameApp::fInstance( ).fDebrisLogicDef( (GameFlags::tDEBRIS_TYPE)ent->fQueryEnumValue( GameFlags::cENUM_DEBRIS_TYPE, GameFlags::cDEBRIS_TYPE_SHELL_CASING ) );
					tDebrisLogic *dl = NEW tDebrisLogic( def );
					tLogicPtr *dlp = NEW tLogicPtr( dl );

					ent->fMoveTo( pos->fObjectToWorld( ) );
					ent->fAcquireLogic( dlp );

					tVec3f dir = tVec3f::cYAxis * 2.f;
					if( mInst.mMuzzles[ attachIndex ].mShellCasingDir )
						dir = mInst.mMuzzles[ attachIndex ].mShellCasingDir->fObjectToWorld( ).fGetTranslation( ) - mInst.mMuzzles[ attachIndex ].mShellCasingSpawn->fObjectToWorld( ).fGetTranslation( );
					else
						log_warning_nospam( 0, "No shell casing dir pt for weapon: " << mDesc.mWeaponDescName );

					dl->fPhysicsSpawn( mParentVelocity, dir, 1/30.f );
				}
			}
			else
				log_warning_nospam( 0, "No shell casing spawn pt for weapon: " << mDesc.mWeaponDescName );
		}
	}
	void tWeapon::fBeginAreaEffect( u32 anchorIndex )
	{
		if( mInst.mOverCharged && mDesc.mFireEffectPathOverCharged.fExists( ) )
		{
			//overcharge
			mFireParticleSystemsOverCharged[ anchorIndex ].fSetEmissionPercent( 1.f );
			if( mDesc.mFireEffectPath.fExists( ) )
				mFireParticleSystems[ anchorIndex ].fSetEmissionPercent( 0.f );
		}
		else if( mDesc.mFireEffectPath.fExists( ) )
		{
			//regular
			mFireParticleSystems[ anchorIndex ].fSetEmissionPercent( 1.f );
			if( mDesc.mFireEffectPathOverCharged.fExists( ) )
				mFireParticleSystemsOverCharged[ anchorIndex ].fSetEmissionPercent( 0.f );
		}
	}
	void tWeapon::fEndAreaEffect( u32 anchorIndex )
	{
		if( mDesc.mFireEffectPath.fExists( ) )
			mFireParticleSystems[ anchorIndex ].fSetEmissionPercent( 0.f );
		if( mDesc.mFireEffectPathOverCharged.fExists( ) )
			mFireParticleSystemsOverCharged[ anchorIndex ].fSetEmissionPercent( 0.f );
	}
	void tWeapon::fSpawnReloadEffect( u32 anchorIndex )
	{
		u32 start, end;
		if( anchorIndex >= mInst.mMuzzles.fCount( ) )
		{
			start = 0;
			end = mInst.mMuzzles.fCount( )-1;
		}
		else
		{
			start = anchorIndex;
			end = anchorIndex;
		}

		for( u32 i = start; i <= end; ++i )
		{
			if( mDesc.mReloadEffectPath.fExists( ) )
			{
				tEntity* reloadEffect = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( mDesc.mReloadEffectPath );
				reloadEffect->fMoveTo( mInst.mMuzzles[ i ].mProjectileSpawn->fObjectToWorld( ) );
			}
		}
	}
	b32 tWeapon::fRequestTrail( )
	{
		b32 result = false;
		mTracerCounter++;

		s32 interval = mInst.mOverCharged ? mDesc.mTracerTrailIntervalOverCharged : mDesc.mTracerTrailInterval;
		if( mTracerCounter >= interval || Debug_Weapons_AlwaysSpawnTracer )
		{
			mTracerCounter = 0; 
			result = true;
		}

		return result;
	}
	u32 tWeapon::fReloadAmmoCount( b32 assumeUserControl ) const
	{
		if( fUnderUserControl( ) || assumeUserControl )
			return mDesc.mPreSpawnProjectiles ? mInst.mMuzzles.fCount( ) : mDesc.mMaxAmmo;
		else
			return mDesc.mMaxAmmoAI; 
	}
	f32 tWeapon::fReloadTime( ) const
	{
		return fUnderUserControl( ) ? mDesc.mReloadTime : mDesc.mReloadTimeAI;
	}
	void tWeapon::fResetAmmoCount( ) 
	{ 
		mShotsFired = 0; 
	}
	void tWeapon::fReload( ) 
	{ 
		mReloadTimerActive = false;
		mFireTimer = fMin( mFireTimer, mDesc.mFireRate );
		fResetAmmoCount( ); 
		fSpawnReloadEffect( ); 

		if( mDesc.mPreSpawnProjectiles )
			fPreSpawnProjectiles( );
	}
	void tWeapon::fReloadAfterTimer( )
	{
		if( fNeedsReload( ) )
		{
			mReloadTimer = 0.f;
			mReloadTimerActive = true;
		}
	}
	const tMat3f& tWeapon::fTargetingRefFrame( ) const
	{
		if( mInst.mTurretEntity ) return mInst.mTurretEntity->fObjectToWorld( );
		else return mInst.mMuzzles[ 0 ].mProjectileSpawn->fObjectToWorld( );
	}
	void tWeapon::fComputeIdealAngleDirect( )
	{
		mIdealPitchAngle = fToDegrees( fAsin( mIdealFacingDirection.fDot( tVec3f::cYAxis ) ) );
		mWithinReach = true;
	}
	void tWeapon::fComputeIdealAngleArc( )
	{
		const tVec3f muzzlePos = fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( );
		
		f32 angle = 0.f;
		if( ProjectileUtility::fComputeLaunchAngle( angle, mCurrentProjectileSpeed, muzzlePos, mPredictedTargetPosition, mDesc.fShellGravity( ), mInvertPitch ) )
		{
			mWithinReach = true;
			mIdealPitchAngle = fToDegrees( angle );

			if( !fInBounds( mIdealPitchAngle, mDesc.mMinPitch, mDesc.mMaxPitch ) )
			{
				mIdealYawAngle = 0.0f;
				mWithinReach = false;
			}
		}
		else // No angle is able to reach the target at this speed
		{
			mWithinReach = false;
		}
	}
	void tWeapon::fComputeIdealYawAngle( )
	{
		const tVec3f muzzlePos = fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( );
		tVec3f delta = mPredictedTargetPosition - muzzlePos;
		mIdealYawAngle = delta.fXZHeading( );
	}
	void tWeapon::fComputeIdealVelocityArc( )
	{
		f32 velocity = 0.f;

		const tVec3f muzzlePos = fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( );
		if( ProjectileUtility::fComputeLaunchVelocity( velocity, mCurrentPitchAngle, muzzlePos, mPredictedTargetPosition, mDesc.fShellGravity( ) ) )
		{
			mWithinReach = true;
			mIdealProjectileSpeed = velocity;

			if( !fInBounds( mIdealProjectileSpeed, 0.f, mDesc.mProjectileSpeed ) )
			{
				mIdealProjectileSpeed = 0.f;
				mWithinReach = false;
			}
		}
		else // Should only get here if trying to take the tangent of 90, so technically all velocities will hit target but let's just ignore it
		{
			mWithinReach = false;
			mIdealProjectileSpeed = 0.f;
		}
	}
	void tWeapon::fProcessShellCamST( f32 dt )
	{
		if( fUnderUserControl( ) && mShellCamEligible && mLastProjectiles.fCount( ) && (!mBehaveContinuously || fReloading( ) || fNeedsReload( )) )
		{
			sigassert( mPlayer );
			mShellCamInitiateTimer += dt;

			if( mShellCaming || fShouldShellCam( ) )
			{
				// still potentially shell camming, push cam if it hasnt been pushed yet
				if( !mShellCamPushed && mShellCamInitiateTimer >= mDesc.mShellCamInitiateTimer )
				{
					mPlayer->fPushCamera( Gfx::tCameraControllerPtr( NEW tShellCamera( *mPlayer, mLastProjectiles, this ) ) );
					mShellCamPushed = true;
					mShellCaming = true;

					tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
					if( level ) level->fHandleTutorialEvent( tTutorialEvent( GameFlags::cTUTORIAL_EVENT_INPUT, Input::tGamepad::cButtonRTrigger, ~0, 1 ) );
				}
			}
			else
			{
				mShellCaming = false;
				// mortars have rapid fire so lose their eligiblity right away
				if( mDesc.mWeaponType == tGameApp::cWeaponDerivedTypeMortar || mShellCamInitiateTimer > Gameplay_Weapon_ShellCam_IneligibilityTimer )
					mShellCamEligible = false;
			}
		}
	}
	void tWeapon::fShellCamDied( )
	{
		mShellCaming = false;
		mShellCamPushed = false;
	}
	void tWeapon::fProjectileDied( tProjectileLogic* proj )
	{
		mLastProjectiles.fFindAndErase( proj );
	}
	f32 tWeapon::fOutputPitchAngle( ) const
	{
		return mCurrentPitchAngle;
	}
	void tWeapon::fPitchTowardsIdealAngle( f32 dt )
	{
		f32 dAngle = mIdealPitchAngle - mCurrentPitchAngle;

		// TODO: Get a pitch speed from somewhere
		const f32 pitchSpeed = 1.0f;

		// TODO: We might want to damp the angle differently
		if( fAbs( dAngle ) > 0.5f )
		{
			const f32 positiveAngle = dAngle > 0.f ? dAngle : -dAngle;
			dAngle = fClamp( dAngle * dt * pitchSpeed * Anim_PitchRotSpeed, -positiveAngle, positiveAngle );
		}

		fAdjustPitchAngle( dAngle );
	}
	f32 tWeapon::fPitchAngle( ) const 
	{ 
		if( !fWorldSpacePitchMode( ) )
			return mCurrentPitchAngle;
		else
			return mWorldSpacePitch;
	}
	void tWeapon::fSetPitchAngle( f32 pitch ) 
	{ 
		if( !fWorldSpacePitchMode( ) )
			mCurrentPitchAngle = fClamp( pitch, fDesc( ).mMinPitch, fDesc( ).mMaxPitch ); 
		else
			mWorldSpacePitch = fClamp( pitch, fDesc( ).mMinPitch, fDesc( ).mMaxPitch );
	}
	void tWeapon::fAdjustPitchAngle( f32 deltaPitch ) 
	{ 
		if( !fWorldSpacePitchMode( ) )
			fSetPitchAngle( deltaPitch + mCurrentPitchAngle );
		else
			fSetPitchAngle( deltaPitch + mWorldSpacePitch ); 
	}
	void tWeapon::fAdjustWorldSpacePitch( f32 dt )
	{
		sigassert( fWorldSpacePitchMode( ) );

		// treat pitch angle as zero to one between range rings
		// compute a pitch that will satisfy out world space distance desires.
		f32 range = mDesc.mMaxPitch - mDesc.mMinPitch;
		f32 zto1 = (mWorldSpacePitch - mDesc.mMinPitch) / range;

		if( mInvertPitch )
			zto1 = 1.f - zto1;

		range = mDesc.mMaxRange - mDesc.mMinRange;
		f32 distance = zto1 * range + mDesc.mMinRange;

		const tVec3f muzzlePos = fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( );
		tVec3f targetVec = mInst.mOwnerUnit->fOwnerEntity( )->fObjectToWorld( ).fXformVector( tVec3f( 0, 0, distance ) );
		f32 headingAngle = targetVec.fXZHeading( );

		tVec3f targetPos = mInst.mOwnerUnit->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) + targetVec;
		targetPos.y = mInst.mStation->fGetGroundHeight( headingAngle, zto1 );

		mIdealPitchAngle = 0.f;
		if( ProjectileUtility::fComputeLaunchAngle( mIdealPitchAngle, mCurrentProjectileSpeed, muzzlePos, targetPos, mDesc.fShellGravity( ), mInvertPitch ) )
		{
			mIdealPitchAngle = fToDegrees( mIdealPitchAngle );
		}
		else
			mIdealPitchAngle = fPitchAngle( );

		mCurrentPitchAngle = fLerp( mCurrentPitchAngle, mIdealPitchAngle, (f32)Gameplay_Weapon_PitchSmoothLerp );
	}
	void tWeapon::fYawTowardsIdealAngle( f32 dt )
	{
		f32 dAngle = mIdealYawAngle - mCurrentYawAngle;

		// TODO: Get a yaw speed from somewhere
		const f32 yawSpeed = 1.0f;

		// TODO: We might want to damp the angle differently
		if( fAbs( dAngle ) > 0.5f )
		{
			const f32 positiveAngle = dAngle > 0.f ? dAngle : -dAngle;
			dAngle = fClamp( dAngle * dt * yawSpeed * Anim_PitchRotSpeed, -positiveAngle, positiveAngle );
		}

		fAdjustYawAngle( dAngle );
	}
	void tWeapon::fAdjustSpeedTowardsIdeal( f32 dt )
	{
		f32 dSpeed = mIdealProjectileSpeed - mCurrentProjectileSpeed;

		// TODO: Get a pitch speed from somewhere
		const f32 velSpeed = 1.0f;

		// TODO: We might want to damp the angle differently
		if( fAbs( dSpeed ) > 0.5f )
		{
			const f32 positiveSpeed = dSpeed > 0.f ? dSpeed : -dSpeed;
			dSpeed = fClamp( dSpeed * dt * velSpeed * Anim_PitchRotSpeed, -positiveSpeed, positiveSpeed );
		}

		fAdjustProjectileSpeed( dSpeed );
	}
	b32 tWeapon::fIsAimingNearTarget( ) const
	{
		if( Perf_Weapons_ParentedTargetting && mInst.mTargettingParent )
			return mInst.mTargettingParent->fIsAimingNearTarget( );
		else
		{
			tVec3f xzFacing = ComputeLeveledAnchorZ();
			tVec3f idealFacing = mIdealFacingDirection;
			idealFacing.y = 0.f;
			idealFacing = idealFacing.fNormalizeSafe( tVec3f::cZAxis );
			const f32 facingAngle = fToDegrees( fAcos( idealFacing.fDot( xzFacing ) ) );
			return mDesc.mDoesntNeedtoPointAtTarget || (fEqual( mIdealPitchAngle, mCurrentPitchAngle, 1.f ) && fEqual( facingAngle, 0.f, 5.f ) && mCurrentProjectileSpeed >= mIdealProjectileSpeed);
		}
	}
	f32 tWeapon::fEstimateTimeToImpactDirect( const Math::tVec3f& pos ) const
	{
		f32 projectileSpeed = mCurrentProjectileSpeed; //todo, incorperate parent vel?

		// The predicted position gets more accurate over time since the time computed gets fed back into the unit logic for predicting the new position
		// which then gets used for estimating a new time, etc...
		const f32 dist = ( pos - mInst.mMuzzles[ 0 ].mProjectileSpawn->fObjectToWorld( ).fGetTranslation( ) ).fLength( );

		if( !fEqual( mDesc.mProjectileAcceleration, 0.f ) )
		{
			if( mDesc.mProjectileAcceleration > 0.f )
			{
				// http://www.wolframalpha.com/input/?i=x+%3D+v+*+t+%2B+.5+*+a+*+t^2+solve+for+t
				return (fSqrt( 2.f * mDesc.mProjectileAcceleration * dist + projectileSpeed*projectileSpeed ) + projectileSpeed) / mDesc.mProjectileAcceleration;
			}
			else // negative, will bork square root
			{
				// http://www.wolframalpha.com/input/?i=x+%3D+v+*+t+%2B+.5+*+-a+*+t^2+solve+for+t
				return (fSqrt( projectileSpeed*projectileSpeed - 2.f * mDesc.mProjectileAcceleration * dist ) + projectileSpeed) / mDesc.mProjectileAcceleration;
			}
		}
		else
			return dist / mCurrentProjectileSpeed;
	}
	f32 tWeapon::fEstimateTimeToImpactArc( const Math::tVec3f& pos ) const
	{
		//the target position y is all that is important
		// we must consider the possibility that the user is aiming
		// down to a certain minimum level height.

		f32 time = -1.f;
		const f32 targetHeight = fUnderUserControl( ) ? Gameplay_Weapon_MinimumLevelHeight : pos.y;
		const f32 deltaHeight = mInst.mMuzzles[ 0 ].mProjectileSpawn->fObjectToWorld( ).fGetTranslation( ).y - targetHeight;
		
		ProjectileUtility::fComputeTimeToTarget( time, fToRadians( fOutputPitchAngle( ) ), mCurrentProjectileSpeed, deltaHeight, mDesc.fShellGravity( ) );
		return time;
	}
	tVec3f tWeapon::fPredictTargetPosition( const tEntityPtr& target )
	{
		if( !target ) return tVec3f::cZeroVector;

		tUnitLogic* unit = NULL;
		if( mDesc.mLeadTargets && (unit = target->fLogicDerived< tUnitLogic >( )) )
		{
			const f32		frameDelta = 1.f / 30.f;
			const tVec3f&	localOffset = unit->fTargetOffset( );
			const tVec3f	currentPos = target->fObjectToWorld( ).fGetTranslation( );
			const tVec3f	vel = unit->fLinearVelocity( localOffset );
			f32 time = fEstimateTimeToImpact( currentPos ) + frameDelta * 2.f;

			// The following stuff tries to compensate for the rotation of the speed, it's experimental
			//const tMat3f&	muzzleXform = mInst.mMuzzles[ 0 ].mProjectileSpawn->fObjectToWorld( );
			//const tVec3f	muzzleFacing = muzzleXform.fZAxis( ).fProjectToXZAndNormalize( );
			//const tVec3f	muzzlePos = muzzleXform.fGetTranslation( );

			//const f32		currentDist = fAbs( muzzleFacing.fDot(currentPos - muzzlePos) );
			//f32 time = fEstimateTimeToImpact( currentPos ) + frameDelta;

			//tVec3f tangentVel = vel - vel.fDot( muzzleFacing ) * muzzleFacing;
			//f32 deltaAngle = atan( tangentVel.fLength( ) * time / currentDist );
			//f32 rotateSpeed = 8.f;
			//f32 rotateTime = deltaAngle / rotateSpeed;

			//time += rotateTime;

			return target->fObjectToWorld( ).fXformPoint( localOffset ) + vel * time;
		}
		else
			return target->fObjectToWorld( ).fGetTranslation( );
	}
	void tWeapon::fBeginUserControl( tPlayer* player )
	{
		mCurrentSpread = 0.f;
		mPlayer = player;
		mLastFireEvent.fReset( );

		fBeginRendering( player );
		fSetAudioRTPC( tGameApp::cPlayerControlRTPC, 1.f );
	}
	void tWeapon::fEndUserControl( ) 
	{ 
		fShellCamDied( );
		fEndFire( );

		mUseRayCastedTarget = false;
		mUseArcTarget = false;

		if( mPlayer )
			mPlayer->fCameraStack( ).fPopCamerasOfType<tShellCamera>( );
		fSetAudioRTPC( tGameApp::cPlayerControlRTPC, 0.f );

		if( !mPlayer || mPlayer->fSelectedUnitLogic( ) != mInst.mOwnerUnit || !mInst.mOwnerUnit->fHasWeapon(0,0,0) || mInst.mOwnerUnit->fWeaponRawPtr(0,0,0) != this )
			fEndRendering( ); //dont end rendering if we're still selected
		
		fReleaseTarget( );
		mPlayer = NULL;
	}
	void tWeapon::fProcessST( f32 dt )
	{
		mAITargetCooloff -= dt;

		if( !Perf_Weapons_ParentedTargetting || !mInst.mTargettingParent ) 
		{
			// This is a targeting parent
			if( mAITargetMT != mAITarget )
			{
				if( !mAITargetMT ) 
				{
					fReleaseTarget( );
					mAdditionalLockTargetsMT.fReset( );
				}
				else
				{
					fAcquireTarget( mAITargetMT );
					mAdditionalLockTargetsMT.fSwap( mAdditionalLockTargets );
				}
			}
			mAITargetMT.fRelease( );
			mAdditionalLockTargetsMT.fReset( );
		}
		else if ( fShouldAcquire( ) )
		{
			// This is a slave weapon to a targeting parent
			if( !mInst.mTargettingParent->fAITarget( ) ) 
				fReleaseTarget( );
			else
				fAcquireTarget( mInst.mTargettingParent->fAITarget( ) );
		}

		if( mAITarget )
			fUpdateTargetRelatedData( );

		if( !fUnderUserControl( ) )
		{
			fPitchTowardsIdealAngle( dt );
			//fYawTowardsIdealAngle( dt );
			fAdjustSpeedTowardsIdeal( dt );
		}

		if( fWorldSpacePitchMode( ) )
			fAdjustWorldSpacePitch( dt );

		if( mDesc.mWantsShellCam )
			fProcessShellCamST( dt );

		fProcessShellBursting( dt );

		if( mBehaveContinuously )
		{
			if( mSpinUpPercentage > 0.f )
				mFireTimer += dt * mSpinUpPercentage*mSpinUpPercentage;
			else
				mFireTimer += dt;

			if( mFiring && fCurrentAmmo( ) > 0 && mFireTimer >= mDesc.mFireRate )
			{
				mFireTimer -= mDesc.mFireRate;
				mFireTimer = fMax( mFireTimer, 0.f );

				if( !mDesc.mAreaDamage ) 
					fSpawnProjectiles( );
				else
				{
					// do this so we still control fire duration and reload frequency
					if( !mInst.fUnlimitedAmmo( ) )
						++mShotsFired;

					// do this so we still get the rumble and screen shake at the fire rate
					mLastFireEvent.mProjectilVel = tVec3f::cZeroVector;
					mLastFireEvent.mLocalMuzzlePt = tVec3f::cZeroVector;
					mLastFireEvent.mSet = true;
				}

				if( fUnderUserControl( ) ) 
					mCurrentSpread += dt * mDesc.mSpreadRate;
				else
					mCurrentSpread = mDesc.mAISpread;
			}
			else
			{
				mCurrentSpread -= dt * mDesc.mSpreadSettleRate;
			}
		}

		mCurrentSpread = fClamp( mCurrentSpread, 0.f, mDesc.mMaxSpread );

		if( mReloadTimerActive )
		{
			mReloadTimer += dt;
			if( mReloadTimer >= fReloadTime( ) )
				fReload( );
		}

		if( !Perf_Weapons_ParentedTargetting || !mInst.mTargettingParent ) 
		{
			fDebugRenderTargetPos( );
		}
	}

	void tWeapon::fProcessMT( f32 dt ) 
	{
		if( (!Perf_Weapons_ParentedTargetting || !mInst.mTargettingParent) )
		{
			// This is a targetting parent or stand alone weapon.

			if( fShouldAcquire( ) )
			{
				if( mAITargetOverride )
				{
					if( fPotentialTargetInRange( mAITargetOverride->fObjectToWorld( ).fGetTranslation( ) ) )
						mAITargetMT = mAITargetOverride;
					else
						mAITargetMT.fRelease( );
				}
				else
				{
					mAITargetMT = mAITarget;
					fReleaseTargetIfNoLongerValid( mAITargetMT );
					fFindTargetMT( this, ~0, mDesc, fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( ), mAITargetMT );
				}
			}
			else
				mAITargetMT.fRelease( );
		}
	}

	void tWeapon::fProcessShellBursting( f32 dt )
	{
		if( mPlayer && mPlayer->fGamepad( ).fButtonHeld( Input::tGamepad::cButtonLTrigger ) )
		{
			for( u32 i = 0; i < mLastProjectiles.fCount( ); ++i )
				mLastProjectiles[ i ]->fBurst( dt );
		}
	}

	void tWeapon::fProcessTarget( tWeapon* weapon, u32 teamFallback, const tWeaponDesc& desc, const Math::tVec3f& muzzlePos, tEntity* targetEnt, u32 priority, tEntityPtr& targetOut, f32& bestDist, u32& bestPriority )
	{
		tUnitLogic* ownerUnit = weapon ? weapon->fInst( ).mOwnerUnit : NULL;
		u32 team = weapon ? weapon->fInst( ).mOwnerUnit->fTeam( ) : teamFallback;

		tUnitLogic* unitLogic = targetEnt->fLogicDerived< tUnitLogic >( );
		if( !unitLogic 
			|| unitLogic->fTeam( ) == team
			|| !unitLogic->fIsValidTarget( ) )
			return;

		if( priority < bestPriority )
			bestDist = desc.mMaxRange * desc.mMaxRange + 1.f; //reset best distance check

		tVec3f theirPos = targetEnt->fObjectToWorld( ).fGetTranslation( ) + unitLogic->fTargetOffset( );


		const f32 dist = ( theirPos - muzzlePos ).fLengthSquared( );
		if( dist < bestDist )
		{
			if( weapon )
			{
				if( !weapon->fPotentialTargetInRange( theirPos ) )
					return;
			}
			else
			{
				if( !fInBounds( dist, desc.mMinRange*desc.mMinRange, desc.mMaxRange*desc.mMaxRange ) )
					return;
			}

			// found candidate
			if( ownerUnit && desc.mCheckTargetLineOfSight && weapon )
			{
				sigassert( weapon->fInst( ).mIgnoreParent );

				// test line of sight to see if we can actually hit them
				tRayf ray( muzzlePos, theirPos - muzzlePos );
				tLineOfSightRayCastCallback rayCallBack( weapon->fInst( ).mIgnoreParent->fOwnerEntity( ) );
				ownerUnit->fOwnerEntity( )->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCallBack );

				if( rayCallBack.mHitEntity )
				{
					tEntity* hitLogicEnt = rayCallBack.mHitEntity->fFirstAncestorWithLogic( );

					if( Debug_Weapons_RenderGunObstructions )
						hitLogicEnt->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( ray.fEvaluate( rayCallBack.mHit.mT ), ray.fEvaluate( rayCallBack.mHit.mT ) + tVec3f(0,2,0), tVec4f(1,0,0,1) );

					if( hitLogicEnt != targetEnt )
					{
						// LOS obstructed
						return;
					}
				}
			}

			// line of sight is clear
			bestPriority = priority;
			bestDist = dist;
			targetOut.fReset( targetEnt );

			if( weapon && weapon->mAdditionalLockTargetsMT.fCapacity( ) )
				weapon->mAdditionalLockTargetsMT.fPut( targetOut );
		}
	}

	void tWeapon::fFindTargetMT( tWeapon* weapon, u32 teamFallback, const tWeaponDesc& desc, const Math::tVec3f& muzzlePos, tEntityPtr& targetInOut )
	{
		if( Debug_Weapons_DisableTargetting ) return;

		// get closest target
		tEntityPtr target;
		f32 bestDist = cInfinity;
		u32 bestPriority = ~0 - 1; //this way a priority of ~0 will be greater than the intial value

		if( targetInOut )
		{
			//dont let any targets of the same priority change the current target just based on distance.
			// in other words, stick with the target until you find a newer high priority one
			bestDist = 0.f;
			bestPriority = desc.fTargetPriority( targetInOut->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE ) );
			target = targetInOut;
		}

		if( Debug_Weapons_UseEntityList )
		{
			for( u32 t = 0; t < desc.mTargetTypes.fCount( ); ++t )
			{
				u32 priority = t;
				if( priority > bestPriority )
					continue;

				const tGrowableArray<tEntityPtr>& ents = tGameApp::fInstance( ).fCurrentLevel( )->fUnitList( desc.mTargetTypes[ t ] );
				for( u32 i = 0; i < ents.fCount( ); ++i )
				{
					tEntity* targetEnt = ents[ i ].fGetRawPtr( );
					fProcessTarget( weapon, teamFallback, desc, muzzlePos, targetEnt, priority, target, bestDist, bestPriority );
				}
			}
		}
		else
		{
			if( !weapon )
				return;

			tUnitLogic* ownerUnit = weapon->fInst( ).mOwnerUnit;

			tProximity proximity;

			// only look for shape entities
			tDynamicArray<u32> spatialSetIndices;
			Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
			proximity.fSetSpatialSetIndices( spatialSetIndices );

			// add sphere corresponding to max range
			const tAabbf& levelBounds = tGameApp::fInstance( ).fCurrentLevel( )->fLevelBounds( );
			tAabbf proxyBounds( tVec3f( -desc.mMaxRange, levelBounds.mMin.y, -desc.mMaxRange ), tVec3f( desc.mMaxRange, levelBounds.mMax.y, desc.mMaxRange ) );
			//proximity.fAddSphere( tSpheref( tVec3f::cZeroVector, fDesc( ).mMaxRange ) );
			proximity.fAddAabb( proxyBounds );

			// set filters
			u32 team = ownerUnit->fTeam( );
			proximity.fFilter( ).fAddProperty( tEntityEnumProperty( GameFlags::cENUM_TEAM, tPlayer::fDefaultEnemyTeam( team ) ) );
			proximity.fSetFilterByLogicEnts( true );
			proximity.fSetQueryInheritedProperties( true );

			// run proximity query
			proximity.fRefreshMT( 0.f, *ownerUnit->fOwnerEntity( ) );

			for( u32 i = 0; i < proximity.fEntityCount( ); ++i )
			{
				tEntity* targetEnt = proximity.fGetEntity( i );
				
				u32 unitType = targetEnt->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE, GameFlags::cUNIT_TYPE_NONE );
				u32 priority = desc.fTargetPriority( unitType );
				if( priority > bestPriority )
					continue;

				fProcessTarget( weapon, teamFallback, desc, muzzlePos, targetEnt, priority, target, bestDist, bestPriority );
			}
		}

		targetInOut = target;
	}
	/*static*/ tProjectileLogic* tWeapon::fSingleShot( const Math::tMat3f& spawnPt, const Math::tVec3f& target, const tStringPtr& weaponID, u32 team, tEntity* ignoreRoot, tPlayer* player )
	{
		
		const tVec3f muzzlePos = spawnPt.fGetTranslation( );
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		const tWeaponDesc& desc = tWeaponDescs::fInstance( ).fDesc( weaponID );

		sync_event_v_c( target, tSync::cSCProjectile | tSync::cSCLogic );
		sync_event_v_c( muzzlePos, tSync::cSCProjectile | tSync::cSCLogic );
		sync_event_v_c( desc.mProjectileSpeed, tSync::cSCProjectile | tSync::cSCLogic );

		f32 speed = desc.mProjectileSpeed;
		tVec3f vel = target - muzzlePos;

		if( desc.mWeaponType == tGameApp::cWeaponDerivedTypeCannon )
		{
			vel.y = 0;
			f32 angle = 0.f;

			if( ProjectileUtility::fComputeLaunchAngle( angle, speed, muzzlePos, target, desc.fShellGravity( ), false ) )
			{
				tVec3f sideAxis = vel.fCross( tVec3f::cYAxis );
				sideAxis.fNormalizeSafe( tVec3f::cXAxis );

				vel = tQuatf( tAxisAnglef( sideAxis, angle ) ).fRotate( vel );
			}
			else
				log_warning( 0, "Could not compute launch vector for single shot shell weapon: " << desc.mWeaponDescName );
		}

		// Align to velocity
		vel.fNormalizeSafe( spawnPt.fZAxis( ) );
		sigassert( !fEqual( vel.fCross( tVec3f::cYAxis ).fLengthSquared( ), 0.f ) );
		tMat3f xForm = spawnPt;
		xForm.fOrientZAxis( vel );

		vel *= speed;

		tEntity* projectile = level->fRootEntity( )->fSpawnChild( desc.mProjectilePath );
		if( projectile )
		{
			projectile->fMoveTo( xForm );			

			tProjectileLogic* logic = projectile->fLogicDerived< tProjectileLogic >( );
			if( logic )
			{
				tUnitLogic* unit = ignoreRoot ? ignoreRoot->fLogicDerived<tUnitLogic>( ) : NULL;
				tDamageID id( unit, player, team );
				id.mDesc = &desc;
				logic->fSetFiredBy( id );
				logic->fSetIgnoreParent( ignoreRoot );
				logic->fSetSpeed( speed );
				logic->fSetLaunchVector( vel );
				logic->fSetTarget( tEntityPtr( ), target );
				logic->fInitPhysics( );
				logic->fSpawnTrailEffects( );

				if( desc.mFireEffectPath.fExists( ) )
				{
					tEntity* muzzleFlash = level->fRootEntity( )->fSpawnChild( desc.mFireEffectPath );
					if( muzzleFlash ) 
						muzzleFlash->fMoveTo( spawnPt );
				}

				Audio::tSource source( "Single shot weapon" );
				source.fMoveTo( xForm );
				source.fUpdatePosition( );
				source.fSetSwitch( tGameApp::cWeaponTypeSwitchGroup, desc.mAudioAlias );
				source.fHandleEvent( AK::EVENTS::PLAY_WEAPON_FIRE );
			}

			return logic;
		}
		else
			log_warning( 0, "Could not spawn projectile: " << desc.mProjectilePath );

		return NULL;
	}
	tLightEffectLogic* tWeapon::fGetBulletExplosionLight( )
	{
		if( !mBulletExplosionLightLogic )
		{
			sigassert( tGameApp::fInstance( ).fCurrentLevel( ) );
			tEntity* parent = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( );

			// these values will all be overwritten
			tLightEffectLogic *le = NEW tLightEffectLogic( 1.f, 0.f, 1.f, true );
			Gfx::tLightEntity* ent = tLightEffectLogic::fSpawnLightEffect( tMat3f::cIdentity, le, *parent );

			if( ent )
			{
				mBulletExplosionLightEntity.fReset( ent );
				mBulletExplosionLightLogic = le;
			}
		}

		return mBulletExplosionLightLogic;
	}
	void tWeapon::fDebugRenderTargetPos( ) const
	{
#ifdef sig_devmenu
		if( Debug_Weapons_RenderTargets )
		{
			tMat3f xform = tMat3f::cIdentity;
			xform.fSetTranslation( mPredictedTargetPosition );
			const tSpheref sphere( tVec3f( 0.f, 0.f, 0.f ), 1.f );
			tGameApp::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( sphere, xform, Debug_Paths_TargetColor );
		}
#endif
	}
	void tWeapon::fDebugRenderShootDirect( ) const
	{
#ifdef sig_devmenu
		if( Debug_Weapons_RenderTargets )
		{
			for( u32 a = 0; a < mInst.mMuzzles.fCount( ); ++a )
			{
				tVec3f launchVelocity = fComputeLaunchVector( );
				const tVec3f spawnPoint = mInst.mMuzzles[ a ].mProjectileSpawn->fObjectToWorld( ).fGetTranslation( );
				const tPair< tVec3f, tVec3f > line( launchVelocity + spawnPoint, spawnPoint );
				tGameApp::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( line, Debug_Paths_DirectionColor );
			}
		}
#endif
	}
	void tWeapon::fDebugRenderShootArc( ) const
	{
#ifdef sig_devmenu
		if( Debug_Weapons_RenderTargets )
		{
			tVec3f pos( mInst.mMuzzles[ 0 ].mProjectileSpawn->fObjectToWorld( ).fGetTranslation( ) );
			tVec3f launchVelocity = fComputeLaunchVector( );

			tShellPhysics shell( pos, launchVelocity, mDesc.fShellGravity( ) );

			const f32 totalTime = fEstimateTimeToImpactArc( mPredictedTargetPosition );
			const f32 step = 1.f / 30.f;

			if( totalTime < 0.f ) return;
			for( f32 time = 0.f; time < totalTime; time += step )
			{
				tVec3f nextPos = shell.fStep( step );
				const tPair< tVec3f, tVec3f > line( nextPos, pos );
				tGameApp::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( line, Debug_Paths_DirectionColor );
				pos = nextPos;
			}
		}
#endif
	}
	tVec3f tWeapon::fComputeLaunchVector( u32 anchorIndex ) const
	{
		tVec3f launchVelocity = ComputeLeveledAnchorZ( anchorIndex );		
		const tVec3f side = launchVelocity.fCross( tVec3f::cYAxis ).fNormalizeSafe( tVec3f::cZAxis );

		const tQuatf quatP( tAxisAnglef( side, fToRadians( fOutputPitchAngle( ) ) ) );
		launchVelocity = quatP.fRotate( launchVelocity );
		launchVelocity *= mCurrentProjectileSpeed;

		return mParentVelocity + launchVelocity;
	}
	tVec3f tWeapon::fComputeIdealLaunchVector( u32 anchorIndex ) const
	{
		tVec3f launchVelocity;
		launchVelocity.fSetXZHeading( mIdealYawAngle );

		const tVec3f side = launchVelocity.fCross( tVec3f::cYAxis ).fNormalizeSafe( tVec3f::cZAxis );

		const tQuatf quatP( tAxisAnglef( side, fToRadians( fOutputPitchAngle( ) ) ) );
		launchVelocity = quatP.fRotate( launchVelocity );
		launchVelocity *= mCurrentProjectileSpeed;

		return mParentVelocity + launchVelocity;
	}
	void tWeapon::fAccumulateFireEvents( tFireEventList& list )
	{
		if( mLastFireEvent.mSet ) 
			list.fPushBack( mLastFireEvent );
		mLastFireEvent.fReset( );
	}

	f32 tWeapon::fReloadProgress()
	{
		return mReloadTimer / fReloadTime( );
	}

	void tWeapon::fSetReloadProgress( f32 percentComplete )
	{
		mReloadTimer = percentComplete * fReloadTime( );
	}

}


namespace Sig
{
	void tWeapon::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tWeapon, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("HasTarget"),			&tWeapon::fHasTarget)
			.Prop(_SC("IsContinuousFire"),	&tWeapon::fIsContinuousFire)
			.Prop(_SC("CanFire"),			&tWeapon::fCanFire)
			.Prop(_SC("Firing"),			&tWeapon::fFiring)
			.Prop(_SC("WithinReach"),		&tWeapon::fWithinReach)
			.Prop(_SC("IsAimingNearTarget"), &tWeapon::fIsAimingNearTarget)
			.Func(_SC("SpawnReloadEffect"),	&tWeapon::fSpawnReloadEffect)
			.Func(_SC("UseUI"),				&tWeapon::fUseUI)
			.Func(_SC("SetTurretEntityNamed"),	&tWeapon::fSetTurretEntityNamed)
			.Func(_SC("SetTurretEntity"),	&tWeapon::fSetTurretEntity)
			.Prop(_SC("TurretEntity"),		&tWeapon::fTurretEntity)
			.Func(_SC("SetAnimated"),		&tWeapon::fSetAnimated)
			.Prop(_SC("SpinUp"),			&tWeapon::fSpinUp)
			.Prop(_SC("RapidFire"),			&tWeapon::fRapidFire)
			.Func(_SC("SetAITargetOverride"), &tWeapon::fSetAITargetOverride)
			.Prop(_SC("AIFireOverride"),	&tWeapon::fAIFireOverride, &tWeapon::fSetAIFireOverride)
			.Prop(_SC("Enabled"),			&tWeapon::fEnabled, &tWeapon::fEnable)
			;
		vm.fRootTable( ).Bind( _SC("Weapon"), classDesc );
		vm.fConstTable( ).Const( _SC("FIRE_MODE_ALL"), (s32)cWeaponFireModeAll );
		vm.fConstTable( ).Const( _SC("FIRE_MODE_ALTERNATE"), (s32)cWeaponFireModeAlternate );
	}
}
