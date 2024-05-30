#include "BasePch.hpp"
#include "tCollidingRigidBodyOld.hpp"

using namespace Sig::Math;

namespace Sig { namespace PhysicsOld
{

	devvar( f32, Physics_CollidingRigidBody_PenetrationLerp, 0.2f );

	tCollidingRigidBody::tCollidingRigidBody( )
		: mCoefRestitution( 0.5f )
		, mImmediateResolve( false )
	{
	}


	void tCollidingRigidBody::fClearContactsST( )
	{
		mPrevCPs = mContactPoints;
		mContactPoints.fSetCount( 0 );
		mFeedback.fReset( );
	}

	void tCollidingRigidBody::fAddContactPtMT( f32 dt, const tContactPoint& cp )
	{
		mContactPoints.fPushBack( cp );

		if( mImmediateResolve )
		{
			if( cp.mRigidBody )
				fTwoBodyResolve( dt, cp );
			else
				fOneBodyResolve( dt, cp );
		}
	}

	void tCollidingRigidBody::fOneBodyResolve( f32 dt, const tContactPoint& cp )
	{
		// remove penetration
		mFeedback.mPenetrationTranslation += cp.mNormal * cp.mDepth * Physics_CollidingRigidBody_PenetrationLerp;

		tVec3f pointV = fPointVelocity( cp.mPoint );
		f32 normalVel = pointV.fDot( cp.mNormal );
		if( normalVel < 0.f )
		{
			f32 delta = -(1 + mCoefRestitution) * normalVel;
			tVec3f impulse = fComputeImpulseToChangePointVel( dt, cp.mPoint, cp.mNormal, delta );
			fApplyDeltaFromImpulse( impulse, cp.mPoint, mFeedback.mCollisionDeltaV, mFeedback.mCollisionDeltaW );
		}
	}

	void tCollidingRigidBody::fTwoBodyResolve( f32 dt, const tContactPoint& cp )
	{
		sigassert( cp.mRigidBody );
		// does not apply any resolution to the other body, assumes the other body will have found the same collision and done the same computation
		// remove penetration
		mFeedback.mPenetrationTranslation += cp.mNormal * cp.mDepth * Physics_CollidingRigidBody_PenetrationLerp * 0.5f;

		tVec3f pointV = fPointVelocity( cp.mPoint ) - cp.mRigidBody->fPointVelocity( cp.mPoint );
		f32 normalVel = pointV.fDot( cp.mNormal );
		if( normalVel < 0.f )
		{
			f32 delta = -(1 + mCoefRestitution) * normalVel;
			tVec3f impulse = fComputeImpulseToChangeRelativePointVel( dt, cp.mPoint, cp.mNormal, delta, *cp.mRigidBody );
			fApplyDeltaFromImpulse( impulse, cp.mPoint, mFeedback.mCollisionDeltaV, mFeedback.mCollisionDeltaW );
		}
	}

	void tCollidingRigidBody::fResolveContactsMT( f32 dt )
	{
		if( !mImmediateResolve )
		{
			for( u32 i = 0; i < mContactPoints.fCount( ); ++i )
			{
				const tContactPoint& cp = mContactPoints[ i ];
				if( cp.mRigidBody )
					fTwoBodyResolve( dt, cp );
				else
					fOneBodyResolve( dt, cp );
			}
		}

		mV += mFeedback.mCollisionDeltaV;
		mW += mFeedback.mCollisionDeltaW;
		mTransform.fTranslateGlobal( mFeedback.mPenetrationTranslation );
	}

}}

namespace Sig { namespace PhysicsOld
{

	//void tCollidingRigidBody::fExportScriptInterface( tScriptVm& vm )
	//{
	//	Sqrat::DerivedClass< tCollidingRigidBody, tRigidBody, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
	//	vm.fNamespace(_SC("Physics")).Bind(_SC("CollidingRigidBody"), classDesc);
	//}

} }