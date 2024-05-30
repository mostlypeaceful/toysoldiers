#ifndef __tPhysicsWorld__
#define __tPhysicsWorld__

#include "tSortedOverlapTree.hpp"
#include "tPhysicsBody.hpp"
#include "tSequentialImpulseSolver.hpp"
#include "Gfx/tDebugGeometry.hpp"

namespace Sig { namespace Physics
{
	class tPhysicsWorld : public tRefCounter
	{
	public:
		tPhysicsWorld( );
		~tPhysicsWorld( );

		void fAddObject( tPhysicsObject* body );
		void fRemoveObject( tPhysicsObject* body );
		u32 fNumObjects( u32 objectType ) const { return mObjectLists[ objectType ].fCount( ); }

		void fStep( f32 dt );

		// bodies manage their own entrance and exit of this tree
		tSortedOverlapTree& fShapeTree( ) { return mShapeTree; }
		
		const tPhysicsBodyPtr& fMainStaticBody( ) const { return mMainStaticBody; }

		static Gfx::tDebugGeometryContainer& fDebugGeometry( );
		static Math::tVec3f fRandomColor( );

		void fLogStats( std::wstringstream& statsText );

		struct tParams
		{
			f32 mCollisionRadius;
			b32 mSequentialSolver;

			tParams( );
		};

		static const tParams& fParams( ) { return sParams; }

	private:
		f32 mTimeAccumulator;
		static tParams sParams;

		tSortedOverlapTree mShapeTree;
		tFixedArray<tGrowableArray<tPhysicsObjectPtr>, cPhysicsObjectTypeCount> mObjectLists;
		tPhysicsBodyPtr mMainStaticBody;
		tSequentialImpulseSolver mSolver;

		void fSubStep( f32 dt );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};
	
}}

#endif//__tPhysicsWorld__
