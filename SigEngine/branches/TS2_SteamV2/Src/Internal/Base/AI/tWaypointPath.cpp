#include "BasePch.hpp"
#include "tWaypointPath.hpp"

namespace Sig { namespace AI
{
	Math::tVec3f tWaypointPath::fRandomOffset( tRandom& rand )
	{
		return rand.fVecNorm<Math::tVec3f>( );
	}
	tWaypointPath::tWaypointPath( )
		: mOffset( Math::tVec3f::cZeroVector )
		, mReachedTheEnd( false )
	{
	}
	void tWaypointPath::fSetOffset( const Math::tVec3f& offset )
	{
		mOffset = offset;
	}
	void tWaypointPath::fResetTarget( const tPathEntityPtr& waypoint )
	{
		mReachedTheEnd = false;
		mTarget = waypoint;
		mNext.fRelease( );
	}
	void tWaypointPath::fComputeRandomNext( tRandom& rand )
	{
		mNext.fRelease( );
		if( !mTarget )
			return;
		const tPathEntity::tConnectionsList& nextPts = mTarget->fNextPathPoints( );
		if( nextPts.fCount( ) == 0 )
			return;
		mNext.fReset( nextPts[ rand.fIntInRange( 0, nextPts.fCount( ) - 1 ) ] );
	}
	void tWaypointPath::fSetNext( const tPathEntityPtr& waypoint )
	{
		mNext = waypoint;
	}
	b32 tWaypointPath::fReachedTarget( const Math::tVec3f& agentPos, f32 tolerance ) const
	{
		return agentPos.fDistance( fTargetPos( ) ) <= tolerance;
	}
	b32 tWaypointPath::fAdvanceTarget( )
	{
		if( !mNext )
			return false;
		mTarget = mNext;
		mNext.fRelease( );
		return true;
	}
}}
