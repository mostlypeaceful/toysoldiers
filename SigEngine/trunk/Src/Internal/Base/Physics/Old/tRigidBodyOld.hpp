#ifndef __tRigidBodyOld__
#define __tRigidBodyOld__

#include "Logic/tPhysical.hpp"
#include "tConstraintOld.hpp"



namespace Sig { namespace PhysicsOld
{

	class tRigidBody : public Physics::tStandardPhysics
	{
		define_dynamic_cast( tRigidBody, Physics::tStandardPhysics );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	public:
		tRigidBody( );
		virtual ~tRigidBody( ) { }
		void fOnDelete( );

		void					fReset( const Math::tMat3f& tm = Math::tMat3f::cIdentity);
		void					fSetTransform( const Math::tMat3f& tm ); //call from fPhysicsMT
		const Math::tMat3f&		fTransform( ) const { return mTransform; }

	public:
		f32						fGravity( ) const { return mGravity; }
		void					fSetGravity( f32 g ) { mGravity = g; }
		void					fSetMassProperties( Math::tVec3f halfExtents, f32 mass, f32 inertiaScale );
		f32						fMass( ) const { return mMass; }
		void					fEnableFancyStuff( b32 enable ) { mDoFancyStuff = enable; }
		const Math::tMat3f&		fWorldInertiaInv( ) const { return mWorldInertiaInv; }

		const Math::tMat3f&		fIntegrate( f32 dt );

		void					fAddForce( const Math::tVec3f& worldForce, const Math::tVec3f& worldPoint ); //This function just calls the next
		void					fAddTorque( const Math::tVec3f& worldTorque );
		void					fAddImpulse( const Math::tVec3f& worldImpulse, const Math::tVec3f& worldPoint ); //This function just calls the next
		void					fApplyDeltaFromImpulse( const Math::tVec3f &worldImp, const Math::tVec3f &worldPoint, Math::tVec3f& v, Math::tVec3f& w );

		virtual Math::tVec3f    fPointVelocity( const Math::tVec3f& worldPoint ) const;
		//Math::tVec3f            fPointMomentum( const Math::tVec3f& worldPoint ) const; //no properly implemented

		Math::tVec3f			fComputeImpulseToChangePointVel( f32 dt, const Math::tVec3f& worldPoint, const Math::tVec3f& direction, float magnitude );
		Math::tVec3f			fComputeImpulseToChangePointVel( f32 dt, const Math::tVec3f& worldPoint, const Math::tVec3f& deltaV );

		Math::tVec3f			fComputeImpulseToChangeRelativePointVel( f32 dt, const Math::tVec3f& worldPoint, const Math::tVec3f& direction, float magnitude, const tRigidBody& otherBody );
		Math::tVec3f			fComputeImpulseToChangeRelativePointVel( f32 dt, const Math::tVec3f& worldPoint, const Math::tVec3f& deltaV, const tRigidBody& otherBody );


		void					fSetAngularDamping( const Math::tVec3f& ad );

		Math::tVec3f			fToWorld( const Math::tVec3f& localVector ) const { return mTransform.fXformVector( localVector ); }
		Math::tVec3f			fToLocal( const Math::tVec3f& worldVector ) const { return mTransform.fInverseXformVector( worldVector ); }

		Math::tVec3f mF; // force accumulator
		Math::tVec3f mW; // ang velocity vector
		Math::tVec3f mT; // torque accumulator

		Math::tVec3f mAngDamping;

		f32          mGravity;

		void fAddConstraint( tConstraint* constraint ) { mConstraints.fFindOrAdd( tConstraintPtr( constraint ) ); }
		void fRemoveConstraint( tConstraint* constraint ) { mConstraints.fFindAndErase( constraint ); }

	protected:
		Math::tVec3f mInertia; // local inertia scalar
		Math::tMat3f mWorldInertiaInv; 
		f32          mMass;
		b32			 mDoFancyStuff; //like maintain inertia matrix

		tGrowableArray<tConstraintPtr> mConstraints;

		void fRecomputeIntertiaMatrix( );
	};



	class tRigidBodyLight
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

#endif//__tRigidBodyOld__
