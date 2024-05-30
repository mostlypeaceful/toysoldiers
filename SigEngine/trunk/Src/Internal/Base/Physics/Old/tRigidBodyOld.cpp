#include "BasePch.hpp"
#include "tRigidBodyOld.hpp"

using namespace Sig::Math;

namespace Sig { namespace PhysicsOld
{
	tRigidBody::tRigidBody( ) 
		: mGravity( -11.8f )
		, mAngDamping( 1.0f )
		, mDoFancyStuff( true )
		, mMass( 1.f )
		, mInertia( 5.f )
	{
		fReset();
	}

	void tRigidBody::fOnDelete( )
	{
		mConstraints.fSetCount( 0 );
	}

	void tRigidBody::fReset( const tMat3f& tm )
	{
		fSetTransform( tm );
		mV = tVec3f::cZeroVector;
		mF = tVec3f::cZeroVector;
		mW = tVec3f::cZeroVector;
		mT = tVec3f::cZeroVector;
	}

	void tRigidBody::fSetTransform( const tMat3f& tm )
	{
		mTransform = tm;

		if( mDoFancyStuff ) fRecomputeIntertiaMatrix( ); //mWorldInertiaInv = tm * tMat3f(1.0f / mInertia) * tm.fInverse(); //
	}

	void tRigidBody::fRecomputeIntertiaMatrix( )
	{
		mWorldInertiaInv = mTransform * tMat3f(1.0f / mInertia) * mTransform.fInverse();
	}
	
	void tRigidBody::fSetMassProperties( tVec3f halfExtents, f32 mass, f32 inertiaScale )
	{
		halfExtents *= 2.0f;
		mInertia = tVec3f( pow(halfExtents.y, 2) + pow(halfExtents.z, 2)
		                 , pow(halfExtents.x, 2) + pow(halfExtents.z, 2)
		                 , pow(halfExtents.x, 2) + pow(halfExtents.y, 2) );

		mInertia *= (inertiaScale/12.0f) * mass;
		mMass = mass;
		if( mDoFancyStuff ) fRecomputeIntertiaMatrix( );
	}
	
	const tMat3f& tRigidBody::fIntegrate( f32 dt )
	{
		//linear velocity and forces
		mV += (mF / mMass + tVec3f(0,mGravity,0)) * dt;
		mF = tVec3f::cZeroVector;

		tVec3f p = mTransform.fGetTranslation( );
		p += mV * dt;

		tQuatf r = tQuatf( mTransform );
		r.fNormalizeSafe( tQuatf::cIdentity );

		//angular velocity and torques
		mW += mT * dt;
		mT = tVec3f::cZeroVector;

		tQuatf w(mW.x, mW.y, mW.z, 0);
		tQuatf spin = 0.5f * w * r;		
		r += spin * dt;

		r.fNormalizeSafe( tQuatf::cIdentity );

		mTransform = tMat3f(r, p);	

		if( mDoFancyStuff )
		{
			fRecomputeIntertiaMatrix( );

			tMat3f angDamp = tMat3f(r) * tMat3f(mAngDamping) * tMat3f(r.fInverse());
			mW = angDamp.fXformVector(mW);
		}

		return mTransform;
	}
	
	void tRigidBody::fAddForce( const tVec3f &worldForce, const tVec3f &worldPoint )
	{
		mF += worldForce;

		tVec3f offset = worldPoint - mTransform.fGetTranslation( );
		tVec3f torque = offset.fCross( worldForce );
		fAddTorque( torque );
	}

	void tRigidBody::fAddTorque( const tVec3f &worldTorque )
	{
		mT += mWorldInertiaInv.fXformVector( worldTorque );
	}

	void tRigidBody::fAddImpulse( const tVec3f &worldImp, const tVec3f &worldPoint )
	{
		fApplyDeltaFromImpulse( worldImp, worldPoint, mV, mW );
	}

	void tRigidBody::fApplyDeltaFromImpulse( const tVec3f &worldImp, const tVec3f &worldPoint, tVec3f& v, tVec3f& w )
	{
		tVec3f offset = worldPoint - mTransform.fGetTranslation( );
		v += worldImp / mMass;
		w += mWorldInertiaInv.fXformVector(offset.fCross(worldImp));
	}

	tVec3f tRigidBody::fPointVelocity( const tVec3f& worldPoint ) const
	{
		tVec3f offset = worldPoint - mTransform.fGetTranslation( );
		tVec3f result = mV;
		result += mW.fCross( offset );
		return result; 
	}

	//tVec3f tRigidBody::fPointMomentum( const tVec3f& worldPoint ) const
	//{
	//	tVec3f angularVAtPoint = mW.fCross( worldPoint - mTransform.fGetTranslation( ) );
	//	//return mV * mMass + mWorldInertiaInv.fInverse().fXformVector( angularVAtPoint );
	//	return (mV + angularVAtPoint) * mMass;
	//}

	void tRigidBody::fSetAngularDamping( const tVec3f& ad )
	{
		mAngDamping = ad;
	}

	tVec3f tRigidBody::fComputeImpulseToChangePointVel( f32 dt, const tVec3f& worldPoint, const tVec3f& direction, float magnitude )
	{
		tVec3f offset = worldPoint - mTransform.fGetTranslation( );
		tVec3f offsetCrossN = offset.fCross(direction);
		offsetCrossN *= offsetCrossN;
		offsetCrossN = mWorldInertiaInv.fXformVector( offsetCrossN );
		float offsetCrossNSquared = offsetCrossN.fLength( );

		//j =  	        magnitude
		//     --------------------------
		//       1/ma + (rap x direction)2 / Ia

		f32 denominator = 1 / mMass + offsetCrossNSquared;
		f32 j = magnitude / denominator;

		tVec3f impulse = direction * j;
		return impulse;
	}

	tVec3f tRigidBody::fComputeImpulseToChangePointVel( f32 dt, const tVec3f& worldPoint, const tVec3f& deltaV )
	{
		tVec3f deltaVDir = deltaV;
		f32 deltaVMag = 0.0f;
		deltaVDir.fNormalizeSafe( deltaVMag );
		return fComputeImpulseToChangePointVel( dt, worldPoint, deltaVDir, deltaVMag );
	}

	tVec3f tRigidBody::fComputeImpulseToChangeRelativePointVel( f32 dt, const tVec3f& worldPoint, const tVec3f& direction, float magnitude, const tRigidBody& otherBody )
	{
		tVec3f offsetA = worldPoint - mTransform.fGetTranslation( );
		tVec3f offsetCrossNA = offsetA.fCross(direction);
		offsetCrossNA *= offsetCrossNA;
		offsetCrossNA = mWorldInertiaInv.fXformVector( offsetCrossNA );
		float offsetCrossNSquaredA = offsetCrossNA.fLength( );

		tVec3f offsetB = worldPoint - otherBody.fTransform( ).fGetTranslation( );
		tVec3f offsetCrossNB = offsetB.fCross(direction);
		offsetCrossNB *= offsetCrossNB;
		offsetCrossNB = otherBody.fWorldInertiaInv( ).fXformVector( offsetCrossNB );
		float offsetCrossNSquaredB = offsetCrossNB.fLength( );

		//j =  	        magnitude
		//     ----------------------------------------------
		//	  1/ma + 1/mb + (rap x direction)2 / Ia + (rbp x direction)2 / Ib

		f32 denominator = 1 / mMass + 1 / otherBody.fMass( ) + offsetCrossNSquaredA + offsetCrossNSquaredB;
		f32 j = magnitude / denominator;

		tVec3f impulse = direction * j;
		return impulse;
	}

	tVec3f tRigidBody::fComputeImpulseToChangeRelativePointVel( f32 dt, const tVec3f& worldPoint, const tVec3f& deltaV, const tRigidBody& otherBody )
	{
		tVec3f deltaVDir = deltaV;
		f32 deltaVMag = 0.0f;
		deltaVDir.fNormalizeSafe( deltaVMag );
		return fComputeImpulseToChangeRelativePointVel( dt, worldPoint, deltaVDir, deltaVMag, otherBody );
	}




	void tRigidBodyLight::fReset( const tMat3f& tm
		, const tVec3f& velocity, const tQuatf& drdt, f32 gravity )
	{
		mP = tm.fGetTranslation( );
		mR = tQuatf( tm );
		mR.fNormalizeSafe( tQuatf::cIdentity );
		mV = velocity;
		mDRDT = drdt;
		mGravity = gravity;
	}

	void tRigidBodyLight::fReset( const tMat3f& tm )
	{
		mP = tm.fGetTranslation( );
		mR = tQuatf( tm );
		mR.fNormalizeSafe( tQuatf::cIdentity );
	}

	void tRigidBodyLight::fIntegrate( f32 dt )
	{
		//linear velocity and forces
		mV.y += mGravity * dt;
		mP += mV * dt;

		//angular velocity and torques
		mR += mDRDT * mR * dt;
		mR.fNormalizeSafe( tQuatf::cIdentity );
	}

}}

namespace Sig { namespace Physics
{

	//void tRigidBody::fExportScriptInterface( tScriptVm& vm )
	//{
	//	Sqrat::DerivedClass< tRigidBody, tStandardPhysics, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
	//	vm.fNamespace(_SC("Physics")).Bind(_SC("RigidBody"), classDesc);
	//}

} }