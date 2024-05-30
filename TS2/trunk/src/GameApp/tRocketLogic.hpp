#ifndef __tRocketLogic__
#define __tRocketLogic__

#include "tProjectileLogic.hpp"
#include "Audio/tSource.hpp"

namespace Sig
{

	class tRocketPhysics
	{
	public:
		Math::tMat3f mTransform;
		Math::tVec3f mVelocity;
		f32 mAcceleration;
		f32 mAccelerationDelayTimer;
		f32 mSmoothThrust;

		void fSetup( const Math::tMat3f& transform, const Math::tVec3f& velocity, f32 acc, f32 accDelay, b32 smoothThrust )
		{
			mTransform = transform;
			mVelocity = velocity;
			mAcceleration = acc;
			mAccelerationDelayTimer = accDelay;
			mSmoothThrust = smoothThrust ? 1.f : 0.f;
		}

		Math::tMat3f& fStep( f32 dt )
		{
			if( mAccelerationDelayTimer > 0.f )
				mAccelerationDelayTimer -= dt;

			if( mAccelerationDelayTimer < 0.f )
			{
				mSmoothThrust = fMax( 0.f, mSmoothThrust - ( dt * 0.5f ) );
				mVelocity += mTransform.fZAxis( ) * ( ( 1.f - mSmoothThrust ) * mAcceleration * dt);
			}

			sigassert( mVelocity == mVelocity );
			mTransform.fTranslateGlobal( mVelocity * dt );
			return mTransform;
		}
	};

	class tRocketLogic : public tProjectileLogic
	{
		define_dynamic_cast( tRocketLogic, tProjectileLogic );
	public:
		tRocketLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fMoveST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		virtual f32 fUpdateShellCam( tPlayer& player, f32 dt );

		enum tGuidanceMode { cStraightFire, cLazyLock, cFullLock };
		void fSetGuidanceMode( u32 mode ) { mGuidanceMode = mode; }
		void fSetNextGuidanceMode( u32 mode ) { mNextGuidanceMode = mode; }
		u32 fGuidanceMode( ) const { return mGuidanceMode; }
		u32 fNextGuidanceMode( ) const { return mNextGuidanceMode; }

		virtual void fInherit( tProjectileLogic& from, f32 dt );
		virtual void fHitSomething( const tEntityPtr& ent );

	protected:
		virtual void fInitPhysics( );
		virtual void fComputeNewPosition( f32 dt );
		virtual void fSetTarget( tEntityPtr& target, const Math::tVec3f& targetPt );

		virtual Math::tVec3f& fCurrentVelocity( ) { return mPhysics.mVelocity; }
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

		tRocketPhysics	mPhysics;
		tEntityPtr		mTargetEntity;
		tUnitLogic		*mTargetLogic;
		Math::tVec3f	mTargetPoint;
		u32				mGuidanceMode;
		u32				mNextGuidanceMode;
		f32				mGuidanceKickinTimer;
		f32				mThrustDelayTimer;
		b32				mSmoothThrust;
		b32				mTargetSet;
		f32				mTurnRate;
		f32				mDistanceToTargetSqrd;
		f32				mInitialDistanceToTarget;
		f32				mTargetDropVelocity; //velocity of target drop off.
		f32				mTargetDrop;
		f32				mShellCamSteerRate;
		f32				mRotation;
		f32				mRotationRate;
	};

}

#endif//__tRocketLogic__
