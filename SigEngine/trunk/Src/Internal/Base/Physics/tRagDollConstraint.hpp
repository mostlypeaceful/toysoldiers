#ifndef __tRagDollConstraint__
#define __tRagDollConstraint__

#include "tPinConstraint.hpp"

namespace Sig { namespace Physics
{

	class tRagDollConstraint : public tPinConstraint
	{
		define_dynamic_cast( tRagDollConstraint, tPinConstraint );
	public:
		tRagDollConstraint( tRigidBody* ownerA, tRigidBody* ownerB, const Math::tMat3f& aRelConstraintPt );

		virtual void fStepConstraintInternal( f32 dt, f32 percentage );

		void fSetDesiredAnimOrient( const Math::tQuatf& orient );
		void fSetChildWeightScale( f32 scale ) { mChildWeightScale = scale; }
		void fSetConstraints( u32 order, const Math::tAabbf& limits ) { mAxisLimitsOrder = order; mAxisLimits = limits; }

	private:
		// Solver data, ie parameters to fStepConstraintInternal
		f32 mChildWeightScale;

		// Animation driven data
		Math::tQuatf mDesiredAnimOrient;

		// Constraint data
		u32				mAxisLimitsOrder; //~0 is un-limited
		Math::tAabbf	mAxisLimits;
	};

	typedef tRefCounterPtr<tRagDollConstraint> tRagDollConstraintPtr;

}}

#endif//__tRagDollConstraint__
