//------------------------------------------------------------------------------
// \file tBuiltNavGraph.hpp - 15 Dec 2010
// \author cbramwell, ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tBuiltNavGraph__
#define __tBuiltNavGraph__
#include "tNavGraph.hpp"
#include "tKdTree.hpp"

namespace Sig { namespace AI
{
	///
	/// \class tBuiltPathNode
	/// \brief A single node containing links to other nodes.
	class base_export tBuiltPathNode
	{
		declare_reflector();

	public:
		Math::tMat3f mObjToWorld;
		Math::tObbf mWorldSpaceObb;
		tDynamicArray<u32> mEdges; // Index to the neighbor node in the nodes array.

		tBuiltPathNode( ) { }
		tBuiltPathNode( tNoOpTag );

		Math::tVec3f fPos( ) const { return mObjToWorld.fGetTranslation( ); }
		b32 fCastRayToFinalPos( const Math::tVec3f& rayOrigin, const Math::tVec3f& finalPos );
		Math::tVec3f fCastRayToCenter( const Math::tVec3f& rayOrigin ) const;
		b32 fOnNode( const Math::tVec3f& p ) const;
	};

	///
	/// \class tPathEdge
	/// \brief Exists as a pseudo-node in between regular nodes. Also connects
	/// nodes to each other.
	class base_export tBuiltPathEdge
	{
		declare_reflector();

	public:
		u16 mNodeA;
		u16 mNodeB;
		tFixedArray<Math::tPlanef, 4> mHalfSpaces;
		tFixedArray<Math::tVec3f, 2> mHighEdge;
		tFixedArray<Math::tVec3f, 2> mLowEdge;

		tBuiltPathEdge( )
			: mNodeA( ~0 )
			, mNodeB( ~0 )
		{}
		tBuiltPathEdge( tNoOpTag );

		b32 fOnEdge( const Math::tVec3f& p ) const;
		f32 fClosestPoint( const Math::tVec3f& pos, Math::tVec3f* outPos ) const;
		Math::tAabbf fConstructAabb( ) const;
	};

	///
	/// \class tBuiltPathHop
	/// \brief 
	struct tBuiltPathHop : public tUncopyable, public tRefCounter
	{
		define_class_pool_new_delete( tBuiltPathHop, 128 );

		u32 mNode;
		f32 mTravelCost; //"g"
		f32 mHeurCost; //"h"
		f32 mTotCost; //"f"
		b32 mInQueue;
		tBuiltPathHop* mPrev;
		tBuiltPathHop( )
			: mNode( ~0 )
			, mTravelCost( Math::cInfinity )
			, mHeurCost( Math::cInfinity )
			, mInQueue( false )
			, mPrev( NULL )
		{}
		inline f32 fCost( ) const;
	};

	///
	/// \class tBuiltNavGraph
	/// \brief The entire search space and search algorithm.
	class base_export tBuiltNavGraph : public tRefCounter
	{
		declare_reflector();

		//data
		tDynamicArray<tBuiltPathNode> mNodes;
		tDynamicArray<tBuiltPathEdge> mEdges;
		u32 mNavGraphID;

		tKdTree mNodeTree;
		tKdTree mEdgeTree;

		//DEBUG
		tDynamicArray<Math::tVec3f> mBadPos;
		tDynamicArray<tBuiltPathEdge> mBadEdges;

	public:
		tBuiltNavGraph( );
		tBuiltNavGraph( tDynamicArray<tBuiltPathNode>& nodes, tDynamicArray<tBuiltPathEdge>& edges );
		tBuiltNavGraph( tNoOpTag );
		virtual ~tBuiltNavGraph( );
		void fSetRoot( const tPathEntityPtr& root, u32 navGraphID );
		void fInit( );
		void fDebugDraw( );
		void fDebugDraw( const tPath& path );
		void fFindPath( tPath& path );
		b32 fOnWalkable( const Math::tVec3f& pos ) const;
		u32 fNavGraphID( ) const { return mNavGraphID; }

	private:
		void fBuildTrees( );
		void fGetNodes( const Math::tVec3f& pos, tGrowableArray<u32>& items ) const;
		void fGetEdges( const Math::tVec3f& pos, tGrowableArray<u32>& items ) const;
		void fRunPerfTest( );
		void fConvertToPath( const tBuiltPathHop* pathHop, tPath& path ) const;
		//helper stuff
		Math::tVec3f fGetNodePos( u32 i ) const;
		tVec3fPair fGetNodeExtents( u32 i ) const;
		//search algorithms
		void fSearchUsingAStar( u32 startNode, u32 endNode, tPath& path );
		//pathing steps
		u32 fFindClosestNode( const Math::tVec3f& pos ) const;
		void fDeletePathHops( tDynamicArray<tBuiltPathHop*>& allPathHops );
		b32 fContainsSameEdge( const Math::tVec3f& aPos, const Math::tVec3f& bPos ) const;
		void fConvertToPosPath( tPath& path );
		//edge stuff
		tVec3fPair fGetHighLowEdgePts( const Math::tVec3f& normal, u32 node ) const;
		const tBuiltPathEdge* fGetEdge( u32 nodeA, u32 nodeB ) const;
		Math::tVec3f fGetClosestWalkablePos( const Math::tVec3f& pos ) const;
	};

	///
	/// \class tTempNavGraphWrapper
	/// \brief This needs to go away. Forever. Here so that I can have a shared interface but not
	/// have the tBuiltNavGraph be virtual because that messes with serialization stuff. Also so the level
	/// logic can hold a pointer it can destroy if it wants to.
	class base_export tTempNavGraphWrapper : public tNavGraphInterface
	{
		tBuiltNavGraph* mGraph;

	public:
		tTempNavGraphWrapper( tBuiltNavGraph* graph ) : mGraph( graph ) { }

		virtual void fSetRoot( const tPathEntityPtr& root, u32 navGraphID ) { mGraph->fSetRoot( root, navGraphID ); }
		virtual void fInit( ) { mGraph->fInit( ); }
		virtual void fDebugDraw( ) { mGraph->fDebugDraw( ); }
		virtual void fDebugDraw( const tPath& path ) { mGraph->fDebugDraw( path ); }
		virtual void fFindPath( tPath& path ) { mGraph->fFindPath( path ); }
		virtual b32 fOnWalkable( const Math::tVec3f& pos ) const { return mGraph->fOnWalkable( pos ); }
		virtual u32 fNavGraphID( ) const { return mGraph->fNavGraphID( ); }
	};
}}

#endif //__tBuiltNavGraph__
