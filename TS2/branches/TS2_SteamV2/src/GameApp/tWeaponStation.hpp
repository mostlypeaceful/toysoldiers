#ifndef __tWeaponStation__
#define __tWeaponStation__

#include "tWeapon.hpp"
#include "tWeaponUI.hpp"
#include "tGamePostEffectMgr.hpp"

namespace Sig
{
	class tWeaponStation;
	class tDataTable;

	namespace Input
	{
		class tRumbleManager;
	}

	// Global singleton desc cacher.
	class tWeaponDescs
	{
		declare_singleton( tWeaponDescs )

		void fSetup( );
		const tWeaponDesc& fDesc( const tStringPtr& weaponID );

	private:
		void fLoadDesc( u32 r );

		tGrowableArray<tWeaponDesc> mDescs;
	};

	class tWeaponBank : public tRefCounter
	{
	public:
		tWeaponBank( tUnitLogic* unitLogic = NULL, tWeaponStation* station = NULL, u32 id = 0 );
		virtual ~tWeaponBank( ) { }

		void fOnSpawn( );
		void fOnDelete( );

		void fSetUnitLogic( tUnitLogic* unitLogic ) { mUnitLogic = unitLogic; }
		tUnitLogic* fUnitLogic( ) const { return mUnitLogic; }

		tWeapon* fAddWeapon( const tStringPtr& weaponName, const tStringPtr& parentEntity  );
		const tWeaponPtr& fWeapon( u32 index ) const;
		tWeaponPtr& fWeapon( u32 index );
		tWeapon* fWeaponRawPtr( u32 index );

		u32 fWeaponCount( ) const { return mWeapons.fCount( ); }
		b32 fHasWeapon( u32 index ) const { return index < fWeaponCount( ); }

		void fBeginUserControl( tPlayer* player );
		void fEndUserControl( );
		void fProcessUserInput( );

		// targetting
		void fProcessST( f32 dt );
		void fProcessMT( f32 dt );
		void fSetAcquireTargets( b32 enable );

		void fSetAIFireOverride( b32 fire );
		b32  fAIFireOverride( ) const;
		void fSetReloadOverride( b32 override );

		// returns true if continuous
		b32 fFire( );
		void fEndFire( );
		void fReload( );
		void fReloadAfterTimer( );
		void fAccumulateFireEvents( tFireEventList& list );
		void fPitchTowardsDesiredAngle( f32 dt );
		void fSpinUp( b32 up );

		b32 fFiring( ) const;
		b32 fCanFire( ) const;
		b32 fShouldFire( ) const;
		b32 fNeedsReload( ) const;
		b32 fIsContinuousFire( ) const;
		b32 fShellCaming( ) const;
		b32 fShouldAcquire( ) const;
		b32 fHasTarget( ) const;
		b32 fReloading( ) const;

		template< class T >
		void fSetParentVelocityMT( T* logic )
		{
			for( u32 i = 0; i < mWeapons.fCount( ); ++i )
				mWeapons[ i ]->fSetParentVelocity( logic->fPointVelocityMT( mWeapons[ i ]->fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( ) ) );
		}
		
		u32  fFireMode( ) const { return mFireMode; }
		void fSetFireMode( u32 mode ) { mFireMode = mode; }
		u32  fTriggerButton( ) const { return mTriggerButton; }
		void fSetTriggerButton( u32 button ) { mTriggerButton = button; }

		void fCacheWeaponData( );
		b8   fWantsLocks( ) const { return mMaxLocks > 0; }
		b32  fAcquireLocks( ) const { return mAcquiringLocks && mLocks.fCount( ) < mMaxLocks && !fReloading( ); }
		b32  fInterestedInLock( GameFlags::tUNIT_TYPE type ) const;
		void fAddLock( const tWeaponTargetPtr& target ) { mLocks.fPushBack( target ); }
		void fRemoveLock( const tWeaponTargetPtr& target ) { mLocks.fFindAndEraseOrdered( target ); }
		void fClearLocks( ) { mLocks.fSetCount( 0 ); }
		b32 fAcquiringLocks( ) const { return mAcquiringLocks; }
		b32 fAnimationDriven( ) const { return mAnimationDriven; }

		void fEnable( b32 enable );
		b32  fEnabled( ) const { return mEnabled; }

		const Audio::tSourcePtr& fBankAudio( ) const { return mBankAudio; }
		void fHandleBankAudioEvent( u32 event );

	private:
		tWeaponPtr fCreateWeapon( const tStringPtr& weaponName, const tStringPtr& parentEntity );
		b32 fReallyFire( const tWeaponTarget* target = NULL );
		
		tGrowableArray< tWeaponPtr > mWeapons;
		tUnitLogic*	mUnitLogic;
		tWeaponStation* mStation;

		u32 mFireMode;
		u32 mNextFireIndex;
		u32 mTriggerButton;

		f32 mFireRateTimer;
		f32 mFireRate;  //this is the fire rank at the bank level

		b8 mAcquiringLocks;
		b8 mShootIfNoLock;
		b8 mEnabled;
		b8 mAnimationDriven;

		u8 mBankID;
		u8 pad0;
		u8 pad1;
		u8 pad2;

		u32 mLastAudioEvent;

		tGrowableArray<GameFlags::tUNIT_TYPE> mLockTypes;
		tGrowableArray<tWeaponTargetPtr> mLocks;
		u32 mMaxLocks;

		Audio::tSourcePtr mBankAudio;
	};
	typedef tRefCounterPtr< tWeaponBank > tWeaponBankPtr;
	
	class tWeaponStation : public tRefCounter
	{
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tWeaponStation( tUnitLogic* unitLogic = NULL );
		virtual ~tWeaponStation( ) { }

		void fOnSpawn( );
		void fOnDelete( );

		tUnitLogic* fUnitLogic( ) const { return mUnitLogic; }

		void fAddBank( );
		const tWeaponBankPtr& fBank( u32 index ) const;
		tWeaponBankPtr& fBank( u32 index );
		tWeaponBank* fBankRawPtr( u32 index );

		u32 fBankCount( ) const { return mBanks.fCount( ); }
		b32 fHasBank( u32 index ) const { return index < fBankCount( ); }
		void fCheckBankSize( u32 index );
		void fCacheBankData( );

		const tUserPtr& fUser( ) const;
		tPlayer*		fPlayer( ) const;
		void fBeginUserControl( tPlayer* player );
		void fEndUserControl( );
		void fBeginRendering( ); // TODO: Maybe take a user for rendering when not under user control
		void fEndRendering( );
		void fProcessUserInput( );

		void fCreateUI( );
		void fPositionUI( );
		Sqrat::Object fGetUIScript( ) { return mWeaponUI->fCanvas( ).fScriptObject( ); }
		void fShowHideReticle( b32 show );
		void fSetScopeBlend( f32 blend ); //0.f no scope, 1.f fully blended into scope
		void fAdjustReticle( const Input::tGamepad *control = NULL );
		Math::tRayf fRayThroughRetical( const Math::tVec3f& muzzlePt ) const;
		b32 fComputeRealTargetThroughRetical( const Math::tVec3f& muzzlePt, Math::tVec3f& worldPosOut, tEntityPtr* entOut = NULL );
		void fClearBankLocks( u32 bankIndex );

		// targetting
		void fProcessST( f32 dt );
		void fProcessMT( f32 dt );
		void fSetAcquireTargets( b32 enable );
		b32 fProcessAdvancedTargetting( );

		void fSetAIFireOverride( b32 fire );

		// returns true if continuous
		b32 fFire( );
		void fEndFire( );
		void fReload( );
		void fReloadAfterTimer( );
		void fAccumulateFireEvents( tFireEventList& list );
		void fPitchTowardsDesiredAngle( f32 dt );
		void fSpinUp( b32 up );
		void fEnable( b32 enable );

		b32 fFiring( ) const;
		b32 fCanFire( ) const;
		b32 fShouldFire( ) const;
		b32 fNeedsReload( ) const;
		b32 fIsContinuousFire( ) const;
		b32 fShellCaming( ) const;
		f32 fMaxRange( ) const { return mMaxRange; }
		f32 fMinRange( ) const { return mMinRange; }
		b32 fWantsAdvanceTargeting( ) const { return mWantsLocks || mStickyReticle; }
		b32 fShouldAcquire( ) const;
		b32 fHasTarget( ) const;
		b32 fReloading( ) const;

		b32 fUnderUserControl( ) const { return (mPlayer != NULL); }

		template< class T >
		void fSetParentVelocityMT( T* logic )
		{
			for( u32 i = 0; i < mBanks.fCount( ); ++i )
				mBanks[ i ]->fSetParentVelocityMT( logic );
		}

		static void fApplyRumbleEvent( const tFireEvent& fireEvent, const tPlayer& player );

		const Gui::tWeaponUIPtr& fUI( ) const { return mWeaponUI; }

		f32 fGetGroundHeight( f32 headingAngle, f32 distZeroTo1 ) const;
		void fSetGroundSamples( f32 start, f32 range, tGrowableArray<f32>& samplesNear, tGrowableArray<f32>& samplesFar );

	private:
		tGrowableArray< tWeaponBankPtr > mBanks;
		
		tPlayer*			mPlayer;
		tUnitLogic*			mUnitLogic;

		Gui::tWeaponUIPtr	mWeaponUI;
		Math::tVec2f		mUIPosition;

		tGrowableArray<GameFlags::tUNIT_TYPE> mTargetTypes;
		tGrowableArray<tWeaponTargetPtr> mTargets;
		tWeaponTargetPtr mCurrentTarget;
		void fAimLost( const tWeaponTargetPtr& target );
		void fTargetLost( const tWeaponTargetPtr& target );
		void fLockMade( const tWeaponTargetPtr& target, u32 bankIndex );
		void fRemoveLock( const tWeaponTargetPtr& target );
		b32 fHasTarget( const tEntity* e ) const;
		b32 fAnyBankWantsLock( u32 unitType ) const;

		//Cached weapon stats
		f32 mMaxRange;
		f32 mMinRange;
		b8 mWantsLocks;
		b8 mStickyReticle;
		b8 mFirstUpdate;
		b8 pad0;

		u32 mToggleCameraButton;
		f32 mLastRaycastDistance;

		// cached ground samples from rts display
		f32 mGroundHeightStartAngle;
		f32 mGroundHeightAngleRange;
		tGrowableArray<f32> mGroundHeightsNear;
		tGrowableArray<f32> mGroundHeightsFar;
	};
	typedef tRefCounterPtr< tWeaponStation > tWeaponStationPtr;
}

#endif //__tWeaponStation__
