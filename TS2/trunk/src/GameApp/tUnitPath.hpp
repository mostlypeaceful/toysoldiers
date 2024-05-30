#ifndef __tUnitPath__
#define __tUnitPath__

#include "tPathEntity.hpp"
#include "tRingBuffer.hpp"

namespace Sig
{
	class tUnitLogic;
	class tUnitPath;

	class tPathContext : public Logic::tEventContext
	{
		define_dynamic_cast( tPathContext, Logic::tEventContext );
	public:
		tUnitPath* mPath;
		tPathEntity* mWaypoint;

		tPathContext( tUnitPath* path = NULL, tPathEntity* pt = NULL ) : mPath( path ), mWaypoint( pt ) { }

		bool fIsFromPath( const tUnitPath* up ) const { return mPath == up; }
		const tPathEntity* fWaypoint( ) const { return mWaypoint; }
		tPathEntity* fWaypointScript( ) { return mWaypoint; }

		static tPathContext* fConvert( const Logic::tEventContext* obj )
		{
			return obj->fDynamicCast< tPathContext >( );
		}
	};

	class tUnitPath : public tRefCounter
	{
	public:
		struct tPathStartEntry
		{
			tPathStartEntry( )
			{ }

			tPathStartEntry( const tPathEntityPtr& startP, f32 distAlongPath = -1.f, b32 stopAtEnd = false, b32 stopAtStart = false, b32 randomizePositioning = true, b32 liveTargetUpdate = false )
				: mStartPt( startP )
				, mDistAlongPath( distAlongPath )
				, mStopAtEnd( stopAtEnd )
				, mStopAtStart( stopAtStart )
				, mRandomPositioning( randomizePositioning )
				, mLiveTargetUpdate( liveTargetUpdate )
				, mUseSpecificPt( false )
			{ }

			tPathEntityPtr mStartPt;
			f32 mDistAlongPath; // to stop if >= 0, for trenching
			b8 mStopAtEnd;
			b8 mStopAtStart;
			b8 mRandomPositioning;
			b8 mLiveTargetUpdate;

			b8 mUseSpecificPt;
			b8 pad0;
			b8 pad1;
			b8 pad2;

			Math::tVec3f mSpecificPt;
			tStringPtr mNameOverride;
		};

		enum tPathMode
		{
			cPathModeFollow,
			cPathModeStop,
			cPathModeCount
		};


		tUnitPath( );
		virtual ~tUnitPath( );

		void fSetDesiredWaypointCnt( u32 cnt );
		void fSetDistanceTolerance( f32 tolerance );
		void fSetYDistanceTolerance( f32 tolerance );
		f32 fGetDistanceTolerance( ) { return mDistanceTolerance; }
		void fSetFlatPath( b32 flat ) { mFlatPath = flat; }

		void fUpdate( f32 dt );

		bool fSentEvent( const Logic::tEvent& event ) const;

		// This allows you to queue up a bunch of paths and execute them as a sequence
		void fAddPathStartEntry( const tPathStartEntry& entry ) { mPathStarts.fPushBack( entry ); }
		void fAddSimpleVehiclePath( const tStringPtr& name );
		void fClearPathStarts( );

		void fStartSimplePath( const tPathEntityPtr& point );
		void fStartSimplePointPath( const Math::tVec3f& pt, const tStringPtr& name );

		// Execute path sequence from the top
		void fStartPathSequence( );
		s32 fCurrentPathSequence( ) const { return mCurrentPathSeq; }
		tPathStartEntry& fSegment( ) { return mPathStarts[ mCurrentPathSeq ]; }
		void fSetPathSequenceLooping( b32 loop ) { mLoopSequence = loop; }
		b32 fIsPathSequenceLooping( ) const { return mLoopSequence; }

		// Resume path sequence after path has been halted ie. current path end reached
		void fResumePathSequence( );
		void fAdvancePathSequence( ); //goes to next seq and calls resume
		void fPausePathSequence( );

		u32 fPathMode( ) const { return mPathMode; }
		//void fSetPathMode( u32 mode ) { mPathMode = (tPathMode)mode; }

		// This initializes the path to some arbitrary start point.
		void fSetStartPoint( tPathStartEntry& start );
		static s32 fFindClosestStartPoint( const Math::tVec3f& to, const tGrowableArray<tRefCounterPtr<tPathEntity>>& from, const tStringPtr& name, u32* matchUnitType = NULL );
		static s32 fFindFarthestStartPoint( const Math::tVec3f& to, const tGrowableArray<tRefCounterPtr<tPathEntity>>& from, const tStringPtr& name );
		static tRefCounterPtr<tPathEntity> fFindClosestStartRecursive( const Math::tVec3f& to, const tGrowableArray<tRefCounterPtr<tPathEntity>>& from, u32 maxDepth = ~0 );

		// Enqueues wayponits until path capacity has been reached
		//  pass true when called from fAdvanceWaypoint
		void fFillPath( b32 oneExtra = false );
		void fClearPath( );

		// Call when the current waypoint is reached and we need to determine which waypoint to go to next.
		// Returns true if a new waypoint is targeted, false if there are no more waypoints queued
		b32 fAdvanceWaypoint( );

		// Call this to add a waypoint to the current path.
		// Returns true if a new way point has been enqueued.  False if the way point path is unchanged.
		b32 fEnqueueWaypoint( );
		
		// Request path to be advanced
		void fRequestNext( ) { mWantNext = true; }

		void	 fSetOwnerEntity( tEntity* owner );
		tEntity* fOwnerEntity( ) { return mOwnerEntity; }
		tEntity* fWaypoint( );
		tEntity* fPrevWayPoint( ) { return mPrevWaypoint.fGetRawPtr( ); }
		b32      fHasWaypoints( u32 atLeast = 1 ) const { return mWaypointPath.fNumItems( ) >= atLeast; }

		const Math::tVec3f& fTargetPosition( ) const { return mTargetOverride ? mTargetOverridePos : mTargetPos; }
		const Math::tVec3f& fPrevTargetPosition( ) const { return mPrevTargetPos; }
		const Math::tVec3f& fNextTargetPosition( ) const { return mNextTargetPos; }

		// Target z dir is the zAxis of the target way point
		const Math::tVec3f& fTargetZDirection( ) const { return mTargetOverride ? mTargetOverrideDirection : mTargetZDir; }
		const Math::tVec3f& fPrevTargetZDirection( ) const { return mPrevTargetZDir; }
		const Math::tVec3f& fNextTargetZDirection( ) const { return mNextTargetZDir; }

		const Math::tVec3f& fCurrentSegment( ) const { return mCurrentSegment; }
		const Math::tVec3f& fNextSegment( ) const { return mNextSegment; }

		f32					fDistanceToTarget( ) const;
		f32					fDistanceToTargetXZ( ) const;
		Math::tVec3f		fDeltaToTarget( ) const;

		void fSetTargetOverridePosition( const Math::tVec3f& position ) { mTargetOverride = true; mTargetOverridePos = position; }
		void fSetTargetOverrideDirection( const Math::tVec3f& direction ) { mTargetOverride = true; mTargetOverrideDirection = direction; }
		void fClearTargetOverride( ) { mTargetOverride = false; }

		//return the number of segments to intersection
		// 0 no intersect, 1 current, 2 next, so on
		// last target and last time are used to prevent you from going backwards on the path, pass null to disable to this behavior
		s32 fDistanceToPath( const Math::tVec3f& pointIn, Math::tVec3f& pointOut, f32 &distanceOut, Math::tVec3f& futurePointOut, f32 futureAdvanceDistance, f32* currentTimeOut = NULL ) const;

		Math::tVec3f fInterpolatedDirection( s32 segment, f32 t ) const;

		// Find data some distance along the current segment
		void fInterpolatePath( f32 distance, Math::tVec3f& pointOut, Math::tVec3f& dirOut );
		void fClearInterpolationDistance( );

		const tStringPtr& fLastStartedPathName( ) const { return mStartedPathName; }
		void fSetLastStartedPathName( const tStringPtr& name ) { mStartedPathName = name; }
		const tPathEntityPtr& fLastStartedPathPt( ) const { return mLastStartedPathPt; }
		u32 fLastStartedPathType( ) const{ return mLastStartedPathType; }

		Math::tVec3f& fContextTarget( ) { return mContextTarget; }

	private:
		void fComputeTargetPosition( b32 firstRun );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tEntity*		mOwnerEntity;
		tUnitLogic*		mOwnerLogic;

		// Current target waypoints.  In general the character is heading towards this area.
		//  the path can be queued up as long as desired.
		u32 mPathCapacity;
		tRingBuffer< tPathEntityPtr > mWaypointPath;
		tPathEntityPtr mPrevWaypoint;

		// Path sequencing
		tGrowableArray< tPathStartEntry > mPathStarts;
		s32 mCurrentPathSeq;
		b32 mLoopSequence;
		tPathMode mPathMode;

		f32 mDistLeftToTravel;
		b8 mLimitedDistance;
		b8 mReachedLimit;
		b8 mWantNext;
		b8 mTargetOverride;
		b32 mFlatPath; //for vehicles, non interpolating stuff
		f32 mCurrentInterpDist;

		Math::tVec3f mTargetOverridePos;
		Math::tVec3f mTargetOverrideDirection;
		
		// Random position within the waypoint which the character will use for movement and to determine if he's reached his destination
		Math::tVec3f mTargetPos, mNextTargetPos, mPrevTargetPos, mTargetZDir, mPrevTargetZDir, mNextTargetZDir;
		Math::tVec3f mCurrentSegment, mNextSegment;     //these are stored normalized
		f32 mCurrentSegmentLength, mNextSegmentLength; //their lengths stored here
		f32 mDistanceTolerance;
		f32 mYDistanceTolerance;

		// The random offset in path space to apply to new waypoints
		Math::tVec3f mObjCoordinates;
		Logic::tEventContextPtr mEventContext;

		tStringPtr mStartedPathName;
		tPathEntityPtr mLastStartedPathPt;
		u32			mLastStartedPathType;

		tPathEntityPtr mLoopPt; //if this is set, go to it when the end of the sequence has been reached.

		Math::tVec3f mContextTarget;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tUnitPath );
}

#endif//__tUnitPath__
