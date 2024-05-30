#include "BasePch.hpp"
#include "tPinConstraint.hpp"
#include "tRigidBody.hpp"
#include "tSceneGraph.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	devvar( f32, Physics_PinConstraint_PositionMix, 12.0f ); //this is unnecessarily high to combat worldly accelerations like gravity which are not taken into account yet.
	devvar( bool, Physics_PinConstraint_DrawError, false );
	
	tPinConstraint::tPinConstraint( tEntity* ownerA, tEntity* ownerB, const Math::tVec3f& aRelConstraintPt )
		: tConstraint( ownerA, ownerB, aRelConstraintPt )
	{
	}

	void tPinConstraint::fStepST( f32 dt )
	{
		tVec3f worldPtA = mBodyA->fTransform( ).fXformPoint( mARelativePt );
		tVec3f worldPtB = mBodyB->fTransform( ).fXformPoint( mBRelativePt );

		tVec3f velDiff = mBodyA->fPointVelocity( worldPtA ) - mBodyB->fPointVelocity( worldPtB ); 
		velDiff *= 0.5f;

		tVec3f sep = worldPtA - worldPtB;
		velDiff += sep * Physics_PinConstraint_PositionMix;

		tVec3f impulseA = mBodyA->fComputeImpulseToChangePointVel( dt, worldPtA, -velDiff );
		tVec3f impulseB = mBodyB->fComputeImpulseToChangePointVel( dt, worldPtB, velDiff );

		mBodyA->fAddImpulse( impulseA, worldPtA );
		mBodyB->fAddImpulse( impulseB, worldPtB );

		if( Physics_PinConstraint_DrawError )
			mOwnerA->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( worldPtA, worldPtB, tVec4f(1,0,0,1) );
	}

	void tPinConstraint::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tPinConstraint, tConstraint, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			;

		vm.fNamespace(_SC("Physics")).Bind(_SC("PinConstraint"), classDesc);
	}
}}