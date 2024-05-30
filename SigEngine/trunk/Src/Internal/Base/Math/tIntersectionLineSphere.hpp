//------------------------------------------------------------------------------
// \file tIntersectionLineSphere.hpp - 22 Jul 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tIntersectionLineSphere__
#define __tIntersectionLineSphere__

#include "Math/tIntersectionSegment.hpp"

namespace Sig { namespace Math
{
	template<class t>
	tIntersectionSegment<t> fIntersectLineSphere( const tRay<t>& ray, const tSphere<t>& s )
	{
		// compute a,b,c for solving quadratic equation
		const t a = ray.mExtent.fLengthSquared();
		const t b = 2 * ray.mExtent.fDot( ray.mOrigin - s.mCenter );
		const t c = ( ray.mOrigin - s.mCenter ).fLengthSquared() - fSquare( s.mRadius );

		if( fEqual( a, 0 ) )
		{
			// ray is a point, see if it's at least inside the sphere.
			
			if( c <= 0 )
				return tIntersectionSegment<t>::cInfinite;
			else
				return tIntersectionSegment<t>::cEmpty;
		}

		// compute discriminant
		const t delta = b*b - 4*a*c;

		if( delta < 0 )
		{
			// no real solution
			return tIntersectionSegment<t>::cEmpty;
		}
		else
		{
			// compute min and max intersection time values
			const t deltaroot	= fSqrt( delta );
			const t minT		= ( -b - deltaroot ) / ( 2 * a );
			const t maxT		= ( -b + deltaroot ) / ( 2 * a );
			return tIntersectionSegment<t>( minT, maxT );
		}
	}

	template<class t>
	tIntersectionSegment<t> fIntersectLineSphere( const tSphere<t>& s, const tRay<t>& ray )
	{
		return fIntersectLineSphere( ray, s );
	}
}}

#endif//__tIntersectionLineSphere__
