#include "GameAppPch.hpp"
#include "tUnitPath.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tWaypointLogic.hpp"
#include "tUnitLogic.hpp"
#include "tSync.hpp"

using namespace Sig::Math;

namespace Sig
{

	devvar( f32, Gameplay_Debug_UnitPath_FlatPathHeight, 15.f );

	tUnitPath::tUnitPath( ) 
		: mOwnerEntity( NULL )
		, mOwnerLogic( NULL )
		, mPathCapacity( 0 )
		, mWaypointPath( mPathCapacity )
		, mCurrentPathSeq( -1 )
		, mLoopSequence( false )
		, mPathMode( cPathModeStop )
		, mDistLeftToTravel( 0 )
		, mLimitedDistance( false )
		, mReachedLimit( true )
		, mWantNext( false )
		, mTargetOverride( false )
		, mFlatPath( false )
		, mCurrentInterpDist( 0.f )
		, mTargetOverridePos( tVec3f::cZeroVector )
		, mTargetOverrideDirection( tVec3f::cZAxis )
		, mTargetPos( tVec3f::cZeroVector )
		, mNextTargetPos( tVec3f::cZeroVector )
		, mPrevTargetPos( tVec3f::cZeroVector )
		, mTargetZDir( tVec3f::cZeroVector )
		, mPrevTargetZDir( tVec3f::cZeroVector )
		, mNextTargetZDir( tVec3f::cZeroVector )
		, mCurrentSegment( tVec3f::cZeroVector )
		, mNextSegment( tVec3f::cZeroVector )
		, mCurrentSegmentLength( 0.f )
		, mNextSegmentLength( 0.f )
		, mDistanceTolerance( 1.f )
		, mYDistanceTolerance( cInfinity )
		, mObjCoordinates( tVec3f::cZeroVector )
		, mLastStartedPathType( tWaypointLogic::cGoalPath )
		, mContextTarget( tVec3f::cZeroVector )
	{
		mEventContext = Logic::tEventContextPtr( NEW tPathContext( this ) );
		fSetDesiredWaypointCnt( 1 ); //default to 1 for soldiers
	}
	tUnitPath::~tUnitPath( )
	{
	}

	void tUnitPath::fSetOwnerEntity( tEntity* owner ) 
	{ 
		mOwnerEntity = owner; 
		mOwnerLogic = owner->fLogicDerived<tUnitLogic>( );
	}

	void tUnitPath::fSetDesiredWaypointCnt( u32 cnt )
	{
		//need one extra because we enqueue before we pop
		mPathCapacity = cnt + 1;
		mWaypointPath.fResize( mPathCapacity );
	}

	void tUnitPath::fSetDistanceTolerance( f32 tolerance )
	{
		mDistanceTolerance = tolerance;
	}

	void tUnitPath::fSetYDistanceTolerance( f32 tolerance )
	{
		mYDistanceTolerance = tolerance;
	}

	void tUnitPath::fUpdate( f32 dt )
	{
		if( !fHasWaypoints( ) || mCurrentPathSeq == -1 )
		{
			if( mPathMode == cPathModeFollow )
			{
				// someone still thinks we have a path.. enlighten them	
				mPathMode = cPathModeStop;
				mOwnerLogic->fFireLevelEvent( GameFlags::cLEVEL_EVENT_REACHED_END_OF_PATH );
				// Level event may have set a new path so double check that we are done before firing the event
				if( ( !fHasWaypoints( ) || mCurrentPathSeq == -1 ) && mPathMode == cPathModeStop )
					mOwnerLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_REACHED_END_OF_PATH, mEventContext ) );
			}

			return; // No sequence active
		}

		tPathStartEntry& currentSeq = fSegment( );

		tVec3f delta = fDeltaToTarget( );
		f32 xzDist = delta.fXZ( ).fLength( );

		b32 getNextWP = (mWantNext || (xzDist < mDistanceTolerance && fAbs( delta.y ) < mYDistanceTolerance) ) && mPathMode == cPathModeFollow; // mWantNext is set externally to toggle advancement

		if( getNextWP )
		{
			// We've reached our target in some sense
			mWantNext = false;
			mEventContext->fStaticCast<tPathContext>( )->mWaypoint = mWaypointPath.fBack( )->fDynamicCast<tPathEntity>( );
			mOwnerLogic->fPathPointReached( *mWaypointPath.fBack( ), mEventContext );

			if( mTargetOverride && currentSeq.mStopAtEnd )
			{
				mPathMode = cPathModeStop;
				mOwnerLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_PATH_WAIT, mEventContext ) );
			}
			else
			{
				// fPathPointReached may have paused the path.
				if( mPathMode == cPathModeFollow )
				{
					if( currentSeq.mStopAtStart )
					{
						currentSeq.mStopAtStart = false;
						mPathMode = cPathModeStop;
						mOwnerLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_PATH_WAIT, mEventContext ) );
					}

					if( !fAdvanceWaypoint( ) )
					{
						mPathMode = cPathModeStop;
						mCurrentPathSeq = -1;

						mOwnerLogic->fFireLevelEvent( GameFlags::cLEVEL_EVENT_REACHED_END_OF_PATH );
						// Level event may have set a new path so double check that we are done before firing the event
						if( mCurrentPathSeq == -1 /*|| ( mCurrentPathSeq == mPathStarts.fCount( )-1 && !mLoopSequence )*/ )
							mOwnerLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_REACHED_END_OF_PATH, mEventContext ) );
					}
				}
			}
		}
		else if( currentSeq.mLiveTargetUpdate )
			fComputeTargetPosition( false );
			
	}

	void tUnitPath::fStartPathSequence( )
	{
		if( mPathStarts.fCount( ) == 0 )
		{
			mCurrentPathSeq = -1;
			fClearPath( );
		}
		else
		{
			mCurrentPathSeq = 0;
			fSetStartPoint( fSegment( ) );
		}		
	}

	void tUnitPath::fResumePathSequence( )
	{
		if( mCurrentPathSeq >= 0 && mCurrentPathSeq < (s32)mPathStarts.fCount( ) )
		{
			//take out the stop at end, incase they haven't gotten to their place in the trench yet
			for( u32 i = 0; i < mPathStarts.fCount( ); ++i )
				mPathStarts[ i ].mStopAtEnd = false;

			mPathMode = cPathModeFollow;
			mOwnerLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FOLLOW_PATH, mEventContext ) );
		}
	}

	void tUnitPath::fAdvancePathSequence( )
	{
		if( mCurrentPathSeq >= 0 && mCurrentPathSeq < (s32)mPathStarts.fCount( ) - 1 )
		{
			++mCurrentPathSeq;
			fSetStartPoint( fSegment( ) );
		}
	}

	void tUnitPath::fPausePathSequence( )
	{
		if( mPathMode == cPathModeFollow )
		{
			mPathMode = cPathModeStop;
			mOwnerLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_PATH_WAIT, mEventContext ) );
		}
	}

	void tUnitPath::fSetStartPoint( tPathStartEntry& start )
	{
		tPathEntity* waypoint = start.mStartPt.fGetRawPtr( );

		sigassert( waypoint && "Attempting to set mWaypoint to null!" );

		fClearPath( );
		fClearTargetOverride( );

		if( fSegment( ).mUseSpecificPt )
			fSetTargetOverridePosition( fSegment( ).mSpecificPt );

		mWaypointPath.fPut( tPathEntityPtr( waypoint ) );
		mStartedPathName = waypoint->fName( );
		if( start.mNameOverride.fExists( ) )
			mStartedPathName = start.mNameOverride;

		mPrevTargetPos = mOwnerEntity->fObjectToWorld( ).fGetTranslation( );
		mPrevTargetZDir = mOwnerEntity->fObjectToWorld( ).fZAxis( );
		mLastStartedPathPt.fReset( waypoint );

		tWaypointLogic* waypointLogic = waypoint->fLogicDerived<tWaypointLogic>( );
		if( waypointLogic )
			mLastStartedPathType = waypointLogic->fPathType( );
		else
			mLastStartedPathType = tWaypointLogic::cGoalPath;

		if( waypoint->fHasGameTagsAny( GameFlags::cFLAG_LOOP ) )
			mLoopPt.fReset( waypoint );

		fComputeTargetPosition( true );

		fFillPath( );

		if( mWaypointPath.fNumItems( ) > 0 ) 
			mPrevTargetPos.y = mWaypointPath[ 0 ]->fObjectToWorld( ).fGetTranslation( ).y;

		fComputeTargetPosition( false );

		if( !fSegment( ).mUseSpecificPt && fSegment( ).mDistAlongPath >= 0 )
		{
			mDistLeftToTravel = fSegment( ).mDistAlongPath;
			mLimitedDistance = true;
		}
		else
		{
			mLimitedDistance = false;
		}

		mPathMode = cPathModeFollow;
		mEventContext->fStaticCast<tPathContext>( )->mWaypoint = waypoint->fDynamicCast<tPathEntity>( );
		mOwnerLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_FOLLOW_PATH, mEventContext ) );
		mOwnerLogic->fPathPointReached( *waypoint, mEventContext );
	}

	s32 tUnitPath::fFindClosestStartPoint( const Math::tVec3f& to, const tGrowableArray<tRefCounterPtr<tPathEntity>>& from, const tStringPtr& name, u32* matchUnitType )
	{
		s32 waypoint = -1;
		f32 shortestDist = cInfinity;

		for( u32 i = 0; i < from.fCount( ); ++i )
		{
			const tPathEntityPtr& pathEnt = from[ i ];

			if( name.fExists( ) && name != pathEnt->fName( ) )
				continue;

			if( matchUnitType && pathEnt->fQueryEnumValue( GameFlags::cENUM_UNIT_TYPE ) != *matchUnitType )
				continue;

			f32 dist = ( pathEnt->fObjectToWorld( ).fGetTranslation( ) - to ).fProjectToXZ( ).fLengthSquared( );
			if( dist < shortestDist )
			{
				waypoint = i;
				shortestDist = dist;
			}
		}

		return waypoint;
	}

	s32 tUnitPath::fFindFarthestStartPoint( const Math::tVec3f& to, const tGrowableArray<tRefCounterPtr<tPathEntity>>& from, const tStringPtr& name )
	{
		s32 waypoint = -1;
		f32 farthestDist = 0;

		for( u32 i = 0; i < from.fCount( ); ++i )
		{
			const tPathEntityPtr& pathEnt = from[ i ];

			if( name.fExists( ) && name != pathEnt->fName( ) )
				continue;

			f32 dist = ( pathEnt->fObjectToWorld( ).fGetTranslation( ) - to ).fProjectToXZ( ).fLengthSquared( );
			if( dist > farthestDist )
			{
				waypoint = i;
				farthestDist = dist;
			}	
		}

		return waypoint;
	}

	namespace 
	{
		void fFindStartRecursion( const Math::tVec3f& to, tPathEntity* from, f32& shortestDist, tPathEntity*& result, u32 maxDepth )
		{
			if( result )
			{
				f32 dist = ( from->fObjectToWorld( ).fGetTranslation( ) - to ).fProjectToXZ( ).fLengthSquared( );
				if( dist < shortestDist )
				{
					result = from;
					shortestDist = dist;
				}										
			}
			else
			{
				result = from;
				shortestDist = ( from->fObjectToWorld( ).fGetTranslation( ) - to ).fProjectToXZ( ).fLengthSquared( );
			}

			if( maxDepth > 0 )
			{
				u32 nextDepth = maxDepth - 1;
				for( u32 i = 0; i < from->fNextPathPoints( ).fCount( ); ++i )
					fFindStartRecursion( to, from->fNextPathPoints( )[ i ], shortestDist, result, nextDepth );
			}
		}
	}
	
	tRefCounterPtr<tPathEntity> tUnitPath::fFindClosestStartRecursive( const Math::tVec3f& to, const tGrowableArray<tRefCounterPtr<tPathEntity>>& from, u32 maxDepth )
	{
		tPathEntity* result = NULL;
		f32 shortestDist = 0;

		for( u32 i = 0; i < from.fCount( ); ++i )
		{
			fFindStartRecursion( to, from[ i ].fGetRawPtr( ), shortestDist, result, maxDepth );
		}

		return tRefCounterPtr<tPathEntity>( result );
	}

	void tUnitPath::fFillPath( b32 oneExtra )
	{
		s32 cntDiff = mPathCapacity - mWaypointPath.fNumItems( );
		if( !oneExtra )
			 --cntDiff;

		while( cntDiff > 0 )
		{
			fEnqueueWaypoint( );
			--cntDiff;
		}

		fComputeTargetPosition( false );
	}
	
	void tUnitPath::fClearPath( )
	{
		mWaypointPath = tRingBuffer< tPathEntityPtr >( mPathCapacity );
		mPrevWaypoint.fRelease( );
		mReachedLimit = false;
		mLoopPt.fRelease( );
	}

	void tUnitPath::fAddSimpleVehiclePath( const tStringPtr& name )
	{
		// WILL IGNORE PATHS WITH UNIT TYPE
		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );

		// no level if we're unloadding
		if( levelLogic )
		{
			const Math::tVec3f startPt = mOwnerEntity->fObjectToWorld( ).fGetTranslation( );

			u32 ignoreAny = ~0;
			const s32 goal = fFindClosestStartPoint( startPt, levelLogic->fPathStarts( ), name, &ignoreAny );
			if( goal > -1 ) 
			{
				if( !levelLogic->fPathEndsInGoal( )[ goal] )
					mOwnerLogic->fSetWillNotEndInGoal( );
				fAddPathStartEntry( tPathStartEntry( levelLogic->fPathStarts( )[ goal ] ) );
			}
			else
			{
				log_warning( 0, "Could not find path start: " << name );
			}
		}
	}

	void tUnitPath::fStartSimplePointPath( const tVec3f& pt, const tStringPtr& name )
	{
		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		if( levelLogic )
		{
			fClearPathStarts( );

			sigassert( levelLogic->fPathStarts( ).fCount( ) && "just need a goal path, anywhere :)" );
			tPathStartEntry entry( levelLogic->fPathStarts( )[ 0 ] );
			entry.mUseSpecificPt = true;
			entry.mSpecificPt = pt;
			entry.mNameOverride = name;

			fAddPathStartEntry( entry );
			fStartPathSequence( );
		}
	}

	void tUnitPath::fStartSimplePath( const tPathEntityPtr& point )
	{
		fClearPathStarts( );
		fAddPathStartEntry( tUnitPath::tPathStartEntry( point ) );
		fStartPathSequence( );
	}

	void tUnitPath::fClearPathStarts( )
	{
		mPathStarts.fSetCount( 0 );
	}

	b32 tUnitPath::fAdvanceWaypoint( )
	{
		if( !mReachedLimit )
		{
			fEnqueueWaypoint( );
			fFillPath( true );
		}
		
		//need at least 2 waypoints, the current one, and the one we just enqueued
		if( mWaypointPath.fNumItems( ) <= 1 )
		{
			// reached end of current path
			if( mCurrentPathSeq == -1 || ( mCurrentPathSeq == mPathStarts.fCount( )-1 && !mLoopSequence ) )
			{
				if( !mLoopPt )
					return false; //path completed

				fStartSimplePath( mLoopPt );
				return true;
			}
			else
			{
				// we have more sequence
				b32 stopAtEnd = fSegment( ).mStopAtEnd;

				// advance sequence
				++mCurrentPathSeq;
				if( mLoopSequence ) mCurrentPathSeq = mCurrentPathSeq % mPathStarts.fCount( );

				fSetStartPoint( fSegment( ) );

				if( stopAtEnd ) 
				{
					mPathMode = cPathModeStop;
					mOwnerLogic->fHandleLogicEvent( Logic::tEvent( GameFlags::cEVENT_PATH_WAIT, mEventContext ) );
				}

				return true;
			}
		}
		else
		{
			// remove the completed node and advance
			tPathEntityPtr prevPoint;
			mWaypointPath.fGet( prevPoint );

			if( mPrevWaypoint )
				mCurrentInterpDist = fMax( mCurrentInterpDist - mCurrentSegmentLength, 0.f );

			mPrevWaypoint = prevPoint;
			fComputeTargetPosition( false );

			if( mLimitedDistance )
			{
				// see if we'll be stopping on this segment
				if( mCurrentSegmentLength >= mDistLeftToTravel )
				{
					mReachedLimit = true;
					mCurrentSegmentLength = mDistLeftToTravel;
					mTargetPos = mPrevTargetPos + mCurrentSegment * mCurrentSegmentLength;
				}
				else
					mDistLeftToTravel -= mCurrentSegmentLength;
			}

			return true;
		}
	}

	b32 tUnitPath::fEnqueueWaypoint( )
	{
		if( !fHasWaypoints( ) )
		{
			sigassert( !"No waypoint set!" );
			return false;
		}

		// loop through current waypoints and remove duds, skip index=0
		s32 wpCnt = mWaypointPath.fNumItems( );
		for( s32 wp = wpCnt-1; wp > 0; --wp )
		{
			tWaypointLogic* pointLogic = mWaypointPath[wp]->fLogicDerived< tWaypointLogic >( );
			b32 accessible = !pointLogic || pointLogic->fIsAccessible( );	
			if( !accessible )
			{
				tPathEntityPtr discard;
				mWaypointPath.fPopLIFO( discard );
			}
		}

		// get the end of the path, the latest point
		tPathEntityPtr waypoint = mWaypointPath.fFront( );

		// Loop through mWaypoint's tPathEntity children and find the best path. Probably have to get something from tWaypointLogic.
		// if mWaypoint has one child, just pick that.  No need to look deeper unless it has multiple immediate children.

		tWaypointLogic* pointLogic = waypoint->fLogicDerived< tWaypointLogic >( );
		b32 accessible = !pointLogic || pointLogic->fIsAccessible( );

		tGrowableArray< tPathEntity* > childPoints;
		u32 childCount = 0;

		//if( accessible )
		{
			// build list off accessable children
			const tPathEntity::tConnectionsList& nextPts = waypoint->fNextPathPoints( );
			for( u32 i = 0; i < nextPts.fCount( ); ++i )
			{
				++childCount;

				tWaypointLogic* logic = nextPts[ i ]->fLogicDerived< tWaypointLogic >( );
				if( !logic || logic->fIsAccessible( ) )
					childPoints.fPushBack( nextPts[ i ]->fDynamicCast< tPathEntity >( ) );
			}
		}

		tPathEntity* nextWaypoint = 0;

		// choose best child to visit next
		if( childPoints.fCount( ) == 0 )
		{
			// can't go on, cause of obstructions?
			if( childCount > 0 ) 
				//go back to parent
				nextWaypoint = waypoint->fParent( )->fDynamicCast< tPathEntity >( );
			else 
				//no where to go, end path
				return false;
		}
		else // go to random child
			nextWaypoint = childPoints[ sync_rand( fIntInRange( 1, childPoints.fCount( ) ) ) - 1 ];

		mWaypointPath.fPut( tPathEntityPtr( nextWaypoint ) );

		if( nextWaypoint->fHasGameTagsAny( GameFlags::cFLAG_LOOP ) )
			mLoopPt.fReset( nextWaypoint );

		return true;
	}

	void tUnitPath::fComputeTargetPosition( b32 firstRun )
	{
		if( fHasWaypoints( ) )
		{
			if( firstRun )
			{
				// Passing false will use the same object space coordinates for the waypoint target position attempting to making a smoother path.
				// The idea being that there will be fewer crossing back and forth over the path and more sticking to one side.
				if( mCurrentPathSeq < 0 || mPathStarts[ mCurrentPathSeq ].mRandomPositioning )
				{
					if( mOwnerLogic->fUnitType( ) == GameFlags::cUNIT_TYPE_AIR )
						mObjCoordinates = sync_rand( fVec<tVec3f>( ) );
					else
					{
						mObjCoordinates = tVec3f( sync_randc( fFloatMinusOneToOne( ), "objx" )
													  , 0
													  , sync_randc( fFloatMinusOneToOne( ), "objz" ) );
						
						mObjCoordinates.fNormalizeSafe( tVec3f::cZeroVector );
					}
				}
				else
					mObjCoordinates = tVec3f::cZeroVector;
			}

			mTargetPos = mWaypointPath[0]->fObjectToWorld( ).fXformPoint( mObjCoordinates );
			mTargetZDir = mWaypointPath[0]->fObjectToWorld( ).fZAxis( );

			if( mPrevWaypoint )
			{
				mPrevTargetPos = mPrevWaypoint->fObjectToWorld( ).fXformPoint( mObjCoordinates );		
				mPrevTargetZDir = mPrevWaypoint->fObjectToWorld( ).fZAxis( );
			}

			if( mFlatPath )
			{
				mTargetPos.y = Gameplay_Debug_UnitPath_FlatPathHeight;
				mPrevTargetPos.y = Gameplay_Debug_UnitPath_FlatPathHeight;
			}

			mCurrentSegment = mTargetPos - mPrevTargetPos;

			if( !mLimitedDistance || !mReachedLimit )
			{
				// get current segment length
				mCurrentSegment.fNormalizeSafe( tVec3f::cYAxis, mCurrentSegmentLength );
			}
			else
			{
				// limited distance, use previous gotten segment length to determine current target pos
				mCurrentSegment.fNormalizeSafe( tVec3f::cYAxis );
				mTargetPos = mPrevTargetPos + mCurrentSegment * mCurrentSegmentLength;
			}

			// see if we need to cache future data
			if( mWaypointPath.fNumItems( ) > 1 ) 
			{
				mNextTargetPos = mWaypointPath[1]->fObjectToWorld( ).fXformPoint( mObjCoordinates );
				mNextTargetZDir = mWaypointPath[1]->fObjectToWorld( ).fZAxis( );
				if( mFlatPath ) mNextTargetPos.y = Gameplay_Debug_UnitPath_FlatPathHeight;

				mNextSegment = mNextTargetPos - mTargetPos;
				mNextSegment.fNormalizeSafe( tVec3f::cZAxis, mNextSegmentLength );
			}
			else
			{
				mNextTargetPos = mTargetPos;
				mNextTargetZDir = mTargetZDir;
			}
		}
	}
	
	tEntity* tUnitPath::fWaypoint( ) 
	{ 
		if( mWaypointPath.fNumItems( ) == 0 )
			return NULL;
		else
			return mWaypointPath.fBack( ).fGetRawPtr( ); 
	}

	f32	tUnitPath::fDistanceToTarget( ) const
	{
		sigassert( mOwnerEntity && "No owner entity set!" );
		return ( mOwnerEntity->fObjectToWorld( ).fGetTranslation( ) - mTargetPos ).fLength( );
	}

	f32 tUnitPath::fDistanceToTargetXZ( ) const
	{
		sigassert( mOwnerEntity && "No owner entity set!" );

		tVec3f xzDist = mOwnerEntity->fObjectToWorld( ).fGetTranslation( ) - fTargetPosition( );
		xzDist.y = 0;
		return xzDist.fLength( );
	}

	tVec3f tUnitPath::fDeltaToTarget( ) const
	{
		sigassert( mOwnerEntity && "No owner entity set!" );

		const tVec3f ownerPos = mOwnerEntity->fObjectToWorld( ).fGetTranslation( );
		const tVec3f targetPos = fTargetPosition( );

		return  ownerPos - targetPos;
	}
	
	f32 fPointTimeOnLine( tVec3f p, const tVec3f& origin, const tVec3f& direction )
	{
		p -= origin;
		return p.fDot( direction );
	}

	void tUnitPath::fClearInterpolationDistance( )
	{
		mCurrentInterpDist = 0.f;
	}

	void tUnitPath::fInterpolatePath( f32 distance, Math::tVec3f& pointOut, Math::tVec3f& dirOut )
	{
		mCurrentInterpDist += distance;
		if( mReachedLimit ) mCurrentInterpDist = fMin( mCurrentInterpDist, mCurrentSegmentLength );

		if( mCurrentInterpDist >= mCurrentSegmentLength )
		{
			f32 t = fClamp( (mCurrentInterpDist - mCurrentSegmentLength) / mNextSegmentLength, 0.f, 1.f );
			pointOut = fLerp( mTargetPos, mNextTargetPos, t );
			dirOut = mTargetZDir;
			fRequestNext( );
		}
		else
		{

			f32 t = fClamp( mCurrentInterpDist / mCurrentSegmentLength, 0.f, 1.f );
			pointOut = fLerp( mPrevTargetPos, mTargetPos, t );
			dirOut = fLerp( mPrevTargetZDir, mTargetZDir, 1.0f );
		}
	}

	s32 tUnitPath::fDistanceToPath( const tVec3f& pointIn, tVec3f& pointOut, f32 &distanceOut, Math::tVec3f& futurePointOut, f32 futureAdvanceDistance, f32* currentTimeOut ) const
	{
		f32 t1 = fPointTimeOnLine( pointIn, mPrevTargetPos, mCurrentSegment );
		f32 t1Clamp = fClamp( t1, 0.0f, mCurrentSegmentLength );
		f32 t1Error = fAbs( t1 - t1Clamp );
		tVec3f point1 = mPrevTargetPos + mCurrentSegment * t1Clamp;
		f32 d1 = (point1 - pointIn).fLength( );

		if( !fHasWaypoints(2) )
		{
			//only 1 way point, no need to go on
			distanceOut = d1;
			pointOut = point1;

			u32 seg = 1;
			f32 futureT = t1Clamp + futureAdvanceDistance;

			if( t1Clamp == mCurrentSegmentLength ) 
			{
				futureT = mCurrentSegmentLength;
				seg = 2;
			}

			futurePointOut = mPrevTargetPos + mCurrentSegment * futureT;
			if( currentTimeOut ) *currentTimeOut = (futureT / mCurrentSegmentLength);

			return seg;
		}

		f32 t2 = fPointTimeOnLine( pointIn, mTargetPos, mNextSegment );
		f32 t2Clamp = fClamp( t2, 0.0f, mNextSegmentLength );
		f32 t2Error = fAbs( t2 - t2Clamp );
		tVec3f point2 = mTargetPos + mNextSegment * t2Clamp;
		f32 d2 = (point2 - pointIn).fLength( );

		if( d1 < d2 )
		{
			distanceOut = d1;
			pointOut = point1;
			f32 t = 0.f;
			u32 seg = 1;

			if( t1Clamp + futureAdvanceDistance < mCurrentSegmentLength )
			{
				// advanced point is still on first segment
				f32 dist = t1Clamp + futureAdvanceDistance;
				t = dist / mCurrentSegmentLength;
				futurePointOut = mPrevTargetPos + mCurrentSegment * dist;
			}
			else
			{
				// advanced point spilled over to second segment
				f32 dist = fMin( t1Clamp + futureAdvanceDistance - mCurrentSegmentLength, mNextSegmentLength );
				t = dist / mNextSegmentLength;
				futurePointOut = mTargetPos + mNextSegment * dist;
				seg = 2;
			}

			if( currentTimeOut ) *currentTimeOut = t;
			return seg;
		}
		else
		{
			distanceOut = d2;
			pointOut = point2;

			f32 secondT = fMin( t2Clamp + futureAdvanceDistance, mNextSegmentLength );
			futurePointOut = mTargetPos + mNextSegment * secondT;

			if( currentTimeOut ) *currentTimeOut = (secondT/mNextSegmentLength);
			return 2;
		}
	}

	tVec3f tUnitPath::fInterpolatedDirection( s32 segment, f32 t ) const
	{
		if( segment == 1 ) 
			return fNLerpSafe( mPrevTargetZDir, fTargetZDirection( ), t, tVec3f::cZAxis );
		else if( segment == 2 ) 
			return fNLerpSafe( fTargetZDirection( ), fNextTargetZDirection( ), t, tVec3f::cZAxis );
		else 
			return mTargetZDir;
	}

	bool tUnitPath::fSentEvent( const Logic::tEvent& event ) const
	{
		const tPathContext* pc = event.fContext<tPathContext>( );
		return pc && pc->mPath == this;
	}

	void tUnitPath::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tPathContext, Logic::tEventContext, Sqrat::NoCopy<tPathContext> > classDesc( vm.fSq( ) );
			classDesc
				.Func( _SC("IsFromPath"), &tPathContext::fIsFromPath )
				.Prop( _SC("Waypoint"), &tPathContext::fWaypointScript )
				.StaticFunc( _SC("Convert"), &tPathContext::fConvert )
				;

			vm.fRootTable( ).Bind(_SC("PathContext"), classDesc);
		}
		{
			Sqrat::Class<tUnitPath, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("EnqueueWaypoint"),		&tUnitPath::fEnqueueWaypoint)
				.Func(_SC("AdvanceWaypoint"),		&tUnitPath::fAdvanceWaypoint)
				.Prop(_SC("Waypoint"),				&tUnitPath::fWaypoint)
				.Func(_SC("TargetPosition"),		&tUnitPath::fTargetPosition)
				.Func(_SC("DistanceToTarget"),		&tUnitPath::fDistanceToTarget)
				.Func(_SC("StartPathSequence"),		&tUnitPath::fStartPathSequence)
				.Func(_SC("Resume"),				&tUnitPath::fResumePathSequence)
				.Func(_SC("Wait"),					&tUnitPath::fPausePathSequence)
				.Func(_SC("ClearPathSequence"),		&tUnitPath::fClearPathStarts)
				.Func(_SC("ClearPath"),				&tUnitPath::fClearPath)
				.Func(_SC("AdvancePathSequence"),	&tUnitPath::fAdvancePathSequence)
				.Func(_SC("AddSimpleVehiclePath"),	&tUnitPath::fAddSimpleVehiclePath)
				.Func(_SC("StartSimplePointPath"),	&tUnitPath::fStartSimplePointPath)
				.Prop(_SC("PathMode"),				&tUnitPath::fPathMode ) //, &tUnitPath::fSetPathMode )
				.Func(_SC("SentEvent"),				&tUnitPath::fSentEvent )
				.Prop(_SC("DistanceTolerance"),		&tUnitPath::fGetDistanceTolerance, &tUnitPath::fSetDistanceTolerance)
				.Prop(_SC("LoopPaths"),				&tUnitPath::fIsPathSequenceLooping, &tUnitPath::fSetPathSequenceLooping)
				.Prop(_SC("LastStartedPathName"),	&tUnitPath::fLastStartedPathName, &tUnitPath::fSetLastStartedPathName)
				;

			vm.fRootTable( ).Bind(_SC("UnitPath"), classDesc);
		}

		vm.fConstTable( ).Const(_SC("PATH_MODE_FOLLOW"), ( int ) cPathModeFollow );
		vm.fConstTable( ).Const(_SC("PATH_MODE_STOP"), ( int ) cPathModeStop );
	}
}

