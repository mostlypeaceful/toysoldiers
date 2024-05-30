//------------------------------------------------------------------------------
// \file tIntersectionLineCapsule.hpp - 22 Jul 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tIntersectionLineCapsule__
#define __tIntersectionLineCapsule__

#include "Math/tRay.hpp"
#include "Math/tConvexHull.hpp" // tCylinder, tCapsule
#include "Math/tIntersectionSegment.hpp"
#include "Math/tIntersectionLineCylinder.hpp"
#include "Math/tIntersectionLineSphere.hpp"

namespace Sig { namespace Math
{
	template< class t >
	tIntersectionSegment< t > fIntersectLineCapsule( const tRay< t >& ray, const tCapsule& c )
	{
		return	fIntersectLineCylinder( ray, Math::tCylinder( c.mCenter, c.mPrimaryAxis, c.mHalfHeight, c.mRadius ) )
			|	fIntersectLineSphere( ray, Math::tSpheref( c.mCenter - c.mPrimaryAxis * c.mHalfHeight, c.mRadius ) )
			|	fIntersectLineSphere( ray, Math::tSpheref( c.mCenter + c.mPrimaryAxis * c.mHalfHeight, c.mRadius ) );
	}

	template<class t>
	tIntersectionSegment<t> fIntersectLineCapsule( const tCapsule& c, const tRay< t >& ray )
	{
		return fIntersectLineCapsule( ray, c );
	}
}} // namespace Sig::Math

#endif //ndef __tIntersectionLineCapsule__
