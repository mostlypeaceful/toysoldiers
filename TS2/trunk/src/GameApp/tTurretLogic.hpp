#ifndef __tTurretLogic__
#define __tTurretLogic__
#include "tUnitLogic.hpp"
#include "Logic/tAnimatable.hpp"
#include "tTurretCameraMovement.hpp"
#include "tTurretUpgradeIndicator.hpp"
#include "tInUseIndicator.hpp"
#include "tUseTurretCamera.hpp"

namespace Sig
{

	class tUseTurretCamera;
	class tVehicleLogic;

	class tTurretLogic 
		: public tUnitLogic
		, public Logic::tAnimatable
	{
		define_dynamic_cast( tTurretLogic, tUnitLogic );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tTurretLogic( );
		virtual ~tTurretLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );
		virtual void fOnSkeletonPropagated( );

		virtual b32 fShouldSelect( ) const { return !mRepairing && !mUpgrading && fSelectionEnabled( ); }

		struct tPlayerData
		{
			tPlayer* mPlayer;
			tUseTurretCameraPtr mCamera;

			tPlayerData( tPlayer* p = NULL, tUseTurretCameraPtr& cam = tUseTurretCameraPtr( ) )
				: mPlayer( p ), mCamera( cam )
			{ }

			b32 operator == ( const tPlayerData& right ) const { return mPlayer == right.mPlayer; }
		};

	public: // query specific components
		virtual Logic::tAnimatable* fQueryAnimatable( ) { return this; }

	protected:
		virtual void fActST( f32 dt );
		virtual void fAnimateMT( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );
	public:
		virtual Gui::tRadialMenuPtr fCreateSelectionRadialMenu( tPlayer& player );
		void fApplyMotionStateToArtillerySoldiers( const char* motionState );
	public:
		Math::tVec3f fIdleTargetUserDir( );
		Math::tVec3f fDesiredFacingUserDirection( );
		Math::tVec3f fConstrainFacingDirection( const Math::tVec3f& dir ) const;
		void fSetUserDirection( const Math::tVec3f& dir ) { mUserDirection = dir; mUserDirectionLerp = 1.f; }
		f32 fConstraintStartAngle( ) const { return fUserToWorld( mConstraintAxis ).fXZHeading( ) - mConstraintAngle; }
		f32 fConstraintRange( ) const { return mConstraintAngle * 2.f; }
		b32 fDisableYawConstraintAdjust( ) const { return mDisableYawConstraintAdjust; }
		void fSetDisableYawConstraintAdjust( b32 disable ) { mDisableYawConstraintAdjust = disable; }

		void fSetupMotionBase( );
		void fSetupMotionBase( const Math::tMat3f& motionParentRelative );

		b32 fQuickSwitchCamera( ) const { return mQuickSwitchCamera; }
		void fSetQuickSwitchCamera( b32 quick ) { mQuickSwitchCamera = quick; }

		void fAddUpgradeAndRepairIndicators( );
		void fSetUpgradeProgress( f32 percent );
		void fStartRepair( );
		void fSetRepairProgress( f32 percent );

		void fAddEnemyTurrentIndicator( );

		void fResetCamera( );
		void fApplyUserControl( f32 dt );
		void fApplyAIControl( f32 dt );
		virtual void fIncrementYawConstraint( s32 dir, b32 onSpawn, tPlayer* player );

		b32 fUpgradeMaxed( tPlayer* player ) const;
		b32 fUpgradeLocked( tPlayer* player ) const;
		b32 fCanUpgrade( tPlayer* player ) const { return !fUpgradeMaxed( player ) && !fUpgradeLocked( player ) && !mUpgrading && !mRepairing; }
		b32 fShouldRepair( ) const;
		b32 fCanRepair( ) const;
		f32 fPitchBlendValue( ) const; // 0 is the low pitch and 1 is the high pitch
		f32 fSpeedBlendValue( ) const;

		void fSetSellingDisabled( b32 disable ) { mDisableSelling = disable; }
		b32  fSellingDisabled( ) const { return mDisableSelling; }

		f32 fPowerUpTimeScale( ) const;
		
		Math::tVec2f fTurretUseCamRotSpeed( ) const;
		Math::tVec2f fTurretUseCamDamping( ) const;

		void fStepRandomEvent( u32 context, f32 dt );

		b32 fOnVehicle( ) const { return mOnVehicle; }
		tVehicleLogic* fOnVehicleLogic( ) const { return mOnVehicleLogic; }

		void fFindAppropriateBuildSite( );

		b32  fDontAlign( ) { b32 val = mDontAlignOnBlendIn; mDontAlignOnBlendIn = false; return val; }
		void fSetDontAlign( b32 dont ) { mDontAlignOnBlendIn = true; }

		const tGrowableArray<tPlayerData>& fPlayers( ) const { return mPlayers; }
		tPlayer* fControllingPlayer( );
		tPlayerData* fControllingPlayerData( );
		const tPlayer* fControllingPlayer( ) const;
		const tPlayerData* fControllingPlayerData( ) const;

		void fSetPurchasedBy( tPlayer* player ) { mPurchasedBy.fReset( player ); }

	private:
		tShapeEntityPtr fFindAppropriateBuildSite( const tGrowableArray<tShapeEntityPtr>& buildSites, f32& closestDistance ) const;
		
	public: // TODO: do these need to stay private?
		virtual tRefCounterPtr<tEntitySaveData> fStoreSaveGameData( b32 entityIsPartOfLevelFile, tSaveGameRewindPreview& preview );
		virtual b32 fTryToUse( tPlayer* player ); // returns true if can use
		virtual b32 fEndUse( tPlayer& player );
		void fEjectAllPlayers( );
		b32 fTryToSell( tPlayer* player ); // returns true if sold
		b32 fTryToRepair( tPlayer* player ); // returns true if can repair
		b32 fTryToUpgrade( tPlayer* player ); // returns true if can upgrade
		void fDoUpgrade( u32 nextID );
		void fUpgradeFinished( );
		void fRepairFinished( );
		void fYoureFlipped( tTurretLogic* logic );
		const char* fUpgradeUnitID( );
		u32 fUpgradeCost( );
		void fQuickSwitchTo( u32 unitID ); //will flip the turret over to the new turret with the player still inside
		b32 fFlipping( ) const { return mTurretFlipping; }

		Math::tVec3f fUserToWorld( const Math::tVec3f& userVec ) const { sigassert( mMotionParent ); return (mMotionParent->fObjectToWorld( ) * mMotionParentRelative).fXformVector( userVec ); }
		Math::tVec3f fWorldToUser( const Math::tVec3f& worldVec ) const { sigassert( mMotionParent ); return (mMotionParent->fObjectToWorld( ) * mMotionParentRelative).fInverseXformVector( worldVec ); }
		Math::tMat3f fUnflippedXform( ) const { sigassert( mMotionParent ); return (mMotionParent->fObjectToWorld( ) * mMotionParentRelative); }
		u32 fUnitIDConsideringUpgrade( ) const { return mUpgrading ? mUpgradeID : mUnitID; } //this is for saving, give them the upgraded turret when rewinding

		virtual void fShowInUseIndicator( Gui::tInUseIndicator* indicator  );
		b32 fShouldPushAimGoal( ) const;

		void fSetSlaveParent( tTurretLogic* parent ) { mSlaveParent = parent; mSlaveParentEnt.fReset( parent ? parent->fOwnerEntity( ) : NULL ); }
		
		tEntity* fIdleTarget( ) const { return mIdleTarget.fGetRawPtr( ); }
		void     fSetIdleTarget( tEntity* target ) { mIdleTarget.fReset( target ); }

		void fSetDeleteCallback( const Sqrat::Function& func ) { mDeleteScriptCallback = func; }
		Sqrat::Function fDeleteCallback( ) const { return mDeleteScriptCallback; }

	private:
		tGrowableArray<tEntityPtr> mArtillerySoldiers;
		tGrowableArray<tEntityPtr> mBaseObjects; //the wood platform base.
		tEntityPtr mMotionParent; //level or vehicle
		Math::tMat3f mMotionParentRelative;

		tTurretLogic* mSlaveParent;
		tEntityPtr mSlaveParentEnt;

		tTurretCameraMovement	mCameraMovement;
		Math::tVec3f			mUserDirection; // turret direction is buffered so it can be lerped to a new location
		f32						mUserDirectionLerp; //if this is 1.0f, mUserDirection is directly applied to turret
		tGrowableArray<tPlayerData> mPlayers;
		
		u32 mUpgradeID;
		b8 mUpgrading;
		b8 mRepairing;
		b8 mFlipTurret;     //true if turret should be upside down, false if not
		b8 mTurretFlipping; //true if the turret is anything but rightside up

		b8 mOnVehicle;
		b8 mDisableYawConstraintAdjust;
		b8 mQuickSwitchCamera;
		b8 mDisableSelling;

		b8 mMotionBaseInitialized;
		b8 mDontAlignOnBlendIn;
		b8 mQuickSwitchFlipIn;
		b8 pad0; 

		tEntityPtr mIdleTarget;

		tVehicleLogic* mOnVehicleLogic;
		Gui::tTurretRadialIndicatorPtr mUpgradeIndicator;
		Gui::tTurretRadialIndicatorPtr mRepairIndicator;
		f32 mRepairStartPercent;
		Gui::tInUseIndicatorPtr mEnemyTurretIndicator;

		Math::tDampedFloat mFlipValue; //0 to 1
		tEntityPtr mUpgradeClock;

		void fConfigureAudio( );
		void fPlayerSpecificAudioEvent( tPlayer* player, u32 event );

		void fBeginEndControl( b32 begin );

		f32 mTimeTillNextRandomAnim;
		tGrowableArray<tPlayerData> mAcquireThisPlayerOnSpawn;
		tTurretLogic* mPreviousTurretForQuickSwitch;

		tPlayerPtr mPurchasedBy;

		void fUpdateWorldLookDir( );
		Math::tVec3f mWorldLookDir; //for turrets on motion bases.

		Sqrat::Function mDeleteScriptCallback;
	};

}

#endif//__tTurretLogic__
