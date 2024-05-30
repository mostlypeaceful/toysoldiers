#include "BasePch.hpp"
#include "tRagDollConstraint.hpp"
#include "tRigidBody.hpp"


using namespace Sig::Math;

namespace Sig { namespace Physics
{

	devvar( bool, Physics_Ragdoll_Enable, true );

	devvar( f32, Physics_Ragdoll_ChildInfluence, 0.25f );

	devvar( f32, Physics_Ragdoll_Anim, 2.0f );
	devvar( f32, Physics_Ragdoll_AnimDamp, 0.075f );
	devvar( bool, Physics_Ragdoll_AnimDo, true );

	devvar( bool, Physics_Ragdoll_RelaxDo, false );
	devvar( f32, Physics_Ragdoll_RelaxAngle, 1.0f );
	devvar( f32, Physics_Ragdoll_RelaxW, 0.1f );

	devvar( f32, Physics_Ragdoll_Torque, 12.0f );
	devvar( f32, Physics_Ragdoll_TorqueDamp, 0.075f );
	devvar( bool, Physics_Ragdoll_TorqueDo, true );

	devvar( f32, Physics_Ragdoll_TestAngle, 0.0f );
	devvar( bool, Physics_Ragdoll_TestAngleEnable, false );


	tRagDollConstraint::tRagDollConstraint( tRigidBody* ownerA, tRigidBody* ownerB, const Math::tMat3f& aRelConstraintPt )
		: tPinConstraint( ownerA, ownerB, aRelConstraintPt )
		, mChildWeightScale( 1.f )
		, mDesiredAnimOrient( tQuatf::cIdentity )
		, mAxisLimitsOrder( ~0 )
		, mAxisLimits( tAabbf::cZeroSized )
	{
		//// testing
		//mAxisLimitsOrder = cEulerOrderXYZ;
		//mAxisLimits = tAabbf( tVec3f( 0, 0, 0 ), tVec3f( 0, fToRadians( 90.f ), 0 ) );
	}

	void tRagDollConstraint::fSetDesiredAnimOrient( const Math::tQuatf& orient )
	{
		mDesiredAnimOrient = orient;
	}

	namespace
	{
		tEulerAnglesf fClampEuler( const tEulerAnglesf& toClamp, const tAabbf& bounds )
		{
			tEulerAnglesf out;
			for( u32 i = 0; i < 3; ++i )
				out.fAxis( i ) = fClamp( toClamp.fAxis( i ), bounds.mMin.fAxis( i ), bounds.mMax.fAxis( i ) );
			return out;
		}

		tQuatf fClamp( u32 order, const tQuatf& delta, const tAabbf& bounds )
		{
			if( order == ~0 )
				return delta;

			tEulerAnglesf angles = fCMLDecompose( order, delta );
			
			angles = fClampEuler( angles, bounds );

			return fCMLCompose( order, angles );
		}

		tQuatf fScaleShortestWay( const tQuatf& input, f32 scale )
		{
			tAxisAnglef aa( input );
			aa.mAngle = fShortestWayAround( 0.f, aa.mAngle );
			aa.mAngle *= scale;
			return tQuatf( aa );
		}
	}
	void tRagDollConstraint::fStepConstraintInternal( f32 dt, f32 percentage )
	{
		tPinConstraint::fStepConstraintInternal( dt, percentage );

		if( Physics_Ragdoll_TestAngleEnable )
			mDesiredAnimOrient = tQuatf( tAxisAnglef( tVec3f::cYAxis, fToRadians( (f32)Physics_Ragdoll_TestAngle ) ) );

		if( Physics_Ragdoll_Enable )
		{
			tMat3f aAnchor = mBodyA->fTransform( ) * mAAnchorPt;
			tMat3f bAnchor = mBodyB->fTransform( ) * mBAnchorPt;

			tQuatf qAAnchor( aAnchor );
			tQuatf qAAnchorInv = qAAnchor.fInverse( );
			tQuatf qBAnchor( bAnchor );
			tQuatf qBAnchorInv = qBAnchor.fInverse( );

			tQuatf currentDelta = qAAnchorInv * qBAnchor;

			tQuatf clampedOrient = fClamp( mAxisLimitsOrder, currentDelta, mAxisLimits );
			tQuatf expectedBAnchor = qAAnchor * clampedOrient;
			tQuatf bCorrection = qBAnchorInv * expectedBAnchor;

			f32 bWeight = Physics_Ragdoll_ChildInfluence * mChildWeightScale;
			f32 aWeight = 1.f - bWeight;

			if( Physics_Ragdoll_RelaxDo )
			{
				f32 relax = 0.5f * Physics_Ragdoll_RelaxAngle;
				tQuatf bDelta = fScaleShortestWay( bCorrection, relax * aWeight );  //if aWeight is high, we want to apply more delta to b
				tQuatf aDelta = fScaleShortestWay( bCorrection, -relax * bWeight ); // and vise versa

				mBodyB->fSetTransform( bAnchor * tMat3f( bDelta ) * mBAnchorPt.fInverse( ) );
				mBodyA->fSetTransform( aAnchor * tMat3f( aDelta ) * mAAnchorPt.fInverse( ) );
			}

			// this is trying to maintain a zero joint velocity, so it will heavily dampen movement in the permitted axis.
			// RelaxW is basically for stability, two much will be extremely wonky
			tVec3f relW = mBodyA->mW * aWeight + mBodyB->mW * bWeight;
			mBodyA->mW = fLerp( mBodyA->mW, relW, (f32)Physics_Ragdoll_RelaxW );
			mBodyB->mW = fLerp( mBodyB->mW, relW, (f32)Physics_Ragdoll_RelaxW );

			if( Physics_Ragdoll_TorqueDo )
			{
				tAxisAnglef aa( bCorrection );
				aa.mAngle = fShortestWayAround( 0.f, aa.mAngle ) * 0.5f;
				// make this world space
				aa.mAxis = qBAnchor.fRotate( aa.mAxis );

				// see how much the bodies are already rotating relative to each other
				tVec3f currentDelta = mBodyB->mW - mBodyA->mW;
				tVec3f currentDeltaAlongChange = currentDelta.fDot( aa.mAxis ) * aa.mAxis;

				tVec3f changeW = aa.mAxis * aa.mAngle * Physics_Ragdoll_Torque * percentage;
				changeW -= currentDeltaAlongChange * Physics_Ragdoll_TorqueDamp;

				mBodyB->mW += changeW;
				mBodyA->mW -= changeW;

				//mBodyB->fAddAngularImpulse( changeW );
				//mBodyA->fAddAngularImpulse( -changeW );
			}

			if( Physics_Ragdoll_AnimDo )
			{
				tQuatf expectedBAnchor = qAAnchor * mDesiredAnimOrient;
				tQuatf bCorrection = qBAnchorInv * expectedBAnchor;

				tAxisAnglef aa( bCorrection );
				aa.mAngle = fShortestWayAround( 0.f, aa.mAngle ) * 0.5f;
				// make this world space
				aa.mAxis = qBAnchor.fRotate( aa.mAxis );

				// see how much the bodies are already rotating relative to each other
				tVec3f currentDelta = mBodyB->mW - mBodyA->mW;
				tVec3f currentDeltaAlongChange = currentDelta.fDot( aa.mAxis ) * aa.mAxis;

				tVec3f changeW = aa.mAxis * aa.mAngle * Physics_Ragdoll_Anim * percentage;
				changeW -= currentDeltaAlongChange * Physics_Ragdoll_AnimDamp;

				mBodyB->mW += changeW;
				mBodyA->mW -= changeW;

				//mBodyB->fAddAngularImpulse( changeW );
				//mBodyA->fAddAngularImpulse( -changeW );
			}
		}

		// merely unit testing compose and decompose
		//for( u32 order = 0; order < cEulerOrderCount; ++order )
		//{
		//	tMat3f test( fCMLCompose( order, fCMLDecompose( order, tQuatf( delta ) ) ) );
		//	
		//	test.fSetTranslation( delta.fGetTranslation( ) );
		//	sigassert( delta.fEqual( test ) );
		//}
	}

}}