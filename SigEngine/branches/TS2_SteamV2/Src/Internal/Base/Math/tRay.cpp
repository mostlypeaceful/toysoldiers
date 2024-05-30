#include "BasePch.hpp"
#include "tRay.hpp"
#include "tVector.hpp"

namespace Sig { namespace Math
{
	
	b32 fClosestPoint( const tRay<f32>& L, const tRay<f32>& R, f32& tOnLOut, f32& tOnROut )
	{
		const tVec3f& p1 = L.mOrigin, p2 = p1 + L.mExtent;
		const tVec3f& p3 = R.mOrigin, p4 = p3 + R.mExtent;

		tVec3f p13 = p1 - p3;
		tVec3f p43 = p4 - p3;
		tVec3f p21 = p2 - p1;

		f32 d1343,d4321,d1321,d4343,d2121;
		f32 numer,denom;

		if( fEqual(p43.x, 0.0f) && fEqual(p43.y, 0.0f) && fEqual(p43.z, 0.0f) )
			return false;

		if( fEqual(p21.x, 0.0f) && fEqual(p21.y, 0.0f) && fEqual(p21.z, 0.0f) )
			return false;

		d1343 = p13.fDot( p43 );
		d4321 = p43.fDot( p21 );
		d1321 = p13.fDot( p21 );
		d4343 = p43.fDot( p43 );
		d2121 = p21.fDot( p21 );

		denom = d2121 * d4343 - d4321 * d4321;
		if( fEqual(denom, 0.0f) )
			return false;

		numer = d1343 * d4321 - d1321 * d4343;

		tOnLOut = numer / denom;
		tOnROut = (d1343 + d4321 * (tOnLOut)) / d4343;

		//pa = p1 + tOnLOut * p21;
		//pb = p3 + mub * p43;

		return true;
	}

}}

