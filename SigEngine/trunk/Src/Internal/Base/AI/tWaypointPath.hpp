#ifndef _tWaypointPath__
#define _tWaypointPath__
#include "tPathEntity.hpp"

namespace Sig { namespace AI
{
	class tWaypointPath
	{
		tPathEntityPtr mTarget;
		tPathEntityPtr mNext;
		Math::tVec3f mOffset;
		b32 mReachedTheEnd;
	public:
		static Math::tVec3f fRandomOffset( tRandom& rand );
	public:
		tWaypointPath( );
		void fOnDelete( );
		void fSetOffset( const Math::tVec3f& offset );
		void fResetTarget( const tPathEntityPtr& waypoint );
		void fComputeRandomNext( tRandom& rand );
		void fSetNext( const tPathEntityPtr& waypoint );
		const tPathEntityPtr& fTarget( ) const { return mTarget; }
		Math::tVec3f fTargetPos( ) const { return mTarget ? mTarget->fObjectToWorld( ).fXformPoint( mOffset ) : Math::tVec3f::cZeroVector; }
		const tPathEntityPtr& fNext( ) const { return mNext; }
		Math::tVec3f fNextPos( ) const { return mNext ? mNext->fObjectToWorld( ).fXformPoint( mOffset ) : Math::tVec3f::cZeroVector; }
		b32 fReachedTarget( const Math::tVec3f& agentPos, f32 tolerance ) const;
		b32 fAdvanceTarget( ); // returns false if no next point to advance to

		b32 fReachedTheEnd( ) const { return mReachedTheEnd; }
		void fSetReachedTheEnd( b32 reached ) { mReachedTheEnd = reached; }
	};
}}

#endif//_tWaypointPath__
