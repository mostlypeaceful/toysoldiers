#ifndef __tRigidBody__
#define __tRigidBody__

#include "tPhysicsBody.hpp"
#include "tConstraint.hpp"

namespace Sig { namespace Physics
{
	class tContactPoint;

	// a little helper for applying a collision delta.
	struct tDelta
	{
		Math::tVec3f mTranslation; // remove penetration
		Math::tVec3f mDeltaV;
		Math::tVec3f mDeltaW;

		tDelta( )
			: mTranslation( Math::tVec3f::cZeroVector )
			, mDeltaV( Math::tVec3f::cZeroVector )
			, mDeltaW( Math::tVec3f::cZeroVector )
		{ }

		tDelta& operator += ( const tDelta& right )
		{
			mTranslation += right.mTranslation;
			mDeltaV += right.mDeltaV;
			mDeltaW += right.mDeltaW;
			return *this;
		}
	};

	struct tCollisionFeedback
	{
		b32 mCollisionHandled;
		tDelta mTotalDelta;

		void fReset( )
		{
			mCollisionHandled = false;
			mTotalDelta = tDelta( );
		}
	};

	struct tSleepingParams
	{
		f32 mWLenSqrd; // angular velocity thresh squared.
		f32 mVLenSqrd; // angular velocity thresh squared.

		// Time body must be at rest before sleeping.
		//  Set this to negative to force body into a sleeping state.
		//  Used by tRagdoll overlords.
		f32 mIdleTime; 

		tSleepingParams( );

		//for debugging only
		void fApplyOverride( );
	};

	class base_export tRigidBody : public tPhysicsBody
	{
		debug_watch( tRigidBody );
		declare_uncopyable( tRigidBody );
		define_dynamic_cast( tRigidBody, tPhysicsBody );

	public:
		tRigidBody( );

		void					fReset( const Math::tMat3f& tm = Math::tMat3f::cIdentity);
		void					fSetTransform( const Math::tMat3f& tm ); //call from fPhysicsMT

	public:
		f32						fGravity( ) const { return mGravity; }
		void					fSetGravity( f32 g ) { mGravity = g; }
		
		void					fSetMassProperties( Math::tVec3f halfExtents, f32 mass, f32 inertiaScale );
		void					fSetMassPropertiesFromShape( f32 mass, f32 inertiaScale );
		void					fSetInertiaInv( const Math::tVec3f& iInv ) { mInertiaInv = iInv; fRecomputeIntertiaMatrix( ); }
		f32						fMassInv( ) const { return mMassInv; }
		f32						fMass( ) const { sigassert (mMassInv > 0.f ); return 1.f / mMassInv; }

		void					fSetFriction( f32 friction ) { mCoefFriction = friction; }
		f32						fFriction( ) const { return mCoefFriction; }

		void					fSetRestitution( f32 restitution ) { mCoefRestitution = restitution; }
		f32						fRestitution( ) const { return mCoefRestitution; }

		void					fSetLinearDamping( f32 d );
		void					fSetAngularDamping( const Math::tVec3f& ad );
		
		const Math::tMat3f&		fWorldInertiaInv( ) const { return mWorldInertiaInv; }

		void					fAddForce( const Math::tVec3f& worldForce, const Math::tVec3f& worldPoint ); //This function just calls the next
		void					fAddTorque( const Math::tVec3f& worldTorque );
		void					fAddImpulse( const Math::tVec3f& worldImpulse, const Math::tVec3f& worldPoint );
		void					fAddImpulseAsForce( const Math::tVec3f& worldImpulse, const Math::tVec3f& worldPoint, f32 dt );
		void					fComputeDeltaFromImpulse( const Math::tVec3f &worldImp, const Math::tVec3f &worldPoint, Math::tVec3f& v, Math::tVec3f& w );
		void					fAddAngularImpulse( const Math::tVec3f& worldWImpulse );

		void					fSetVelocity( const Math::tVec3f& v ) { sigassert( !v.fIsNan( ) ); mV = v; }
		const Math::tVec3f&		fVelocity( ) const { return mV; }
		virtual Math::tVec3f    fPointVelocity( const Math::tVec3f& worldPoint ) const;
		Math::tVec3f			fPointVelocityWorldOffset( const Math::tVec3f& worldOffset ) const;

		void					fSetAngularVelocity( const Math::tVec3f& w ) { sigassert( !w.fIsNan( ) ); mW = w; }
		const Math::tVec3f&		fAngularVelocity( ) const { return mW; }
		
		f32						fComputeImpulseDenominator( const Math::tVec3f& worldOffset, const Math::tVec3f& direction ) const;

		f32						fComputeImpulseToChangePointVel( f32 dt, const Math::tVec3f& worldPoint, const Math::tVec3f& direction, float magnitude );
		Math::tVec3f			fComputeImpulseToChangePointVel( f32 dt, const Math::tVec3f& worldPoint, const Math::tVec3f& deltaV );

		f32						fComputeImpulseToChangeRelativePointVel( f32 dt, const Math::tVec3f& worldPointa, const Math::tVec3f& worldPointb, const Math::tVec3f& direction, float magnitude, const tRigidBody& otherBody );
		Math::tVec3f			fComputeImpulseToChangeRelativePointVel( f32 dt, const Math::tVec3f& worldPointa, const Math::tVec3f& worldPointb, const Math::tVec3f& deltaV, const tRigidBody& otherBody );
		
		Math::tVec3f			fOneBodyChangeNormalVelAndFrictionImpulse( f32 dt, f32 deltaNormalV, const Math::tVec3f& normal, const Math::tVec3f& worldPt, Math::tVec3f tangentV );
		Math::tVec3f			fTwoBodyChangeNormalVelAndFrictionImpulse( f32 dt, f32 deltaNormalV, const Math::tVec3f& normal, const Math::tVec3f& worldPt, const Math::tVec3f& worldPt2, tRigidBody& body2, Math::tVec3f tangentV );

		Math::tVec3f			fOneBodyResolve( f32 dt, const tContactPoint& cp, f32 percentage = 1.f );
		Math::tVec3f			fTwoBodyResolve( f32 dt, const tContactPoint& cp, tRigidBody& otherbody, f32 percentage = 1.f );

		// Called by the physics engine
		void fPreCollide( f32 dt );
		void fPostCollide( f32 dt );

		const tCollisionFeedback& fGetFeedback( ) const { return mFeedback; }
		tSleepingParams& fSleepingParams( ) { return mSleepingParams; }
		void fSleepNow( );

		static Math::tVec3f fWFromDeltaR( const Math::tQuatf& startR, const Math::tQuatf& endR, f32 dt );

	public:
		Math::tVec3f mF; // force accumulator
		Math::tVec3f mV; // linear vel
		Math::tVec3f mPseudoV; //gets added included with v for one frame and then zeroed. put all position resolution here.
		Math::tVec3f mT; // torque accumulator
		Math::tVec3f mW; // ang velocity vector

		f32          mGravity;

		void fDebugDraw( tPhysicsWorld& world );

		void fAddConstraint( tConstraint* constraint );
		void fRemoveConstraint( tConstraint* constraint );
		b32 fConstrainedTo( tPhysicsBody* body ) const;

	protected:
		Math::tVec3f	mInertiaInv; // local inertia scalar
		Math::tMat3f	mWorldInertiaInv; 

		f32				mMassInv;
		f32				mCoefRestitution;
		f32				mCoefFriction;
		f32				mLinearDamp;
		Math::tVec3f	mAngDamping;
		tSleepingParams mSleepingParams;

		// contact information
		tCollisionFeedback	mFeedback;
		tGrowableArray<tRefCounterPtr<tConstraint>> mConstraints;

		virtual void fSetWorld( tPhysicsWorld* world );
		void fRecomputeIntertiaMatrix( );
		b32  fCheckSleeping( );
	};

	typedef tRefCounterPtr< tRigidBody > tRigidBodyPtr;


	// This preserves basic a linear and angular simulation without the baggage of a full physics system.
	class base_export tRigidBodyLight
	{
	public:
		tRigidBodyLight( )
			: mP( Math::tVec3f::cZeroVector )
			, mV( Math::tVec3f::cZeroVector )
			, mR( Math::tQuatf::cIdentity )
			, mDRDT( Math::tQuatf::cIdentity )
			, mGravity( 0.f )
		{ }

		void fReset( const Math::tMat3f& tm
			, const Math::tVec3f& velocity
			, const Math::tQuatf& drdt
			, f32 gravity );

		void fReset( const Math::tMat3f& tm );
		
		f32						fGravity( ) const { return mGravity; }
		void					fSetGravity( f32 g ) { mGravity = g; }

		const Math::tVec3f&		fPosition( ) const { return mP; }	
		Math::tVec3f&			fPosition( ) { return mP; }		

		const Math::tVec3f&		fVelocity( ) const { return mV; }	
		Math::tVec3f&			fVelocity( ) { return mV; }	

		const Math::tQuatf&		fDRDT( ) const { return mDRDT; }	
		Math::tQuatf&			fDRDT( ) { return mDRDT; }	

		void					fIntegrate( f32 dt );

		Math::tMat3f			fTransform( ) const { return Math::tMat3f( mR, mP ); }

	//protected:
		Math::tVec3f mP;
		Math::tVec3f mV;
		Math::tQuatf mR;
		Math::tQuatf mDRDT;
		f32          mGravity;
	};

}}

#endif//__tRigidBody__
