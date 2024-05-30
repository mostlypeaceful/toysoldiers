#include "BasePch.hpp"
#include "tContactIsland.hpp"
#include "tRigidBody.hpp"
#include "tPhysicsWorld.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	namespace
	{
		static u32 cValidator = 0;
		//tGrowableArray<u32> sFreed;
	}

	devvar( bool, Physics_RigidBody_Sleeping_Enable, true );

	tContactIsland::tContactIsland( tPhysicsObject* b )
		: tPhysicsObject( cPhysicsObjectTypeIsland )
		, mSleeping( false )
		, mTestSleep( false )
		, mDebugColor( tVec4f( tPhysicsWorld::fRandomColor( ), 0.5f ) )
	{
		mMembers.fPushBack( tContactIsland::tMember( b ) );
	}

	tContactIsland::~tContactIsland( )
	{

	}

	void tContactIsland::fBeginContact( tPhysicsObject& a, tPhysicsObject& b, u32 id )
	{
		++cValidator;

		if( a.fIslandData( ).fContactIsland( ) != b.fIslandData( ).fContactIsland( ) )
			a.fIslandData( ).fContactIsland( )->fAcquire( *b.fIslandData( ).fContactIsland( ), false );

		a.fIslandData( ).fContactRefCounter( ).fIncrementRef( b.fIslandData( ).fCollisionIslandItem( b ) );
		b.fIslandData( ).fContactRefCounter( ).fIncrementRef( a.fIslandData( ).fCollisionIslandItem( a ) );
	}

	void tContactIsland::fEndContact( tPhysicsObject& a, tPhysicsObject& b, u32 id )
	{
		sigassert( cValidator );
		--cValidator;

		tPhysicsObjectPtr aRef( &a );
		tPhysicsObjectPtr bRef( &b );

		// more validation
		//sigassert( !sFreed.fFind( id ) && "Already freed!" );
		//sFreed.fPushBack( id );

		a.fIslandData( ).fContactRefCounter( ).fDecrementRef( b.fIslandData( ).fCollisionIslandItem( b ) );
		b.fIslandData( ).fContactRefCounter( ).fDecrementRef( a.fIslandData( ).fCollisionIslandItem( a ) );

		// shouldnt need to make two new islands for one separation
		if( !a.fIslandData( ).fContactRefCounter( ).mRefs.fCount( ) && a.fWorld( ) )
			a.fIslandData( ).fIsolateContactIsland( a );
		
		if( !b.fIslandData( ).fContactRefCounter( ).mRefs.fCount( ) && b.fWorld( ) )
			b.fIslandData( ).fIsolateContactIsland( b );			
	}

	void tContactIsland::fMergeProxies( tPhysicsObject& a, tPhysicsObject& b, u32 id )
	{
		++cValidator;

		if( !a.fIslandData( ).mProxyIsland )
			a.fIslandData( ).fCreateProxy( a );

		if( !b.fIslandData( ).mProxyIsland )
			b.fIslandData( ).fCreateProxy( b );

		if( a.fIslandData( ).mProxyIsland != b.fIslandData( ).mProxyIsland )
			a.fIslandData( ).mProxyIsland->fAcquire( *b.fIslandData( ).mProxyIsland, true );

		a.fIslandData( ).fIncrementRef( b );
		b.fIslandData( ).fIncrementRef( a );
	}

	void tContactIsland::fSplitProxies( tPhysicsObject& a, tPhysicsObject& b, u32 id )
	{
		sigassert( cValidator );
		--cValidator;

		// more validation
		//sigassert( !sFreed.fFind( id ) && "Already freed!" );
		//sFreed.fPushBack( id );

		a.fIslandData( ).fDecrementRef( b );
		b.fIslandData( ).fDecrementRef( a );

		// shouldnt need to make two new islands for one separation
		if( !a.fIslandData( ).mRefs.fCount( ) && a.fWorld( ) )
			a.fIslandData( ).fCreateProxy( a );

		if( !b.fIslandData( ).mRefs.fCount( ) && b.fWorld( ) )
			b.fIslandData( ).fCreateProxy( b );			
	}

	void tContactIsland::fAwaken( )
	{
		if( mSleeping )
		{
			for( u32 i = 0; i < mMembers.fCount( ); ++i )
				mMembers[ i ].mPtr->fIslandData( ).mIdleTime = 0.f;
		}

		mTestSleep = false; 
		mSleeping = false;
	}

	tContactIsland* tContactIsland::fLiberate( tPhysicsObject& b )
	{
		//sigassert( b.fIslandData( ).mCollisionIsland == this );
		sigassert( mMembers.fFind( &b ) && "Not in this island!" );

		//remove all island refs.
		for( u32 i = 0; i < b.fIslandData( ).mRefs.fCount( ); ++i )
			b.fIslandData( ).mRefs[ i ].mBody->fIslandData( ).fRemoveAllRefs( b );
		b.fIslandData( ).mRefs.fSetCount( 0 );

		mMembers.fFindAndErase( &b );
		if( mMembers.fCount( ) == 0 )
		{
			fRemoveFromWorld( );
			return NULL;
		}
		else
			return this;
	}

	void tContactIsland::fAdd( tPhysicsObject& b )
	{
		//sigassert( b.fIslandData( ).mCollisionIsland == this );
		sigassert( !mMembers.fFind( &b ) && "Member already in island!" );
		mMembers.fPushBack( tMember( &b ) );
	}

	void tContactIsland::fAcquire( tContactIsland& island, b32 proxy )
	{
#ifdef sig_logging
		for( u32 i = 0; i < island.mMembers.fCount( ); ++i )
			sigassert( !mMembers.fFind( island.mMembers[ i ].mPtr ) && "Error Acquiring island!" );
#endif

		mMembers.fJoin( island.mMembers );

		if( proxy )
		{
			for( s32 i = island.mMembers.fCount( ) - 1; i >= 0; --i )
			{
				// Construct reference so the below code can't destruct the object
				tPhysicsObjectPtr obj( island.mMembers[ i ].mPtr );

				//if( obj->fIslandData( ).mCollisionIsland )
				//	obj->fIslandData( ).mCollisionIsland = obj->fIslandData( ).mCollisionIsland->fLiberate( island );
				obj->fIslandData( ).fReleaseProxy( *obj );
				obj->fIslandData( ).mProxyIsland = this;
			}

			//// merge collision if necessary
			//if( island.fIslandData( ).mCollisionIsland )
			//	island.fIslandData( ).mCollisionIsland->fLiberate( island );
		}
		else
		{
			for( u32 i = 0; i < island.mMembers.fCount( ); ++i )
				island.mMembers[ i ].mPtr->fIslandData( ).mCollisionIsland = this;

			island.fRemoveFromWorld( );
		}

		fAwaken( );
	}

	Math::tAabbf tContactIsland::fBounds( ) const
	{
		tAabbf bounds;
		bounds.fInvalidate( );

		for( u32 i = 0; i < mMembers.fCount( ); ++i )
		{
			tPhysicsBody* body = mMembers[ i ].mPtr->fDynamicCast<tPhysicsBody>( );
			if( body )
				bounds |= body->fLocalAABB( ).fTransform( body->fTransform( ) );
			else
			{
				tContactIsland* island = mMembers[ i ].mPtr->fDynamicCast<tContactIsland>( );
				if( island )
					bounds |= island->fBounds( );
			}
		}

		return bounds;
	}

	void tContactIsland::fDebugDraw( tPhysicsWorld& world )
	{
		tVec4f color = mSleeping ? tVec4f( 1,0,0,0.25f ) : mDebugColor;
		world.fDebugGeometry( ).fRenderOnce( fBounds( ), color );
	}

	void tContactIsland::fPreCollide( f32 dt )
	{
		mTestSleep = true;
	}

	void tContactIsland::fPostCollide( f32 dt )
	{
		mTestSleep = mTestSleep && Physics_RigidBody_Sleeping_Enable;

		mSleeping = mTestSleep;

		if( mSleeping && !Physics_RigidBody_Sleeping_Enable )
			fAwaken( );		
	}

} }
