#include "BasePch.hpp"
#include "tPhysicsObject.hpp"
#include "tPhysicsWorld.hpp"
#include "tContactIsland.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{
	tFixedArray<u32, cPhysicsObjectTypeCount> tPhysicsObject::sObjsInstantiated;


	b32 tIslandData::fSleeping( ) const 
	{ 
		//sigassert( mCollisionIsland ); 
		if( mProxyIsland )
			return mProxyIsland->fIslandData( ).fSleeping( );
		else if( mCollisionIsland )
			return mCollisionIsland->fSleeping( );
		return false;
	}

	void tIslandData::fAwaken( ) 
	{ 
		//sigassert( mCollisionIsland ); 
		if( mProxyIsland )
			mProxyIsland->fIslandData( ).fAwaken( );
		else if( mCollisionIsland )
			mCollisionIsland->fAwaken( ); 
	}

	tIslandData& tIslandData::fContactRefCounter( ) 
	{ 
		return mProxyIsland ? mProxyIsland->fIslandData( ) : *this; 
	}

	tContactIsland* tIslandData::fContactIsland( )
	{
		tContactIsland* result = mProxyIsland ? mProxyIsland->fIslandData( ).mCollisionIsland : mCollisionIsland; 
		sigassert( result );
		return result;
	}

	void tIslandData::fCreateProxy( tPhysicsObject& obj )
	{
		sigassert( obj.fWorld( ) );

		if( mProxyIsland )
			fReleaseProxy( obj );

		mProxyIsland = NEW tContactIsland( &obj );
		obj.fWorld( )->fAddObject( mProxyIsland );

		if( mCollisionIsland )
		{
			// move the structure around
			mCollisionIsland->fAdd( *mProxyIsland );
			mCollisionIsland->fLiberate( obj );

			// put collision ptr on proxy and clear ours
			mProxyIsland->fIslandData( ).mCollisionIsland = mCollisionIsland;
			mCollisionIsland = NULL;
		}
		else
			fIsolateContactIsland( obj );
	}

	void tIslandData::fReleaseProxy( tPhysicsObject& obj )
	{
		sigassert( mProxyIsland );
		
		// pull the proxy out of any collision.
		if( mProxyIsland->fMembers( ).fCount( ) == 1 )
		{
			// no longer needed, this implicitly removes our obj
			if( mCollisionIsland )
				mCollisionIsland = mCollisionIsland->fLiberate( *mProxyIsland );
			mProxyIsland->fRemoveFromWorld( );
		}
		else
		{
			//still referenced, just pull our obj out
			mProxyIsland->fLiberate( obj );
		}

		mProxyIsland = NULL;		
	}

	void tIslandData::fIsolateContactIsland( tPhysicsObject& obj )
	{
		sigassert( obj.fWorld( ) );

		tPhysicsObject& collisionItem = fCollisionIslandItem( obj );
		tIslandData& collideData = collisionItem.fIslandData( );

		if( collideData.mCollisionIsland )
		{
			collideData.fLiberate( collisionItem );
			collideData.mCollisionIsland = NULL;
		}

		// new blank island
		tContactIsland* newIsland = NEW tContactIsland( &collisionItem );
		collideData.mCollisionIsland = newIsland;
		obj.fWorld( )->fAddObject( newIsland );
	}

	void tIslandData::fLiberate( tPhysicsObject& obj )
	{
		//sigassert( mCollisionIsland );
		//tPhysicsObject& collisionItem = fCollisionIslandItem( obj );
		mCollisionIsland = mCollisionIsland->fLiberate( obj );
	}

	void tIslandData::fIncrementRef( tPhysicsObject& obj )
	{
		//sigassert( mCollisionIsland == obj.fIslandData( ).mCollisionIsland || mProxyIsland == obj.fIslandData( ).mProxyIsland );

		tRef* r = mRefs.fFind( &obj );
		if( r )
			++r->mCount;
		else
			mRefs.fPushBack( tRef( &obj ) );
	}

	void tIslandData::fDecrementRef( tPhysicsObject& obj )
	{
		u32 index = mRefs.fIndexOf( &obj );
		if( index == ~0 ) //must have already been removed from island (destruction)
			return;

		tRef* r = &mRefs[ index ];
		sigassert( r->mCount > 0 );
		--r->mCount;

		if( !r->mCount )
			mRefs.fErase( index );
	}

	void tIslandData::fRemoveAllRefs( tPhysicsObject& obj )
	{
		u32 index = mRefs.fIndexOf( &obj );
		sigassert( index != ~0 );
		mRefs.fErase( index );
	}
	
	tPhysicsObject& tIslandData::fCollisionIslandItem( tPhysicsObject& obj ) 
	{ 
		return mProxyIsland ? *mProxyIsland : obj; 
	}

	void tIslandData::fGetAllManifolds( tPhysicsBody& obj, tGrowableArray<tPersistentContactManifold*>& output )
	{
		const tGrowableArray<tPersistentContactManifoldPtr>& manifolds = obj.fManifolds( );
		for( u32 m = 0; m < manifolds.fCount( ); ++m )
			output.fPushBack( manifolds[ m ].fGetRawPtr( ) );

		tIslandData& contacts = obj.fIslandData( ).fContactRefCounter( );
		for( u32 b = 0; b < contacts.mRefs.fCount( ); ++b )
		{
			tPhysicsObject* o = contacts.mRefs[ b ].mBody;
			if( o->fObjectType( ) == cPhysicsObjectTypeRigid )
			{
				tPhysicsBody* bod = static_cast<tPhysicsBody*>( o );
				for( u32 m = 0; m < bod->fManifolds( ).fCount( ); ++m )
					if( bod->fManifolds( )[ m ]->fForBody( obj ) )
						output.fPushBack( bod->fManifolds( )[ m ].fGetRawPtr( ) );
			}
			else if( o->fObjectType( ) == cPhysicsObjectTypeIsland )
			{
				tContactIsland* isl = static_cast<tContactIsland*>( o );
				for( u32 i = 0; i < isl->fMembers( ).fCount( ); ++i )
				{
					o = isl->fMembers( )[ i ].mPtr;
					if( o->fObjectType( ) == cPhysicsObjectTypeRigid )
					{
						tPhysicsBody* bod = static_cast<tPhysicsBody*>( o );
						for( u32 m = 0; m < bod->fManifolds( ).fCount( ); ++m )
							if( bod->fManifolds( )[ m ]->fForBody( obj ) )
								output.fPushBack( bod->fManifolds( )[ m ].fGetRawPtr( ) );
					}
				}
			}
		}
	}

	void tPhysicsObject::fSetWorld( tPhysicsWorld* world ) 
	{ 
		if( mInWorld )
		{
			if( fIslandData( ).fHasContactIsland( ) ) 
				fIslandData( ).fLiberate( *this );
			if( fIslandData( ).mProxyIsland )
				fIslandData( ).fReleaseProxy( *this );
		}

		mInWorld = world; 
	}

	void tPhysicsObject::fRemoveFromWorld( )
	{
		if( mInWorld )
			mInWorld->fRemoveObject( this );
	}

}}