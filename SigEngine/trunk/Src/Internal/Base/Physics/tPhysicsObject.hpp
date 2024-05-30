#ifndef __tPhysicsObject__
#define __tPhysicsObject__

#include "Logic/tPhysical.hpp"

namespace Sig { namespace Physics
{

	class tPhysicsWorld;
	class tContactIsland;
	class tPhysicsObject;
	class tPhysicsBody;
	class tCollisionShape;
	struct tPersistentContactManifold;

	enum tPhysicsBodyType
	{
		cPhysicsObjectTypeRigid,
		cPhysicsObjectTypeFixed,
		cPhysicsObjectTypeIsland,
		cPhysicsObjectTypeConstraint,
		cPhysicsObjectTypeCount
	};

	struct tIslandData
	{
		struct tRef
		{
			tPhysicsObject* mBody;
			u32 mCount;

			tRef( tPhysicsObject* b = NULL )
				: mBody( b )
				, mCount( 1 )
			{ }

			b32 operator == ( const tPhysicsObject* b ) const { return mBody == b; }
		};

		tIslandData( )
			: mProxyIsland( NULL )
			, mCollisionIsland( NULL )
			, mIdleTime( 0.f )
		{ }

		void fIncrementRef( tPhysicsObject& obj );
		void fDecrementRef( tPhysicsObject& obj );
		void fRemoveAllRefs( tPhysicsObject& obj );

		// called by the owner of this data.
		void fCreateProxy( tPhysicsObject& obj );
		void fReleaseProxy( tPhysicsObject& obj );

		void fIsolateContactIsland( tPhysicsObject& obj );
		void fLiberate( tPhysicsObject& obj );
		void fInsertProxy( tPhysicsObject& obj );

		b32 fSleeping( ) const;
		void fAwaken( );

		b32 fHasContactIsland( ) const { return (mCollisionIsland != NULL); }
		tIslandData& fContactRefCounter( );
		tContactIsland* fContactIsland( );

		static void fGetAllManifolds( tPhysicsBody& obj, tGrowableArray<tPersistentContactManifold*>& output );

		// either the proxy island or the tIslandDataOwner
		tPhysicsObject& fCollisionIslandItem( tPhysicsObject& obj );

		tContactIsland*			mProxyIsland;
		tContactIsland*			mCollisionIsland;
		tGrowableArray<tRef>	mRefs;
		f32						mIdleTime;
	};

	class tPhysicsObject : public Logic::tPhysical
	{
		debug_watch( tPhysicsObject );
		declare_uncopyable( tPhysicsObject );
		define_dynamic_cast( tPhysicsObject, Logic::tPhysical );
	public:
		tPhysicsObject( u32 type = cPhysicsObjectTypeCount )
			: mType( type )
			, mInWorld( NULL )
		{
			++sObjsInstantiated[ type ];
		}

		virtual ~tPhysicsObject( )
		{
			sigassert( !mInWorld && "tPhysicsObject destroyed while still in the world!" );
			sigassert( sObjsInstantiated[ mType ] );
			--sObjsInstantiated[ mType ];
		}

		u32 fObjectType( ) const { return mType; }
		tPhysicsWorld* fWorld( ) const { return mInWorld; }

		void fRemoveFromWorld( );

		// only called by tPhysicsWorld itself.
		virtual void fSetWorld( tPhysicsWorld* world );
		tIslandData& fIslandData( ) { return mIslandData; }

		static u32 fObjectsInstantiated( u32 type ) { return sObjsInstantiated[ type ]; }

	protected:
		u32				mType;
		tPhysicsWorld*	mInWorld;

		// no one should call this stuff
		tIslandData mIslandData;

	private:
		static tFixedArray<u32, cPhysicsObjectTypeCount> sObjsInstantiated;
	};

	typedef tRefCounterPtr< tPhysicsObject > tPhysicsObjectPtr;

}}

#endif//__tPhysicsObject__
