#ifndef __tContactIsland__
#define __tContactIsland__

#include "tPhysicsObject.hpp"

namespace Sig { namespace Physics
{
	class tConstraint;

	// NO ONE SHOULD BE CALLING ANY OF THIS, it should be managed through the tIslandData structure.
	class tContactIsland : public tPhysicsObject
	{
		define_dynamic_cast( tContactIsland, tPhysicsObject );
	public:
		tContactIsland( tPhysicsObject* b = NULL );
		~tContactIsland( );

		static void fBeginContact( tPhysicsObject& a, tPhysicsObject& b, u32 id );
		static void fEndContact( tPhysicsObject& a, tPhysicsObject& b, u32 id );

		static void fMergeProxies( tPhysicsObject& a, tPhysicsObject& b, u32 id );
		static void fSplitProxies( tPhysicsObject& a, tPhysicsObject& b, u32 id );

		b32 fSleeping( ) const { return mSleeping; }
		void fAwaken( );

		void fPreCollide( f32 dt );
		void fPostCollide( f32 dt );


		Math::tAabbf fBounds( ) const;
		void fDebugDraw( tPhysicsWorld& world );

		// returns the island you should be pointing to.
		tContactIsland* fLiberate( tPhysicsObject& b );
		void fAdd( tPhysicsObject& b );

		struct tMember
		{
			tPhysicsObject* mPtr;

			tMember( tPhysicsObject* b = NULL )
				: mPtr( b )
			{ }

			b32 operator == ( const tPhysicsObject* b ) const { return mPtr == b; }
		};

		const tGrowableArray<tMember>& fMembers( ) const { return mMembers; }

	private:
		Math::tVec4f mDebugColor;
		b32 mSleeping;
		b32 mTestSleep;
		tGrowableArray<tMember> mMembers;

		void fAcquire( tContactIsland& island, b32 proxy );
	};
	typedef tRefCounterPtr< tContactIsland > tContactIslandPtr;
	
}}

#endif//__tContactIsland__
