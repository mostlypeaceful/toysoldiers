#include "BasePch.hpp"
#include "tNavGraph.hpp"
#include "tApplication.hpp"
#include "tStack.hpp"
#include "tPriorityQueue.hpp"
#include "Math/tIntersectionRaySphere.hpp"

using namespace Sig::Math;

namespace Sig{ namespace AI{
	devvar( bool,	Debug_NavGraph_Generate,			true );
	devvar( bool,	Debug_NavGraph_RenderGraph,			false );
	devvar( bool,	Debug_NavGraph_Render_Connections,	false );
	devvar( bool,	Debug_NavGraph_Render_Nodes,		true );
	devvar( bool,	Debug_NavGraph_Render_Walkable,		true );
	devvar( bool,	Debug_NavGraph_Render_HalfSpaces,	false );
	devvar( bool,	Debug_NavGraph_RunTests,			false ); //this WILL hitch the game
	devvar( bool,	Debug_Navgraph_LogBadPositions,		false );
	devvar( bool,	Debug_NavGraph_AssertOnBadPos,		false );
	devvar( bool,	Debug_NavGraph_DrawCrapEdges,		false );
	devvar( float,	Debug_NavGraph_HeuristicWeight,		3 );
	devvar( float,	Debug_NavGraph_InWalkableEpsilon, 0.001f );

	namespace {
		const tVec3f cStatsPos( 400, 20, 0.5 );
		const f32 cStatsWidth = 500;
		const tVec4f cColorOfEllipsoid( 1, 0, 1, 0.25 );
		const tVec4f cColorOfCenterNode( 0, 1, 1, 0.5 );
		const f32 cNodeRadius = 1.0f;
		const tVec4f cColorRed( 1, 0, 0, 1 );
		const tVec4f cColorBlue( 0, 0, 1, 1 );
		const tVec4f cColorCyan( 0, 1, 1, 1 );
		inline f32 fDist( const tVec3f& a, const tVec3f& b )
		{
			return (a-b).fLength( ); //this cannot be fLengthSquared( ). if it is non-optimal paths will be found.
			// Why? MATH.
			// pathA -------------- length = 10.		10^2 = 100
			// pathB ------|------- length = 5 + 5.		5^2 + 5^2 = 50  <-- big difference!
		}
		void fDrawAreaNode( const tMat3f& xform )
		{
			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tSpheref( tVec3f::cZeroVector, 1 ), xform, cColorOfEllipsoid );
		}
		void fDrawNode( const tVec3f& center, const tVec4f& color = cColorRed )
		{
			tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( ).fRenderOnce( tSpheref( center, cNodeRadius ), color );
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
			inline b32 operator( )( const tPathHopPtr& pathA, const tPathHopPtr& pathB )
			{
				return pathA->fCost( ) < pathB->fCost( );
			}
		};
	}//unnamed namespace

	tVec3f tPathNode::fCastRayToCenter( const Math::tVec3f& rayOrigin ) const
	{
		const tVec3f nodePos = mEntity->fObjectToWorld( ).fGetTranslation( );
		const tMat3f& worldToObj = mEntity->fWorldToObject( );

		//check to see if point is in sphere first
		const tVec3f posInSphereSpace = worldToObj.fXformPoint( tVec3f( rayOrigin.x, nodePos.y, rayOrigin.z ) );
		if( posInSphereSpace.fXZLength( ) < 1 ) //because we are in the sphere's space we are checking against a unit sphere's radius
			return rayOrigin;

		//not in sphere so do ray check
		Math::tRayf ray( rayOrigin, nodePos - rayOrigin );
		ray.mOrigin.y = nodePos.y;
		ray.mExtent.y = 0;
		const tIntersectionRaySphere<f32> test( ray.fTransform( worldToObj ), tSpheref( tVec3f::cZeroVector, 1 ) );
		//sigassert( test.fIntersects( ) );
		if( !test.fIntersects( ) )
		{
			log_warning( "Failed to intersect a node that we are shooting a ray directly at. WTF. Figure this out when you have time..." << std::endl
				<< "Ray o:" << ray.mOrigin << " e:" << ray.mExtent << " t:" << test.fT( ) << std::endl
				<< "sphere pt len: " << posInSphereSpace.fLength( ) );
			return nodePos;
		}
		const f32 time = test.fT( );
		return ray.fPointAtTime( time );
	}
	b32 tPathNode::fOnNode( const Math::tVec3f& p ) const
	{
		return ( p - fCastRayToCenter( p ) ).fIsZero( Debug_NavGraph_InWalkableEpsilon );
	}

	b32 tPathEdge::fOnEdge( const Math::tVec3f& p ) const
	{
		for( u32 i = 0; i < mHalfSpaces.fCount( ); ++i )
		{
			const f32 dist = mHalfSpaces[ i ].fSignedDistance( p );
			if( dist < -Debug_NavGraph_InWalkableEpsilon )
				return false;
		}
		return true;
	}
	f32 tPathEdge::fClosestPoint( const Math::tVec3f& pos, Math::tVec3f* outPos ) const
	{
		tGrowableArray<tVec3fPair> segments;
		segments.fPushBack( tVec3fPair( mHighEdge.mA, mLowEdge.mA ) );
		segments.fPushBack( tVec3fPair( mHighEdge.mB, mLowEdge.mB ) );
		segments.fPushBack( mHighEdge );
		segments.fPushBack( mLowEdge );

		f32 bestDist = Math::cInfinity;
		for( u32 i = 0; i < segments.fCount( ); ++i )
		{
			const tVec3fPair& seg = segments[ i ];

			f32 length;
			const tVec3f normalized = ( seg.mA - seg.mB ).fNormalize( length );
			f32 t = normalized.fDot( pos - seg.mB );
			if( t < 0 )
				t = 0;
			if( t > length )
				t = length;
			const tVec3f segPos = seg.mB + normalized * t;
			const f32 dist = fDist( pos, segPos );
			if( dist < bestDist )
			{
				bestDist = dist;
				*outPos = segPos;
			}
		}
		return bestDist;
	}


	f32 tPathHop::fCost( ) const
	{
		return mTravelCost + mHeurCost * Debug_NavGraph_HeuristicWeight;
	}

	tPath::tPath( )
		: mDesiredStart		( Math::tVec3f::cZeroVector )
		, mDesiredEnd		( Math::tVec3f::cZeroVector )
		, mOnWalkableStart	( Math::tVec3f::cZeroVector )
		, mOnWalkableEnd	( Math::tVec3f::cZeroVector )
		, mCalcTime( -1.f )
	{}

	f32 tPath::fLen( )
	{
		f32 len = 0.f;
		for( u32 i = 0; i < mPosPath.fCount( )-1; ++i )
		{
			const Math::tVec3f iPos = mPosPath[i];
			const Math::tVec3f iPosPlus = mPosPath[i+1];
			len += (iPosPlus - iPos).fLength( );
		}
		return len;
	}

	tNavGraph::tNavGraph( )
		: mNavGraphID( ~0 )
	{}
	tNavGraph::~tNavGraph( )
	{
		fClearData( );
	}
	void tNavGraph::fSetRoot( const tPathEntityPtr& root, u32 navGraphID )
	{
		mRoot = root; //we'll init later.
		mNavGraphID = navGraphID; //NOTE: navGraphID of ~0 is VALID, it is considered to be the "default" navgraph.
	}
	void tNavGraph::fInit( )
	{
		sigassert( mRoot );
		fClearData( );

		if( Debug_NavGraph_Generate )
		{
			fGenerateGraph( mRoot.fGetRawPtr( ) );

			if( Debug_NavGraph_RunTests )
				fRunPerfTest( );
		}
	}
	void tNavGraph::fClearData( )
	{
		mNodes.fSetCount( 0 );
		mEdges.fSetCount( 0 );
	}
	void tNavGraph::fGenerateGraph( tPathEntity* root )
	{
		Time::tStopWatch timer; //START!

		tStack<tPathEntity*> nodeStack;
		tHashTable<tPathEntity*, u32> entityTable;
		nodeStack.fPush( root );
		fGetOrAddNode( entityTable, nodeStack.fTop( ) );

		while( nodeStack.fCount( ) )
		{
			tPathEntity* nodeEntity = nodeStack.fTop( );
			u32 nodeIndex = fGetOrAddNode( entityTable, nodeEntity );
			nodeStack.fPop( );
			

			for( u32 i = 0; i < nodeEntity->fNextPointCount( ); ++i )
			{
				tPathEntity* nextNodeEntity = nodeEntity->fNextPoint( i );
				u32 nextNodeIndex = fGetOrAddNode( entityTable, nextNodeEntity );

				u32* edgeIndex = mNodes[ nodeIndex ].mEdges.fFind( nextNodeIndex );
				if( edgeIndex )
				{
					//we've already processed this edge
					continue;
				}

				fAddEdge( nodeIndex, nextNodeIndex );

				nodeStack.fPush( nextNodeEntity );
			}
			
			for( u32 i = 0; i < nodeEntity->fPrevPointCount( ); ++i )
			{
				tPathEntity* nextNodeEntity = nodeEntity->fPrevPoint( i );
				u32 nextNodeIndex = fGetOrAddNode( entityTable, nextNodeEntity );

				u32* edgeIndex = mNodes[ nodeIndex ].mEdges.fFind( nextNodeIndex );
				if( edgeIndex )
				{
					//we've already processed this edge
					continue;
				}

				fAddEdge( nodeIndex, nextNodeIndex );

				nodeStack.fPush( nextNodeEntity );
			}
		}

		sigassert( entityTable.fGetItemCount( ) == mNodes.fCount( ) );
		std::stringstream ss;
		ss << "NavGraph (";
		if( mNavGraphID == ~0 )
			ss << "~0";
		else
			ss << mNavGraphID;
		ss << ") Generated: " << mNodes.fCount( ) << " nodes " << mEdges.fCount( ) << " edges in " << timer.fGetElapsedMs( ) << "ms";
		log_line( 0, ss.str( ) );
	}
	void tNavGraph::fRunPerfTest( )
	{
		f32 worstTime = 0;
		f32 avgTime = 0;
		u32 worstStart = 0;
		u32 worstEnd = 0;
		const u32 totalNodes = mNodes.fCount( );
		for( u32 start = 0; start < totalNodes; ++start )
		{
			for( u32 end = 0; end < totalNodes; ++end )
			{
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

				avgTime += path.mCalcTime;
				if( path.mCalcTime > worstTime )
				{
					worstTime = path.mCalcTime;
					worstStart = start;
					worstEnd = end;
				}
			}
		}
		avgTime /= totalNodes * totalNodes;
		log_line( 0, "NavGraph PerfTest - AvgTime: " << avgTime << "ms WorstTime: node " << worstStart << " to " << worstEnd << " in " << worstTime << "ms" );
	}
	u32 tNavGraph::fGetOrAddNode( tHashTable<tPathEntity*, u32>& entityTable, tPathEntity* node )
	{
		const u32* nodeIndexFromTable = entityTable.fFind( node );
		if( nodeIndexFromTable )
			return *nodeIndexFromTable;

		u32 nodeIndex = fAddNode( node );
		entityTable.fInsert( node, nodeIndex );
		return nodeIndex;
	}
	u32 tNavGraph::fAddNode( tPathEntity* node )
	{
		tPathNode n;
		n.mEntity = tPathEntityPtr( node );
		mNodes.fPushBack( n );
		return mNodes.fCount( ) - 1;
	}
	tVec3fPair tNavGraph::fGetHighLowEdgePts( const tVec3f& normal, u32 node ) const
	{
		const tVec3f nodePos = fGetNodePos( node );
		const tVec3fPair nodeExtents = fGetNodeExtents( node );

		tFixedArray<tVec3f,4> pts;
		pts[ 0 ] = nodePos + nodeExtents.mA;
		pts[ 1 ] = nodePos - nodeExtents.mA;
		pts[ 2 ] = nodePos + nodeExtents.mB;
		pts[ 3 ] = nodePos - nodeExtents.mB;

		u32 highIndex = 0, lowIndex = 0;
		f32 highVal = pts[ 0 ].fDot( normal );
		f32 lowVal = pts[ 0 ].fDot( normal );

		for( u32 i = 1; i < pts.fCount( ); ++i )
		{
			const f32 val = pts[ i ].fDot( normal );
			if( val < lowVal )
			{
				lowVal = val;
				lowIndex = i;
			}
			if( val > highVal )
			{
				highVal = val;
				highIndex = i;
			}
		}

		return tVec3fPair( pts[ highIndex ], pts[ lowIndex ] );
	}
	void tNavGraph::fAddEdge( u32 parent, u32 child )
	{
		if( Debug_NavGraph_RunTests )
		{
			sigassert( fGetEdge( parent, child ) == NULL );
			sigassert( fGetEdge( child, parent ) == NULL );
		}

		tPathEdge edge;
		edge.mNodeA = parent;
		edge.mNodeB = child;

		//generate halfspaces for edge
		const tVec3f parentPos = fGetNodePos( parent );
		const tVec3f childPos = fGetNodePos( child );
		const tVec3f delta = ( parentPos - childPos ).fNormalize( );
		const tVec3f normal = tVec3f::cYAxis.fCross( delta ).fNormalize( );
		const tVec3fPair edgePtsParent = fGetHighLowEdgePts( normal, parent );
		const tVec3fPair edgePtsChild = fGetHighLowEdgePts( normal, child );
		const tVec3f parentNormal = tVec3f::cYAxis.fCross( edgePtsParent.mA	- edgePtsParent.mB ).fProjectToXZAndNormalize( );
		const tVec3f childNormal =	tVec3f::cYAxis.fCross( edgePtsChild.mB	- edgePtsChild.mA ).fProjectToXZAndNormalize( );
		const tVec3f highNormal =	tVec3f::cYAxis.fCross( edgePtsChild.mA	- edgePtsParent.mA ).fProjectToXZAndNormalize( );
		const tVec3f lowNormal =	tVec3f::cYAxis.fCross( edgePtsParent.mB - edgePtsChild.mB ).fProjectToXZAndNormalize( );
		
		edge.mHighEdge.mA = edgePtsParent.mA;
		edge.mHighEdge.mB = edgePtsChild.mA;
		edge.mLowEdge.mA = edgePtsParent.mB;
		edge.mLowEdge.mB = edgePtsChild.mB;

		edge.mHalfSpaces.fPushBack( tPlanef( parentNormal, parentPos ) );
		edge.mHalfSpaces.fPushBack( tPlanef( childNormal, childPos ) );
		edge.mHalfSpaces.fPushBack( tPlanef( highNormal, edgePtsParent.mA ) );
		edge.mHalfSpaces.fPushBack( tPlanef( lowNormal, edgePtsParent.mB ) );

		edge.mHalfSpaceCenters.fPushBack( parentPos );
		edge.mHalfSpaceCenters.fPushBack( childPos );
		edge.mHalfSpaceCenters.fPushBack( edgePtsParent.mA + ( edgePtsChild.mA	- edgePtsParent.mA ) / 2 );
		edge.mHalfSpaceCenters.fPushBack( edgePtsChild.mB  + ( edgePtsParent.mB - edgePtsChild.mB ) / 2 );

#ifdef sig_devmenu
		//VERIFY MATH
		const tVec3f halfDelta = ( parentPos - childPos ) / 2;
		const tVec3f ptInMiddle = childPos + halfDelta;
		if( !edge.fOnEdge( ptInMiddle ) )
		{
			log_warning( "Bad NavGraph Edge found!" );
			mBadEdges.fPushBack( edge );
		}
		//sigassert( edge.fOnEdge( ptInMiddle ) ); //this and all below _should_ be the case. sadly there are edge cases because we aren't generating tangent lines between nodes correctly
		//sigassert( edge.fOnEdge( parentPos ) );
		//sigassert( edge.fOnEdge( childPos ) );
		//sigassert( edge.fOnEdge( edge.mHighEdge.mA ) );
		//sigassert( edge.fOnEdge( edge.mHighEdge.mB ) );
		//sigassert( edge.fOnEdge( edge.mLowEdge.mA ) );
		//sigassert( edge.fOnEdge( edge.mLowEdge.mB ) );
#endif//sig_devmenu

		mEdges.fPushBack( edge );
		mNodes[ parent ].mEdges.fPushBack( child );
		mNodes[ child ].mEdges.fPushBack( parent );
	}
	tNodePath tNavGraph::fConvertToPath( const tPathHopPtr& pathHopEnd ) const
	{
		tNodePath path;
		tPathHopPtr ph = pathHopEnd;
		while( ph->mPrev )
		{
			path.fPushBack( ph->mNode );
			const tPathHopPtr tempPrev = ph->mPrev; //somehow ph = ph->mPrev caused ph to NULL out so leave tempPrev here!
			ph = tempPrev;
		}
		path.fPushBack( ph->mNode ); //start node
		return path;
	}
	tVec3f tNavGraph::fGetNodePos( u32 i ) const
	{
		return mNodes[ i ].mEntity->fObjectToWorld( ).fGetTranslation( );
	}
	tVec3fPair tNavGraph::fGetNodeExtents( u32 i ) const
	{
		const tMat3f& objToWorld = mNodes[ i ].mEntity->fObjectToWorld( );
		return tVec3fPair( objToWorld.fXformVector( tVec3f::cXAxis ), objToWorld.fXformVector( tVec3f::cZAxis ) );
	}
	void tNavGraph::fDebugDraw( )
	{
#ifdef sig_devmenu

		if( Debug_NavGraph_DrawCrapEdges )
		{
			for( u32 i = 0; i < mBadEdges.fCount( ); ++i )
			{
				fDrawAreaNode( mNodes[ mBadEdges[ i ].mNodeA ].mEntity->fObjectToWorld( ) );
				fDrawAreaNode( mNodes[ mBadEdges[ i ].mNodeB ].mEntity->fObjectToWorld( ) );

				fDrawEdge( mBadEdges[ i ].mHighEdge );
				fDrawEdge( mBadEdges[ i ].mLowEdge );
				fDrawEdge( tVec3fPair( mBadEdges[ i ].mHighEdge.mA, mBadEdges[ i ].mLowEdge.mA ) );
				fDrawEdge( tVec3fPair( mBadEdges[ i ].mHighEdge.mB, mBadEdges[ i ].mLowEdge.mB ) );

				for( u32 hsi = 0; hsi < mBadEdges[ i ].mHalfSpaces.fCount( ); ++hsi )
					fDrawPlane( mBadEdges[ i ].mHalfSpaces[ hsi ], mBadEdges[ i ].mHalfSpaceCenters[ hsi ] );
			}
		}

		if( Debug_Navgraph_LogBadPositions )
		{
			for( u32 i = 0; i < mBadPos.fCount( ); ++i )
				fDrawNode( mBadPos[ i ] );
		}

		if( !Debug_NavGraph_RenderGraph )
			return;

		if( Debug_NavGraph_Render_Connections )
		{
			for( u32 i = 0; i < mEdges.fCount( ); ++i )
			{
				const u32 start = mEdges[ i ].mNodeA;
				const u32 end = mEdges[ i ].mNodeB;
				fDrawLine( fGetNodePos( start ), fGetNodePos( end ) );
			}
		}

		if( Debug_NavGraph_Render_Nodes )
		{
			for( u32 i = 0; i < mNodes.fCount( ); ++i )
			{
				fDrawAreaNode( mNodes[ i ].mEntity->fObjectToWorld( ) );
				fDrawNode( fGetNodePos( i ), cColorOfCenterNode );
			}
		}

		if( Debug_NavGraph_Render_Walkable )
		{
			for( u32 i = 0; i < mEdges.fCount( ); ++i )
			{
				//const u32 badParentNode = 29;
				//const u32 badChildNode = 26;
				//if( mEdges[ i ].mNodeA != badParentNode || mEdges[ i ].mNodeB != badChildNode )
				//	continue;

				fDrawEdge( mEdges[ i ].mHighEdge );
				fDrawEdge( mEdges[ i ].mLowEdge );
				fDrawEdge( tVec3fPair( mEdges[ i ].mHighEdge.mA, mEdges[ i ].mLowEdge.mA ) );
				fDrawEdge( tVec3fPair( mEdges[ i ].mHighEdge.mB, mEdges[ i ].mLowEdge.mB ) );

				if( Debug_NavGraph_Render_HalfSpaces )
				{
					for( u32 hsi = 0; hsi < mEdges[ i ].mHalfSpaces.fCount( ); ++hsi )
						fDrawPlane( mEdges[ i ].mHalfSpaces[ hsi ], mEdges[ i ].mHalfSpaceCenters[ hsi ] );
				}
			}
		}

#endif//sig_devmenu
	}
	void tNavGraph::fDebugDraw( const tPath& path )
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
		fDrawNode( path[ 0 ] );
		fDrawNode( path[ path.fCount( ) - 1 ] );

#endif//sig_devmenu
	}
	void tNavGraph::fFindPath( tPath& path )
	{
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
			if( !fOnWalkable( path.mOnWalkableStart ) )
			{
				if( Debug_Navgraph_LogBadPositions )
					mBadPos.fPushBack( path.mOnWalkableStart );
				log_warning( "(start) walkable pos " << path.mOnWalkableStart << " is not on walkable space" );
			}
		}
		else
			path.mOnWalkableStart = path.mDesiredStart;

		if( !fOnWalkable( path.mDesiredEnd ) )
		{
			path.mOnWalkableEnd = fGetClosestWalkablePos( path.mDesiredEnd );
			if( !fOnWalkable( path.mOnWalkableEnd ) )
			{
				if( Debug_Navgraph_LogBadPositions )
					mBadPos.fPushBack( path.mOnWalkableEnd );
				log_warning( "(end) walkable pos " << path.mOnWalkableEnd << " is not on walkable space" );
			}
		}
		else
			path.mOnWalkableEnd = path.mDesiredEnd;

		const u32 startNode = fFindClosestNode( path.mOnWalkableStart );
		const u32 endNode = fFindClosestNode( path.mOnWalkableEnd );

		path.mNodePath = fSearchUsingAStar( startNode, endNode );
		path.mPosPath = fConvertToPosPath( path.mNodePath, path.mOnWalkableStart, path.mOnWalkableEnd );
		path.mCalcTime = timer.fGetElapsedMs( );
	}
	b32 tNavGraph::fOnWalkable( const Math::tVec3f& pos ) const
	{
		//if we are in a node return true
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
		{
			if( mNodes[ i ].fOnNode( pos ) )
				return true;
		}
		//if we are in an edge return true
		for( u32 i = 0; i < mEdges.fCount( ); ++i )
		{
			if( mEdges[ i ].fOnEdge( pos ) )
				return true;
		}
		return false;
	}
	u32 tNavGraph::fFindClosestNode( const tVec3f& pos ) const
	{
		//if we are on a node already, use that node
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
		{
			if( mNodes[ i ].fOnNode( pos ) )
				return i;
		}

		//we must be on an edge
		u32 best = ~0;
		f32 bestDist = Math::cInfinity;
		for( u32 i = 0; i < mEdges.fCount( ); ++i )
		{
			if( mEdges[ i ].fOnEdge( pos ) )
			{
				tVec3f nodePos = fGetNodePos( mEdges[ i ].mNodeA );
				f32 dist = fDist( nodePos, pos );
				if( dist < bestDist )
				{
					bestDist = dist;
					best = mEdges[ i ].mNodeA;
				}

				nodePos = fGetNodePos( mEdges[ i ].mNodeB );
				dist = fDist( nodePos, pos );
				if( dist < bestDist )
				{
					bestDist = dist;
					best = mEdges[ i ].mNodeB;
				}
			}
		}
		
		//but just in case we aren't on a node or edge
		if( best == ~0 )
		{
			best = 0;
			bestDist = fDist( fGetNodePos( 0 ), pos);
			for( u32 i = 1; i < mNodes.fCount( ); ++i )
			{
				const f32 tempDist = fDist( fGetNodePos( i ), pos);
				if( tempDist < bestDist )
				{
					bestDist = tempDist;
					best = i;
				}
			}
		}
		return best;
	}
	tNodePath tNavGraph::fSearchUsingAStar( u32 startNode, u32 endNode )
	{
		tDynamicArray<tPathHopPtr> allPathHops( mNodes.fCount( ) );

		const tVec3f endNodePos = fGetNodePos( endNode );
		allPathHops[ startNode ] = tPathHopPtr( NEW tPathHop );
		tPathHopPtr pathHop = allPathHops[ startNode ];
		pathHop->mNode = startNode;
		pathHop->mTravelCost = 0;
		pathHop->mHeurCost = fDist( fGetNodePos( startNode ), endNodePos );

		
		tPriorityQueue<tPathHopPtr,tPathHopLessThan> nodeQueue;
		pathHop->mInQueue = true;
		nodeQueue.fPut( pathHop );

		while( nodeQueue.fCount( ) )
		{
			sigassert( nodeQueue.fCount( ) <= allPathHops.fCount( ) );

			pathHop = nodeQueue.fGet( ); //fGet( ) pops as well
			sigassert( pathHop->mInQueue );
			pathHop->mInQueue = false;

			if( pathHop->mNode == endNode )
				break;

			const tPathNode& node = mNodes[ pathHop->mNode ];

			for( u32 i = 0; i < node.mEdges.fCount( ); ++i )
			{
				const u32 nextNodeIndex = node.mEdges[ i ];
				tPathHopPtr nextPathHop = allPathHops[ nextNodeIndex ];
				const tVec3f nextNodePos = fGetNodePos( nextNodeIndex );

				if( nextPathHop )
				{
					//only update if better path
					const f32 newTravelCost = pathHop->mTravelCost + fDist( fGetNodePos( pathHop->mNode ), nextNodePos );
					const f32 newHeurCost = fDist( nextNodePos, endNodePos );
					if( newTravelCost < nextPathHop->mTravelCost )
					{
						nextPathHop->mNode = nextNodeIndex;
						nextPathHop->mPrev = pathHop;						
						nextPathHop->mTravelCost = newTravelCost;
						nextPathHop->mHeurCost = newHeurCost;
						if( nextPathHop->mInQueue )
							nodeQueue.fUpdate( nextPathHop );
						else
						{
							nextPathHop->mInQueue = true;
							nodeQueue.fPut( nextPathHop );
						}
					}
				}
				else
				{
					allPathHops[ nextNodeIndex ] = tPathHopPtr( NEW tPathHop );
					nextPathHop = allPathHops[ nextNodeIndex ];
					nextPathHop->mNode = nextNodeIndex;
					nextPathHop->mPrev = pathHop;
					nextPathHop->mTravelCost = pathHop->mTravelCost + fDist( fGetNodePos( pathHop->mNode ), nextNodePos );
					nextPathHop->mHeurCost = fDist( nextNodePos, endNodePos );
					nextPathHop->mInQueue = true;
					nodeQueue.fPut( nextPathHop );
				}
			}//loop across edges
		}

		if( pathHop->mNode == endNode )
		{
			const tNodePath path = fConvertToPath( pathHop );
			sigassert( path[ 0 ] == endNode );
			sigassert( path[ path.fCount( ) - 1 ] == startNode );
			return path;
		}
		log_warning( "A* algorithm failed to find path between node index " << startNode << " and " << endNode );
		return tNodePath( );
	}
	b32 tNavGraph::fContainsSameEdge( const tVec3f& aPos, const tVec3f& bPos ) const
	{
		for( u32 i = 0; i < mEdges.fCount( ); ++i )
		{
			if( mEdges[ i ].fOnEdge( aPos ) && mEdges[ i ].fOnEdge( bPos ) )
				return true;
		}
		return false;
	}
	tPosPath tNavGraph::fConvertToPosPath( const tNodePath& path, const tVec3f& startPos, const tVec3f& endPos )
	{
		//NOTE: path is assumed to be in reverse order
		tPosPath posPath;
		posPath.fPushBack( startPos );

		//early out
		if( path.fCount( ) == 1 && fContainsSameEdge( startPos, endPos ) )
		{
			posPath.fPushBack( endPos );
			return posPath;
		}

		s32 startIndex = path.fCount( ) - 1;
		s32 endIndex = 0;
		if( path.fCount( ) > 1 )
		{
			//prevent us from moving backwards to reach next node
			const tPathEdge* firstEdge = fGetEdge( path[ startIndex ], path[ startIndex - 1 ] );
			if( firstEdge->fOnEdge( startPos ) )
				--startIndex;

			//prevent us from trying to go to the node past our target if we are on the same edge
			const tPathEdge* lastEdge = fGetEdge( path[ endIndex + 1 ], path[ endIndex ] );
			if( lastEdge->fOnEdge( endPos ) )
				++endIndex;
		}

		tVec3f lastPos = startPos;
		for( s32 i = startIndex; i >= endIndex; --i )
		{
			const tVec3f nextPos = mNodes[ path[ i ] ].fCastRayToCenter( lastPos );
			lastPos = nextPos;
			posPath.fPushBack( nextPos );
		}

		posPath.fPushBack( endPos );
		return posPath;
	}
	const tPathEdge* tNavGraph::fGetEdge( u32 nodeA, u32 nodeB ) const
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
	tVec3f tNavGraph::fGetClosestWalkablePos( const tVec3f& pos ) const
	{
		f32 bestDist = Math::cInfinity;
		tVec3f bestPos = pos;

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

		for( u32 i = 0; i < mNodes.fCount( ); ++i )
		{
			const tVec3f tempPos = mNodes[ i ].fCastRayToCenter( pos );
			const f32 tempDist = fDist( tempPos, pos );
			if( tempDist < bestDist )
			{
				bestDist = tempDist;
				bestPos = tempPos;
			}
		}

		if( Debug_NavGraph_AssertOnBadPos )
			log_assert( bestDist != Math::cInfinity, "Failed to find closest pos to " << pos << " edges: " << mEdges.fCount( ) << " nodes: " << mNodes.fCount( ) );

		return bestPos;
	}
}}//Sig::AI
