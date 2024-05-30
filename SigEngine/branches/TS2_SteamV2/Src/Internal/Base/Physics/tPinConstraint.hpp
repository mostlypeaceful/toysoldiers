#ifndef __tPinConstraint__
#define __tPinConstraint__

#include "tConstraint.hpp"

namespace Sig { namespace Physics
{

	class tPinConstraint : public tConstraint
	{
		define_dynamic_cast( tPinConstraint, tConstraint );
	public:
		tPinConstraint( tEntity* ownerA, tEntity* ownerB, const Math::tVec3f& aRelConstraintPt );

		void fStepST( f32 dt );

		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr<tPinConstraint> tPinConstraintPtr;

}}

#endif//__tPinConstraint__
