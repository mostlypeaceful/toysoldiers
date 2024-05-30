#ifndef __tPinConstraint__
#define __tPinConstraint__

#include "tConstraint.hpp"

namespace Sig { namespace Physics
{

	class base_export tPinConstraint : public tConstraint
	{
		define_dynamic_cast( tPinConstraint, tConstraint );
	public:
		tPinConstraint( tRigidBody* ownerA, tRigidBody* ownerB, const Math::tMat3f& aRelConstraintPt );

		virtual void fStepConstraintInternal( f32 dt, f32 percentage );
		virtual void fDebugDraw( tPhysicsWorld& world );

		Math::tVec3f mIntegral;
	};

	typedef tRefCounterPtr<tPinConstraint> tPinConstraintPtr;

}}

#endif//__tPinConstraint__
