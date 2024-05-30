#include "GameAppPch.hpp"
#include "tAirbornePathFollowing.hpp"

using namespace Sig::Physics;
using namespace Sig::Math;

namespace Sig
{
	devvar( bool, Debug_Airborne_Pathing_DrawDebug, false );
	devvar( bool, Debug_Airborne_Pathing_DrawDebugYielding, false );

	devvar( f32, Gameplay_Airborne_Pathing_SlowdownSpeed, 0.5f );
	devvar( f32, Gameplay_Airborne_Pathing_SlowdownDistMult, 4.0f );
	devvar( bool, Gameplay_Airborne_Pathing_SlowdownEnable, false );
	devvar( f32, Gameplay_Airborne_Pathing_LookAheadMin, 30.0f );
	devvar( f32, Gameplay_Airborne_Pathing_LookAheadMax, 200.0f );
	devvar( f32, Gameplay_Airborne_Pathing_AdditionalTargetSeparation, 0.0f );

	bool tAirbornePathFollower::fComputeInput( Physics::tAirborneInput &inputOut, const tAirborneLogic& vehicle, tUnitPath& path, const tGrowableArray<tVehicleOffender>& offenders, f32 dt, Gfx::tDebugGeometryContainer *debugging )
	{		
		const Physics::tAirbornePhysics& phys = vehicle.fPhysics( );
		const tMat3f &transform = phys.fTransform( );
		const tMat3f transformInv = transform.fInverse( );

		tVec3f sideVec = transform.fXAxis( );
		tVec3f upVec = transform.fYAxis( );
		const tVec3f vehicleFacing = transform.fZAxis( );
		const tVec3f vehiclePos = transform.fGetTranslation( );
		f32 currentSpeed = phys.fSpeed( );
		
		//const f32 cautiousness = 0.5f; //1.f stays on path, 0.f is like fuck it
		//f32 intendedSpeed = phys.fProperties( ).mCruiseSpeed * phys.fProperties( ).mAIThrottle;
		//f32 originalIntendedSpeed = intendedSpeed;
		b32 slowing = false;

		//tObbf myObb( vehicle.fGetCollisionBounds( ), transform );

		// default stop, no steer
		inputOut.fZero( );
		inputOut.mActive = true;

		////see if we need to yeild for anyone
		//if( offenders.fCount( ) )
		//{
		//	for( u32 o = 0; o < offenders.fCount( ); ++o )
		//	{
		//		const tVehicleOffender &offender = offenders[o];

		//		tVec3f theirFutureTranslation(0.0f);

		//		//// compute where we'll intersect
		//		//if( !fEqual( offender.mVelocity.fLengthSquared( ), 0.0f ) && !fEqual( vehicle.fGetSpeed( ), 0.0f ) )
		//		//{
		//		//	//see if we're going to cross paths
		//		//	tRayf theirRay( offender.mBounds.fCenter( ), offender.mVelocity );
		//		//	tRayf myRay( transform.fGetTranslation( ), vehicle.fGetVelocity( ) );

		//		//	f32 theirSpeed;
		//		//	theirRay.mExtent.fNormalize( theirSpeed );
		//		//	myRay.mExtent.fNormalize( );

		//		//	f32 distForMe, distForThem;
		//		//	if( fClosestPoint( myRay, theirRay, distForMe, distForThem ) )
		//		//	{
		//		//		if( distForMe > 0.0f && distForThem > 0.0f )
		//		//		{
		//		//			f32 time = distForMe / vehicle.fGetSpeed( );
		//		//			theirFutureTranslation = theirRay.mExtent * time * theirSpeed;
		//		//			draw = true;
		//		//		}
		//		//	}
		//		//}

		//		tObbf theirFuturebounds = offender.mBounds.fTransform( tMat3f( tQuatf::cIdentity, theirFutureTranslation ) );		

		//		if( Debug_Airborne_Pathing_DrawDebugYielding )
		//			debugging->fRenderOnce( theirFuturebounds, tVec4f(1,0,0,1) );

		//		// compute if we do intersect
		//		tVec3f posDelta = theirFuturebounds.fCenter( ) - vehiclePos;

		//		f32 upSep = theirFuturebounds.fSeparationOnAxis( myObb, upVec );
		//		if( upSep < 0.0f )
		//		{
		//			f32 sideSep = theirFuturebounds.fSeparationOnAxis( myObb, sideVec );
		//			if( sideSep < 0.0f )
		//			{
		//				f32 forwardSep = theirFuturebounds.fSeparationOnAxis( myObb, vehicleFacing );
		//				f32 usableVel = fMax( offender.mVelocity.fDot( vehicleFacing ), 0.0f );
		//				//0.5 is the value found in fComputeAccelerationAndDeccelerationToReachSpeed
		//				f32 maxSlowDownDist = vehicle.fTimeToReachSpeed( intendedSpeed, usableVel, 0.5 ) * currentSpeed;

		//				f32 separationDist = vehicle.fFollowDistance( ) + Gameplay_Airborne_Pathing_AdditionalTargetSeparation;
		//				f32 slowdownDist = maxSlowDownDist + separationDist; 
		//				if( forwardSep < slowdownDist && posDelta.fDot( vehicleFacing ) > 0.0f )
		//				{
		//					intendedSpeed = usableVel + (forwardSep - separationDist - currentSpeed);

		//					if( Debug_Airborne_Pathing_DrawDebugYielding )
		//					{
		//						debugging->fRenderOnce( theirFuturebounds.fCenter( ), vehiclePos, tVec4f(1,0,0,1) );
		//					}
		//				}
		//			}
		//		}
		//	}

		//	
		//	if( Debug_Airborne_Pathing_DrawDebugYielding )
		//	{
		//		f32 yielding = originalIntendedSpeed - intendedSpeed;
		//		tVec3f p = transform.fXformPoint( tVec3f(0,5,0) );
		//		tVec3f d = transform.fXformVector( tVec3f(0,0,yielding) );
		//		debugging->fRenderOnce( p, p + d, tVec4f(1,1,0,1) );
		//	}
		//}

		if( path.fHasWaypoints( ) )
		{

			if( path.fPathMode( ) == tUnitPath::cPathModeFollow )
			{
				// additional vehicle info
				f32 lookAhead = fClamp( currentSpeed, (f32)Gameplay_Airborne_Pathing_LookAheadMin, (f32)Gameplay_Airborne_Pathing_LookAheadMax ); //, fAbs( currentSpeed / vehicle.fGetMaxSpeed( ) ) );
				
				// compute current and future path positions
				tVec3f pathPoint, futurePoint;
				f32 pathDist;
				int segment = path.fDistanceToPath( vehiclePos, pathPoint, pathDist, futurePoint, lookAhead );

				// compute heading
				tVec3f delta = futurePoint - vehiclePos;
				b32 passedTarget = false;
				inputOut.mStick = phys.fComputeStickToReachHeading( delta, passedTarget );

				// Throttle scale -1 == full slow, 1 == full fast. 0.5 is normal
				f32 throttle = phys.fProperties( ).mAIThrottle + (vehicle.fThrottleScale( ) * 2.f - 1.f);
				inputOut.mThrottle = fClamp( throttle, 0.f, 2.f );

				if( segment == 2 ) // dunno why this extra part of the check was ever around -> && path.fHasWaypoints( 2 ) )
					path.fRequestNext( );


				//// compute strafe
				//tVec3f groundVel = phys.fVelocity( );
				//groundVel.y = 0;

				//tVec3f intendedVel = delta;
				//intendedVel.fSetLength( intendedSpeed );

				//tVec3f velDelta = intendedVel - groundVel;
				//f32 velDeltaLen;
				//velDelta.fNormalizeSafe( tVec3f::cZeroVector, velDeltaLen );
				//velDeltaLen = fMin( velDeltaLen, 1.f );
				//velDelta *= velDeltaLen;
				//inputOut.mWorldStrafe = velDelta.fXZ( );

				if( debugging && Debug_Airborne_Pathing_DrawDebug )
				{
					tVec3f offset( 0, 0, 0 );
					debugging->fRenderOnce( vehiclePos + offset, vehiclePos + offset + delta, tVec4f(1,0,0,1) );

					//draw target vectors
					tPair<tVec3f,tVec3f> line1( path.fPrevTargetPosition( ), path.fTargetPosition( ) );
					debugging->fRenderOnce( line1, tVec4f(0,1,0,1) );

					tPair<tVec3f,tVec3f> line2( path.fTargetPosition( ), path.fNextTargetPosition( ) );
					debugging->fRenderOnce( line2, tVec4f(1,0,0,1) );

					//draw target
					debugging->fRenderOnce( tSpheref( futurePoint, 1.0 ), slowing ? tVec4f(1,0,0,0.125f) : tVec4f(0,0,1,0.125f) );
				}
			}
		}

		return true;
	}

}