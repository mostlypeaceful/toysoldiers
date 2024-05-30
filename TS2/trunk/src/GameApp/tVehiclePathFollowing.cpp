#include "GameAppPch.hpp"
#include "tVehiclePathFollowing.hpp"
#include "tUnitPath.hpp"
#include "tWheeledVehicleLogic.hpp"

using namespace Sig::Physics;
using namespace Sig::Math;

namespace Sig
{
	devvar( bool, Debug_Vehicle_Pathing_DrawDebug, false );
	devvar( bool, Debug_Vehicle_Pathing_DrawDebugYielding, false );
	devvar( bool, Debug_Vehicle_Pathing_DrawDebugMore, false );
	//devvar( bool, Debug_Vehicle_Pathing_DrawDesiredDirection, false );

	devvar( f32, Gameplay_Vehicle_Pathing_SlowdownSpeed, 0.5f );
	devvar( f32, Gameplay_Vehicle_Pathing_SlowdownDistMult, 4.0f );
	devvar( bool, Gameplay_Vehicle_Pathing_SlowdownEnable, false );
	devvar( f32, Gameplay_Vehicle_Pathing_LookAheadMin, 4.0f );
	devvar( f32, Gameplay_Vehicle_Pathing_LookAheadMax, 10.0f );
	devvar( f32, Gameplay_Vehicle_Pathing_AdditionalTargetSeparation, 0.0f ); //tuning value, this should be set to 0 by adjusting the data tables
	devvar( f32, Gameplay_Vehicle_Pathing_IntendedSpeedBlend, 0.2f );

	bool tVehiclePathFollower::fComputeInput(tVehicleInput &inputOut, const tWheeledVehicleLogic &vehicle, tUnitPath& path, const tGrowableArray<tVehicleOffender>& offenders, f32 dt, Gfx::tDebugGeometryContainer *debugging)
	{		
		const Physics::tVehiclePhysics& phys = vehicle.fPhysics( );
		const tMat3f &transform = phys.fTransform( );
		const tMat3f transformInv = transform.fInverse( );

		tVec3f sideVec = transform.fXAxis( );
		tVec3f upVec = transform.fYAxis( );
		tVec3f vehicleFacing = transform.fZAxis( );
		tVec3f vehiclePos = transform.fGetTranslation( );
		
		f32 currentSpeed = phys.fSpeed( );
		f32 intendedSpeed = phys.fMaxSpeed( ) * phys.fMaxAIThrottle( ) * vehicle.fThrottleScale( );
		f32 originalIntendedSpeed = intendedSpeed;
		b32 slowing = false;

		tMat3f flatXform;
		flatXform.fOrientYWithZAxis( tVec3f::cYAxis, transform.fZAxis( ) );
		flatXform.fSetTranslation( vehiclePos );
		tObbf myObb( vehicle.fGetCollisionBounds( ), flatXform );

		// default stop, no steer
		inputOut.fZero( );

		//see if we need to yeild for anyone
		if( offenders.fCount( ) )
		{
			for( u32 o = 0; o < offenders.fCount( ); ++o )
			{
				const tVehicleOffender &offender = offenders[o];

				tVec3f theirFutureTranslation(0.0f);

				//// compute where we'll intersect
				//if( !fEqual( offender.mVelocity.fLengthSquared( ), 0.0f ) && !fEqual( vehicle.fGetSpeed( ), 0.0f ) )
				//{
				//	//see if we're going to cross paths
				//	tRayf theirRay( offender.mBounds.fCenter( ), offender.mVelocity );
				//	tRayf myRay( transform.fGetTranslation( ), vehicle.fGetVelocity( ) );

				//	f32 theirSpeed;
				//	theirRay.mExtent.fNormalize( theirSpeed );
				//	myRay.mExtent.fNormalize( );

				//	f32 distForMe, distForThem;
				//	if( fClosestPoint( myRay, theirRay, distForMe, distForThem ) )
				//	{
				//		if( distForMe > 0.0f && distForThem > 0.0f )
				//		{
				//			f32 time = distForMe / vehicle.fGetSpeed( );
				//			theirFutureTranslation = theirRay.mExtent * time * theirSpeed;
				//			draw = true;
				//		}
				//	}
				//}

				tObbf theirFuturebounds = offender.mBounds.fTransform( tMat3f( tQuatf::cIdentity, theirFutureTranslation ) );		

				if( Debug_Vehicle_Pathing_DrawDebugYielding )
					debugging->fRenderOnce( theirFuturebounds, tVec4f(1,0,0,1) );

				// compute if we do intersect
				tVec3f posDelta = theirFuturebounds.fCenter( ) - vehiclePos;

				f32 upSep = theirFuturebounds.fSeparationOnAxis( myObb, upVec );
				if( upSep < 0.0f )
				{
					f32 sideSep = theirFuturebounds.fSeparationOnAxis( myObb, sideVec );
					if( sideSep < 0.0f && posDelta.fDot( vehicleFacing ) > 0.0f )
					{
						f32 forwardSep = theirFuturebounds.fSeparationOnAxis( myObb, vehicleFacing );
						f32 usableVel = fMax( offender.mVelocity.fDot( vehicleFacing ), 0.0f );
						//0.5 is the value found in fComputeAccelerationAndDeccelerationToReachSpeed
						f32 maxSlowDownDist = phys.fTimeToReachSpeed( intendedSpeed, usableVel, 0.5 ) * currentSpeed;

						f32 separationDist = vehicle.fFollowDistance( ) + Gameplay_Vehicle_Pathing_AdditionalTargetSeparation;
						f32 slowdownDist = maxSlowDownDist + separationDist; 
						if( forwardSep < slowdownDist )
						{
							// we want to go the speed of the offender, plus some separation adjustment factor
							intendedSpeed = usableVel * 2.0f + (forwardSep - separationDist - currentSpeed * dt );

							if( Debug_Vehicle_Pathing_DrawDebugYielding )
							{
								debugging->fRenderOnce( theirFuturebounds.fCenter( ), vehiclePos, tVec4f(1,0,0,1) );
							}
						}
					}
				}
			}

			
			if( Debug_Vehicle_Pathing_DrawDebugYielding )
			{
				f32 yielding = originalIntendedSpeed - intendedSpeed;
				tVec3f p = transform.fXformPoint( tVec3f(0,5,0) );
				tVec3f d = transform.fXformVector( tVec3f(0,0,yielding) );
				debugging->fRenderOnce( p, p + d, tVec4f(1,1,0,1) );
			}
		}

		if( path.fHasWaypoints( ) && path.fPathMode( ) == tUnitPath::cPathModeFollow )
		{
			// additional vehicle info
			f32 minTurnRadius = phys.fMinTurningRadiusAtSpeed( currentSpeed );

			f32 lookAhead = fClamp( currentSpeed, (f32)Gameplay_Vehicle_Pathing_LookAheadMin, (f32)Gameplay_Vehicle_Pathing_LookAheadMax ); //, fAbs( currentSpeed / vehicle.fGetMaxSpeed( ) ) );
			
			// compute current and future path positions
			tVec3f pathPoint, futurePoint;
			f32 pathDist;
			int segment = path.fDistanceToPath( vehiclePos, pathPoint, pathDist, futurePoint, lookAhead );
			if( segment == 2 && path.fHasWaypoints( 2 ) )
				path.fRequestNext( );

			f32 cornerRad = 0.0f;
			if( Gameplay_Vehicle_Pathing_SlowdownEnable )
			{
				// compute corner slow down and slow down distance
				f32 cornerAngle = acos( path.fCurrentSegment( ).fDot(path.fNextSegment( )) );
				cornerAngle = fClamp( cornerAngle, 0.0f, cPiOver2 ) / cPiOver2;
				f32 desiredCornerSpeed = fLerp( phys.fMaxSpeed( ) * Gameplay_Vehicle_Pathing_SlowdownSpeed, phys.fMaxSpeed( ), 1.0f - cornerAngle );

				cornerRad = phys.fMinTurningRadiusAtSpeed( phys.fMaxSpeed( ) - desiredCornerSpeed ) ;
				cornerRad *= Gameplay_Vehicle_Pathing_SlowdownDistMult;
				if( (path.fTargetPosition( ) - vehiclePos).fLength( ) < cornerRad && currentSpeed > desiredCornerSpeed )
				{
					slowing = true;
					intendedSpeed = fMin( intendedSpeed, desiredCornerSpeed );
				}
			}

			// start targeting
			tVec3f target = futurePoint;
			target.y = transform.fGetTranslation( ).y; //put target into our leveled xz plane
			tVec3f localTarget = transformInv.fXformPoint( target );
			localTarget -= phys.fGetAxlePivotPoint( );
			localTarget.y = 0;

			// compute radius to intersect point
			f32 radius = 0;
			f32 adjacent = localTarget.fLength( ) / 2.0f;
			b32 reverseSteer = false;
			f32 reverseSpeed = originalIntendedSpeed * -0.5f;

			if( !fEqual(adjacent, 0.0f) )
			{
				f32 targetAngle = atan2( localTarget.z, localTarget.x );
				f32 cosVal = cos( targetAngle );

				if( !fEqual(cosVal, 0.0f) )
				{
					// compute turning radius
					radius = adjacent / cosVal;

					f32 steerVal = phys.fComputeSteerValueForRadius( radius, currentSpeed );

					// check if we can ever reach it
					bool canReach = fAbs(radius) > minTurnRadius;
					canReach = canReach && localTarget.z > 0.0f;

					// if not, reverse
					if( !canReach )
						intendedSpeed = reverseSpeed;
					
					inputOut.mSteer = ( currentSpeed < 0.f ) ? -steerVal : steerVal;
				}
				else
					intendedSpeed = reverseSpeed;
			}
			else
			{
				// we're right over the target, back up so we can "see" it
				intendedSpeed = reverseSpeed;
			}


			mIntendedSpeed.fSetBlends( Gameplay_Vehicle_Pathing_IntendedSpeedBlend );
			mIntendedSpeed.fStep( intendedSpeed, dt );
			phys.fComputeAccelerationAndDeccelerationToReachSpeed( mIntendedSpeed.fValue( ), currentSpeed, dt, inputOut );


			if( debugging && Debug_Vehicle_Pathing_DrawDebug )
			{
				//draw target vectors
				tPair<tVec3f,tVec3f> line1( path.fPrevTargetPosition( ), path.fTargetPosition( ) );
				debugging->fRenderOnce( line1, tVec4f(0,1,0,1) );

				tPair<tVec3f,tVec3f> line2( path.fTargetPosition( ), path.fNextTargetPosition( ) );
				debugging->fRenderOnce( line2, tVec4f(1,0,0,1) );

				//draw target
				debugging->fRenderOnce( tSpheref( target, 1.0 ), slowing ? tVec4f(1,0,0,0.125f) : tVec4f(0,0,1,0.125f) );
				
				if( Debug_Vehicle_Pathing_DrawDebugMore )
				{
					//draw current turn sphere
					tVec3f sphereCenter = tVec3f(radius,0,0) + phys.fGetAxlePivotPoint( );
					sphereCenter = transform.fXformPoint( sphereCenter );
					debugging->fRenderOnce( tSpheref( sphereCenter, fAbs(radius) ), tVec4f(0,0,1,0.125f) );

					//draw inside of corner

					//tPair<tVec3f,tVec3f> line3( vehicle.fGetTransform( ).fGetTranslation( ), interceptPt );
					//debugging->fRenderOnce( line3, tVec4f(1,1,0,1) );
					//debugging->fRenderOnce( tSpheref( insideCornerPos, insideCornerRadius ), tVec4f(0,0,1,0.125f) );

					//draw corner info
					debugging->fRenderOnce( tSpheref( path.fTargetPosition( ), cornerRad ), tVec4f(1,1,0,0.125f) );
				}
			}
		}
		else
			inputOut.mDeccelerate = 1.f; // no path, put on the brakes


		return true;
	}

}