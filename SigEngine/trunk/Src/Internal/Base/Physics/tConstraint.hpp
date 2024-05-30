#ifndef __tConstraint__
#define __tConstraint__

#include "tPhysicsObject.hpp"

namespace Sig { namespace Physics
{
	class tPhysicsBody;
	class tRigidBody;
	class tPhysicsWorld;

	class base_export tConstraint : public tPhysicsObject
	{
		debug_watch( tConstraint );
		declare_uncopyable( tConstraint );
		define_dynamic_cast( tConstraint, tPhysicsObject );
	public:
		tConstraint( );
		tConstraint( tRigidBody* ownerA, tRigidBody* ownerB, const Math::tMat3f& aRelConstraintPt );
		
		tPhysicsBody* fOtherBody( const tPhysicsBody* b ) const;

		void fStepST( f32 dt, f32 percentage );
		virtual void fDebugDraw( tPhysicsWorld& world ) { }

		tRigidBody* fBodyA( ) const { return mBodyA.fGetRawPtr( ); }
		tRigidBody* fBodyB( ) const { return mBodyB.fGetRawPtr( ); }

		const Math::tMat3f& fAnchorPointA( ) const { return mAAnchorPt; }
		const Math::tMat3f& fAnchorPointB( ) const { return mBAnchorPt; }

		virtual void fSetWorld( tPhysicsWorld* world );

		virtual void fStepConstraintInternal( f32 dt, f32 percentage ) { }

	protected:
		tRefCounterPtr<tRigidBody> mBodyA;
		tRefCounterPtr<tRigidBody> mBodyB;
		Math::tMat3f mAAnchorPt;
		Math::tMat3f mBAnchorPt;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr<tConstraint> tConstraintPtr;

}}

#endif//__tConstraint__
