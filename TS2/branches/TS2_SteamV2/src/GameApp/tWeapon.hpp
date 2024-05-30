#ifndef __tWeapon__
#define __tWeapon__

#include "tWeaponUI.hpp"
#include "tUser.hpp"
#include "tFxFileRefEntity.hpp"
#include "Audio/tSource.hpp"
#include "tDamageContext.hpp"

namespace Sig
{
	class tUnitLogic;
	class tProjectileLogic;
	class tPlayer;
	class tWeaponStation;
	class tWeapon;
	class tLightEffectLogic;

	namespace Input
	{
		class tGamepad;
	}

	class tWeapon;
	typedef tRefCounterPtr<tWeapon> tWeaponPtr;

	enum tWeaponFireMode
	{
		cWeaponFireModeAll,
		cWeaponFireModeAlternate,
		cWeaponFireModeOneShotMinigameMode,
		cWeaponFireModeCount
	};

	struct tWeaponDesc
	{
		tWeaponDesc( )
			: mWeaponDescIndex( ~0 )
			, mEffectDamageType( GameFlags::cDAMAGE_TYPE_NONE )
			, mMinRange( 0.f )
			, mMaxRange( 0.f )
			, mMaxAngle( 0.f )
			, mMinPitch( 0.f )
			, mMaxPitch( 0.f )
			, mTurnRate( 0.f )
			, mFireRate( 0.f )
			, mBankFireRate( -1.f )
			, mMaxSpread( 0.f )
			, mSpreadRate( 0.f )
			, mSpreadSettleRate( 0.f )
			, mAISpread( 0.f )
			, mReloadTime( 0.f )
			, mReloadTimeAI( 0.f )
			, mProjectileSpeed( 0.f )
			, mProjectileAcceleration( 0.f )
			, mParentVelScale( 1.f )
			, mReactionImpulse( 0.f )
			, mDamageImpulseScale( 0.f )
			, mMaxAmmo( 0 )
			, mMaxAmmoAI( 0 )
			, mExplosionFullSize( 0 )
			, mExplosionGrowthRate( 0 )
			, mExplosionFallOff( 0 )
			, mMuzzleFlashLightSize( 0 )
			, mWantsWorldSpaceUI( false )
			, mIsContinuousFire( false )
			, mRaycastAdjustTargets( false )
			, mPreSpawnProjectiles( false )
			, mStickyReticle( false )
			, mShootIfNoLock( true )
			, mLeadTargets( true )
			, mDoesntNeedtoPointAtTarget( false )
			, mWantsShellCam( false )
			, mAreaDamage( false )
			, mSpinUp( false )
			, mBarrageWeapon( false )
			, mAnimationDriven( false )
			, mRapidFire( false )
			, mShotCount( 1 )
			, mPersistentDamageType( ~0 )
			, mBulletTracerType( ~0 )	// starts off in an invalid state. To be valid, must match an enum value in GameFlags::tTracer_Type.
			, mTracerTrailType( ~0 )	// starts off in an invalid state. To be valid, must match an enum value in GameFlags::tTracer_Type.
			, mTracerTrailInterval( 1 )
			, mBulletTracerTypeOverCharged( ~0 )
			, mTracerTrailTypeOverCharged( ~0 )
			, mTracerTrailIntervalOverCharged( 1 )
			, mMaxLocks( 0 )
			, mWeaponType( 0 )
			, mHitPointsVersus( 1.f )
			, mFireMode( cWeaponFireModeAll )
			, mDamageModDirectHit( 1.f )
			, mUserModeDamageMultiplier( 1.f )
			, mUserModeDamageMultiplierVersus( 1.f )
			, mOverChargeMultiplier( 1.f )
			, mAudioType( 1 )
			, mUseBankAudio( 0 )
			, mUseStatToInc( -1 )
		{
			mDamageMod.fFill( 1.f );
			mHitPoints.fFill( 1.f );
		}

		f32 fShellGravity( ) const
		{
			return mProjectileGravity == 0.f ? -9.8f : mProjectileGravity;
		}

		u32 fTargetPriority( u32 unitType ) const
		{
			return mTargetTypes.fIndexOf( unitType );
		}

		u32 mWeaponDescIndex;
		tStringPtr mWeaponDescName;
		tStringPtr mAudioAlias;
		GameFlags::tDAMAGE_TYPE mEffectDamageType;
		tFilePathPtr mProjectilePath;
		tStringPtr	 mHitEffect;
		tStringPtr	 mHitEffectOverCharged;
		tFilePathPtr mFireEffectPath;
		tFilePathPtr mFireEffectPathOverCharged;
		tFilePathPtr mAfterFireEffectPath;
		tFilePathPtr mReloadEffectPath;
		tFilePathPtr mShellCasingPath;
		tFilePathPtr mUIScriptPath;
		tFilePathPtr mUIAmmoIconPath;
		tFilePathPtr mUIAmmoTickMarkPath;

		f32 mMinRange, mMaxRange;
		f32 mMaxAngle;
		f32 mMinPitch, mMaxPitch;
		f32 mTurnRate;
		f32 mFireRate; //time between shots for continuous
		f32 mBankFireRate; // for multiple weapons in bank
		f32 mMaxSpread;
		f32 mSpreadRate;
		f32 mSpreadSettleRate;
		f32 mAISpread;
		f32 mReloadTime;
		f32 mReloadTimeAI;
		f32 mHitPointsVersus; // to remove
		tFixedArray<f32, GameFlags::cDIFFICULTY_COUNT> mHitPoints;
		tFixedArray<f32, GameFlags::cUNIT_TYPE_HEAVY_PROP + 1> mDamageMod;
		f32 mDamageModDirectHit;
		f32 mUserModeDamageMultiplier;
		f32 mUserModeDamageMultiplierVersus;
		f32 mOverChargeMultiplier;

		f32 mProjectileSpeed;
		f32 mProjectileAcceleration;
		f32 mProjectileGravity;
		f32 mProjectileSpin;
		f32 mParentVelScale; //how much parent influence to inherit
		f32 mReactionImpulse;
		f32 mDamageImpulseScale;
		u32 mMaxAmmo;
		u32 mMaxAmmoAI;
		f32 mExplosionFullSize;
		f32 mExplosionGrowthRate;
		f32 mExplosionFallOff;
		f32 mMuzzleFlashLightSize;
		f32 mMuzzleFlashLightLife;
		Math::tVec4f mMuzzleFlashLightColor;
		f32 mImpactLightSize;
		f32 mImpactLightLife;
		Math::tVec4f mImpactLightColor;

		b8 mWantsWorldSpaceUI;
		b8 mIsContinuousFire;
		b8 mRaycastAdjustTargets;		
		b8 mPreSpawnProjectiles;

		b8 mStickyReticle;
		b8 mShootIfNoLock;
		b8 mLeadTargets;
		b8 mDoesntNeedtoPointAtTarget; //rocket pods who dont pitch towards targets.

		b8 mWantsShellCam;
		b8 mAreaDamage;
		b8 mCheckTargetLineOfSight;
		b8 mSpinUp;

		b8 mBarrageWeapon;
		b8 mAnimationDriven;
		b8 mRapidFire;
		u8 mShotCount;
		
		u16 mBulletTracerType;
		u16 mBulletTracerTypeOverCharged;

		u16 mTracerTrailType;
		u16 mTracerTrailInterval;

		u16 mTracerTrailTypeOverCharged;
		u16 mTracerTrailIntervalOverCharged;

		u16 mMaxLocks;
		u16 mWeaponType;

		f32 mShellCamInitiateTimer;
		f32 mShellCamBlendInDepart;
		f32 mShellCamBlendInArrive;
		f32 mShellCamOverallBlend;
		
		f32 mArrowWidth; //for world space arc arrow

		u32 mPersistentDamageType;

		tWeaponFireMode mFireMode;

		tGrowableArray<GameFlags::tUNIT_TYPE> mTargetTypes;
		tGrowableArray<GameFlags::tUNIT_TYPE> mLockTypes;

		enum tAudioType { cAudioTypeLooping, cAudioTypeSingleShot };
		u8 mAudioType;
		u8 mUseBankAudio;
		u16 mUseStatToInc;

		tStringPtr mNormalStartAudioID;
		tStringPtr mNormalStopAudioID;
		tStringPtr mShellCamStartAudioID;
		tStringPtr mShellCamStopAudioID;
		tStringPtr mScopeStartAudioID;
		tStringPtr mScopeStopAudioID;
		tStringPtr mSpecialStartAudioID;
		tStringPtr mSpecialStopAudioID;

		/* Possible additional weapon attributes
		f32 mRateofFire;
		f32 mReloadTime;
		f32 mOverheatTime;
		f32 mCooldownRate;
		*/
	};

	struct tMuzzle
	{
		tMuzzle( ) : mLightEntityLogic( NULL ) { }

		tEntityPtr mProjectileSpawn;
		tEntityPtr mShellCasingSpawn;
		tEntityPtr mShellCasingDir;
		tEntityPtr mLightEntity;
		tLightEffectLogic* mLightEntityLogic;
	};

	struct tWeaponInstData
	{
		tWeaponInstData( )
			: mOwnerUnit( NULL )
			, mIgnoreParent( NULL )
			, mAnimated( NULL )
			, mStation( NULL )
			, mTurretEntity( NULL )
			, mOverCharged( false )
			, mWantsUI( false )
			, mTriggerButton( ~0 )
			, mUserDamageMultiplier( 1.f )
			, mBankID( 0 )
		{ }

		void fOnDelete( )
		{
			mMuzzles.fSetCount( 0 );
			mAudioSources.fSetCount( 0 );
			mTargettingParent.fRelease( );
			mAnimated.fRelease( );
			mTurretEntity.fRelease( );
			mStation = NULL;
			mOwnerUnit = NULL;
			mIgnoreParent = NULL;
		}

		tGrowableArray<tMuzzle>		mMuzzles;
		tUnitLogic*					mOwnerUnit;
		tUnitLogic*					mIgnoreParent; //the most ancestorial entity to ignore
		tWeaponStation*				mStation;
		tEntityPtr					mAnimated;		// what handles the recoil and reload anim
		tEntityPtr					mTurretEntity;	// what yaws for this weapon
		tGrowableArray<Audio::tSourcePtr> mAudioSources;
		tWeaponPtr					mTargettingParent; //if this is set, dont do our own targetting. just use the parents.

		b8 mOverCharged;
		b8 mWantsUI; //this is used to override any ui desires
		u8 mBankID;
		u8 pad1;

		u32 mTriggerButton;
		f32 mUserDamageMultiplier;

		b32 fUnlimitedAmmo( ) const;

	};

	// This object gets inserted between effects and their parent so the effect parent contains weapon info (for persistent area damage)
	class tAreaEffectParentLogic : public tLogic
	{
		define_dynamic_cast(tAreaEffectParentLogic, tLogic);
	};
	class tAreaEffectParent : public tEntity
	{
		define_dynamic_cast(tAreaEffectParent, tEntity);
	public:
		tRefCounterPtr<tWeapon> mWeapon;

		// creates an entity with this logic
		static tEntity* fCreate( const tRefCounterPtr<tWeapon>& weap )
		{
			sigassert( weap );
			tAreaEffectParent* newEnt = NEW tAreaEffectParent( );
			newEnt->mWeapon = weap;
				
			tAreaEffectParentLogic *dl = NEW tAreaEffectParentLogic( );
			tLogicPtr *dlp = NEW tLogicPtr( dl );

			newEnt->fAcquireLogic( dlp );
			return newEnt;
		}

		virtual void fOnEmptyNest( ) { fDelete( ); }
	};

	struct tFireEvent
	{
		Math::tVec3f	mProjectilVel;
		Math::tVec3f	mLocalMuzzlePt;
		b32				mSet;
		tWeapon*		mWeapon;
		tEntityPtr		mProjectile;

		tFireEvent( )
			: mSet( false ), mWeapon( NULL )
		{ }

		void fReset( )
		{
			mSet = false;
			mProjectile.fRelease( );
		}
	};
	typedef tGrowableArray<tFireEvent> tFireEventList;

	class tWeaponTarget : public tRefCounter
	{
	public:
		u32			mUID;
		tEntityPtr	mEntity;
		tUnitLogic *mLogic;
		GameFlags::tUNIT_TYPE mType;
		Math::tVec2f mScreenPos;
		u32			mLockedForBank;
		u32			mMuzzleIndex;
		b32			mAddedToUI;

		b32 fLocked( ) const { return mLockedForBank != ~0; }

		tWeaponTarget( ) 
			: mUID( 0 )
			, mAddedToUI( false )
			, mMuzzleIndex( ~0 )
		{ }

		tWeaponTarget( u32 uid, const tEntityPtr& ent, tUnitLogic *logic, GameFlags::tUNIT_TYPE type, const Math::tVec2f& screenPos ) 
			: mUID( uid )
			, mEntity( ent )
			, mLogic( logic )
			, mType( type )
			, mScreenPos( screenPos )
			, mLockedForBank( ~0 )
			, mAddedToUI( false )
			, mMuzzleIndex( ~0 )
		{ }
	};
	typedef tRefCounterPtr<tWeaponTarget> tWeaponTargetPtr;

	class tWeapon : public tUncopyable, public tRefCounter
	{
		define_dynamic_cast_base(tWeapon);
	public:
		static void fExportScriptInterface( tScriptVm& vm );
		static b32 fRenderTargets( );

		static const tStringPtr cWeaponAttachName;
	public:
		explicit tWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst );
		virtual ~tWeapon( );

		b32				fValid( ) { return mInst.mStation != NULL; }
		tWeaponStation& fStation( ) const { sigassert( mInst.mStation ); return *mInst.mStation; }
		// Turret entity is merely user data, not affected from inside the weapon.
		tEntity*		fSetTurretEntityNamed( const tStringPtr& name );
		tEntity*		fSetTurretEntity( tEntity* ent );
		tEntity*		fTurretEntity( ) const { return mInst.mTurretEntity.fGetRawPtr( ); }
		tEntity*		fSetAnimated( tEntity* ent );

		void fInitParticles( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );

		const tEntityPtr& fFirstAnchorPoint( ) const { return mInst.mMuzzles[ 0 ].mProjectileSpawn; }
		const Math::tMat3f& fTargetingRefFrame( ) const;
		const Math::tVec3f& fIdealFacingDirection( ) const { return mIdealFacingDirection; }
		b32 fHasTarget( ) const { return mHasTarget; }
		b32 fIsContinuousFire( ) const { return fDesc( ).mIsContinuousFire; }
		b32 fSpinUp( ) const { return fDesc( ).mSpinUp; }
		b32 fRapidFire( ) const { return fDesc( ).mRapidFire; }

		b32 fCanFire( ) const { return mEnable && !mShellCaming && !fNeedsReload( ) && !fReloading( ); }
		b32 fNeedsReload( ) const { return !mReloadTimerActive && fCurrentAmmo( ) == 0; }
		b32 fReloading( ) const { return mReloadTimerActive || mReloadOverride; }
		b32 fIsHoldAndReleaseFire( ) const { return mIsHoldAndReleaseFire; }
		b32 fFiring( ) const { return mFiring; }
		b32 fWithinReach( ) const { return mWithinReach; } //Target is within our targeting range but the projectile can't actually hit it
		b32 fShellCaming( ) const { return mShellCaming; }
		u32 fCurrentAmmo( ) const { return fMax<s32>( 0, fReloadAmmoCount( ) - (s32)mShotsFired); }
		b32 fShouldFire( ) const;
		b32 fShouldAcquire( ) const { return ((mAcquireTargets || mAITargetOverride) && !mPlayer && mEnable); }
		b32 fReticalOverTarget( ) const { return mReticalOverTarget; }
		f32 fCurrentSpreadPercentage( ) const { return mCurrentSpread / mDesc.mMaxSpread; }
		b32 fTargetCooled( ) const;

		void fSetParentVelocity( const Math::tVec3f& vel ) { mParentVelocity = vel; }

		u32 fReloadAmmoCount( b32 assumeUserControl = false ) const;
		f32 fReloadTime( ) const;
		void fResetAmmoCount( );
		void fReload( );
		void fReloadAfterTimer( );
		f32 fReloadProgress( );
		void fSetReloadProgress( f32 percentComplete );

		void fSetReloadOverride( b32 override ) { mReloadOverride = override; } //set this ifyou're waiting to reload in your own code.
		void fSetAIFireOverride( b32 fire ) { mAIFireOverride = fire; if( mAITargetOverride && fFiring( ) ) fEndFire( ); }
		b32	 fAIFireOverride( ) const { return mAIFireOverride; }
		void fEnable( b32 enable ) { mEnable = enable; }
		b32	 fEnabled( ) const { return mEnable; }
		void fSpinUp( b32 up );
		void fSetSpinUpPercentage( f32 percent ) { mSpinUpPercentage = percent; }

		b32 fShouldShellCam( ) const;
		
		// returns true if continuous fire
		virtual b32 fFire( const tWeaponTarget* target = NULL );
		virtual void fEndFire( );
		virtual void fProcessST( f32 dt );
		virtual void fProcessMT( f32 dt );
	public:
		inline const tWeaponDesc& fDesc( ) const { return mDesc; }
		inline const tWeaponInstData& fInst( ) const { return mInst; }
		inline tWeaponInstData& fInst( ) { return mInst; }
		void fUseUI( ) { mInst.mWantsUI = true; }

		f32 fOutputPitchAngle( ) const;
		f32 fPitchAngle( ) const;
		void fSetPitchAngle( f32 pitch );
		void fAdjustPitchAngle( f32 deltaPitch );
		void fPitchTowardsIdealAngle( f32 dt );

		b32 fWorldSpacePitchMode( ) const		{ return mDesc.mWantsWorldSpaceUI && (mDesc.mMinRange > 1.0f); }
		void fAdjustWorldSpacePitch( f32 dt );

		f32 fYawAngle( ) const { return mCurrentYawAngle; }
		f32 fIdealYawAngle( ) const { return mIdealYawAngle; }
		void fSetYawAngle( f32 yaw ) { mCurrentYawAngle = yaw; }
		void fAdjustYawAngle( f32 deltaYaw ) { fSetYawAngle( deltaYaw + mCurrentYawAngle ); }
		void fYawTowardsIdealAngle( f32 dt );

		f32 fProjectileSpeed( ) const { return mCurrentProjectileSpeed; }
		f32 fIdealProjectileSpeed( ) const { return mDesc.mProjectileSpeed; }
		f32 fMinimumProjectileSpeed( ) const { return mMinimumProjectileSpeed; }
		void fSetProjectileSpeed( f32 speed ) { mCurrentProjectileSpeed = fClamp( speed, mMinimumProjectileSpeed, fDesc( ).mProjectileSpeed ); }
		void fAdjustProjectileSpeed( f32 deltaSpeed ) { fSetProjectileSpeed( deltaSpeed + mCurrentProjectileSpeed ); }
		void fAdjustSpeedTowardsIdeal( f32 dt );

		b32 fInvertPitch( ) const { return mInvertPitch; }

		const tEntityPtr& fAITarget( ) const { return mAITarget; }
		const tRingBuffer< tEntityPtr >& fAdditionalLockTargets( ) const { return mAdditionalLockTargets; }

		static tProjectileLogic* fSingleShot( const Math::tMat3f& spawnPt, const Math::tVec3f& target, const tStringPtr& weaponID, u32 team = GameFlags::cTEAM_COUNT, tEntity* ignoreRoot = NULL, tPlayer* player = NULL );

	public:
		b32 fIsAimingNearTarget( ) const;
		b32 fPotentialTargetInRange( const Math::tVec3f& targetPosition ) const;
		
		void fBeginUserControl( tPlayer* player );
		void fEndUserControl( );
		b32 fUnderUserControl( ) const { return mPlayer != NULL; }
		tPlayer* fPlayer( ) const { return mPlayer; }

		//enable this from ai controlled units to enable targetting (if respected by derived classes)
		b32 fAcquireTargets( ) const { return mAcquireTargets; }
		void fSetAcquireTargets( b32 enable ) { mAcquireTargets = enable; }
		void fSetAITargetOverride( tEntity* target ) { mAITargetOverride.fReset( target ); }

		void fSetWorldSpaceArcTargetPosition(const Math::tVec3f& target ) { mWorldSpaceArcTargetPosition = target; mUseArcTarget = true; }

		Math::tVec3f fComputeLaunchVector( u32 anchorIndex = 0 ) const;
		Math::tVec3f fComputeIdealLaunchVector( u32 anchorIndex = 0 ) const;
		void fAccumulateFireEvents( tFireEventList& list );

		void fComputeIdealAngleDirect( );
		void fComputeIdealAngleArc( );

		void fProcessShellCamST( f32 dt );
		void fShellCamDied( );
		void fProjectileDied( tProjectileLogic* proj );

		void fProcessShellBursting( f32 dt );
		void fAddProjectileToLastFiredList( tProjectileLogic* proj ) { mLastProjectiles.fPushBack( proj ); }

		virtual void fSpawnFireEffect( tProjectileLogic* projectile, u32 anchorIndex );
		virtual void fAudioBegin( );
		virtual void fAudioEnd( );

		void fPlayRecoilMotion( u32 anchorIndex );
		void fSpawnCasing( u32 attachIndex );
		virtual void fBeginAreaEffect( u32 anchorIndex );
		virtual void fEndAreaEffect( u32 anchorIndex );
		void fSpawnReloadEffect( u32 anchorIndex = -1 );

		b32 fRequestTrail( ); //returns true if a projectile should spawn a trail.

		void fSetYawConstraint( const Math::tVec3f& axis, f32 angle )
		{
			mYawConstraintAxis = axis;
			mYawConstaintAngle = angle;
			mYawConstraintSet = true;
		}

		// this assumes the previous one had been called
		void fSetYawConstraint( const Math::tVec3f& axis )
		{
			mYawConstraintAxis = axis;
		}

		virtual void fBeginRendering( tPlayer* player ) { } // TODO: Maybe take a user for rendering when not under user control
		virtual void fEndRendering( ) { }
		b32 fRendering( ) const { return mRendering; }

		tLightEffectLogic* fGetBulletExplosionLight( );

		static void fFindTargetMT( tWeapon* weapon, u32 teamFallback, const tWeaponDesc& desc, const Math::tVec3f& muzzlePos, tEntityPtr& targetInOut );

		tDamageID fBuildID( );

	protected:
		b32 fReleaseTargetIfNoLongerValid( tEntityPtr& target ); // returns true if target was previously valid but released during this call
		void fUpdateTargetRelatedData( f32 dt );
		void fSpawnProjectiles( const tWeaponTarget* target = NULL );
		void fSpawnNewProjectileFromMuzzle( u32 attachIndex, const tWeaponTarget* target, u32 shotIndex );
		void fSpawnProjectile( tEntity* projectile, u32 attachIndex, const Math::tMat3f& xForm, const tWeaponTarget* target, u32 shotIndex );
		void fPreSpawnProjectiles( );
		void fPreSpawnProjectiles( u32 index );

		Math::tVec3f ComputeLeveledAnchorZ( u32 anchorIndex = 0 ) const;

		static void fProcessTarget( tWeapon* weapon, u32 teamFallback, const tWeaponDesc& desc, const Math::tVec3f& muzzlePos, tEntity* target, u32 priority, tEntityPtr& targetOut, f32& bestDist, u32& bestPriority );
		void fRayCastAndAdjustTarget( );
		virtual void fComputeIdealAngle( ) { }
		virtual void fComputeIdealVelocity( ) { }
		void fComputeIdealVelocityArc( );
		void fComputeIdealYawAngle( );
		virtual f32 fEstimateTimeToImpact( const Math::tVec3f& pos ) const { return 0.f; }
		f32 fEstimateTimeToImpactDirect( const Math::tVec3f& pos ) const;
		f32 fEstimateTimeToImpactArc( const Math::tVec3f& pos ) const;
		Math::tVec3f fPredictTargetPosition( const tEntityPtr& target, f32 dt );
		void fDebugRenderTargetPos( ) const;
		void fDebugRenderShootDirect( ) const;
		void fDebugRenderShootArc( ) const;

		void fConfigureAudio( );

		void fReleaseTarget( );
		void fAcquireTarget( const tEntityPtr& target );

		b32 fSpinningUp( ) const { return mSpinUpPercentage < 1.f; }
		b32 fWantsSpinUp( ) const { return mDesc.mSpinUp != 0; }

		b32 fReadyToRaycast( u8 interval ) { ++mRayCastCounter; if( mRayCastCounter > interval ) { mRayCastCounter = 0; return true; } return false; }

	protected:
		tWeaponInstData mInst;
		const tWeaponDesc& mDesc;
		tPlayer*		mPlayer;
		Math::tVec3f	mParentVelocity;

		// if the bullets spawn an explosion, there will only be one light that moves with each new shot
		tEntityPtr		mBulletExplosionLightEntity;
		tLightEffectLogic* mBulletExplosionLightLogic;

		// For ai targetting
		tEntityPtr		mAITarget;
		tEntityPtr		mAITargetMT;
		tEntityPtr		mAITargetOverride;
		Math::tVec3f	mPredictedTargetPosition;
		f32				mAITargetCooloff; //how long we have to wait to shoot the target
		tRingBuffer<tEntityPtr> mAdditionalLockTargets;
		tRingBuffer<tEntityPtr> mAdditionalLockTargetsMT;

		// For user targetting
		tEntityPtr		mReticalTarget;
		Math::tVec3f	mReticalTargetPosition;
		Math::tVec3f	mWorldSpaceArcTargetPosition;
		tUnitLogic		*mReticalTargetLogic;

		// Yaw constraints for turrets
		Math::tVec3f	mYawConstraintAxis;
		f32				mYawConstaintAngle; //max deviation from constraint axis

		Math::tVec3f	mIdealFacingDirection;
		f32 mIdealPitchAngle;
		f32 mCurrentPitchAngle;
		f32	mZeroToOnePitch; //for arc weapons
		f32 mIdealYawAngle;
		f32 mCurrentYawAngle;
		f32 mIdealProjectileSpeed;
		f32 mCurrentProjectileSpeed;
		f32 mMinimumProjectileSpeed;
		f32 mCurrentSpread;
		f32 mFireTimer;
		f32 mSpinUpPercentage;

		b8 mHasTarget;
		b8 mBehaveContinuously;
		b8 mWithinReach;
		b8 mRendering;

		u8 mRayCastCounter;
		b8 pad1;
		b8 pad2;
		b8 mUseArcTarget;

		b8 mUseRayCastedTarget;  //this means it was computed
		b8 mFiring;
		b8 mShellCamEligible;
		b8 mShellCaming;

		b8 mShellCamDeath;
		b8 mShellCamPushed;
		b8 mReticalOverTarget;
		b8 mIsHoldAndReleaseFire;

		b8 mYawConstraintSet;
		b8 mInvertPitch; // for mortar
		b8 mAcquireTargets;
		b8 mReloadTimerActive;

		b8 mReloadOverride;
		b8 mAIFireOverride;
		b8 mEnable;	
		b8 mLoopingAudioPushed;

		tFireEvent mLastFireEvent;
		tGrowableArray<tProjectileLogic*> mLastProjectiles; //they inform us when they're destroyed
		f32 mShellCamInitiateTimer;
		f32 mShellCamDeathTimer;
		f32 mReloadTimer;
		u32 mShotsFired;

		f32 mWorldSpacePitch;

		u32 mNextMuzzleIndex; //for alternate fire mode
		Sqrat::Function	mRecoilMotionState;
		s32 mTracerCounter;
 
		tGrowableArray<FX::tFxSystemsArray> mFireParticleSystems, mFireParticleSystemsOverCharged; //one for each muzzle attachment
		tGrowableArray<tEntityPtr> mPreSpawnedProjectiles; //one for each muzzle attachment

	protected:
		void fHandleAudioEvent( u32 eventID );
		void fSetAudioRTPC( const tStringPtr& rtpc, f32 value );


	};
}

#endif//__tWeapon__
