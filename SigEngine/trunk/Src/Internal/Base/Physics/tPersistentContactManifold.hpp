#ifndef __tPersistentContactManifold__
#define __tPersistentContactManifold__


namespace Sig { namespace Physics
{
	class tPhysicsBody;
	class tRigidBody;
	class tContactPoint;
	class tCollisionShape;

	struct tPersistentContactPt
	{
		Math::tVec3f	mARel;
		Math::tVec3f	mBRel;
		Math::tVec3f	mBNormal;
		Math::tVec3f	mWorldNormal;

		f32				mDepth;
		u32				mLife;
		f32				mHorizSepLenSqr;

		f32				mLastImpulse;
		Math::tVec2f	mLastImpulseTangent;
		Math::tVec3f	mLastImpulseTangentWorld;

		b32 fSimilar( const tPersistentContactPt& other ) const;

		// A newer but similar point has been detected, inherent it's current state.
		void fUpdate( const tPersistentContactPt& newPt );

		// Adjust penetration and recompute offsets for a point.
		void fRecompute( tRigidBody& a, tRigidBody* b );

		// Returns true if point is no long valid.
		b32 fInvalid( tRigidBody& a );

		Math::tVec3f fAWorldPt( const tRigidBody& a ) const;
		Math::tVec3f fBWorldPt( const tRigidBody* b ) const;
		tContactPoint fAContactPt( const tRigidBody& a ) const;
		tContactPoint fBContactPt( const tRigidBody* b ) const;

		tPersistentContactPt( ) { }
		tPersistentContactPt( tRigidBody& a, const tContactPoint& cp, tRigidBody* b = NULL );

		//debugging
		u32 mID;
		static u32 sNextID;
	};

	struct tPersistentContactManifold : tRefCounter
	{
		define_class_pool_new_delete( tPersistentContactManifold, 128 );

		tPersistentContactManifold( tRigidBody* a, tRigidBody* b, tCollisionShape& sa, tCollisionShape& sb, b32 flipped, u32 key );
		~tPersistentContactManifold( );

		typedef tFixedGrowingArray< tPersistentContactPt, 4 > tPtArray;

		void fAddContact( const tContactPoint& cp );
		void fRemoveOldContacts( );

		void fClear( );

		tRigidBody* fA( ) const { return mA; }
		tRigidBody* fB( ) const { return mB; }
		b32 fForBody( const tPhysicsBody& b ) const;

		tCollisionShape& fAShape( ) const { return mAShape; }
		tCollisionShape& fBShape( ) const { return mBShape; }
		b32 fForShape( const tCollisionShape& s ) const { return ( &mAShape == &s || &mBShape == &s ); }

		tPtArray& fContacts( ) { return mColliding; }
		const tPtArray& fContacts( ) const { return mColliding; }

		u32 fKey( ) const { return mKey; }
		b32 fFlipped( ) const { return mFlipped; }

		void fSetPassive( b32 passive ) { mPassive = passive; }
		b32 fPassive( ) const { return mPassive; }

		void fSetUnlimitedPts( b32 unlimited ) { mUnlimited = unlimited; }

	private:
		tRigidBody* mA;
		tRigidBody* mB;
		tCollisionShape& mAShape;
		tCollisionShape& mBShape;

		b32 mFlipped; //true if contact pts are going to be coming in backwards.
		b32 mUnlimited;
		u32 mKey;
		b32 mPassive;

		tPtArray mColliding;

		u32  fComputeNewPtIndex( const tPersistentContactPt& pt ) const;
	};

	typedef tRefCounterPtr< tPersistentContactManifold > tPersistentContactManifoldPtr;

}}

#endif//__tPersistentContactManifold__
