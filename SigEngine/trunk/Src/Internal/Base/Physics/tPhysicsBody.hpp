#ifndef __tPhysicsBody__
#define __tPhysicsBody__

#include "tPhysicsObject.hpp"
#include "tCollisionShapes.hpp"
#include "tPersistentContactManifold.hpp"


namespace Sig { namespace Physics
{

	class tPhysicsWorld;

	class tPhysicsBody : public tPhysicsObject
	{
		define_dynamic_cast( tPhysicsBody, tPhysicsObject );
	public:
		tPhysicsBody( u32 type );

		const Math::tMat3f& fTransform( ) const;
		const Math::tMat3f& fInvTransform( );

		Math::tVec3f fToWorld( const Math::tVec3f& localVector ) const;
		Math::tVec3f fToLocal( const Math::tVec3f& worldVector ) const;

		void fSetTransform( const Math::tMat3f& xform );
		void fTranslate( const Math::tVec3f& delta );
		void fSetPosition( const Math::tVec3f& pos );

		void fAddShape( tCollisionShape* shape );
		void fRemoveShape( tCollisionShape* shape );
		void fShapeMoved( tCollisionShape* shape );
		const tGrowableArray<tCollisionShapePtr>& fShapes( ) const { return mShapes; }

		void fClearShapes( );

		// This is _in place_ of any kind of "user data" pointer at the moment, don't preserve two systems.
		tEntity* fOwnerEntity( ) const { return mOwnerEntity; }
		tEntity*& fOwnerEntity( ) { return mOwnerEntity; }

		// called by the physics system
		virtual void fSetWorld( tPhysicsWorld* world );
		void fUpdateBroadphase( const Math::tVec3f& deltaFromVel );
		void fUpdateBroadphaseSleeping( );
		void fDebugDraw( tPhysicsWorld& world );

		Math::tAabbf fLocalAABB( ) const;

		void fAddPersistentManifold( tPersistentContactManifold* manifold );
		void fRemovePersistentManifold( tPersistentContactManifold& manifold );

		tPersistentContactManifold* fFindManifold( u32 key ) const;
		void fClearManifolds( );
		tGrowableArray< tPersistentContactManifoldPtr >& fManifolds( ) { return mManifolds; }

		void fSetAllShapesCollisionMasks( u32 identity, u32 collisionMask );

	protected:
		tEntity*		mOwnerEntity;

		tGrowableArray<tCollisionShapePtr> mShapes;
		tGrowableArray< tPersistentContactManifoldPtr > mManifolds;

		void fRegisterShape( tCollisionShape& shape );
		void fUnRegisterShape( tCollisionShape& shape );
		void fRegisterAllShapes( );
		void fUnregisterAllShapes( );

	private:
		Math::tMat3f	mTransform;
		Math::tMat3f	mInvTransform;
		b32				mShapesDirty;
		b32				mInvDirty;
		b32				mShapesSleeped;
	};

	typedef tRefCounterPtr<tPhysicsBody> tPhysicsBodyPtr;

}}

#endif//__tPhysicsBody__
