#include "BasePch.hpp"
#include "ProjectileUtility.hpp"

namespace Sig { namespace Math { namespace ProjectileUtility
{

	b32 fComputeLaunchAngle( f32& outAngle, f32 launchVelocity, const tVec3f& originPos, const tVec3f& targetPos, f32 gravity, b32 positiveRoot )
	{
		// The other fComputeLaunchAngle function works on a 2D plane.
		// So pass in the x and y coordinates of the target's position in a coordinate system with originPos as the origin
		// and the basis vectors are the y axis and the vector between the origin and target positions, projected to XZ
		// so that it's orthogonal to Y
		const f32 x = ( targetPos - originPos ).fXZLength( );
		const f32 y = targetPos.y - originPos.y;
		return fComputeLaunchAngle( outAngle, launchVelocity, tVec2f( x, y ), gravity, positiveRoot );
	}

	// deltaPos is a 2D position on the plane made by the y-axis and the projectiles launch vector projected to XZ
	b32 fComputeLaunchAngle( f32& outAngle, f32 launchVelocity, const tVec2f& deltaPos, f32 gravity, b32 positiveRoot )
	{
		const f32 v2 = launchVelocity * launchVelocity;
		const f32 squaredValue = ( v2 * v2 ) - ( gravity * ( ( gravity * deltaPos.x * deltaPos.x ) + ( 2.f * deltaPos.y * v2 ) ) );
		// Check for real roots
		if( squaredValue < 0 )
			return false;

		// Assume the positive root for now.  I think that will be good enough.
		const f32 posRoot = fSqrt( squaredValue );
		const f32 root = positiveRoot ? posRoot : -posRoot;
		outAngle = fAtan( ( v2 + root ) / ( gravity * deltaPos.x ) );

		return true;
	}
	b32 fComputeLaunchVelocity( f32& outVel, f32 launchAngle, const tVec3f& originPos, const tVec3f& targetPos, f32 gravity )
	{
		const f32 x = ( targetPos - originPos ).fXZLength( );
		const f32 y = targetPos.y - originPos.y;
		return fComputeLaunchVelocity( outVel, launchAngle, tVec2f( x, y ), gravity );
	}
	b32 fComputeLaunchVelocity( f32& outVel, f32 launchAngle, const tVec2f& deltaPos, f32 gravity )
	{
		launchAngle = fToRadians( launchAngle );

		if( launchAngle == cPi || launchAngle == -cPi )
			return false;

		const f32 tanAngle = fTan( launchAngle );
		const f32 gRoot = fSqrt( -gravity );

		outVel = ( gRoot * fSqrt( deltaPos.x ) * fSqrt( tanAngle * tanAngle + 1.f ) ) / fSqrt( 2.f * tanAngle - ( ( 2.f * deltaPos.y ) / deltaPos.x ) );

		return true;
	}
	b32 fComputeTimeToTarget( f32& outTime, f32 angle, f32 launchVelocity, f32 height, f32 gravity )
	{
		const f32 vSinAngle = launchVelocity * fSin( angle );
		const f32 squaredValue = vSinAngle * vSinAngle + 2 * -gravity * height;
		if( squaredValue < 0 )
			return false;

		outTime = ( vSinAngle + fSqrt( squaredValue ) ) / -gravity;
		return true;
	}


}}}//Sig::Math::ProjectileUtility
