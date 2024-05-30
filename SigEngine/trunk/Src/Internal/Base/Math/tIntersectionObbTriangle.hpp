#ifndef __tIntersectionObbTriangle__
#define __tIntersectionObbTriangle__

#include "Physics/tContactPoint.hpp"

namespace Sig { namespace Math
{

	class tIntersectionObbTriangle
	{
	protected:
		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

	public:
		Physics::tContactPoint mResult;
		Math::tVec3f	mTriangleNormal;

		inline tIntersectionObbTriangle( const tObbf& a, const tTrianglef& triangle, b32 keepNonPenetrating = false )
		{
			fIntersect( a, triangle, keepNonPenetrating );
		}

		inline tIntersectionObbTriangle( const tTrianglef& triangle, const tObbf& a, b32 keepNonPenetrating = false )
		{
			fIntersect( a, triangle, keepNonPenetrating );
		}

		inline b32 fIntersects( ) const { return mIntersects; }

		void fIntersect( const tObbf& a, const tTrianglef& triangle, b32 keepNonPenetrating = false );

	};

}}

#endif//__tIntersectionObbTriangle__
