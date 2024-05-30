#include "BasePch.hpp"
#include "tHoverPhysics.hpp"
#include "tGroundRayCastCallback.hpp"
#include "Gfx/tRenderableEntity.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	devvar( bool, Physics_Hover_DrawDebug, false ); 
	devvar( bool, Physics_Hover_DrawCollisionDetection, false );
	
	tHoverPhysics::tHoverPhysics( )
		: mDitch( 0 )
		, mLoad( 0.f )
		, mLoadVec( tVec3f::cZeroVector )
		, mCurrentHoverHeight( 0.f )
	{
		// Most values are initialized here
		fReset( tMat3f::cIdentity );
	}

	void tHoverPhysics::fSetProperties( const tHoverProperties& ap )
	{
		mProperties = ap;
	}
	
	void tHoverPhysics::fSetInput( const tHoverInput& input )
	{
		mInput = input;
	}
	
	void tHoverPhysics::fSetTransform( const tMat3f& tm )
	{
		mTransform = tm;
		mFlatTransform = tMat3f::cIdentity;
		tVec3f flatZ = tm.fZAxis( );
		flatZ.y = 0;
		flatZ.fNormalizeSafe( tVec3f::cZAxis );
		mFlatTransform.fOrientZAxis( flatZ );
	}

	void tHoverPhysics::fReset( const Math::tMat3f& tm )
	{
		fSetTransform( tm );
		mInput.fZero( );

		mHeading = tm.fZAxis( ).fXZHeading( );
		mSpeed = 0.0f;
		mGroundOffset = 0.f;
		mCurrentHoverHeight = 0.f;
		mLoad = 0.f;
		mYawRate = 0.f;
		mStrafeRate = tVec2f::cZeroVector;
		mRollPos.fSetValue( tVec2f::cZeroVector );
		mStartupBlendOut.fSetValue( 1.f );
		mV = tVec3f::cZeroVector;
		mOldV = mV;
	}

	void tHoverPhysics::fDitch( bool enable )
	{
		//ditch right or left depending on current roll sign
		mDitch = 1;
	}

	f32 tHoverPhysics::fYawAcc( ) const 
	{
		return mInput.mUserControl ? mProperties.mYawAcc : mProperties.mYawAccAI;
	}

	f32 tHoverPhysics::fYawDamp( ) const 
	{
		return mInput.mUserControl ? mProperties.mYawDamping : mProperties.mYawDampingAI;
	}

	void tHoverPhysics::fPhysicsMT( tLogic* logic, f32 dt )
	{	
		f32 maxRoll = mProperties.mMaxRoll;

		if( mDitch != 0 )
		{
			//override inputs to make hover crash
		}

		// Startup blend is used to smooth the transition from takeoff to flight mode
		f32 targetStartupBlend = (mInput.mActive && mInput.mStartingUp) ? 0.f : 1.f;
		if( mInput.mStartingUp ) mStartupBlendOut.fSetValue( 0.f );
		mStartupBlendOut.fSetBlends( 0.01f );
		mStartupBlendOut.fStep( targetStartupBlend, dt );

		tVec2f oldStrafe = mStrafeRate;

		// apply inputs to buffers
		f32 headingDelta = mInput.mActive ? fShortestWayAround( mHeading, mInput.mIntendedHeading ) : 0;
		mYawRate += headingDelta * fYawAcc( ) - (mYawRate * fYawDamp( ));

		f32 worldStrafeLen, currentRate;
		tVec2f worldStrafeDir = mInput.mWorldStrafe;
		worldStrafeDir.fNormalizeSafe( tVec2f::cZeroVector, worldStrafeLen );

		tVec2f currentStrafeDir = mStrafeRate;
		currentStrafeDir.fNormalizeSafe( tVec2f::cZeroVector, currentRate );

		b32 sharpInput = worldStrafeDir.fDot( currentStrafeDir ) < 0.f;
		b32 strafeInput = mInput.mActive && !fEqual( worldStrafeLen, 0.0f );
		tVec2f strafeRateChangeDir;
		f32 strafeRateChangeMag;

		// When under ai control, always use the acceleration rate, so its a continous function, not piecewise
		if( strafeInput || (!mInput.mUserControl && mInput.mActive) )
		{
			// Accelerate
			f32 acc = mProperties.mSpeedAcc;
			if( sharpInput ) acc *= 2.f;
			strafeRateChangeDir = mInput.mWorldStrafe;
			strafeRateChangeMag = acc * dt;
		}
		else
		{
			// Decelerate
			f32 dec = mProperties.mSpeedDamping;
			f32 newRate = fMax( 0.f, currentRate - dec * dt );
			strafeRateChangeDir = -currentStrafeDir;
			strafeRateChangeMag = currentRate - newRate; //these are backwards because the dir is backwards. for the dot product below.
		}

		// The influence factor is how much the hover is rolled into the direction of intended acceleration.
		//  this adds a lag as the application of the acceleration is deferred until the ship has rolled into the intended direction.
		tVec2f strafeRateChange = strafeRateChangeDir * strafeRateChangeMag;
		tVec2f flatUpDir = mTransform.fYAxis( ).fXZ( );
		f32 influence = fRemapMinimum( fMax( 0.f, flatUpDir.fDot( strafeRateChangeDir ) ), mProperties.mMinStrafeInfluence );
		mStrafeRate += strafeRateChange * influence;
		
		// yaw rate will modify strafe velocity so we dont "drift" so much (slide out in corners)
		const tQuatf strafeYawer( tAxisAnglef( tVec3f::cYAxis, mYawRate * dt * 0.25f ) );
		mStrafeRate = strafeYawer.fRotate( tVec3f( mStrafeRate.x, 0, mStrafeRate.y ) ).fXZ( );

		// FlatTransform is the space where our roll is computed, it helps visually is this is a little pre-yawed (keeps the nose down)
		mFlatTransform *= tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, mYawRate * mProperties.mPreYaw ) ) );

		// compute pitch and roll
		tVec2f strafeAcc = strafeRateChange;
		strafeAcc = mFlatTransform.fInverseXformVector( tVec3f( strafeAcc.x, 0, strafeAcc.y ) ).fXZ( );
		//strafeAcc is now the local target rollPos
		
		tVec2f targetRoll;
		f32 maxRate;

		if( strafeInput )
		{
			// Calculate what the roll would look like if the user was controlling it.
			maxRate = 2.f;
			tVec2f localStrafeDir = strafeAcc.fNormalizeSafe( tVec2f::cZeroVector );

			// taper off roll at high speed
			f32 minRollFact = 0.2f;
			if( sharpInput ) 
			{
				minRollFact = 1.f;
				maxRate = 10.f;
			}

			f32 speedFactor = fRemapMinimum( 1.f - fMin( 1.0f, mSpeed / mProperties.mMaxSpeed ), minRollFact);
			f32 targetRollFactor = speedFactor * (worldStrafeLen * worldStrafeLen); //squared to reduce small displacement sensitivity
			
			targetRoll = localStrafeDir * targetRollFactor;
		}
		else
		{
			// decelerating
			tVec2f localSpeedDir = mFlatTransform.fInverseXformVector( tVec3f( mStrafeRate.x, 0, mStrafeRate.y ) ).fXZ( ).fNormalizeSafe( tVec2f::cZeroVector );
			targetRoll = localSpeedDir * -fMin( 1.0f, mSpeed / mProperties.mMaxSpeed );
			maxRate = 10.f;
		}

		// Roll when we yaw
		targetRoll.x += fClamp( mYawRate * mProperties.mYawRoll, -mProperties.mYawRoll, mProperties.mYawRoll );

		// buffers to state variables
		mHeading += mYawRate * dt;
		mStrafeRate.fNormalizeSafe( tVec2f::cZeroVector, mSpeed );
		mSpeed = fMin( mProperties.mMaxSpeed, mSpeed );
		mStrafeRate *= mSpeed;

		mRollPos.fSetPD( mProperties.mRollP, mProperties.mRollD );
		mRollPos.fStep( targetRoll, dt );

		// generate transform
		while( mHeading > c2Pi ) mHeading -= c2Pi;
		while( mHeading < 0.f ) mHeading += c2Pi;
		const tQuatf yaw( tAxisAnglef( tVec3f::cYAxis, mHeading ) );
		const tQuatf roll( tAxisAnglef( tVec3f::cZAxis, -mRollPos.fValue( ).x * mProperties.mMaxRoll ) );
		const tQuatf pitch( tAxisAnglef( tVec3f::cXAxis, mRollPos.fValue( ).y * mProperties.mMaxRoll ) );

		const tVec3f strafe3D( mStrafeRate.x * dt, 0, mStrafeRate.y * dt );
		const tVec3f worldStrafe = strafe3D;
		tVec3f newPos = mTransform.fGetTranslation( ) + worldStrafe;

		// find ground height
		tGroundRayCastCallback callback( *logic->fOwnerEntity( ), mProperties.mGroundMask );
		tRayf ray;

		const f32 heightStart = 10000.f;
		ray.mOrigin = newPos;
		ray.mOrigin.y = heightStart;
		ray.mExtent = tVec3f( 0, -heightStart * 2, 0 );
		logic->fSceneGraph( )->fRayCastAgainstRenderable( ray, callback );

		f32 realGroundHeight = 0.f;

		if( callback.mHit.fHit( ) )
		{
			realGroundHeight = ray.fEvaluate( callback.mHit.mT ).y;
			mCurrentHoverHeight = newPos.y - realGroundHeight;
		}
		else if( mInput.mActive )
			realGroundHeight =  newPos.y - mCurrentHoverHeight; // fake the ground height
		else
		{
			mCurrentHoverHeight = mProperties.mLandingHeight; //with no ground, we can't park, so just stay here
			realGroundHeight = newPos.y - mCurrentHoverHeight; 
		}

		// correct for ground height
		if( !mInput.mStartingUp )
		{
			f32 correctedHeight = newPos.y;
			f32 vDelta = 0.f;

			if( mInput.mActive )
			{
				f32 intendedVel = 0.f;
				f32 blendRate = mProperties.mElevationVelDamping;
				if( !fEqual( mInput.mHeightAdjustment, 0.f ) )
				{
					intendedVel = mInput.mHeightAdjustment * mProperties.mElevationVelMax;
					blendRate = mProperties.mElevationVelAcc;
				}

				intendedVel = fClamp( intendedVel, -mProperties.mElevationVelMax, mProperties.mElevationVelMax );
				vDelta += (intendedVel - mV.y + vDelta) * blendRate;
			}

			if( !mInput.mActive || (mInput.mStayAboveGround && mCurrentHoverHeight < mProperties.mMinGroundHeight) )
			{
				// determine ideal height above terrain
				f32 targetGroundHeight = mProperties.mLandingHeight;
				if( mInput.mActive ) 
					targetGroundHeight = mProperties.mMinGroundHeight;

				mGroundOffset = fLerp( mGroundOffset, targetGroundHeight, 0.2f );

				const f32 gravity = -10.f;
				vDelta += gravity * dt;

				f32 idealHeight = realGroundHeight + mGroundOffset;
				f32 diff = idealHeight - correctedHeight;
				//if( diff > 0 )
				{
					f32 spring = ( diff > 0 ) ? 1.f : 0.25f;
					f32 damp = 0.2f;
					vDelta += diff * spring - (mV.y + vDelta) * damp;
				}
			}

			correctedHeight += mV.y * dt;
			mV.y += mStartupBlendOut.fValue( ) * vDelta;

			// hard ground penetration clamp
			newPos.y = fMax( correctedHeight, realGroundHeight );

		}

		// Apply final transform
		tQuatf newRot = yaw * pitch * roll;

		//tQuatf existingRot( mTransform );
		//newRot = fNLerpRotation( existingRot, newRot, mStartupBlendOut.fValue( ) );

		mTransform = tMat3f( newRot, newPos );
		mV = tVec3f( mStrafeRate.x, mV.y, mStrafeRate.y );

		mLoadVec = mV - mOldV;
		mLoad = mLoadVec.fLength( ) / (mProperties.mSpeedAcc * dt);
		mOldV = mV;

		fResolveCollisionMT( logic, dt );
	}

	void tHoverPhysics::fResolveCollisionMT( tLogic* logic, f32 dt )
	{
		tSpheref worldSphere = mCollisionShape.fTranslate( mTransform.fGetTranslation( ) );

		if( Physics_Hover_DrawCollisionDetection ) logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( worldSphere, tVec4f(1.f,0.f,0.f,0.2f) );

		for( u32 i = 0; i < mContactPoints.fCount( ); ++i )
		{
			tContactPoint& cp = mContactPoints[ i ];

			f32 separation = 0.f;

			sigassert( cp.mShape );
			cp.mPoint = cp.mShape->fClosestPoint( worldSphere.fCenter( ) );
			cp.mNormal = worldSphere.fCenter( ) - cp.mPoint;
			cp.mNormal.fNormalizeSafe( tVec3f::cYAxis, separation );
			separation -= worldSphere.fRadius( );

			if( Physics_Hover_DrawCollisionDetection ) logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( cp.mPoint, cp.mPoint + tVec3f( 0,10,0 ), tVec4f(0,1,0,1) );
			
			if( separation < 0.f )
			{
				f32 penVel = mV.fDot( cp.mNormal );

				if( penVel < 0.f )
					mV -= cp.mNormal * penVel; // resolve velocity

				// add outward velocity to resolve penetration
				const f32 sepStrength = 4.f; // How stiff the separation is.
				mV += cp.mNormal * (separation*separation * sepStrength);
			}
		}

		mStrafeRate.x = mV.x;
		mStrafeRate.y = mV.z;
	}

	f32 tHoverPhysics::fComputeHeightAdjustment( f32 newHeight ) const
	{
		f32 diff = newHeight - mTransform.fRow( 1 ).w;

		return diff * 2.f - mV.y * 0.5f;
	}

	void tHoverPhysics::fThinkST( tLogic* logic, f32 dt )
	{
	}

	void tHoverPhysics::fClearContacts( )
	{
		mPrevCPs = mContactPoints;
		mContactPoints.fSetCount( 0 );
	}

	void tHoverPhysics::fAddContactPt( f32 dt, const Physics::tContactPoint& cp )
	{
		mContactPoints.fPushBack( cp );
	}

	tVec3f tHoverPhysics::fComputeContactResponse( const Physics::tContactPoint& cp, f32 mass )
	{
		tVec3f result = tVec3f::cZeroVector;

		//TODO HELI COLLISION RESPONSE
		//b32 newContact = true;

		//u32 pCPCnt = mPrevCPs.fCount( );
		//for( u32 p = 0; p < pCPCnt; ++p )
		//{
		//	if( mPrevCPs[p].mEntity == cp.mEntity )
		//	{
		//		//we've already dealt with this guy, no response
		//		//store contact point though (already done)
		//		newContact = false;
		//		break;
		//	}
		//}

		//b32 wouldDestroy = false;
		//if( newContact && !fEqual( mSpeed, 0.0f ) )
		//{
		//	//see if we have an obstruction in our move direction
		//	tVec3f velDir = mGroundVelocity;
		//	velDir.fNormalize( );

		//	tVec3f ptDir = cp.mPoint - mTransform.fGetTranslation( );
		//	ptDir.y = 0;
		//	ptDir.fNormalizeSafe( );

		//	f32 dot = ptDir.fDot( velDir );
		//	if( dot > 0.01f )
		//	{
		//		////totally stop for now
		//		//mStopFactor *= 0.0f; 

		//		f32 impactStrength = fAbs( mSpeed ) / fMaxSpeed( );
		//		f32 weightRatio = fMin( mass / mProperties.mMass, 1.0f );

		//		result = ptDir * impactStrength;
		//		mCollisionDeltaV += result * weightRatio * mProperties.mMass * mProperties.mImpactImpulse;
		//	}
		//}

		return result;
	}
	void tHoverPhysics::fAddCollisionResponse( const Math::tVec3f& response, f32 mass )
	{
		//TODO HELI COLLISION RESPONSE
		//Threads::tMutex m(gVehicleCollisionCS);

		//f32 weightRatio = fMin( mass / mProperties.mMass, 1.0f );
		//mCollisionDeltaV -= response * weightRatio * mProperties.mMass * mProperties.mImpactImpulse;
	}

	void tHoverPhysics::fDrawDebugInfo( tLogic* logicP )
	{
		if( Physics_Hover_DrawDebug )
		{
			//center of mass
			logicP->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mTransform, 1.0f );			
		}
	}

	void tHoverPhysics::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass< tHoverPhysics, tStandardPhysics, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		vm.fNamespace(_SC("Physics")).Bind(_SC("Hover"), classDesc);
	}
}}
