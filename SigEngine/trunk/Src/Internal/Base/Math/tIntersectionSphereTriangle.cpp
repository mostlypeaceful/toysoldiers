#include "BasePch.hpp"
#include "tIntersectionSphereTriangle.hpp"

namespace Sig { namespace Math
{

	tVec3f fPointOnPlane( const tPlanef& plane, const tVec3f& pt )
	{
		f32 dot = plane.fGetNormal( ).fDot( pt );
		dot += plane.d;

		tVec3f output = pt + plane.fGetNormal( ) * dot;
		return output;
	}
	
	void tIntersectionSphereTriangle::fIntersect( const tSpheref& a, const tTrianglef& triangle, b32 keepNonPenetrating )
	{
		mIntersects = true;

		mTriangleNormal = triangle.fComputeUnitNormal( );
		tPlanef plane( mTriangleNormal, triangle.mA );

		tVec3f planePt = fPointOnPlane( plane, a.mCenter );
		planePt = triangle.fClampPtInTriangle( planePt );

		tVec3f sep = a.mCenter - planePt;
		
		f32 radSqr = a.mRadius*a.mRadius;
		f32 sepLenSqr = sep.fLengthSquared( );

		if( sepLenSqr > radSqr )
		{
			mIntersects = false;
			if( !keepNonPenetrating )
				return;
		}

		sigassert( sepLenSqr > cEpsilon );
		f32 sepDist = fSqrt( sepLenSqr );
		sep /= sepDist; //normalize

		f32 dist = a.mRadius - sepDist;
		mResult = Physics::tContactPoint( planePt, sep, dist );
	}


}}