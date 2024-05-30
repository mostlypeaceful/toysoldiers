#ifndef __tVehicleLogic__
#define __tVehicleLogic__
#include "tUnitLogic.hpp"
#include "tVehiclePassengerLogic.hpp"
#include "Logic/tAnimatable.hpp"
#include "Audio/tSource.hpp"
#include "tProximity.hpp"
#include "tUnitPath.hpp"
#include "tFxFileRefEntity.hpp"
#include "Physics/tContactPoint.hpp"
#include "tHoverTimer.hpp"
#include "Physics/tHinge.hpp"
#include "tGeneratorLogic.hpp"

namespace Sig
{
	class tVehicleLogic;
	class tCharacterLogic;
	class tTurretCameraMovement;

	namespace Input
	{
		class tRumbleEvent;
	}

	class tVehicleOffender
	{
	public:
		Math::tObbf mBounds;
		Math::tVec3f mVelocity;

		tVehicleOffender( ) { }

		tVehicleOffender( const Math::tObbf& bounds, const Math::tVec3f& velocity )
			: mBounds(bounds), mVelocity(velocity)
		{ }
	};

	class tVehicleCargo
	{
	public:
		tFilePathPtr	mPath;
		u32				mCount;
		u32				mIndex;
		f32				mSpawnRate;
		b8				mStopWhileSpawning;
		b8				mRemoveCargoAfterDropped;
		b8				mRelativeToDropPt; //for rope slide and climbing out of hatches
		b8				mParachute;
		tStringPtr		mFocusPrompt;

		//these are set after spawning
		u32				mTotalCount;
		tGrowableArray<tEntityPtr> mTrenchedUnits;
		tPathEntity*	mPathPt;

		b8 mFocusShown;
		b8 mDisablePhysics;
		b8 pad1;
		b8 pad2;

		tVehicleCargo( const tFilePathPtr& path = tFilePathPtr( )
			, u32 count = 0
			, f32 spawnRate = 1.f
			, b32 stopWhileSpawning = false
			, b32 removeCargoAfter = false
			, b32 relativeToDropPt = false
			, b32 parachute = false
			, b32 disablePhysics = false
			, const tStringPtr& focusPrompt = tStringPtr::cNullPtr )
			: mPath( path )
			, mCount( count )
			, mIndex( 0 )
			, mSpawnRate( spawnRate )
			, mStopWhileSpawning( stopWhileSpawning )
			, mRemoveCargoAfterDropped( removeCargoAfter )
			, mRelativeToDropPt( relativeToDropPt )
			, mParachute( parachute )
			, mDisablePhysics( disablePhysics )
			, mFocusPrompt( focusPrompt )
			, mPathPt( NULL )
			, mFocusShown( false )
		{ }
	};

	struct tVehicleSeat
	{
		// Extra Stuff
		tEntityPtr	mEnterPoint;
		tEntityPtr	mSeatPoint;
		tEntityPtr	mDoor;
		tEntityPtr	mDoorHinge;
		Physics::tOneWayHingePtr mHinge;
		b8			mOpeningDoor;
		b8			mClosingDoor;
		b8 pad1;
		b8 pad2;
		// Extra stuff

		b32			mOccupied;
		tRefCounterPtr<tPlayer> mPlayer;

		tVehicleSeat( );

		void fStep( f32 dt );
		void fOpenCloseDoor( b32 open );
	};

	enum tVehicleSeatName { cVehicleDriverSeat = 0, cVehicleGunnerSeat, cVehicleSeatCount };

	class tVehicleLogic : public tUnitLogic
	{
		define_dynamic_cast( tVehicleLogic, tUnitLogic );
	public:
		tVehicleLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );
		virtual void fOnSkeletonPropagated( );

	public: // query specific components
		virtual Logic::tAnimatable* fQueryAnimatable( ) { return &mAnimatable; }

	public:
		void fOccupySeat( tPlayer* player, u32 seat );
		void fVacateSeat( u32 seat );
		virtual void fShowInUseIndicator( Gui::tInUseIndicator* indicator );

		const tVehicleSeat& fSeat( u32 seat ) const { return mSeats[ seat ]; }
		tVehicleSeat&		fSeat( u32 seat ) { return mSeats[ seat ]; }
		u32 fSeatCount( ) const { return mSeats.fCount( ); }
		b32 fSeatOccupied( u32 seat ) { return seat >= mSeats.fCount( ) ? false : mSeats[ seat ].mOccupied; }
		tPlayer* fGetPlayer( u32 seat ) const { return mSeats[ seat ].mPlayer.fGetRawPtr( ); }
		void fAddRumbleEvent( const Input::tRumbleEvent& event );
		void fSetExplicitRumble( f32 rumble );

		void fOverrideOccupy( tPlayer* player, b32 set ) { mSeats[ 0 ].mOccupied = set; mSeats[ 0 ].mPlayer.fReset( set ? player : NULL ); }

		virtual Gui::tRadialMenuPtr fCreateSelectionRadialMenu( tPlayer& player );
		virtual tUnitPath* fUnitPath( ) { return mUnitPath.fGetRawPtr( ); }
		virtual Math::tVec3f fLinearVelocity( const Math::tVec3f& localOffset ) { return fPointVelocityMT( fOwnerEntity( )->fObjectToWorld( ).fXformPoint( localOffset ) ); }
		virtual f32 fMaxSpeed( ) const { return 1.f; }

		const Math::tAabbf& fGetCollisionBounds( ) const { return mCollisionBounds; }
		f32 fMass( ) const { return mMass; }
		f32 fThrottleScale( ) const { return mThrottleScale; }
		f32 fPowerLevel( ) const { return mPowerLevel; }
		void fSetPowerLevel( f32 power ) { mPowerLevel = power; }
		void fAddPowerLevel( f32 power ) { mPowerLevel = fMin( 1.f, mPowerLevel + power ); }
		b32	 fChargeIfNotCharging( );

		void fAddCargo( const tStringPtr& cargoName );
		void fDropCargo( u32 index );
		void fClearCargo( ) { fCancelCargoDrop( ); mCargo.fSetCount( 0 ); }
		void fAddSimpleCargo( u32 unitID, u32 count, f32 rate );
		b32	 fDroppingEnabled( ) const { return mDroppingEnabled; }
		void fSetDroppingEnabled( b32 enabled ) { mDroppingEnabled = enabled; }

		void fCancelCargoDrop( );
		void fReleaseTrenchedCargo( );
		b32  fHasEverDroppedCargo( ) const { return mHasEverDroppedCargo; }
		b32  fIsDroppingCargo( ) const { return mDroppingCargo.mCount > 0; }
		tVehicleCargo* fCurrentCargo( ) { return &mDroppingCargo; }
		void fStopToDropCargo( );
		void fResumeFromDropCargo( );
		virtual b32 fPathPointReached( tPathEntity& point, const Logic::tEventContextPtr& context );

		b32 fShowStats( ) const;

		virtual void fAddCargoDropSmokePtr( tRefCounterPtr<tSmokeDestroyer>& ptr );

		static f32 fAudioRPMBlend( );
		static f32 fAudioDriveBlend( );

		// Extra Stuff

		u32 fFindEmptySeat( const Math::tVec3f& myPos ) const;
		b32 fTryToUse( tPlayer* player, u32 seat );
		void fSwitchSeats( u32 fromSeat, u32 toSeat );
		u32 fSeatIndex( const tPlayer* player ) const;
		/*private*/ tVehicleSeat& fValidateSeatArray( tEntity* newData ); //check for seat index enum and ensure array size before returning seat element

		Physics::tOneWayHinge* fDoorHingeForScript( u32 index ) const;

		void fProcessMovementFx( f32 dt );

		// End ExtraStuff

	public: // called from goals
		virtual b32 fCanBeUsed( );
		virtual b32 fTryToUse( tPlayer* player ); // returns true if can use
		virtual b32 fEndUse( tPlayer& player ); //return true if totally vacated
		void fVacateAllPlayers( );
		void fEjectAllPassengers( const Math::tVec3f& spawnVel );
		virtual void fComputeUserInput( f32 dt );
		virtual void fComputeAIInput( f32 dt );
		void fCommonInput( );
		b32  fBoostInput( b32 skipBoostFx = false );
		virtual void fReactToWeaponFire( const tFireEvent& event ) { }
		virtual void fStartup( b32 startup );
		virtual void fShutDown( b32 shutDown );
		void fHandleWeaponAction( u32 weaponAction, u32 weaponIndex );
		void fSetWeaponTargetIndex( u32 weaponIndex, u32 targetIndex );

		virtual void fResetPhysics( const Math::tMat3f& xform ) { }
		virtual const Math::tMat3f& fCurrentTransformMT( ) const { sigassert( 0 && "Implement this fCurrentTransformMT( )!" ); return Math::tMat3f::cIdentity; }
		virtual const Math::tVec3f& fCurrentVelocityMT( ) const { sigassert( 0 && "Implement this fCurrentVelocityMT( )!" ); return Math::tVec3f::cZeroVector; }
		virtual Math::tVec3f fPointVelocityMT( const Math::tVec3f& worldPoint ) const { sigassert( 0 && "Implement this fPointVelocityMT( )!" ); return Math::tVec3f::cZeroVector; }

		void fSetVehicleController( tVehicleLogic* logic ) { mVehicleControllerLogic = logic; mVehicleController.fReset( logic ? logic->fOwnerEntity( ) : NULL ); }
		
		f32  fBoostProgress( ) const;

		void fSetSlaveLinkTurrentChildren( b32 link )	{ mSlaveLinkTurrentChildren = link; }
		b32  fSlaveLinkTurrentChildren( ) const			{ return mSlaveLinkTurrentChildren; }

		b32  fHasWaitTimer( ) const { return mWaitTimer.fGetRawPtr( ) != NULL; }
		void fSetDontRespawn( b32 dont ) { mDontRespawn = dont; }
		void fSetDoCollisionTest( b32 doit ) { mDoCollisionTest = doit; }

		void fSetPurchasedBy( tPlayer* player ) { mPurchasedBy.fReset( player ); }

		virtual f32 fGroundHeight( ) const { return 0.f; }

		virtual tTurretCameraMovement* fRequestCameraMovement( ) { return NULL; }

		void fHideShowBToExit( b32 show );

		virtual void fPushCamera( tPlayer* player, u32 seat ) { }
		virtual void fPopCamera( tPlayer* player ) { }

		void fSetExitedTheMap( b32 etm ) { mExitedTheMap = etm; }
		b32  fExitedTheMap( ) const { return mExitedTheMap; }

		void fSetDontInstaDestroyOFB( b32 dont ) { mDontInstaDestroyOutsideOfFlyBox = dont; }
		b32  fDontInstaDestroyOFB( ) const { return mDontInstaDestroyOutsideOfFlyBox; }
	
	public:
		// This is currently expected not to be time scaled
		virtual void fActST( f32 dt );

		// These are all expected to be time scaled by the derived types before being called
		virtual void fMoveST( f32 dt );
		virtual void fCollideMT( f32 dt );
		virtual void fPhysicsMT( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		Logic::tAnimatable			mAnimatable;
		tUnitPathPtr				mUnitPath;
		Math::tMat3f				mInitialTransform;
		tDynamicArray<tEntityPtr>	mPassengers;
		tVehiclePassengerLogic* fGetPassengerLogic( u32 i );
		void fApplyMotionStateToArtillerySoldiers( const char* motionState );
		void fApplyMotionStateToTurrets( const char* motionState );

		tGrowableArray<tVehicleCargo>	mCargo;
		f32								mDropTimer;
		tVehicleCargo					mDroppingCargo; //The actively dropping cargo.
		tGrowableArray<tGrowableArray<tEntityPtr>>	mCargoDropPts; //[cargo index][point index]
		tGrowableArray<tVehicleSeat>	mSeats;
		void fCheckAndSetSelectionEnable( );

		void fAddCargoPoint( tEntity* e );
		tGrowableArray<tEntityPtr>* fCargoPoints( u32 index );
		void fDestroyBreakablesWeHit( );
		tUnitLogic* fDropCargoItem( const tEntityPtr& dropPtr );
		b32 fGiveAGoalPathByUnitType( tUnitLogic* unit, Math::tVec3f searchPt );

		//used for collision detection and avoidance
		void fFindCollisionMT( f32 dt );
		Math::tAabbf	mCollisionBounds;     //narrowphase
		Math::tSpheref   mDetectionPrimitive;  //broadphase
		tGrowableArray<tEntityPtr> mIgnoreCollisionFrom;
		tGrowableArray<tVehicleOffender> mOffenders; //for avoidance
		tGrowableArray<tCharacterLogic*> mPeepsWeHit;
		tGrowableArray<tUnitLogic*> mBreakablesWeHit;
		tGrowableArray<tEntityPtr> mAdditionalBreakablePtrs; //these pointers keep the above logics alive if they arent in the main cached query
		f32 mSpeedWeHit; //the speed we had when these arrays were populated
		f32 mMass; //our mass

		f32 mThrottleScale; //set by waypoints or script.
		f32 mThrottleScaleTarget;
		f32 mThrottleScaleTargetOverride; //so internally we can temporarily override the target without destroying the existing one, set to 0-1 to be valid
		f32 mThrottleBlend; //so the scale doesnt change immediately

		Math::tVec4f    mColor;
		b8 mFallingThrough;
		b8 mDeleteAfterFallThrough;
		b8 mDeleteMyEntity;
		b8 mExitedTheMap;

		b8 mEjectUser;
		b8 mStartingUp;
		b8 mDoCollisionTest;
		b8 mOversizeCollisionTest;

		b8 mDroppingEnabled; // cargo stuff
		b8 mWaitingToLaunchCargo;
		b8 mFull3DPhysics;
		b8 mAICheckForCollisions;

		b8 mLockedToPathStart;
		b8 mHasDoneSpecialEntrance;
		b8 mDontInstaDestroyOutsideOfFlyBox; //set this for planes because they will crash themselves
		b8 mSlaveLinkTurrentChildren;

		b8 mDoMovementFX;
		b8 mDontRespawn;
		b8 mHasEverDroppedCargo;
		b8 mConstrainYaw;

		f32 mPowerLevel;
		f32 mBoostTimer;

		tStringPtr mBoostEffect;
		FX::tFxSystemsArray mBoostEffects;

		tFixedArray< FX::tFxSystemsArray, GameFlags::cVEHICLE_MOTION_FX_COUNT > mMovementEffects;

		virtual void fStopSpecialMove( ) { }
		virtual void fStartSpecialMove( u32 move ) { }

		b32  fLockedToPathStart( ) const { return mLockedToPathStart; }
		void fSetLockedToPathStart( b32 locked );
		b32  fHasDoneSpecialEntrance( ) const { return mHasDoneSpecialEntrance; }
		void fSetHasDoneSpecialEntrance( b32 hasDone ) { mHasDoneSpecialEntrance = hasDone; }

		//weapon stuff

		struct tTurretData
		{
			tTurretData( )
				: mTargetYaw( 0.f )
				, mBlendBlend( 0.f )
				, mLastSetYaw( 0.f )
			{ }

			tEntityPtr mEntity;
			tGrowableArray<tWeaponPtr> mWeapons;
			Math::tMat3f mOriginalTurretTransform;
			Math::tVelocityDamped mTurretYawBlend;
			f32 mBlendBlend; //0.f is all blend controller, 1.f is locked on, for user mode.
			f32 mTargetYaw;
			f32 mLastSetYaw;

			b32 operator == ( const tEntity* ent ) const { return mEntity == ent; }
		};

		tGrowableArray<tTurretData> mTurrets;
		void fResetTurretBlendData( );

		virtual void fRespawn( const Math::tMat3f& tm ) { }
		virtual void fSetupVehicle( ) { }
		void fInitPassengers( );
		void fInitWeapon( u32 index, const tStringPtr& attachementName );
		void fUpdateCharactersMT( f32 dt );
		void fShowWaitTimer( );
		virtual void fReapplyTable( ) { }

		virtual void fClearContacts( ) { }
		virtual void fAddContactPt( f32 dt, const Physics::tContactPoint& cp ) { }
		virtual Math::tVec3f fComputeContactResponse( tUnitLogic* theirUnit, const Physics::tContactPoint& cp, f32 mass, b32& ignore ) { return Math::tVec3f::cZeroVector; }
		virtual void fAddCollisionResponse( const Math::tVec3f& response, f32 mass ) { }
		void fSetAICheckForCollisions( b32 check ) { mAICheckForCollisions = check; }
		b32  fAICheckForCollisions( ) const { return mAICheckForCollisions; }
		void fUpdateYawConstraint( b32 fullUpdate );

		Gui::tHoverTimerPtr mExpireTimer; //counts down when the player exits
		Gui::tHoverTimerPtr mWaitTimer; //counts down when the player exits

		Audio::tSourcePtr mExhaustSource;

		// Some one can own us, such as a tank owning a plane to fly
		tEntityPtr		mVehicleController;
		tVehicleLogic*	mVehicleControllerLogic; //Tank
		tVehicleLogic*	mSlaveVehicleLogic;		// Plane

		tGrowableArray<tRefCounterPtr<tSmokeDestroyer>> mCargoSmokes;
		tPersistentEffectPtr mPersistentEffect;

		tGrowableArray<tEntityPtr> mExplicitTargets;
		void fAddExplicitTarget( tEntity* entity ) { mExplicitTargets.fPushBack( tEntityPtr( entity ) ); }

		tPlayerPtr mPurchasedBy;
		tEntityPtr mEnteredEnemyGoalBox;
	};

}

#endif//__tVehicleLogic__
