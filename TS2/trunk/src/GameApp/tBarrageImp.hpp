#ifndef __tBarrageImp__
#define __tBarrageImp__

#include "tBarrage.hpp"
#include "tWaveList.hpp"
#include "gfx/tCameraController.hpp"
#include "tShellCamera.hpp"

namespace Sig
{
	class tRtsCamera;
	class tUnitLogic;

	namespace tBarrageImp
	{
		void fExportScriptInterface( tScriptVm& vm );
	}

	class tArtilleryBarrage : public tBarrage
	{
	public:
		tArtilleryBarrage( );

		tStringPtr		mEffectID;
		tStringPtr		mWeaponID;
		tStringPtr		mAudioEventUse;
		u32				mNumberOfExplosions;
		f32				mDelayMin;
		f32				mDelayMax;
		b32				mSpawnStraightOver;

		virtual void fBegin( tPlayer* player );
		virtual void fReset( tPlayer* player );
		virtual f32 fProcessST( tPlayer* player, f32 dt );
		virtual b32 fBarrageUsable( ) const;

		void fSetSpawnPtName( const tStringPtr& name );

		void fTargetBegin( tPlayer* player ); //target has been set

		void fSetTargetPt( const Math::tMat3f& xform )
		{
			mTargetSet = true;
			mTarget = xform.fGetTranslation( );
		}

		void fPushPopCamera( tPlayer* player, b32 push );
		void fEnableProximities( b32 enable, tPlayer* player );


	private:
		f32 mTimer;
		u32 mExplosionsLeft;
		b32 mStarted;

		// Target overriding
		b32 mTargetSet;
		Math::tVec3f mTarget;

		tEntityPtr mSpawnPt;

		tEntityPtr			mLastProjectile;
		tProjectileLogic*	mLastProjLogic;

		tEntityPtr mWasInUnit;
		tUnitLogic* mWasInUnitLogic;

		tShellCamera* mShellCam;
		Gfx::tCameraControllerPtr mShellCamPtr;
		

		void fSpawnExplosion( tPlayer* player );
	};


	class tUserControllableCharacterLogic;

	class tRamboBarrage : public tBarrage
	{
	public:
		tRamboBarrage( );

		tStringPtr		mDropPtName;
		tFilePathPtr	mCharacterPath;
		f32				mDuration;
		tStringPtr		mAudioEventReady;
		tStringPtr		mAudioEventFirstUse;

		virtual void fForceUse( tPlayer* player );
		virtual void fSetTarget( tPlayer* player, tEntity* target );
		virtual void fSkipInto( tPlayer* player );
		virtual void fBegin( tPlayer* player );
		virtual void fReset( tPlayer* player );
		virtual f32 fProcessST( tPlayer* player, f32 dt );
		virtual b32 fBarrageUsable( ) const;

		virtual b32 fEnteredBarrageUnit( tUnitLogic* logic, b32 enter );

	private:
		f32					mTimer;
		tEntityPtr			mCharacterEnt;
		tUserControllableCharacterLogic*	mCharacter;
		tPlayer*			mPlayer;
		tEntityPtr			mSkipToTarget;

		b32 mCanUseLastFrame;
		b32 mTimerStarted;
		b32 mEverCouldUse;
		b32 mForceInto;
		b32 mLockIn;
		b32 mAchievementAwarded;

		void fFirstUse( tPlayer* player );
	};


	class tWaveLaunchBarrage : public tBarrage
	{
	public:
		tWaveLaunchBarrage( );

		tStringPtr		mWaveListName; //from common wave lists
		f32				mDuration;
		b32				mSelectableUnits;
		tStringPtr		mAudioEventLaunched;

		virtual void fBegin( tPlayer* player );
		virtual f32 fProcessST( tPlayer* player, f32 dt );
		virtual b32 fBarrageUsable( ) const { return false; }

		void fSetTargetPt( const Math::tMat3f& pt );
		void fSetPathSigml( tEntity* ent );

	protected:
		f32				mTimer;
		tWaveListPtr	mWaveList;
		tEntityPtr		mPathSigml;
	};


	class tUsableWaveLaunchBarrage : public tWaveLaunchBarrage
	{
	public:
		tUsableWaveLaunchBarrage( );

		virtual void fForceUse( tPlayer* player );

		virtual void fBegin( tPlayer* player );
		virtual void fReset( tPlayer* player );
		virtual f32 fProcessST( tPlayer* player, f32 dt );
		virtual b32 fBarrageUsable( ) const;
		void fAddUsableName( const tStringPtr& name ) { mNames.fPushBack( name ); mSelectableUnits = true; }

		virtual b32 fEnteredBarrageUnit( tUnitLogic* logic, b32 enter, tPlayer* player );
		virtual b32 fUsedBarrageUnit( tUnitLogic* logic, tEntity* ent, tPlayer* player );

		tStringPtr mAudioEventUse;
		tStringPtr mAudioEventFire;

		void fTargetBegin( tPlayer* player ); //target has been set

	private:
		tGrowableArray<tStringPtr> mNames;
		tGrowableArray<tEntityPtr> mUnits; //cooresponding to the names;
		tGrowableArray<tUnitLogic*> mLogics; //cooresponding to the names;

		tPlayer*   mPlayer;
		mutable tUnitLogic* mCurrentUsingLogic;
		mutable tStringPtr mLastUsedName;
		mutable b8 mUsable;
		b8 mFoundUsable;
		u8 pad3;
		b8 pad2;

		b8 mForceInto;
		b8 mLockIn;
		b8 pad0;
		b8 pad1;

		b32 fPlayerInNamedUnit( ) const;
		void fLookForLogics( );
	};



}

#endif//__tBarrageImp__
