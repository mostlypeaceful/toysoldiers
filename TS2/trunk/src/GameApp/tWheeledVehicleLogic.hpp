#ifndef __tWheeledVehicleLogic__
#define __tWheeledVehicleLogic__
#include "tVehicleLogic.hpp"
#include "Physics/tVehiclePhysics.hpp"
#include "tVehiclePathFollowing.hpp"
#include "tTurretCameraMovement.hpp"
#include "Physics/tPinConstraint.hpp"
#include "tFxFileRefEntity.hpp"

namespace Sig
{
	class tTankPalette;
	class tAirborneLogic;

	// Very simple geared transmission simulator.
	class tTransmission
	{
	public:
		tTransmission( );

		struct tResults
		{
			b8 mUpShift;
			b8 mDownShift;
			b8 mChangedDirections;
			b8 pad1;

			b32 fGearChanged( ) const { return mUpShift || mDownShift || mChangedDirections; }
		};

		void fSetup( u32 gears, f32 transMaxSpeed );
		void fSetupAdvanced( f32 idle, f32 upShiftDelay, f32 downShiftDelay, f32 downShiftPoint );
		void fStep( f32 dt, f32 speed, tResults& output );

		f32 fRPMPercentage( ) const { return mRPM; }
		f32 fClutch( ) const { return mClutch * mGears[ mGear ].mTorque; }
		u32 fGear( ) const { return mGear; }
		b32 fReverse( ) const { return mReversing; }

		void fSetShiftHold( b32 hold ) { mShiftHold = hold; }

	private:
		struct tGear
		{
			tGear( f32 ratio = 1.f, f32 torque = 1.f ) : mRatio( ratio ), mTorque( torque ) { }

			f32 mRatio;
			f32 mTorque;
		};

		f32 fComputeRatio( f32 speed ) { return speed; }
		f32 fRatio( ) const { return mGears[ mGear ].mRatio; }
		f32 fTopSpeed( ) const { return mGears[ mGear ].mRatio; }
		f32 fMinSpeed( ) const { return mGears[ mGear > 0 ? mGear-1 : mGear ].mRatio * mDownShiftPoint; }
		b32 fTopGear( ) const { return mGear == mGears.fCount( ) - 1; }
		b32 fBottomGear( ) const { return mGear == 0; }
		void fUpShift( ) { ++mGear; mShiftDelay = mUpShiftDelayTime; }
		void fDownShift( ) { --mGear; mShiftDelay = mDownShiftDelayTime; }
		f32 fShiftDelayPercentage( ) { return mShiftDelay / mUpShiftDelayTime; }

		// config
		tGrowableArray<tGear> mGears;
		f32 mIdle;
		f32 mUpShiftDelayTime;
		f32 mDownShiftDelayTime;
		f32 mShiftDelay;
		f32 mDownShiftPoint;

		// output :)
		f32 mRPM;
		f32 mClutch;
		u32 mGear;

		b8 mRedLining;
		b8 mReversing;
		b8 mShiftHold;
		b8 pad1;
	};

	class tWheeledVehicleLogic : public tVehicleLogic, public Physics::tVehiclePhysicsCallBack
	{
		define_dynamic_cast( tWheeledVehicleLogic, tVehicleLogic );
	public:
		tWheeledVehicleLogic( );

		virtual Logic::tPhysical*	fQueryPhysical( ) { return &mPhysics; }
		Physics::tVehiclePhysics& fPhysics( ) { return mPhysics; }
		const Physics::tVehiclePhysics& fPhysics( ) const { return mPhysics; }

		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );
		virtual void fOnSpawn( );

		virtual void fAnimateMT( f32 dt );
		virtual void fPhysicsMT( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		virtual void fOnDelete( );
		virtual Math::tVec4f fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const;

		virtual const Math::tMat3f& fCurrentTransformMT( ) const { return mPhysics.fTransform( ); }
		virtual const Math::tVec3f& fCurrentVelocityMT( ) const { return mPhysics.fVelocity( ); }
		virtual Math::tVec3f fPointVelocityMT( const Math::tVec3f& worldPoint ) const { return mPhysics.fPointVelocity( worldPoint ); }
		virtual f32 fMaxSpeed( ) const { return mPhysics.fMaxSpeed( ); }

		virtual void fRespawn( const Math::tMat3f& tm );
		virtual void fSetupVehicle( );
		virtual void fComputeUserInput( f32 dt );
		virtual void fComputeAIInput( f32 dt );
		virtual void fReactToDamage( const tDamageContext& dc, const tDamageResult& dr );
		virtual void fReactToWeaponFire( const tFireEvent& event );
		virtual void fPushCamera( tPlayer* player, u32 seat );
		virtual void fPopCamera( tPlayer* player );
		virtual void fReapplyTable( );
		virtual void fClearContacts( ) { mPhysics.fClearContactsST( ); }
		virtual void fAddContactPt( f32 dt, const Physics::tContactPoint& cp ) { mPhysics.fAddContactPtMT( dt, cp ); }
		virtual Math::tVec3f fComputeContactResponse( tUnitLogic* theirUnit, const Physics::tContactPoint& cp, f32 mass, b32& ignore ) { return mPhysics.fComputeContactResponse( cp, mass );}
		virtual void fAddCollisionResponse( const Math::tVec3f& response, f32 mass ) { mPhysics.fAddCollisionResponse( response, mass ); }

		virtual void fStartup( b32 startup );
		virtual void fShutDown( b32 shutDown );

		virtual b32 fPathPointReached( tPathEntity& point, const Logic::tEventContextPtr& context );
		virtual b32 fShouldSuspendWheels( tLogic* logic ); //from Physics::tVehiclePhysicsCallBack

		struct tCameraData
		{
			f32 mCameraTurnIn;	//angle to turn camera in with full steer
			f32 mCameraZoomOut;	//distance to zoom out at full speed
			f32 mCameraLerp;	//lerp factor for camera blending
		};

		b32								fIsTank( ) const { return mPhysics.fProperties( ).mIsTank; }
		const tCameraData&				fCameraData( ) const { return mCameraData; }
		const tTurretCameraMovement&	fCameraMovement( u32 seat ) const { return mCameraMovement[ seat ]; }
		f32								fFollowDistance( ) const { return mFollowDistance; }


		void fSetParachuteing( b32 parachuting ) { mParachuting = parachuting; }
		b32  fParachuting( ) const { return mParachuting; }
		b32  fChuteOpen( ) const { return mChuteOpen; }
		void fSetChuteOpen( b32 open ) { mChuteOpen = open; }

		void fSetWheelDustSigmlPath( const tFilePathPtr& path ) { mWheelDustSigmlPath = path; }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tGrowableArray<tEntityPtr> mWheelEntities, mFenderEntities;
		tGrowableArray<FX::tFxFileRefEntityPtr> mWheelDusts;
		tGrowableArray<tEntityPtr> mScrewWheels;
		f32 mGroundHeightOffset;
		f32 mFollowDistance;
		f32 mAngDamping;
		f32 mMaxSpeed;
		f32 mWheelRumble;
		f32 mImpulseMeter;

		void fApplySurfaceSwitch( );
		u32 mNextSurfaceTypeMT;
		tFixedArray<u32, 2> mSurfaceTypesMT;

		tCameraData					mCameraData;
		tFixedArray<tTurretCameraMovement, cVehicleSeatCount> mCameraMovement;
		tVehiclePathFollower		mFollower;
		Physics::tVehiclePhysics	mPhysics;

		tEntityPtr					mDeliveryBase;
		tTankPalette*				mDeliveryLogic;

		void fApplyTableProperties( Physics::tVehicleProperties &props );
		f32 fMeterImpulse( f32 desired );

		tTransmission mTrans;

		// Extra variables
		f32 mReverseTimer;
		tEntityPtr mTrailer;
		b8 mIsTrailer;
		b8 mBurnout;
		b8 mParachuting;
		b8 mChuteOpen;

		f32 mBurnoutTimer;

		void fSetIsTrailer( tEntity* parent );
		Physics::tPinConstraintPtr mTrailerConstraint;

		// If we're attached to a flying thing
		tFilePathPtr mFlyingBasePath;
		tAirborneLogic* mFlyingBaseLogic;
		tEntityPtr		mFlyingBaseEntity;

		b8 mFlyingBaseInitialized;
		b8 pad3;
		b8 pad4;
		b8 pad5;

		void fSetFlyingBase( const tFilePathPtr& path ) { mFlyingBasePath = path; }
		void fAttachFlyingBase( b32 attach );
		b32  fOnFlyingBase( ) const { return mFlyingBaseInitialized; }
		void fInitializeFlyingBase( );
		tAirborneLogic* fFlyingBase( ) { return mFlyingBaseLogic; }

		// Debugging, removable
		Math::tAabbf mDebugWheelShape;

		tEntityPtr	mParachute;
		tLogic		*mParachuteLogic;

		tFilePathPtr mWheelDustSigmlPath;

	};

}

#endif//__tVehicleLogic__
