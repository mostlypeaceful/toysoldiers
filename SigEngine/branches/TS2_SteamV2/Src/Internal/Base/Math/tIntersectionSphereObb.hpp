#ifndef __tIntersectionSphereObb__
#define __tIntersectionSphereObb__
#include "tIntersectionAabbSphere.hpp"

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
	class tIntersectionSphereObbWithContact : public tIntersectionAabbSphereWithContact<t>
	{
	public:

		inline tIntersectionSphereObbWithContact( ) { }

		inline tIntersectionSphereObbWithContact( const tSphere<t>& a, const tObb<t>& b )
		{
			fIntersect( b, a );
		}
		inline tIntersectionSphereObbWithContact( const tObb<t>& a, const tSphere<t>& b )
		{
			fIntersect( a, b );
		}

		inline void fIntersect( const tObb<t>& a, const tSphere<t>& b )
		{
			tIntersectionAabbSphereWithContact<t>::fIntersect( 
				tAabb<t>( -a.fExtents( ), a.fExtents( ) ),
				tSphere<t>( a.fPointToLocalVector( b.fCenter( ) ), b.fRadius( ) ) );

			tMat3f xform = a.fGetTransform( );
			tIntersectionAabbSphereWithContact<t>::mContactPt = xform.fXformPoint( tIntersectionAabbSphereWithContact<t>::mContactPt );
			tIntersectionAabbSphereWithContact<t>::mNormal = xform.fXformVector( tIntersectionAabbSphereWithContact<t>::mNormal );
		}

	};

}}

#endif//__tIntersectionSphereObb__
