#include "BasePch.hpp"
#include "tRigidBody.hpp"
#include "tContactPoint.hpp"
#include "tPhysicsWorld.hpp"

using namespace Sig::Math;


namespace Sig { namespace Physics
{

	devvar_clamp( f32, Physics_RigidBody_PenetrationLerp, 1.0f, 0.0f, 1.f, 2 );
	devvar( bool, Physics_RigidBody_Override, false );
	devvar_clamp( f32, Physics_RigidBody_AngDamp, 0.99f, 0.0f, 1.f, 2 );
	devvar_clamp( f32, Physics_RigidBody_LinDamp, 0.99f, 0.0f, 1.f, 2 );
	devvar_clamp( f32, Physics_RigidBody_Friction, 0.5f, 0.0f, 1.f, 2 );
	devvar_clamp( f32, Physics_RigidBody_Restitution, 0.0f, 0.0f, 1.f, 2 );
	

	devvar( bool, Physics_RigidBody_Sleeping_Override, false );
	devvar( f32, Physics_RigidBody_Sleeping_VThresh, 0.6f );
	devvar( f32, Physics_RigidBody_Sleeping_WThresh, 0.75f );
	devvar( f32, Physics_RigidBody_Sleeping_TimeThresh, 1.0f );

	devvar( bool, Physics_RigidBody_ApplyFriction, true );
	devvar( bool, Physics_StepBodies, true );

	devvar( bool, Physics_Translate, false );
	devvar( f32, Physics_TranslateM, 0.0f );
	devvar( f32, Physics_TranslateA, 0.4f );

	tSleepingParams::tSleepingParams( )
		: mWLenSqrd( Physics_RigidBody_Sleeping_WThresh * Physics_RigidBody_Sleeping_WThresh )
		, mVLenSqrd( Physics_RigidBody_Sleeping_VThresh * Physics_RigidBody_Sleeping_VThresh )
		, mIdleTime( Physics_RigidBody_Sleeping_TimeThresh )
	{ }

	void tSleepingParams::fApplyOverride( )
	{
		if( Physics_RigidBody_Sleeping_Override )
		{
			mVLenSqrd = Physics_RigidBody_Sleeping_VThresh * Physics_RigidBody_Sleeping_VThresh;
			mWLenSqrd = Physics_RigidBody_Sleeping_WThresh * Physics_RigidBody_Sleeping_WThresh;
			mIdleTime = Physics_RigidBody_Sleeping_TimeThresh;
		}
	}

	tRigidBody::tRigidBody( ) 
		: tPhysicsBody( cPhysicsObjectTypeRigid )
		, mF( Math::tVec3f::cZeroVector )
		, mV( Math::tVec3f::cZeroVector )
		, mPseudoV( Math::tVec3f::cZeroVector )
		, mT( Math::tVec3f::cZeroVector )
		, mW( Math::tVec3f::cZeroVector )
		, mGravity( -11.8f )
		, mInertiaInv( Math::tVec3f::cOnesVector )
		, mWorldInertiaInv( Math::tVec3f::cZeroVector )
		, mMassInv( 1.0f )
		, mCoefRestitution( Physics_RigidBody_Restitution )
		, mCoefFriction( Physics_RigidBody_Friction )
		, mLinearDamp( Physics_RigidBody_LinDamp )
		, mAngDamping( Physics_RigidBody_AngDamp )
	{
		fReset( );
	}

	void tRigidBody::fSetWorld( tPhysicsWorld* world )
	{
		tPhysicsBody::fSetWorld( world );

		if( mInWorld && !mIslandData.fHasContactIsland( ) )
			mIslandData.fIsolateContactIsland( *this );
	}

	void tRigidBody::fReset( const tMat3f& tm )
	{
		fSetTransform( tm );

		mF = tVec3f::cZeroVector;
		mV = tVec3f::cZeroVector;
		mPseudoV = tVec3f::cZeroVector;
		mT = tVec3f::cZeroVector;
		mW = tVec3f::cZeroVector;

		mIslandData.fAwaken( );

		fClearManifolds( );
	}

	void tRigidBody::fSetTransform( const tMat3f& tm )
	{
		tPhysicsBody::fSetTransform( tm );
		fRecomputeIntertiaMatrix( );
	}

	void tRigidBody::fRecomputeIntertiaMatrix( )
	{
		mWorldInertiaInv = fTransform( ) * tMat3f( mInertiaInv ) * fInvTransform( );
	}
	
	void tRigidBody::fSetMassProperties( tVec3f halfExtents, f32 mass, f32 inertiaScale )
	{
		sigassert( mass > 0.f );
		mMassInv = 1.f / mass;

		// Inertia
		halfExtents *= 2.0f;
		mInertiaInv = tVec3f( pow(halfExtents.y, 2) + pow(halfExtents.z, 2)
		                 , pow(halfExtents.x, 2) + pow(halfExtents.z, 2)
		                 , pow(halfExtents.x, 2) + pow(halfExtents.y, 2) );

		inertiaScale *= 2.f; //hack, for stability
		mInertiaInv *= (inertiaScale/12.0f) * mass;
		mInertiaInv = 1.f / mInertiaInv;

		fRecomputeIntertiaMatrix( );
	}

	void tRigidBody::fSetMassPropertiesFromShape( f32 mass, f32 inertiaScale )
	{
		fSetMassProperties( fLocalAABB( ).fComputeDiagonal( ) * 0.5f, mass, inertiaScale );
	}

	void tRigidBody::fPreCollide( f32 dt )
	{
		fUpdateBroadphaseSleeping( );

		if( mIslandData.fSleeping( ) )
			return;

		for( s32 i = mManifolds.fCount( ) - 1; i >= 0; --i )
			mManifolds[ i ]->fRemoveOldContacts( );

		if( Physics_RigidBody_Override )
		{
			mAngDamping = tVec3f( (f32)Physics_RigidBody_AngDamp );
			mLinearDamp = Physics_RigidBody_LinDamp;
			mCoefFriction = Physics_RigidBody_Friction;
			mCoefRestitution = Physics_RigidBody_Restitution;
		}

		//linear velocity and forces
		tVec3f a = mF * mMassInv;
		mV += a * dt;
		mV *= mLinearDamp;
		mV.y += mGravity * dt;
		mF = tVec3f::cZeroVector;

		//angular velocity and torques
		mW += mT * dt;
		mT = tVec3f::cZeroVector;

		//tMat3f angDamp = tMat3f(r) * tMat3f(mAngDamping) * tMat3f(r.fInverse());
		//mW = angDamp.fXformVector(mW);
		mW *= Physics_RigidBody_AngDamp;

		mFeedback.fReset( );	

		// preupdate the broadphase so it includes both end points.
		fUpdateBroadphase( mV * dt );
	}

	void tRigidBody::fPostCollide( f32 dt )
	{		
		if( mIslandData.fSleeping( ) )
			return;

		if( Physics_StepBodies )
		{
			// update location
			tVec3f p = fTransform( ).fGetTranslation( );
			p += (mV + mPseudoV) * dt;

			tQuatf r = tQuatf( fTransform( ) );
			r.fNormalizeSafe( tQuatf::cIdentity );
			tQuatf w(mW.x, mW.y, mW.z, 0);
			tQuatf spin = 0.5f * w * r;		
			r += spin * dt;
			r.fNormalizeSafe( tQuatf::cIdentity );
			tPhysicsBody::fSetTransform( tMat3f(r, p) );

			mPseudoV = tVec3f::cZeroVector;

			fRecomputeIntertiaMatrix( );
		}

		if( fCheckSleeping( ) )
			mIslandData.mIdleTime += dt;
		else
			mIslandData.mIdleTime = 0.f;

		if( mIslandData.mIdleTime < mSleepingParams.mIdleTime )
			mIslandData.fAwaken( );
	}

	b32 tRigidBody::fCheckSleeping( )
	{
		// sleeping
		f32 vLenSqrd = mV.fLengthSquared( );
		f32 wLenSwrd = mW.fLengthSquared( );
		mSleepingParams.fApplyOverride( );

		return (vLenSqrd < mSleepingParams.mVLenSqrd && wLenSwrd < mSleepingParams.mWLenSqrd);
	}

	void tRigidBody::fSleepNow( )
	{
		mIslandData.mIdleTime = mSleepingParams.mIdleTime;
	}

	void tRigidBody::fAddConstraint( tConstraint* constraint ) 
	{ 
		sigassert( mConstraints.fIndexOf( constraint ) == ~0 );
		mConstraints.fPushBack( tConstraintPtr( constraint ) ); 
	}

	void tRigidBody::fRemoveConstraint( tConstraint* constraint ) 
	{ 
		b32 found = mConstraints.fFindAndErase( constraint ); 
		sigassert( found );
	}

	b32 tRigidBody::fConstrainedTo( tPhysicsBody* body ) const
	{
		sigassert( body != this && "Really?" );
		for( u32 i = 0; i < mConstraints.fCount( ); ++i )
			if( mConstraints[ i ]->fOtherBody( this ) == body )
				return true;
		return false;
	}



	/// Really boring shit from here down, equations.





	tVec3f tRigidBody::fWFromDeltaR( const tQuatf& startR, const tQuatf& endR, f32 dt )
	{
		tQuatf spin = endR - startR;
		spin /= dt;
		spin *= 2.f;
		spin = startR.fInverse( ) * spin;

		return tVec3f( spin.x, spin.y, spin.z );
	}
	
	void tRigidBody::fAddForce( const tVec3f &worldForce, const tVec3f &worldPoint )
	{
		mF += worldForce;

		tVec3f offset = worldPoint - fTransform( ).fGetTranslation( );
		tVec3f torque = offset.fCross( worldForce );
		fAddTorque( torque );
	}

	void tRigidBody::fAddTorque( const tVec3f &worldTorque )
	{
		mT += mWorldInertiaInv.fXformVector( worldTorque );
	}

	void tRigidBody::fAddImpulse( const tVec3f &worldImp, const tVec3f &worldPoint )
	{
		fComputeDeltaFromImpulse( worldImp, worldPoint, mV, mW );
	}

	void tRigidBody::fAddImpulseAsForce( const Math::tVec3f& worldImpulse, const Math::tVec3f& worldPoint, f32 dt )
	{
		fAddForce( worldImpulse / dt, worldPoint ); 
	}

	void tRigidBody::fComputeDeltaFromImpulse( const tVec3f &worldImp, const tVec3f &worldPoint, tVec3f& v, tVec3f& w )
	{
		tVec3f offset = worldPoint - fTransform( ).fGetTranslation( );
		v += worldImp * mMassInv;
		w += mWorldInertiaInv.fXformVector( offset.fCross( worldImp ) );
	}

	void tRigidBody::fAddAngularImpulse( const Math::tVec3f& worldWImpulse )
	{
		mW += mWorldInertiaInv.fXformVector( worldWImpulse );	
	}

	tVec3f tRigidBody::fPointVelocity( const tVec3f& worldPoint ) const
	{
		tVec3f offset = worldPoint - fTransform( ).fGetTranslation( );
		return fPointVelocityWorldOffset( offset ); 
	}

	tVec3f tRigidBody::fPointVelocityWorldOffset( const tVec3f& worldOffset ) const
	{
		tVec3f result = mV;
		result += mW.fCross( worldOffset );
		return result; 
	}

	//tVec3f tRigidBody::fPointMomentum( const tVec3f& worldPoint ) const
	//{
	//	tVec3f angularVAtPoint = mW.fCross( worldPoint - fTransform( ).fGetTranslation( ) );
	//	//return mV * mMass + mWorldInertiaInv.fInverse().fXformVector( angularVAtPoint );
	//	return (mV + angularVAtPoint) * mMass;
	//}

	void tRigidBody::fSetLinearDamping( f32 d )
	{
		mLinearDamp = d;
	}

	void tRigidBody::fSetAngularDamping( const tVec3f& ad )
	{
		mAngDamping = ad;
	}

	f32 tRigidBody::fComputeImpulseDenominator( const tVec3f& worldOffset, const tVec3f& direction ) const
	{
		//j =  	        magnitude
		//     --------------------------
		//       1/ma + Dot( N, ((rAp x direction) / Ia) x rAp)

		tVec3f offsetCrossNA = worldOffset.fCross( direction );
		offsetCrossNA = mWorldInertiaInv.fXformVector( offsetCrossNA );
		offsetCrossNA = offsetCrossNA.fCross( worldOffset );
		f32 offsetCrossNSquaredA = offsetCrossNA.fDot( direction );

		return offsetCrossNSquaredA + mMassInv;
	}

	f32 tRigidBody::fComputeImpulseToChangePointVel( f32 dt, const tVec3f& worldPoint, const tVec3f& direction, float magnitude )
	{
		tVec3f offset = worldPoint - fTransform( ).fGetTranslation( );
		f32 denominator = fComputeImpulseDenominator( offset, direction );
		f32 j = magnitude / denominator;
		return j;
	}

	tVec3f tRigidBody::fComputeImpulseToChangePointVel( f32 dt, const tVec3f& worldPoint, const tVec3f& deltaV )
	{
		tVec3f deltaVDir = deltaV;
		f32 deltaVMag = 0.0f;
		deltaVDir.fNormalizeSafe( deltaVMag );
		return deltaVDir * fComputeImpulseToChangePointVel( dt, worldPoint, deltaVDir, deltaVMag );
	}

	f32 tRigidBody::fComputeImpulseToChangeRelativePointVel( f32 dt, const tVec3f& worldPointa, const tVec3f& worldPointb, const tVec3f& direction, float magnitude, const tRigidBody& otherBody )
	{
		tVec3f offsetA = worldPointa - fTransform( ).fGetTranslation( );
		tVec3f offsetB = worldPointb - otherBody.fTransform( ).fGetTranslation( );

		//j =  	        magnitude
		//     ----------------------------------------------
		//	  1/ma + 1/mb + Dot(N, ((rap x direction) / Ia) x rap) + Dot(N, ((rbp x direction)2 / Ib) / rbp)

		f32 denominator = fComputeImpulseDenominator( offsetA, direction ) + otherBody.fComputeImpulseDenominator( offsetB, direction );
		f32 j = magnitude / denominator;

		return j;
	}

	tVec3f tRigidBody::fComputeImpulseToChangeRelativePointVel( f32 dt, const tVec3f& worldPointa, const tVec3f& worldPointb, const tVec3f& deltaV, const tRigidBody& otherBody )
	{
		tVec3f deltaVDir = deltaV;
		f32 deltaVMag = 0.0f;
		deltaVDir.fNormalizeSafe( deltaVMag );
		return deltaVDir * fComputeImpulseToChangeRelativePointVel( dt, worldPointa, worldPointb, deltaVDir, deltaVMag, otherBody );
	}

	Math::tVec3f tRigidBody::fOneBodyChangeNormalVelAndFrictionImpulse( f32 dt, f32 deltaNormalV, const Math::tVec3f& normal, const Math::tVec3f& worldPt, Math::tVec3f tangentV )
	{
		tVec3f impulse = normal * fComputeImpulseToChangePointVel( dt, worldPt, normal, deltaNormalV );

		// Friction
		f32 tangentVLenSqr = tangentV.fLengthSquared( );

		if( tangentVLenSqr > 0.001f && mCoefFriction > 0.f )
		{
			f32 tangentVMag = Math::fSqrt( tangentVLenSqr );
			tangentV /= tangentVMag; // normalized

			tVec3f fricImpulse = tangentV * fComputeImpulseToChangePointVel( dt, worldPt, tangentV, -tangentVMag );

			// clamp it
			f32 impulseMax = impulse.fLength( ) * mCoefFriction;
			fricImpulse.fClampLength( impulseMax );

			if( Physics_RigidBody_ApplyFriction )
				impulse += fricImpulse;
		}

		return impulse;
	}

	Math::tVec3f tRigidBody::fOneBodyResolve( f32 dt, const tContactPoint& cp, f32 percentage )
	{
		if( !Physics_StepBodies )
			return tVec3f::cZeroVector;

		// Compute resolution
		tVec3f impulse = tVec3f::cZeroVector;

		tVec3f pointV = fPointVelocity( cp.mPoint );
		f32 normalVel = pointV.fDot( cp.mNormal );
		f32 deltaV = -(1 + mCoefRestitution) * normalVel;


		if( Physics_Translate )
		{
			tVec3f displace = cp.mNormal * cp.mDepth * Physics_RigidBody_PenetrationLerp * percentage;
			fTranslate( displace );
		}
		else
			deltaV += (Physics_TranslateM * (-cp.mDepth + Physics_TranslateA));

		if( deltaV > 0.01f )
		{
			// Collision
			tVec3f tangentV = pointV + cp.mNormal * normalVel;
			impulse = fOneBodyChangeNormalVelAndFrictionImpulse( dt, deltaV, cp.mNormal, cp.mPoint, tangentV );
		}

		return impulse;
	}

	Math::tVec3f tRigidBody::fTwoBodyChangeNormalVelAndFrictionImpulse( f32 dt, f32 deltaNormalV, const Math::tVec3f& normal, const Math::tVec3f& worldPt, const Math::tVec3f& worldPt2, tRigidBody& body2, Math::tVec3f tangentV )
	{
		tVec3f impulse = normal * fComputeImpulseToChangeRelativePointVel( dt, worldPt, worldPt2, normal, deltaNormalV, body2 );

		// Friction
		f32 tangentVLenSqr = tangentV.fLengthSquared( );

		if( tangentVLenSqr > 0.001f )
		{
			f32 tangentVMag = Math::fSqrt( tangentVLenSqr );
			tangentV /= tangentVMag; // normalized

			tVec3f fricImpulse = tangentV * fComputeImpulseToChangeRelativePointVel( dt, worldPt, worldPt2, tangentV, -tangentVMag, body2 );

			// clamp it
			f32 impulseMax = impulse.fLength( ) * mCoefFriction;
			fricImpulse.fClampLength( impulseMax );

			if( Physics_RigidBody_ApplyFriction )
				impulse += fricImpulse;
		}

		return impulse;
	}

	Math::tVec3f tRigidBody::fTwoBodyResolve( f32 dt, const tContactPoint& cp, tRigidBody& otherbody, f32 percentage )
	{
		if( !Physics_StepBodies )
			return tVec3f::cZeroVector;

		tVec3f ptB = cp.fOtherBodyPt( );

		// Compute resolution
		tVec3f impulse = tVec3f::cZeroVector;
		tVec3f displace = cp.mNormal * cp.mDepth * Physics_RigidBody_PenetrationLerp * 0.5f * percentage;

		fTranslate( displace );
		otherbody.fTranslate( -displace );

		tVec3f pointV = fPointVelocity( cp.mPoint ) - otherbody.fPointVelocity( ptB );
		f32 normalVel = pointV.fDot( cp.mNormal );
		if( normalVel < 0.f )
		{
			// Collision
			f32 deltaV = -(1 + mCoefRestitution) * normalVel;
			tVec3f tangentV = pointV + cp.mNormal * normalVel;
			impulse = fTwoBodyChangeNormalVelAndFrictionImpulse( dt, deltaV, cp.mNormal, cp.mPoint, ptB, otherbody, tangentV );
		}

		return impulse;
	}

	void tRigidBody::fDebugDraw( tPhysicsWorld& world )
	{
		world.fDebugGeometry( ).fRenderOnce( fTransform( ), 1.5f, 1.f );
		tPhysicsBody::fDebugDraw( world );
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
