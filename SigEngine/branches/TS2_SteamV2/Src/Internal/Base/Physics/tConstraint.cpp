#include "BasePch.hpp"
#include "tConstraint.hpp"
#include "tRigidBody.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	tConstraint::tConstraint( tEntity* ownerA, tEntity* ownerB, const Math::tVec3f& aRelConstraintPt )
		: mOwnerA( ownerA )
		, mOwnerB( ownerB )
		, mARelativePt( aRelConstraintPt )
	{
		sigassert( ownerA->fLogic( ) && ownerB->fLogic( ) );

		mBodyA = ownerA->fLogic( )->fQueryPhysicalDerived<tRigidBody>( );
		mBodyB = ownerB->fLogic( )->fQueryPhysicalDerived<tRigidBody>( );
		sigassert( mBodyA && mBodyB );

		tVec3f worldConstraintPt = mBodyA->fTransform( ).fXformPoint( mARelativePt );
		mBRelativePt = mBodyB->fTransform( ).fInverse( ).fXformPoint( worldConstraintPt );

		mBodyA->fAddConstraint( this );
		mBodyB->fAddConstraint( this );
	}

	tConstraint::~tConstraint( )
	{
		mBodyA->fRemoveConstraint( this );
		mBodyB->fRemoveConstraint( this );
	}

	void tConstraint::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tConstraint, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			;

		vm.fNamespace(_SC("Physics")).Bind(_SC("Constraint"), classDesc);
	}
}}