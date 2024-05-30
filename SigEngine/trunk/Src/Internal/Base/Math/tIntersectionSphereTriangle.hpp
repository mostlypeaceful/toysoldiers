#ifndef __tIntersectionSphereTriangle__
#define __tIntersectionSphereTriangle__

#include "Physics/tContactPoint.hpp"

namespace Sig { namespace Math
{

	class tIntersectionSphereTriangle
	{
	protected:

		b32						mIntersects; ///< This will be true as long as they are not totally disjoint

	public:
		inline tIntersectionSphereTriangle( const tSpheref& a, const tTrianglef& triangle, b32 keepNonPenetrating = false )
		{
			fIntersect( a, triangle, keepNonPenetrating );
		}

		inline tIntersectionSphereTriangle( const tTrianglef& triangle, const tSpheref& a, b32 keepNonPenetrating = false )
		{
			fIntersect( a, triangle, keepNonPenetrating );
		}

		void fIntersect( const tSpheref& a, const tTrianglef& triangle, b32 keepNonPenetrating = false );

		inline b32 fIntersects( ) const { return mIntersects; }

		Physics::tContactPoint	mResult;
		Math::tVec3f			mTriangleNormal;
	};

}}

#endif//__tIntersectionSphereTriangle__
