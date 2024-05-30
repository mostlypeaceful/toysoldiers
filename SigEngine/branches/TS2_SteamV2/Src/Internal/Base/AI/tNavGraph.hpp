#ifndef __tNavGraph__
#define __tNavGraph__

#include "tPathEntity.hpp"
#include "Gui/tText.hpp"

namespace Sig { namespace AI {

	typedef tPair< Math::tVec3f, Math::tVec3f > tVec3fPair;
	struct tPathNode
	{
		tPathEntityPtr mEntity;
		tGrowableArray<u32> mEdges;
		Math::tVec3f fPos( ) const { return mEntity->fObjectToWorld( ).fGetTranslation( ); }
		Math::tVec3f fCastRayToCenter( const Math::tVec3f& rayOrigin ) const;
		b32 fOnNode( const Math::tVec3f& p ) const;
	};
	struct tPathEdge
	{
		u32 mNodeA;
		u32 mNodeB;
		tGrowableArray< Math::tPlanef > mHalfSpaces;
		tGrowableArray< Math::tVec3f > mHalfSpaceCenters; //for debug only
		tVec3fPair mHighEdge;
		tVec3fPair mLowEdge;
		tPathEdge( )
			: mNodeA( ~0 )
			, mNodeB( ~0 )
		{}
		b32 fOnEdge( const Math::tVec3f& p ) const;
		f32 fClosestPoint( const Math::tVec3f& pos, Math::tVec3f* outPos ) const;
	};
	struct tPathHop : public tUncopyable, public tRefCounter
	{
		u32 mNode;
		f32 mTravelCost; //"g"
		f32 mHeurCost; //"h"
		b32 mInQueue;
		tRefCounterPtr< tPathHop > mPrev;
		tPathHop( )
			: mNode( ~0 )
			, mTravelCost( Math::cInfinity )
			, mHeurCost( Math::cInfinity )
			, mInQueue( false )
		{}
		inline f32 fCost( ) const;
	};
	typedef tRefCounterPtr< tPathHop > tPathHopPtr;
	typedef tGrowableArray<u32> tNodePath;
	typedef tGrowableArray<Math::tVec3f> tPosPath;
	struct tPath
	{
		//INPUT
		Math::tVec3f mDesiredStart;
		Math::tVec3f mDesiredEnd;
		//OUTPUT
		Math::tVec3f mOnWalkableStart;
		Math::tVec3f mOnWalkableEnd;
		tPosPath mPosPath;
		tNodePath mNodePath;
		f32 mCalcTime;
		//HELPFUL FUNCS
		tPath( );
		u32 fCount( ) const { return mPosPath.fCount( ); }
		const Math::tVec3f& operator[]( u32 i ) const { sigassert( i < fCount( ) ); return mPosPath[ i ]; }
		f32 fLen( );
	};
	class tNavGraphInterface : public tRefCounter
	{
	public:
		virtual ~tNavGraphInterface( ) { }
		virtual void fSetRoot( const tPathEntityPtr& root, u32 navGraphID ) = 0;
		virtual void fInit( ) = 0;
		virtual void fDebugDraw( ) = 0;
		virtual void fDebugDraw( const tPath& path ) = 0;
		virtual void fFindPath( tPath& path ) = 0;
		virtual b32 fOnWalkable( const Math::tVec3f& pos ) const = 0;
		virtual u32 fNavGraphID( ) const = 0;
	};
	class tNavGraph : public tNavGraphInterface
	{
	public:
		tNavGraph( );
		virtual ~tNavGraph( );
		void fSetRoot( const tPathEntityPtr& root, u32 navGraphID );
		void fInit( );
		void fDebugDraw( );
		void fDebugDraw( const tPath& path );
		void fFindPath( tPath& path );
		b32 fOnWalkable( const Math::tVec3f& pos ) const;
		u32 fNavGraphID( ) const { return mNavGraphID; }
	private:
		void fClearData( );
		//graph generation
		void fGenerateGraph( tPathEntity* root );
		void fRunPerfTest( );
		u32 fGetOrAddNode( tHashTable<tPathEntity*, u32>& entityTable, tPathEntity* node );
		u32 fAddNode( tPathEntity* node );
		void fAddEdge( u32 nodeA, u32 nodeB );
		tNodePath fConvertToPath( const tPathHopPtr& pathHop ) const;
		//helper stuff
		Math::tVec3f fGetNodePos( u32 i ) const;
		tVec3fPair fGetNodeExtents( u32 i ) const;
		//search algorithms
		tNodePath fSearchUsingAStar( u32 startNode, u32 endNode );
		//pathing steps
		u32 fFindClosestNode( const Math::tVec3f& pos ) const;
		b32 fContainsSameEdge( const Math::tVec3f& aPos, const Math::tVec3f& bPos ) const;
		tPosPath fConvertToPosPath( const tNodePath& path, const Math::tVec3f& startPos, const Math::tVec3f& endPos );
		//edge stuff
		tVec3fPair fGetHighLowEdgePts( const Math::tVec3f& normal, u32 node ) const;
		const tPathEdge* fGetEdge( u32 nodeA, u32 nodeB ) const;
		Math::tVec3f fGetClosestWalkablePos( const Math::tVec3f& pos ) const;
	private:
		//data
		tPathEntityPtr mRoot;
		tGrowableArray<tPathNode> mNodes;
		tGrowableArray<tPathEdge> mEdges;
		u32 mNavGraphID;

		//DEBUG
		tGrowableArray<Math::tVec3f> mBadPos;
		tGrowableArray<tPathEdge> mBadEdges;
	};
	define_smart_ptr( base_export, tRefCounterPtr, tNavGraphInterface );
	typedef tNavGraphInterfacePtr tNavGraphPtr;
}}//Sig::AI

#endif//__tNavGraph__
