#include "BasePch.hpp"
#include "tPinConstraint.hpp"
#include "tRigidBody.hpp"
#include "tPhysicsWorld.hpp"


using namespace Sig::Math;

namespace Sig { namespace Physics
{

	devvar( bool, Physics_PinConstraint_RotateAlso, false );
	devvar( f32, Physics_PinConstraint_PositionMix, 0.929f );
	devvar( f32, Physics_PinConstraint_PositionDMix, 1.0f );
	devvar( f32, Physics_PinConstraint_PositionIMix, 0.0f );
	devvar( f32, Physics_PinConstraint_PositionPMix, 8.5f );
	
	tPinConstraint::tPinConstraint( tRigidBody* ownerA, tRigidBody* ownerB, const Math::tMat3f& aRelConstraintPt )
		: tConstraint( ownerA, ownerB, aRelConstraintPt )
		, mIntegral( tVec3f::cZeroVector )
	{
	}

	void tPinConstraint::fStepConstraintInternal( f32 dt, f32 percentage )
	{
		// Setup data
		tVec3f worldPtA = mBodyA->fTransform( ).fXformPoint( mAAnchorPt.fGetTranslation( ) );
		tVec3f worldPtB = mBAnchorPt.fGetTranslation( );
		tVec3f bVel = tVec3f::cZeroVector;

		f32 shareScale = 1.0;

		if( mBodyB )
		{
			shareScale = 0.5f;
			worldPtB = mBodyB->fTransform( ).fXformPoint( worldPtB );
			bVel = mBodyB->fPointVelocity( worldPtB );
		}

		// Constraint work
		tVec3f sep = worldPtB - worldPtA;
		tVec3f velDiff = bVel - mBodyA->fPointVelocity( worldPtA );
		
		velDiff *= percentage;
		
		velDiff *= Physics_PinConstraint_PositionDMix;
		velDiff += sep * Physics_PinConstraint_PositionPMix;

		mIntegral += sep * Physics_PinConstraint_PositionIMix;
		velDiff += mIntegral;

		f32 velLen = velDiff.fLengthSquared( );
		if( velLen > 0.001f )
		{
			velLen = fSqrt( velLen );
			velDiff /= velLen; //normalize

			if( mBodyB )
			{
				tVec3f midPt = worldPtA + sep * 0.5f;
				tVec3f impulse = velDiff * mBodyA->fComputeImpulseToChangeRelativePointVel( dt, midPt, midPt, velDiff, velLen, *mBodyB );
				mBodyA->fAddImpulse( impulse, midPt );
				mBodyB->fAddImpulse( -impulse, midPt );

				//tVec3f force = impulse / dt;
				//mBodyA->fAddForce( force, midPt );
				//mBodyB->fAddForce( -force, midPt );
			}
			else
			{
				tVec3f impulse = velDiff * mBodyA->fComputeImpulseToChangePointVel( dt, worldPtA, velDiff, velLen ) * percentage;
				mBodyA->fAddImpulse( impulse, worldPtA );
				//tVec3f force = impulse / dt;
				//mBodyA->fAddForce( force, worldPtA );
			}
		}

		f32 mixFactor = shareScale * Physics_PinConstraint_PositionMix * percentage;

		if( Physics_PinConstraint_RotateAlso )
		{
			// relax the constraint error by rotating the bodies to align to the target pt and also translate
			tVec3f aPos = mBodyA->fTransform( ).fGetTranslation( );
			tVec3f aTarget = worldPtB - aPos;
			tVec3f aCurrent = worldPtA - aPos;
			f32 offsetLen, targetLen;
			aTarget.fNormalizeSafe( tVec3f::cZeroVector, targetLen );
			aCurrent.fNormalizeSafe( tVec3f::cZeroVector, offsetLen );

			tAxisAnglef rotate2( aCurrent, aTarget );
			rotate2.mAngle *= mixFactor;

			tMat3f xform = tMat3f( tQuatf( rotate2 ) ) * mBodyA->fTransform( );
			xform.fSetTranslation( aPos + aTarget * (targetLen - offsetLen) * mixFactor );
			mBodyA->fSetTransform( xform );

			if( mBodyB )
			{
				//worldPtA = mBodyA->fTransform( ).fXformPoint( mAAnchorPt.fGetTranslation( ) );
				tVec3f bPos = mBodyB->fTransform( ).fGetTranslation( );
				tVec3f bTarget = worldPtA - bPos;
				tVec3f bCurrent = worldPtB - bPos;
				f32 offsetLen, targetLen;
				bTarget.fNormalizeSafe( tVec3f::cZeroVector, targetLen );
				bCurrent.fNormalizeSafe( tVec3f::cZeroVector, offsetLen );

				tAxisAnglef rotate( bCurrent, bTarget );
				rotate.mAngle *= mixFactor;

				tMat3f xform = tMat3f( tQuatf( rotate ) ) * mBodyB->fTransform( );
				xform.fSetTranslation( bPos + bTarget * (targetLen - offsetLen) * mixFactor );
				mBodyB->fSetTransform( xform );
			}
		}
		else
		{
			sep *= mixFactor;
			mBodyA->fTranslate( sep );
			if( mBodyB )
				mBodyB->fTranslate( -sep );
		}
	}

	void tPinConstraint::fDebugDraw( tPhysicsWorld& world )
	{
		tVec3f worldPtA = mBodyA->fTransform( ).fXformPoint( mAAnchorPt.fGetTranslation( ) );
		world.fDebugGeometry( ).fRenderOnce( mBodyA->fTransform( ).fGetTranslation( ), worldPtA, tVec4f( 1,0,0,1 ) );
		
		tVec3f worldPtB = mBAnchorPt.fGetTranslation( );
		if( mBodyB )
		{
			worldPtB = mBodyB->fTransform( ).fXformPoint( worldPtB );
			world.fDebugGeometry( ).fRenderOnce( mBodyB->fTransform( ).fGetTranslation( ), worldPtB, tVec4f( 0,1,0,1 ) );
		}

		world.fDebugGeometry( ).fRenderOnce( worldPtB, worldPtA, tVec4f( 0,0,1,1 ) );
	}
}}
