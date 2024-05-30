#include "BasePch.hpp"
#include "tPhysicsBody.hpp"
#include "tPhysicsWorld.hpp"
#include "tCollisionShapes.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	tPhysicsBody::tPhysicsBody( u32 type )
		: tPhysicsObject( type )
		, mOwnerEntity( NULL )
		, mTransform( Math::tMat3f::cIdentity )
		, mInvTransform( Math::tMat3f::cIdentity )
		, mShapesDirty( false )
		, mInvDirty( false )
		, mShapesSleeped( false )
	{ }

	void tPhysicsBody::fAddShape( tCollisionShape* shape )
	{
		sigassert( shape );
		sigassert( mShapes.fIndexOf( shape ) == ~0 );

		mShapes.fPushBack( tCollisionShapePtr( shape ) );
		if( mInWorld )
			fRegisterShape( *mShapes.fBack( ) );
	}

	void tPhysicsBody::fRemoveShape( tCollisionShape* shape )
	{
		sigassert( shape );
		sigassert( mShapes.fIndexOf( shape ) != ~0 );

		if( mInWorld )
			fUnRegisterShape( *shape );
		mShapes.fFindAndErase( shape );
	}

	void tPhysicsBody::fShapeMoved( tCollisionShape* shape )
	{
		sigassert( shape );
		sigassert( mShapes.fIndexOf( shape ) != ~0 );

		if( mInWorld )
			shape->fUpdateTree( mInWorld->fShapeTree( ) );
	}

	void tPhysicsBody::fClearShapes( )
	{
		if( mInWorld )
			fUnregisterAllShapes( );

		mShapes.fSetCount( 0 );
	}

	void tPhysicsBody::fSetWorld( tPhysicsWorld* world )
	{
		if( mInWorld )
			fUnregisterAllShapes( );

		tPhysicsObject::fSetWorld( world );

		if( mInWorld )
		{
			mIslandData.mIdleTime = 0.f;
			fRegisterAllShapes( );
		}
	}

	void tPhysicsBody::fRegisterShape( tCollisionShape& shape )
	{
		sigassert( mInWorld );
		sigassert( !shape.mOwner );
		shape.mOwner = this;
		shape.fAddToTree( mInWorld->fShapeTree( ) );
	}

	void tPhysicsBody::fUnRegisterShape( tCollisionShape& shape )
	{
		sigassert( mInWorld );
		sigassert( shape.mOwner == this );
		shape.mOwner = NULL;
		shape.fRemoveFromTree( mInWorld->fShapeTree( ) );
	}

	void tPhysicsBody::fRegisterAllShapes( )
	{
		if( mShapes.fCount( ) )
		{
			sigassert( mInWorld );

			tSortedOverlapTree::tBatch batch;

			for( u32 i = 0; i < mShapes.fCount( ); ++i )
			{
				tCollisionShape& shape = *mShapes[ i ];

				sigassert( !shape.mOwner );
				shape.mOwner = this;
				shape.fAddToBatch( batch );
			}

			mInWorld->fShapeTree( ).fAddBatch( batch );
		}
	}

	void tPhysicsBody::fUnregisterAllShapes( )
	{
		// no faster way to do this yet :(
		for( u32 i = 0; i < mShapes.fCount( ); ++i )
			fUnRegisterShape( *mShapes[ i ] );
	}

	void tPhysicsBody::fUpdateBroadphase( const Math::tVec3f& deltaFromVel )
	{
		if( mShapesDirty && mInWorld )
		{
			mShapesDirty = false;
			for( u32 i = 0; i < mShapes.fCount( ); ++i )
				mShapes[ i ]->fUpdateTree( mInWorld->fShapeTree( ), deltaFromVel );
		}
	}

	void tPhysicsBody::fUpdateBroadphaseSleeping( )
	{
		b32 sleeping = mIslandData.fSleeping( );
		if( sleeping != mShapesSleeped )
			for( u32 i = 0; i < mShapes.fCount( ); ++i )
				mShapes[ i ]->fSleep( mInWorld->fShapeTree( ), sleeping );

		mShapesSleeped = sleeping;
	}

	const Math::tMat3f& tPhysicsBody::fTransform( ) const
	{
		return mTransform;
	}

	const Math::tMat3f& tPhysicsBody::fInvTransform( )
	{
		if( mInvDirty )
		{
			mInvTransform = mTransform.fInverse( );
			mInvDirty = false;
		}
		return mInvTransform;
	}

	Math::tVec3f tPhysicsBody::fToWorld( const Math::tVec3f& localVector ) const
	{
		return mTransform.fXformVector( localVector );
	}

	Math::tVec3f tPhysicsBody::fToLocal( const Math::tVec3f& worldVector ) const
	{
		return mTransform.fInverseXformVector( worldVector );
	}

	void tPhysicsBody::fSetTransform( const Math::tMat3f& xform ) 
	{
		mTransform = xform; 
		mInvDirty = true;
		mShapesDirty = true;
	}

	void tPhysicsBody::fTranslate( const Math::tVec3f& delta )
	{
		mTransform.fTranslateGlobal( delta );
		mInvDirty = true;
		mShapesDirty = true;
	}

	void tPhysicsBody::fSetPosition( const Math::tVec3f& pos )
	{
		mTransform.fSetTranslation( pos );
		mInvDirty = true;
		mShapesDirty = true;
	}

	Math::tAabbf tPhysicsBody::fLocalAABB( ) const
	{
		sigassert( mShapes.fCount( ) );

		Math::tAabbf shape;
		shape.fInvalidate( );

		for( u32 i = 0; i < mShapes.fCount( ); ++i )
			shape |= mShapes[ i ]->fLocalAABB( );

		return shape;
	}

	void tPhysicsBody::fDebugDraw( tPhysicsWorld& world )
	{
		for( u32 i = 0; i < mShapes.fCount( ); ++i )
			mShapes[ i ]->fDebugDraw( );
	}

	void tPhysicsBody::fAddPersistentManifold( tPersistentContactManifold* manifold )
	{
		sigassert( mManifolds.fIndexOf( manifold ) == ~0 );
		mManifolds.fPushBack( tPersistentContactManifoldPtr( manifold ) ); 
	}

	tPersistentContactManifold* tPhysicsBody::fFindManifold( u32 key ) const
	{
		for( u32 i = 0; i < mManifolds.fCount( ); ++i )
			if( mManifolds[ i ]->fKey( ) == key )
				return mManifolds[ i ].fGetRawPtr( );
		return NULL;
	}

	void tPhysicsBody::fRemovePersistentManifold( tPersistentContactManifold& manifold )
	{
		b32 found = mManifolds.fFindAndErase( &manifold ); 
		sigassert( found );
	}

	void tPhysicsBody::fClearManifolds( )
	{
		tGrowableArray<tPersistentContactManifold*> manifolds;
		tIslandData::fGetAllManifolds( *this, manifolds );

		for( u32 m = 0; m < manifolds.fCount( ); ++m )
			manifolds[ m ]->fClear( );
	}

	void tPhysicsBody::fSetAllShapesCollisionMasks( u32 identity, u32 collisionMask )
	{
		for( u32 i = 0; i < mShapes.fCount( ); ++i )
		{
			mShapes[ i ]->fSetIdentityMask( identity );
			mShapes[ i ]->fSetCollisionMask( collisionMask );

			if( mInWorld )
				mShapes[ i ]->fClearPairData( mInWorld->fShapeTree( ) );
		}
	}

}}
