#ifndef __tIntersectionSphereObb__
#define __tIntersectionSphereObb__
#include "tIntersectionAabbSphere.hpp"
#include "Physics/tContactPoint.hpp"

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionSphereObb : public tIntersectionAabbSphere<t>
	{
	public:

		inline tIntersectionSphereObb( ) { }

		inline tIntersectionSphereObb( const tSphere<t>& a, const tObb<t>& b )
		{
			fIntersect( b, a );
		}
		inline tIntersectionSphereObb( const tObb<t>& a, const tSphere<t>& b )
		{
			fIntersect( a, b );
		}

		inline void fIntersect( const tObb<t>& a, const tSphere<t>& b )
		{
			tIntersectionAabbSphere<t>::fIntersect( 
				tAabb<t>( -a.fExtents( ), a.fExtents( ) ),
				tSphere<t>( a.fPointToLocalVector( b.fCenter( ) ), b.fRadius( ) ) );
		}

	};

	template<class t>
	class tIntersectionSphereObbWithContact : public tIntersectionAabbSphereWithContact< t >
	{
	public:

		inline tIntersectionSphereObbWithContact( ) { }

		inline tIntersectionSphereObbWithContact( const tSphere<t>& a, const tObb<t>& b, b32 keepNonPenetrating = false )
		{
			fIntersect( b, a, keepNonPenetrating );
		}

		inline tIntersectionSphereObbWithContact( const tObb<t>& a, const tSphere<t>& b, b32 keepNonPenetrating = false )
		{
			fIntersect( a, b, keepNonPenetrating );
		}

		inline void fIntersect( const tSphere<t>& a, const tObb<t>& b, b32 keepNonPenetrating = false )
		{
			fIntersect( b, a, keepNonPenetrating );
		}

		inline void fIntersect( const tObb<t>& a, const tSphere<t>& b, b32 keepNonPenetrating = false )
		{
			tIntersectionAabbSphereWithContact<t>::fIntersect( 
				tAabb<t>( -a.fExtents( ), a.fExtents( ) ),
				tSphere<t>( a.fPointToLocalVector( b.fCenter( ) ), b.fRadius( ) ), keepNonPenetrating );

			tMat3f xform = a.fGetTransform( );
			tIntersectionAabbSphereWithContact<t>::mContactPt = xform.fXformPoint( tIntersectionAabbSphereWithContact<t>::mContactPt );
			tIntersectionAabbSphereWithContact<t>::mNormal = xform.fXformVector( tIntersectionAabbSphereWithContact<t>::mNormal );

			// result is on obb facing sphere
			mResult = Physics::tContactPoint( mContactPt, -mNormal, mPenetration );
		}

		Physics::tContactPoint mResult;
	};

}}

#endif//__tIntersectionSphereObb__
