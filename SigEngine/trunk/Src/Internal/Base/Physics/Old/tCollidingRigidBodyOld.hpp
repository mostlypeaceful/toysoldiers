#ifndef __tCollidingRigidBodyOld__
#define __tCollidingRigidBodyOld__

#include "tRigidBodyOld.hpp"
#include "tContactPointOld.hpp"



namespace Sig { namespace PhysicsOld
{

	struct tCollisionFeedback
	{
		b32 mCollisionHandled;
		b32 mStoppedInFront;
		b32 mStoppedInBack;
		b32 mStoppedAlongLeft;
		b32 mStoppedAlongRight;
		Math::tVec3f mCollisionDeltaV;
		Math::tVec3f mCollisionDeltaW;
		Math::tVec3f mPenetrationTranslation;

		void fReset( )
		{
			mCollisionHandled = false;
			mStoppedInFront = false;
			mStoppedInBack = false;
			mStoppedAlongLeft = false;
			mStoppedAlongRight = false;
			mCollisionDeltaV = Math::tVec3f::cZeroVector;
			mCollisionDeltaW = Math::tVec3f::cZeroVector;
			mPenetrationTranslation = Math::tVec3f::cZeroVector;
		}
	};

	class tCollidingRigidBody : public tRigidBody
	{
		define_dynamic_cast( tCollidingRigidBody, tRigidBody );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

		tCollidingRigidBody( );

		void fClearContactsST( );
		void fAddContactPtMT( f32 dt, const tContactPoint& cp );
		const tCollisionFeedback& fGetFeedback( ) const { return mFeedback; }

		void fResolveContactsMT( f32 dt );

	protected:
		f32				mCoefRestitution;
		Math::tVec3f	mCollisionDeltaV;
		tCollisionFeedback mFeedback;

		tGrowableArray<tContactPoint> mContactPoints;
		tGrowableArray<tContactPoint> mPrevCPs;

		b32 mImmediateResolve; //will compute delta W and V at the time contact pt is added.

		void fOneBodyResolve( f32 dt, const tContactPoint& cp );
		void fTwoBodyResolve( f32 dt, const tContactPoint& cp );

	};

}}

#endif//__tCollidingRigidBodyOld__
