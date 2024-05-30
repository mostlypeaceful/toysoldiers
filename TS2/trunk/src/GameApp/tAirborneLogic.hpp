#ifndef __tAirborneLogic__
#define __tAirborneLogic__

#include "tVehicleLogic.hpp"
#include "Physics/tAirbornePhysics.hpp"
#include "FX/tTracerTrailEntity.hpp"

namespace Sig
{

	class tAirborneLogic : public tVehicleLogic
	{
		define_dynamic_cast( tAirborneLogic, tVehicleLogic );
	public:
		tAirborneLogic( );

		virtual Logic::tPhysical*	fQueryPhysical( ) { return &mPhysics; }
		virtual b32 fHandleLogicEvent( const Logic::tEvent& e );

		virtual void fAnimateMT( f32 dt );
		virtual void fPhysicsMT( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fThinkST( f32 dt );
		virtual void fCoRenderMT( f32 dt ) { tVehicleLogic::fCoRenderMT( dt * fTimeScale( ) ); }

		virtual void fOnDelete( );

		virtual const Math::tMat3f& fCurrentTransformMT( ) const { return mPhysics.fGetTransform( ); }
		virtual const Math::tVec3f& fCurrentVelocityMT( ) const { return mPhysics.fVelocity( ); }
		virtual Math::tVec3f fPointVelocityMT( const Math::tVec3f& worldPoint ) const { return mPhysics.fPointVelocity( worldPoint ); }
		virtual f32 fMaxSpeed( ) const { return mPhysics.fProperties( ).mMaxSpeed; }

		virtual void fRespawn( const Math::tMat3f& tm );
		virtual void fSetupVehicle( );
		virtual void fComputeUserInput( f32 dt );
		virtual void fComputeAIInput( f32 dt );
		virtual void fReactToDamage( const tDamageContext& dc, const tDamageResult& dr );
		virtual void fReactToWeaponFire( const tFireEvent& event );
		virtual void fFindCollisionMT( f32 dt );
		virtual void fPushCamera( tPlayer* player, u32 seat );
		virtual void fPopCamera( tPlayer* player );
		virtual void fReapplyTable( );

		virtual s32 fComputeChangeState( const tDamageContext& dc, const tDamageResult& dr );
		virtual void fStartup( b32 startup );

		Physics::tAirbornePhysics& fPhysics( ) { return mPhysics; }
		const Physics::tAirbornePhysics& fPhysics( ) const { return mPhysics; }
		b32 fInBombCam( ) const { return mInBombCam; }

		void fAvert( );
		b32  fNewPath( b32 needNewPath ); //returns true if it could find out
		b32 fGoToGoal( );
		void fDitch( );

		b32 fOnFinalGoalPath( ) const { return mOnFinalGoalPath; }

		void fSetDisableEvasion( b32 disable ) { mDisableEvasion = disable; }
		b32 fDisableEvasion( ) const { return mDisableEvasion; }

		void fLockInBombCam( b32 lock ) { mLockedInBombCam = lock; if( lock && !fInBombCam( ) ) fEnableBombCam( true ); }
		void fDontDitchOnExit( b32 dontDitch ) { mDontDitchOnExit = dontDitch; }

		void fInitializeJetEngineFx( const tFilePathPtr& ignitionFx, const tFilePathPtr& boostFx );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
		void fApplyTableProperties( Physics::tAirborneProperties &props );

	private:
		Physics::tAirbornePhysics mPhysics;
		b8 mInBombCam;
		b8 mSpecialMove;
		b8 mSpecialMoveInputLock;
		b8 mOnFinalGoalPath;

		b8 mDisableEvasion;
		b8 mLockedInBombCam;
		b8 mDontDitchOnExit;
		b8 mWasFullSpeed;

		b8 mEnginesStarted;
		b8 pad0;
		b8 pad1;
		b8 pad2;

		s32 mDeathState; //store the death state so it doesnt change
		f32 mSpecialMoveInputTimer;

		f32 mRandomFlyTimeRemaining; // when this gets to zero go to goal
		void fSetRandomFlyTimeRemaining( f32 time );

		f32 mNewPathTimeout; //dont switch paths while this is positive
		f32 mSpeedUpTimer;	 //go full speed while this is positive
		f32 mConTrailIntensity;

		void fApplyUserRestrictedBanks( );
		void fEnableBombCam( b32 enable );
		virtual void fStopSpecialMove( );
		virtual void fStartSpecialMove( u32 move );

		Math::tDampedFloat mAnimationBlend;

		// Enable and disable banks as we enter/exit bomb cam
		tGrowableArray<u32> mEnableBombBank;
		tGrowableArray<u32> mDisableBombBank;
		void fAddEnableBombBank( u32 bank ) { mEnableBombBank.fPushBack( bank ); }
		void fAddDisableBombBank( u32 bank ) { mDisableBombBank.fPushBack( bank ); }

		tStringPtr mConTrailTracerName;
		tGrowableArray<FX::tTracerTrailEntityPtr> mConTrails;
		Math::tVec3f mConTrailPosLastFrame;

		class tJetEngineFx
		{
		private:
			tGrowableArray< Sig::tAttachmentEntityPtr > mEngineAttachments;
			tFilePathPtr mIgnitionFx;
			tFilePathPtr mBoostFx;
			FX::tFxFileRefEntityPtr mCurrentBoost;

		public:
			tJetEngineFx( ) { }
			void fInit( tEntity* airplane, const tFilePathPtr& ignition, const tFilePathPtr& boost );
			void fEngineIgnition( );
			void fEngineBoost( );
		};


		tJetEngineFx mJetEngineFx;
	};

}

#endif//__tAirborneLogic__
