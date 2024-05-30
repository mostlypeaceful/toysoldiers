#ifndef __tIntersectionSphereSphere__
#define __tIntersectionSphereSphere__

#include "Physics/tContactPoint.hpp"

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionSphereSphereWithContact
	{
	public:

		inline tIntersectionSphereSphereWithContact( ) 
			: mIntersects( false )
		{ }

		inline tIntersectionSphereSphereWithContact( const tSphere<t>& a, const tSphere<t>& b, b32 keepNonPenetrating = false )
		{
			fIntersect( b, a, keepNonPenetrating );
		}

		inline void fIntersect( const tSphere<t>& a, const tSphere<t>& b, b32 keepNonPenetrating = false )
		{
			tVector3<t> normalAToB = b.fCenter( ) - a.fCenter( );
			t dist;
			normalAToB.fNormalizeSafe( tVector3<t>::cYAxis, dist );

			t touchingDist = a.fRadius( ) + b.fRadius( );
			t penetration = touchingDist - dist;
			mIntersects = (penetration >= 0.f);

			// point on b, facing a
			mResult = Physics::tContactPoint( b.mCenter - normalAToB * b.mRadius, -normalAToB, penetration );
		}

		b32 fIntersects( ) const { return mIntersects; }

		b32 mIntersects;
		Physics::tContactPoint mResult;
	};

}}

#endif//__tIntersectionSphereSphere__
