#include "BasePch.hpp"
#include "tConstraint.hpp"
#include "tRigidBody.hpp"
#include "tContactIsland.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	tConstraint::tConstraint( )
		: tPhysicsObject( cPhysicsObjectTypeConstraint )
		, mBodyA( NULL )
		, mBodyB( NULL )
		, mAAnchorPt( tMat3f::cIdentity )
		, mBAnchorPt( tMat3f::cIdentity )
	{
	}

	tConstraint::tConstraint( tRigidBody* ownerA, tRigidBody* ownerB, const Math::tMat3f& aRelConstraintPt )
		: tPhysicsObject( cPhysicsObjectTypeConstraint )
		, mBodyA( ownerA )
		, mBodyB( ownerB )
		, mAAnchorPt( aRelConstraintPt )
	{
		sigassert( mBodyA );

		// this assumes b is not set and is referring to the world
		mBAnchorPt = mBodyA->fTransform( ) * mAAnchorPt;

		if( mBodyB )
		{
			// b is set, convert to b-local
			mBAnchorPt = mBodyB->fInvTransform( ) * mBAnchorPt;
		}
	}

	tPhysicsBody* tConstraint::fOtherBody( const tPhysicsBody* b ) const 
	{ 
		return (b == mBodyA.fGetRawPtr( )) ? mBodyB.fGetRawPtr( ) : mBodyA.fGetRawPtr( ); 
	}

	void tConstraint::fStepST( f32 dt, f32 percentage )
	{
		if( !mIslandData.fSleeping( ) )
			fStepConstraintInternal( dt, percentage );
	}
	
	void tConstraint::fSetWorld( tPhysicsWorld* world )
	{
		if( mInWorld && mBodyA )
		{
			mBodyA->fRemoveConstraint( this );
			tContactIsland::fSplitProxies( *mBodyA, *this, (u32)this );

			if( mBodyB )
			{
				mBodyB->fRemoveConstraint( this );
				tContactIsland::fSplitProxies( *mBodyB, *this, (u32)this );
			}
		}
		
		tPhysicsObject::fSetWorld( world );

		if( mInWorld && mBodyA )
		{
			mBodyA->fAddConstraint( this );			
			tContactIsland::fMergeProxies( *mBodyA, *this, (u32)this );

			if( mBodyB )
			{
				mBodyB->fAddConstraint( this );
				tContactIsland::fMergeProxies( *mBodyB, *this, (u32)this );
			}
		}
	}

	void tConstraint::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tConstraint, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		//classDesc
		//	;

		vm.fNamespace(_SC("Physics")).Bind(_SC("Constraint"), classDesc);
	}
}}
