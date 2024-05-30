#ifndef __tConstraint__
#define __tConstraint__


namespace Sig { namespace Physics
{

	class tRigidBody;

	class tConstraint : public tRefCounter
	{
		define_dynamic_cast_base( tConstraint );
	public:
		tConstraint( tEntity* ownerA, tEntity* ownerB, const Math::tVec3f& aRelConstraintPt );
		~tConstraint( );

		tEntity* fOtherEntity( tEntity* e ) const { return (e == mOwnerA) ? mOwnerB.fGetRawPtr( ) : mOwnerA.fGetRawPtr( ); }

		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		tEntityPtr mOwnerA, mOwnerB;
		tRigidBody* mBodyA, *mBodyB;
		Math::tVec3f mARelativePt, mBRelativePt;
	};

	typedef tRefCounterPtr<tConstraint> tConstraintPtr;

}}

#endif//__tConstraint__
