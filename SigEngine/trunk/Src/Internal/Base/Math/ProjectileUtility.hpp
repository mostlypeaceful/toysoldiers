#ifndef __ProjectileUtiltiy__
#define __ProjectileUtiltiy__

namespace Sig { namespace Math { namespace ProjectileUtility
{
	 // Helper functions for computing the angle required to hit a target and time it takes to get there

		// Returns true if targetPos is within reach and writes the angle to outAngle.
		base_export b32 fComputeLaunchAngle( f32& outAngle, f32 launchVelocity, const tVec3f& originPos, const tVec3f& targetPos, f32 gravity, b32 positiveRoot );
		base_export b32 fComputeLaunchAngle( f32& outAngle, f32 launchVelocity, const tVec2f& deltaPos, f32 gravity, b32 positiveRoot );

		base_export b32 fComputeLaunchVelocity( f32& outVel, f32 launchAngle, const tVec3f& originPos, const tVec3f& targetPos, f32 gravity );
		base_export b32 fComputeLaunchVelocity( f32& outVel, f32 launchAngle, const tVec2f& deltaPos, f32 gravity );

		base_export b32 fComputeTimeToTarget( f32& outTime, f32 angle, f32 launchVelocity, f32 height, f32 gravity );


	const f32 cStandardGravity = -9.8f;

}}}//Sig::Math::ProjectileUtility


#endif//__ProjectileUtiltiy__
