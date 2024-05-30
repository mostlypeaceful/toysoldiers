//------------------------------------------------------------------------------
// \file tBuiltNavGraph.cpp - 15 Dec 2010
// \author cbramwell, ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tBuiltNavGraph.hpp"
#include "tApplication.hpp"
#include "tStack.hpp"
#include "tPriorityQueue.hpp"
#include "Math/tIntersectionRayObb.hpp"

using namespace Sig::Math;

namespace Sig { namespace AI
{
	devvar( bool,	Debug_BuiltNavGraph_RenderGraph,		false );
	devvar( bool,	Debug_BuiltNavGraph_Render_Connections,	false );
	devvar( bool,	Debug_BuiltNavGraph_Render_Nodes,		false );
	devvar( bool,	Debug_BuiltNavGraph_Render_Walkable,	false );
	devvar( bool,	Debug_BuiltNavGraph_RunTests,			false ); //this WILL hitch the game
	devvar( bool,	Debug_BuiltNavGraph_LogBadPositions,	false );
	devvar( bool,	Debug_BuiltNavGraph_AssertOnBadPos,		false );
	devvar( bool,	Debug_BuiltNavGraph_DrawCrapEdges,		false );
	devvar( float,	Debug_BuiltNavGraph_HeuristicWeight,	3 );
	devvar( float,	Debug_BuiltNavGraph_InWalkableEpsilon,	0.001f );
	devvar( bool,	Debug_BuiltNavGraph_DrawKdTree,			false );
	devvar( bool,	Debug_BuiltNavGraph_DrawEdgeKdTree,		false );
	devvar( int,	Debug_BuiltNavGraph_KdTreeRenderLevel,	0 );
	devvar( bool,	Debug_BuiltNavGraph_UseKDTree,			true );
	devvar( bool,	Debug_BuiltNavGraph_PullNodes,			false );

	namespace {
		const tVec4f cColorOfEllipsoid( 1, 0, 1, 0.25 );
		const tVec4f cColorOfCenterNode( 0, 1, 1, 0.5 );
		const tVec4f cColorRed( 1, 0, 0, 1 );
		const tVec4f cColorBlue( 0, 0, 1, 1 );
		const tVec4f cColorCyan( 0, 1, 1, 1 );
		void fDrawAreaNode( const tObbf& obb )
		{
			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( obb, cColorOfEllipsoid );
		}
		void fDrawNode( const tVec3f& center, const tVec4f& color = cColorRed )
		{
			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tSpheref( center, 1.0f ), color );
		}
		void fDrawLine( const tVec3f& start, const tVec3f& end, const tVec4f& color = cColorBlue )
		{
			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( start, end, color );
		}
		void fDrawEdge( const tVec3fPair& edge )
		{
			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( edge.mA, edge.mB, cColorCyan );
		}
		void fDrawPlane( const tPlanef& plane, const tVec3f& center )
		{
			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( plane, center, 10, cColorRed );
		}
		struct tPathHopLessThan
		{
			inline b32 operator( )( const tBuiltPathHop* pathA, const tBuiltPathHop* pathB )
			{
				return pathA->mTotCost > pathB->mTotCost;
			}
		};
	}//unnamed namespace

	//------------------------------------------------------------------------------
	// tBuiltPathNode
	//------------------------------------------------------------------------------
	tBuiltPathNode::tBuiltPathNode( tNoOpTag )
		: mEdges( cNoOpTag )
	{
	}

	b32 tBuiltPathNode::fCastRayToFinalPos( const Math::tVec3f& startPt, const Math::tVec3f& finalPt )
	{
		const tVec3f nodePos = mObjToWorld.fGetTranslation( );

		//check to see if point is in the box first
		const Math::tVec3f rayOrigin( startPt.x, nodePos.y, startPt.z );
		if( mWorldSpaceObb.fContains( rayOrigin ) ) //NOTE: using NODE's Y pos for containment check making this essentially a 2D check.
		{
			return true;
		}

		//not in box so do ray check
		const Math::tVec3f finalPos( finalPt.x, nodePos.y, finalPt.z );
		Math::tRayf ray( rayOrigin, finalPos - rayOrigin );
		sigassert( rayOrigin.y == nodePos.y );
		sigassert( ray.mExtent.y == 0 );

		const tIntersectionRayObb<f32> test( ray, mWorldSpaceObb );
		return test.fIntersects( );
	}

	tVec3f tBuiltPathNode::fCastRayToCenter( const Math::tVec3f& startPt ) const
	{
		const tVec3f nodePos = mObjToWorld.fGetTranslation( );

		//check to see if point is in the box first
		const Math::tVec3f rayOrigin( startPt.x, nodePos.y, startPt.z );
		if( mWorldSpaceObb.fContains( rayOrigin ) ) //NOTE: using NODE's Y pos for containment check making this essentially a 2D check.
			return rayOrigin;

		return mWorldSpaceObb.fClosestPoint( rayOrigin );
	}

	b32 tBuiltPathNode::fOnNode( const Math::tVec3f& p ) const
	{
		const Math::tVec3f testPt( p.x, mWorldSpaceObb.fCenter( ).y, p.z );
		return mWorldSpaceObb.fContains( testPt );
	}

	//------------------------------------------------------------------------------
	// tBuiltPathEdge
	//------------------------------------------------------------------------------
	tBuiltPathEdge::tBuiltPathEdge( tNoOpTag )
	{
	}

	b32 tBuiltPathEdge::fOnEdge( const Math::tVec3f& p ) const
	{
		const f32 epsilon = -0.001f;
		const f32 dist0 = mHalfSpaces[ 0 ].fSignedDistance( p );
		const f32 dist1 = mHalfSpaces[ 1 ].fSignedDistance( p );
		const f32 dist2 = mHalfSpaces[ 2 ].fSignedDistance( p );
		const f32 dist3 = mHalfSpaces[ 3 ].fSignedDistance( p );

#ifdef platform_xbox360
#define fpminf(a,b) __fself((a)-(b), b, a)
		const f32 dmin0 = fpminf( dist0, dist1 );
		const f32 dmin1 = fpminf( dist2, dist3 );
		const f32 dminFinal = fpminf( dmin0, dmin1 );
		return dminFinal >= epsilon;
#undef fpminf
#else
		return dist0 >= epsilon && dist1 >= epsilon && dist2 >= epsilon && dist3 >= epsilon;
#endif
	}

	f32 tBuiltPathEdge::fClosestPoint( const Math::tVec3f& pos, Math::tVec3f* outPos ) const
	{
		tFixedArray<tVec3fPair,4> segments;
		segments[0] = tVec3fPair( mHighEdge[0], mLowEdge[0] );
		segments[1] = tVec3fPair( mHighEdge[1], mLowEdge[1] );
		segments[2] = tVec3fPair( mHighEdge[0], mHighEdge[1] );
		segments[3] = tVec3fPair( mLowEdge[0], mLowEdge[1] );

		f32 bestDist = Math::cInfinity;
		for( u32 i = 0; i < segments.fCount( ); ++i )
		{
			const tVec3fPair& seg = segments[ i ];

			f32 length;
			const tVec3f normalized = ( seg.mA - seg.mB ).fNormalize( length );
			const f32 t = fClamp<f32>( normalized.fDot( pos - seg.mB ), 0, length );
			const tVec3f segPos = seg.mB + normalized * t;
			const f32 dist = pos.fDistance( segPos );
			if( dist < bestDist )
			{
				bestDist = dist;
				*outPos = segPos;
			}
		}
		return bestDist;
	}

	Math::tAabbf tBuiltPathEdge::fConstructAabb( ) const
	{
		Math::tAabbf box;
		box.fInvalidate( );
		box |= mHighEdge[0];
		box |= mHighEdge[1];
		box |= mLowEdge[0];
		box |= mLowEdge[1];

		sigassert( box.fContains( mHighEdge[0] ) );
		sigassert( box.fContains( mHighEdge[1] ) );
		sigassert( box.fContains( mLowEdge[0] ) );
		sigassert( box.fContains( mLowEdge[1] ) );
		return box;
	}

	//------------------------------------------------------------------------------
	// tBuiltPathHop
	//------------------------------------------------------------------------------
	f32 tBuiltPathHop::fCost( ) const
	{
		return mTravelCost + mHeurCost * Debug_BuiltNavGraph_HeuristicWeight;
	}

	//------------------------------------------------------------------------------
	// tBuiltNavGraph
	//------------------------------------------------------------------------------
	tBuiltNavGraph::tBuiltNavGraph( )
		: mNavGraphID( ~0 )
	{}

	tBuiltNavGraph::tBuiltNavGraph( tDynamicArray<tBuiltPathNode>& nodes, tDynamicArray<tBuiltPathEdge>& edges )
		: mNodes( nodes )
		, mEdges( edges )
		, mNavGraphID( ~0 )
	{
		fBuildTrees( );
	}

	tBuiltNavGraph::tBuiltNavGraph( tNoOpTag )
		: mNodes( cNoOpTag )
		, mEdges( cNoOpTag )
		, mNavGraphID( ~0 )
		, mBadEdges( cNoOpTag )
		, mBadPos( cNoOpTag )
	{}

	tBuiltNavGraph::~tBuiltNavGraph( )
	{
	}

	void tBuiltNavGraph::fSetRoot( const tPathEntityPtr& root, u32 navGraphID )
	{
		mNavGraphID = navGraphID; //NOTE: navGraphID of ~0 is VALID, it is considered to be the "default" navgraph.
	}

	void tBuiltNavGraph::fInit( )
	{
		std::stringstream ss;
		ss << "BuiltNavGraph (";
		if( mNavGraphID == ~0 )
			ss << "~0";
		else
			ss << mNavGraphID;
		ss << ") " << mNodes.fCount( ) << " nodes " << mEdges.fCount( ) << " edges";
		log_line( 0, ss.str( ) );

		if( Debug_BuiltNavGraph_RunTests )
			fRunPerfTest( );
	}

	void tBuiltNavGraph::fBuildTrees( )
	{
		// Build the nodes KD-Tree
		tDynamicArray< Math::tAabbf > nodeAabbs( mNodes.fCount( ) );
		for( u32 i = 0; i < nodeAabbs.fCount( ); ++i )
			nodeAabbs[ i ] = mNodes[ i ].mWorldSpaceObb.fToAabb( );

		mNodeTree.fConstruct( nodeAabbs );

		// Build the edges KD-Tree
		tDynamicArray< Math::tAabbf > edgeAabbs( mEdges.fCount( ) );
		for( u32 i = 0; i < mEdges.fCount( ); ++i )
			edgeAabbs[ i ] = mEdges[ i ].fConstructAabb( );

		mEdgeTree.fConstruct( edgeAabbs );
	}

	void tBuiltNavGraph::fGetNodes( const Math::tVec3f& pos, tGrowableArray<u32>& items ) const
	{
		mNodeTree.fCollectItems( items, Math::tAabbf( Math::tSpheref( pos, 0.f ) ) );
	}

	void tBuiltNavGraph::fGetEdges( const Math::tVec3f& pos, tGrowableArray<u32>& items ) const
	{
		Math::tAabbf aabb( Math::tSpheref( pos, 1.f ) );
		aabb |= Math::tVec3f( 0, Math::cInfinity, 0 );
		aabb |= Math::tVec3f( 0, -Math::cInfinity, 0 );
		mEdgeTree.fCollectItems( items, aabb );
	}

	void tBuiltNavGraph::fRunPerfTest( )
	{
		profile_pix( "fRunPerfTest" );

		f32 worstTime = 0;
		f32 avgTime = 0;
		u32 worstStart = 0;
		u32 worstEnd = 0;
		const u32 totalNodes = mNodes.fCount( );

		Time::tStopWatch timer;

		for( u32 start = 0; start < totalNodes; ++start )
		{
			for( u32 end = 0; end < totalNodes; ++end )
			{
				timer.fResetElapsedS( );
				timer.fStart( );

				tPath path;
				path.mDesiredStart = fGetNodePos( start );
				path.mDesiredEnd = fGetNodePos( end );
				fFindPath( path );

				//if( start <= 100 )
				//{
				//	log_output( 0, start << "-" << end << " l-" << path.fLen( ) << " : " );
				//	for( u32 i = 0; i < path.fCount( ); ++i )
				//		log_output( 0, path[i] << ", " );
				//	log_newline( );
				//}

				timer.fStop( );
				const f32 elapsed = timer.fGetElapsedMs( );
				avgTime += elapsed;
				if( elapsed > worstTime )
				{
					worstTime = elapsed;
					worstStart = start;
					worstEnd = end;
				}
			}
		}
		const f32 totalTime = avgTime;
		avgTime /= totalNodes * totalNodes;
		log_line( 0, "BuiltNavGraph PerfTest - TotalTime: " << totalTime << "ms AvgTime: " << avgTime << "ms WorstTime: node " << worstStart << " to " << worstEnd << " in " << worstTime << "ms" );
	}
	void tBuiltNavGraph::fConvertToPath( const tBuiltPathHop* pathHopEnd, tPath& path ) const
	{
		const tBuiltPathHop* ph = pathHopEnd;
		while( ph->mPrev )
		{
			path.mNodePath.fPushBack( ph->mNode );
			path.mPosPath.fPushBack( mNodes[ ph->mNode ].fPos( ) );
			const tBuiltPathHop* tempPrev = ph->mPrev; //somehow ph = ph->mPrev caused ph to NULL out so leave tempPrev here!
			ph = tempPrev;
		}
		//path.mNodePath.fPushBack( ph->mNode ); //start node
	}

	tVec3f tBuiltNavGraph::fGetNodePos( u32 i ) const
	{
		return mNodes[ i ].mObjToWorld.fGetTranslation( );
	}

	void tBuiltNavGraph::fDebugDraw( )
	{
#ifdef sig_devmenu

		if( Debug_BuiltNavGraph_DrawKdTree )
			mNodeTree.fDraw( Math::tMat3f::cIdentity, tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ), Debug_BuiltNavGraph_KdTreeRenderLevel-1 );

		if( Debug_BuiltNavGraph_DrawEdgeKdTree )
			mEdgeTree.fDraw( Math::tMat3f::cIdentity, tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ), Debug_BuiltNavGraph_KdTreeRenderLevel-1 );

		if( Debug_BuiltNavGraph_DrawCrapEdges )
		{
			for( u32 i = 0; i < mBadEdges.fCount( ); ++i )
			{
				fDrawAreaNode( mNodes[ mBadEdges[ i ].mNodeA ].mWorldSpaceObb );
				fDrawAreaNode( mNodes[ mBadEdges[ i ].mNodeB ].mWorldSpaceObb );

				fDrawEdge( tVec3fPair( mBadEdges[ i ].mHighEdge[0], mBadEdges[ i ].mHighEdge[1] ) );
				fDrawEdge( tVec3fPair( mBadEdges[ i ].mLowEdge[0], mBadEdges[ i ].mLowEdge[1] ) );
				fDrawEdge( tVec3fPair( mBadEdges[ i ].mHighEdge[0], mBadEdges[ i ].mLowEdge[1] ) );
				fDrawEdge( tVec3fPair( mBadEdges[ i ].mHighEdge[1], mBadEdges[ i ].mLowEdge[0] ) );
			}
		}

		if( Debug_BuiltNavGraph_LogBadPositions )
		{
			for( u32 i = 0; i < mBadPos.fCount( ); ++i )
				fDrawNode( mBadPos[ i ] );
		}

		if( !Debug_BuiltNavGraph_RenderGraph )
			return;

		if( Debug_BuiltNavGraph_Render_Connections )
		{
			for( u32 i = 0; i < mEdges.fCount( ); ++i )
			{
				const u32 start = mEdges[ i ].mNodeA;
				const u32 end = mEdges[ i ].mNodeB;
				fDrawLine( fGetNodePos( start ), fGetNodePos( end ) );
			}
		}

		if( Debug_BuiltNavGraph_Render_Nodes )
		{
			for( u32 i = 0; i < mNodes.fCount( ); ++i )
			{
				fDrawAreaNode( mNodes[ i ].mWorldSpaceObb );
				fDrawNode( fGetNodePos( i ), cColorOfCenterNode );
			}
		}

		if( Debug_BuiltNavGraph_Render_Walkable )
		{
			for( u32 i = 0; i < mEdges.fCount( ); ++i )
			{
				//const u32 badParentNode = 29;
				//const u32 badChildNode = 26;
				//if( mEdges[ i ].mNodeA != badParentNode || mEdges[ i ].mNodeB != badChildNode )
				//	continue;

				fDrawEdge( tVec3fPair( mEdges[ i ].mHighEdge[0], mEdges[ i ].mHighEdge[1] ) );
				fDrawEdge( tVec3fPair( mEdges[ i ].mLowEdge[0], mEdges[ i ].mLowEdge[1] ) );
				fDrawEdge( tVec3fPair( mEdges[ i ].mHighEdge[0], mEdges[ i ].mLowEdge[1] ) );
				fDrawEdge( tVec3fPair( mEdges[ i ].mHighEdge[1], mEdges[ i ].mLowEdge[0] ) );
			}
		}

#endif//sig_devmenu
	}
	void tBuiltNavGraph::fDebugDraw( const tPath& path )
	{
#ifdef sig_devmenu
		if( path.fCount( ) <= 0 )
			return;

		for( u32 i = 1; i < path.fCount( ); ++i )
		{
			const tVec3f& ap = path[ i - 1 ];
			const tVec3f a( ap.x, ap.y + 2, ap.z );
			const tVec3f& bp = path[ i ];
			const tVec3f b( bp.x, bp.y + 2, bp.z );
			fDrawLine( a, b, cColorRed );
		}

		//draw nodes at start and end
		fDrawNode( path[ 0 ], tVec4f( 0.f, 1.f, 0.f, 1.f ) );
		fDrawNode( path[ path.fCount( ) - 1 ] );

#endif//sig_devmenu
	}
	void tBuiltNavGraph::fFindPath( tPath& path )
	{
		profile_pix( "tBuiltNavGraph::fFindPath" );
		Time::tStopWatch timer;

		if( mNodes.fCount( ) <= 0 )
		{
			path.mPosPath.fPushBack( path.mDesiredStart );
			path.mPosPath.fPushBack( path.mDesiredEnd );
			path.mCalcTime = timer.fGetElapsedMs( );
			return;
		}

		if( !fOnWalkable( path.mDesiredStart ) )
		{
			path.mOnWalkableStart = fGetClosestWalkablePos( path.mDesiredStart );

			if( Debug_BuiltNavGraph_LogBadPositions )
			{
				if( !fOnWalkable( path.mOnWalkableStart ) )
				{
					mBadPos.fPushBack( path.mOnWalkableStart );
					log_warning( 0, "(start) walkable pos " << path.mOnWalkableStart << " is not on walkable space" );
				}
			}
		}
		else
			path.mOnWalkableStart = path.mDesiredStart;

		if( !fOnWalkable( path.mDesiredEnd ) )
		{
			path.mOnWalkableEnd = fGetClosestWalkablePos( path.mDesiredEnd );
			if( Debug_BuiltNavGraph_LogBadPositions )
			{
				if( !fOnWalkable( path.mOnWalkableEnd ) )
				{
					mBadPos.fPushBack( path.mOnWalkableEnd );
					log_warning( 0, "(end) walkable pos " << path.mOnWalkableEnd << " is not on walkable space" );
				}
			}
		}
		else
			path.mOnWalkableEnd = path.mDesiredEnd;

		const u32 startNode = fFindClosestNode( path.mOnWalkableStart );
		const u32 endNode = fFindClosestNode( path.mOnWalkableEnd );

		path.mPosPath.fDeleteArray( );
		path.mNodePath.fDeleteArray( );

		fSearchUsingAStar( startNode, endNode, path );
		fConvertToPosPath( path );
		path.mCalcTime = timer.fGetElapsedMs( );
	}

	b32 tBuiltNavGraph::fOnWalkable( const Math::tVec3f& pos ) const
	{
		profile_pix( "fOnWalkable" );
		if( Debug_BuiltNavGraph_UseKDTree )
		{
			profile_pix( "on walk node kd tree" );
			tGrowableArray<u32> items;
			f32 radius = 1.f;
			while( items.fCount( ) <= 0 )
			{
				const f32 temp = radius * 5.f; //prevent LHS
				const Math::tSpheref grabSphere( pos, radius );
				mNodeTree.fCollectItems( items, Math::tAabbf( grabSphere ) );
				radius *= 10.f;
				if( temp >= 1000.f ) //~4 tries total
					break;
			}

			for( u32 i = 0; i < items.fCount( ); ++i )
			{
				if( mNodes[ items[i] ].fOnNode( pos ) )
					return true;
			}
		}
		else
		{
			// if we are in a node return true
			for( u32 i = 0; i < mNodes.fCount( ); ++i )
			{
				if( mNodes[ i ].fOnNode( pos ) )
					return true;
			}
		}

		if( Debug_BuiltNavGraph_UseKDTree )
		{
			profile_pix( "on walk edge kd tree" );
			tGrowableArray<u32> items;
			fGetEdges( pos, items );
			for( u32 i = 0; i < items.fCount( ); ++i )
			{
				if( mEdges[ items[i] ].fOnEdge( pos ) )
					return true;
			}
		}
		else
		{
			//if we are in an edge return true
			for( u32 i = 0; i < mEdges.fCount( ); ++i )
			{
				if( mEdges[ i ].fOnEdge( pos ) )
					return true;
			}
		}
		return false;
	}

	u32 tBuiltNavGraph::fFindClosestNode( const tVec3f& pos ) const
	{
		profile_pix( "fFindClosestNode" );
		//if we are on a node already, use that node
		if( Debug_BuiltNavGraph_UseKDTree )
		{
			profile_pix( "find node kd tree" );
			tGrowableArray< u32 > nodes;
			fGetNodes( pos, nodes );
			for( u32 i = 0; i < nodes.fCount( ); ++i )
			{
				if( mNodes[ nodes[i] ].fOnNode( pos ) )
					return nodes[i];
			}
		}
		else
		{
			for( u32 i = 0; i < mNodes.fCount( ); ++i )
			{
				if( mNodes[ i ].fOnNode( pos ) )
					return i;
			}
		}

		u32 best = ~0;
		f32 bestDist = Math::cInfinity;
		if( Debug_BuiltNavGraph_UseKDTree )
		{
			profile_pix( "find edge kd tree" );
			tGrowableArray<u32> items;
			fGetEdges( pos, items );
			for( u32 i = 0; i < items.fCount( ); ++i )
			{
				const tBuiltPathEdge& edge = mEdges[ items[i] ];
				if( edge.fOnEdge( pos ) )
				{
					const tVec3f posA = fGetNodePos( edge.mNodeA );
					const tVec3f posB = fGetNodePos( edge.mNodeB );
					const f32 distA = pos.fDistance( posA );
					const f32 distB = pos.fDistance( posB );

					if( distA < bestDist )
					{
						bestDist = distA;
						best = edge.mNodeA;
					}
					if( distB < bestDist )
					{
						bestDist = distB;
						best = edge.mNodeB;
					}
				}
			}
		}
		else
		{
			//we must be on an edge
			for( u32 i = 0; i < mEdges.fCount( ); ++i )
			{
				if( mEdges[ i ].fOnEdge( pos ) )
				{
					const tVec3f posA = fGetNodePos( mEdges[ i ].mNodeA );
					const tVec3f posB = fGetNodePos( mEdges[ i ].mNodeB );
					const f32 distA = pos.fDistance( posA );
					const f32 distB = pos.fDistance( posB );

					if( distA < bestDist )
					{
						bestDist = distA;
						best = mEdges[ i ].mNodeA;
					}
					if( distB < bestDist )
					{
						bestDist = distB;
						best = mEdges[ i ].mNodeB;
					}
				}
			}
		}

		//but just in case we aren't on a node or edge
		if( best == ~0 )
		{
			best = 0;
			bestDist = fGetNodePos( 0 ).fDistance( pos );
			for( u32 i = 1; i < mNodes.fCount( ); ++i )
			{
				const f32 tempDist = fGetNodePos( i ).fDistance( pos );
				if( tempDist < bestDist )
				{
					bestDist = tempDist;
					best = i;
				}
			}
		}
		return best;
	}

	void tBuiltNavGraph::fDeletePathHops( tDynamicArray<tBuiltPathHop*>& allPathHops )
	{
		for( u32 i = 0; i < allPathHops.fCount( ); ++i )
		{
			if( allPathHops[i] )
			{
				delete allPathHops[i];
				allPathHops[i] = NULL;
			}
		}
	}

	void tBuiltNavGraph::fSearchUsingAStar( u32 startNode, u32 endNode, tPath& path )
	{
		profile_pix( "fSearchUsingAStar" );
		tDynamicArray<tBuiltPathHop*> allPathHops( mNodes.fCount( ) );
		allPathHops.fFill( NULL );

		const tVec3f endNodePos = fGetNodePos( endNode );
		allPathHops[ startNode ] = NEW tBuiltPathHop;
		tBuiltPathHop* pathHop = allPathHops[ startNode ];
		pathHop->mNode = startNode;
		pathHop->mTravelCost = 0;
		pathHop->mHeurCost = fGetNodePos( startNode ).fDistance( endNodePos );
		pathHop->mTotCost = pathHop->fCost( );


		tGrowableArray<tBuiltPathHop*> nodeQueue;
		pathHop->mInQueue = true;
		nodeQueue.fPushBack( pathHop );

		while( nodeQueue.fCount( ) )
		{
			sigassert( nodeQueue.fCount( ) <= allPathHops.fCount( ) );

			pathHop = nodeQueue.fPopBack( ); //fGet( ) pops as well
			sigassert( pathHop->mInQueue );
			pathHop->mInQueue = false;

			if( pathHop->mNode == endNode )
				break;

			b32 doSort = false;

			const tBuiltPathNode& node = mNodes[ pathHop->mNode ];

			for( u32 i = 0; i < node.mEdges.fCount( ); ++i )
			{
				const u32 nextNodeIndex = node.mEdges[ i ];
				tBuiltPathHop* nextPathHop = allPathHops[ nextNodeIndex ];
				const tVec3f nextNodePos = fGetNodePos( nextNodeIndex );

				if( nextPathHop )
				{
					//only update if better path
					Math::tVec3f pastPos = fGetNodePos( pathHop->mNode );
					const f32 newTravelCost = pathHop->mTravelCost + pastPos.fDistance( nextNodePos );
					const f32 newHeurCost = nextNodePos.fDistance( endNodePos );
					if( newTravelCost < nextPathHop->mTravelCost )
					{
						nextPathHop->mNode = nextNodeIndex;
						nextPathHop->mPrev = pathHop;						
						nextPathHop->mTravelCost = newTravelCost;
						nextPathHop->mHeurCost = newHeurCost;
						nextPathHop->mTotCost = nextPathHop->fCost( );
						if( !nextPathHop->mInQueue )
						{
							nextPathHop->mInQueue = true;
							nodeQueue.fPushBack( nextPathHop );
							doSort = true;
						}
					}
				}
				else
				{
					allPathHops[ nextNodeIndex ] = NEW tBuiltPathHop;
					nextPathHop = allPathHops[ nextNodeIndex ];
					nextPathHop->mNode = nextNodeIndex;
					nextPathHop->mPrev = pathHop;
					nextPathHop->mTravelCost = pathHop->mTravelCost + fGetNodePos( pathHop->mNode ).fDistance( nextNodePos );
					nextPathHop->mHeurCost = nextNodePos.fDistance( endNodePos );
					nextPathHop->mTotCost = nextPathHop->fCost( );
					nextPathHop->mInQueue = true;
					nodeQueue.fPushBack( nextPathHop );
					doSort = true;
				}
			}//loop across edges

			// Do sort.
			if( doSort )
				std::sort( nodeQueue.fBegin( ), nodeQueue.fEnd( ), tPathHopLessThan( ) );
		}

		if( pathHop->mNode == endNode )
		{
			path.mPosPath.fPushBack( path.mOnWalkableEnd );
			path.mNodePath.fPushBack( endNode );

			fConvertToPath( pathHop, path );

			path.mPosPath.fPushBack( path.mOnWalkableStart );
			path.mNodePath.fPushBack( startNode );

			sigassert( path.mNodePath.fFront( ) == endNode );
			sigassert( path.mNodePath.fBack( ) == startNode );
			sigassert( path.mPosPath.fCount( ) == path.mNodePath.fCount( ) );
		}
		else
			log_warning( 0, "A* algorithm failed to find path between node index " << startNode << " and " << endNode );

		fDeletePathHops( allPathHops );
	}
	b32 tBuiltNavGraph::fContainsSameEdge( const tVec3f& aPos, const tVec3f& bPos ) const
	{
		for( u32 i = 0; i < mEdges.fCount( ); ++i )
		{
			if( mEdges[ i ].fOnEdge( aPos ) && mEdges[ i ].fOnEdge( bPos ) )
				return true;
		}
		return false;
	}
	void tBuiltNavGraph::fConvertToPosPath( tPath& path )
	{
		profile_pix( "fConvertToPosPath" );
		//NOTE: path is assumed to be in reverse order
		const Math::tVec3f& startPos = path.mOnWalkableStart;
		const Math::tVec3f& endPos = path.mOnWalkableEnd;

		//early out
		if( path.fCount( ) <= 2 && fContainsSameEdge( startPos, endPos ) )
		{
			path.mPosPath.fDeleteArray( );
			path.mPosPath.fPushBack( endPos );
			return;
		}

		tVec3f lastPos = startPos;
		if( Debug_BuiltNavGraph_PullNodes )
		{
			//pull nodes toward closest edge
			for( s32 i = path.fCount( ) - 1; i >= 1; --i )
			{
				tVec3f nextPos = path.mPosPath[i - 1];

				tVec3f interpPos = (lastPos + nextPos) / 2.f;
				tVec3f thisPos = mNodes[ path.mNodePath[i] ].fCastRayToCenter( interpPos );

				lastPos = thisPos;
				path.mPosPath[i] = thisPos;
			}
		}

		// cull out pieces.
		s32 startIndex = path.fCount( ) - 1;
		s32 endIndex = 0;
		if( path.fCount( ) >= 3 )
		{
			//prevent us from moving backwards to reach next node
			const tBuiltPathEdge* firstEdge = fGetEdge( path.mNodePath[ startIndex ], path.mNodePath[ startIndex - 1 ] );
			if( firstEdge && firstEdge->fOnEdge( startPos ) )
				--startIndex;

			//prevent us from trying to go to the node past our target if we are on the same edge
			const tBuiltPathEdge* lastEdge = fGetEdge( path.mNodePath[ endIndex + 2 ], path.mNodePath[ endIndex + 1 ] );
			if( lastEdge && lastEdge->fOnEdge( endPos ) )
			{
				path.mNodePath.fEraseOrdered( 1 );
				path.mPosPath.fEraseOrdered( 1 );
				--startIndex;
			}
		}

		tPosPath finalPath;
		lastPos = startPos;
		for( s32 i = startIndex; i >= endIndex+1; --i )
		{
			tVec3f middlePos = path.mPosPath[i - 1];

			//skip this node if we can hit the next node's pos no problem.
			if( mNodes[ path.mNodePath[i] ].fCastRayToFinalPos( lastPos, middlePos ) )
				continue;

			lastPos = path.mPosPath[i];
			finalPath.fPushBack( path.mPosPath[i] );
		}
		finalPath.fPushBack( path.mPosPath[endIndex] );

		path.mPosPath = finalPath;
	}
	const tBuiltPathEdge* tBuiltNavGraph::fGetEdge( u32 nodeA, u32 nodeB ) const
	{
		for( u32 i = 0; i < mEdges.fCount( ); ++i )
		{
			const b32 isAtoB = mEdges[ i ].mNodeA == nodeA && mEdges[ i ].mNodeB == nodeB;
			const b32 isBtoA = mEdges[ i ].mNodeA == nodeB && mEdges[ i ].mNodeB == nodeA;

			if( isAtoB || isBtoA )
				return &mEdges[ i ];
		}
		return NULL;
	}
	tVec3f tBuiltNavGraph::fGetClosestWalkablePos( const tVec3f& pos ) const
	{
		profile_pix( "fGetClosestWalkablePos" );
		f32 bestDist = Math::cInfinity;
		tVec3f bestPos = pos;

		// Check edges.
		if( Debug_BuiltNavGraph_UseKDTree )
		{
			tGrowableArray<u32> items;
			Math::tSpheref grabSphere( pos, 8.f );

			s32 maxAttempts = 4;
			while( items.fCount( ) == 0 )
			{
				mEdgeTree.fCollectItems( items, Math::tAabbf( grabSphere ) );
				grabSphere.mRadius *= 4.f;

				if( --maxAttempts <= 0 )
					break;
			}

			for( u32 i = 0; i < items.fCount( ); ++i )
			{
				tVec3f tempPos;
				const f32 tempDist = mEdges[ items[ i ] ].fClosestPoint( pos, &tempPos );
				if( tempDist < bestDist )
				{
					bestDist = tempDist;
					bestPos = tempPos;
				}
			}
		}
		else
		{
			for( u32 i = 0; i < mEdges.fCount( ); ++i )
			{
				tVec3f tempPos;
				const f32 tempDist = mEdges[ i ].fClosestPoint( pos, &tempPos );
				if( tempDist < bestDist )
				{
					bestDist = tempDist;
					bestPos = tempPos;
				}
			}
		}

		// Check nodes.
		if( Debug_BuiltNavGraph_UseKDTree )
		{
			tGrowableArray<u32> items;
			Math::tSpheref grabSphere( pos, 8.f );

			s32 maxAttempts = 4;
			while( items.fCount( ) == 0 )
			{
				mNodeTree.fCollectItems( items, Math::tAabbf( grabSphere ) );
				grabSphere.mRadius *= 4.f;

				if( --maxAttempts <= 0 )
					break;
			}

			for( u32 i = 0; i < items.fCount( ); ++i )
			{
				const tVec3f tempPos = mNodes[ items[i] ].fCastRayToCenter( pos );

				const f32 tempDist = tempPos.fDistance( pos );
				if( tempDist < bestDist )
				{
					bestDist = tempDist;
					bestPos = tempPos;
				}
			}
		}
		else
		{
			for( u32 i = 0; i < mNodes.fCount( ); ++i )
			{
				const tVec3f tempPos = mNodes[ i ].fCastRayToCenter( pos );
				const f32 tempDist = tempPos.fDistance( pos );
				if( tempDist < bestDist )
				{
					bestDist = tempDist;
					bestPos = tempPos;
				}
			}
		}

		if( Debug_BuiltNavGraph_AssertOnBadPos )
			log_assert( bestDist != Math::cInfinity, "Failed to find closest pos to " << pos << " edges: " << mEdges.fCount( ) << " nodes: " << mNodes.fCount( ) );

		return bestPos;
	}
}}
