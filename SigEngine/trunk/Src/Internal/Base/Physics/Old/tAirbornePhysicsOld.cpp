#include "BasePch.hpp"
#include "tAirbornePhysicsOld.hpp"
#include "../tGroundRayCastCallback.hpp"
#include "tSceneGraphCollectTris.hpp"

using namespace Sig::Math;

namespace Sig { namespace PhysicsOld
{

	devvar( bool, Debug_Airborne_DrawDebug, false );  
	devvar( f32, Gameplay_Airborne_Pathing_ElevSpring, 15.f ); 
	devvar( f32, Gameplay_Airborne_Pathing_ElevDamp, 0.3f ); 
	//devvar( f32, Gameplay_Airborne_Pathing_ElevAngDamp, 0.5f );
	
	tAirbornePhysics::tAirbornePhysics( )
		: mDitch( 0 )
		, mCollision( false )
	{
		// Expects you to call fReset( )
	}

	void tAirbornePhysics::fSetProperties( const tAirborneProperties& ap )
	{
		mProperties = ap;
		mRoll.fSetPD( mProperties.mRollP, mProperties.mRollD );
		mYawInput.fSetPD( mProperties.mRollP, mProperties.mRollD );
		mElevationInput.fSetBlends( mProperties.mElevationLerp );
	}
	
	void tAirbornePhysics::fSetInput( const tAirborneInput& input )
	{
		if( mDitch == 0 ) mInput = input;
	}
	
	void tAirbornePhysics::fSetTransform( const tMat3f& tm )
	{
		mTransform = tm;
		mHeading = mTransform.fZAxis( ).fXZHeading( );
	}

	void tAirbornePhysics::fReset( const Math::tMat3f& tm )
	{
		fSetTransform( tm );
		mInput.fZero( );

		mSpeed = 0.0f;
		mHeading = tm.fZAxis( ).fXZHeading( );
		mElevation = 0.0f;
		mIdealZoom = 0.f;
		mDitch = 0;
		mLoad = 0.f;

		mRoll.fSetValue( 0.0f );
		mElevationInput.fSetValue( 0.0f );
	}

	void tAirbornePhysics::fDitch( bool enable )
	{
		//ditch right or left depending on current roll sign
		mDitch = !enable ? 0 : ( mRoll.fValue( ) > 0 ) ? 1 : -1;
		//mRoll.fSetValue( mRoll.fValue( ) * 2.f );
	}

	void tAirbornePhysics::fPhysicsMT( tLogic* logic, f32 dt )
	{	
		if( mDitch != 0 )
		{
			//override inputs to make plane crash
			mInput.mThrottle = 1.0f;
			mInput.mStick.y = 0.8f;
			mInput.mStick.x += 0.5f * mDitch * dt;
			mInput.mStick.x = fClamp( mInput.mStick.x, -3.f, 3.f );
			mInput.mActive = true;
		}

		// yaw input
		f32 yawStick = mInput.mStick.x * 1.1f;
		b32 yawInput = !fEqual( yawStick, 0.f );
		yawStick *= yawStick*yawStick;
		f32 targetRoll = yawStick * mProperties.mMaxRoll;
		mRoll.fStep( targetRoll, dt );

		// Yaw is a parallel but separate interpretation of the roll
		if( mDitch == 0 )
		{
			mYawInput.fStep( targetRoll, dt );
			if( !yawInput ) mYawInput.fSetValue( mYawInput.fValue( ) * mProperties.mYawDamping );
		}

		// compute effective mixing
		f32 elevationMixing = 1.0f;

		// elevation input
		f32 pitchStick = mInput.mStick.y;
		b32 pitchInput = !fEqual( pitchStick, 0.f );
		//pitchStick *= pitchStick * fSign( pitchStick );
		mElevationInput.fStep( pitchStick * elevationMixing, dt );

		//mElevationInput.fStep( pitchStick * elevationMixing, dt );
		if( !pitchInput ) mElevationInput.fSetValue( mElevationInput.fValue( ) * mProperties.mElevationDamping );


		// effect orientation
		mHeading += fCurrentYawRate( ) * dt;
		mElevation += fCurrentElevRate( ) * dt;
		mElevation = fClamp( mElevation, -mProperties.mMaxPitch, mProperties.mMaxPitch );

		// generate transform
		tQuatf roll( tAxisAnglef( tVec3f::cZAxis, mRoll.fValue( ) ) );
		tQuatf pitch( tAxisAnglef( tVec3f::cXAxis, mElevation ) );
		tQuatf yaw( tAxisAnglef( tVec3f::cYAxis, mHeading ) );

		// speed and position
		f32 intendedSpeed = 0.f;
		f32 maxSpeed = mProperties.mMaxSpeed;
		if( mInput.mBoost > 0.f )
			maxSpeed += mInput.mBoost * maxSpeed;

		if( mInput.mActive )
		{
			if( mInput.mThrottle >= 1.f )
				intendedSpeed = fLerp( mProperties.mCruiseSpeed, maxSpeed, mInput.mThrottle - 1.0f );
			else
				intendedSpeed = fLerp( mProperties.mSlowSpeed, mProperties.mCruiseSpeed, mInput.mThrottle );
		}

		mSpeed = mV.fLength( );

		f32 speedDelta = intendedSpeed - mSpeed;
		f32 speedLoad = fAbs( speedDelta ) / (maxSpeed - mProperties.mCruiseSpeed);
		f32 pitchLoad = fAbs( mElevation ) / mProperties.mMaxPitch;
		f32 rollLoad = fAbs( mRoll.fValue( ) ) / mProperties.mMaxRoll;
		mLoad = fMax( speedLoad, pitchLoad, rollLoad );

		f32 speedLerp = fLerp( mProperties.mAccelerationLerp, 0.5f, mInput.mBoost );
		mSpeed = fLerp( mSpeed, intendedSpeed, speedLerp ); 

		tVec3f oldPos = mTransform.fGetTranslation( );
		mTransform = tMat3f( yaw * pitch * roll );
		mTransform.fSetTranslation( oldPos + mTransform.fXformVector( tVec3f(0,0,mSpeed * dt) ) );

		// Ray cast to see if we crashed.
		// do this while we stll have a full world space V
		if( mSpeed > 0.1f )
		{
			for( u32 i = 0; i < mProperties.mCollisionProbes.fCount( ); ++i )
			{
				// Ray will terminate at point marked collision probe.
				tRayf ray;
				tVec3f z = mTransform.fZAxis( );
				ray.mExtent = mV * dt;
				ray.mOrigin = mTransform.fXformPoint( mProperties.mCollisionProbes[ i ] ) - ray.mExtent;
				
				// give ourselves a little extra check
				f32 extraRay = 2.f;
				ray.mExtent += z * extraRay;

				Physics::tGroundRayCastCallback cb( *logic->fOwnerEntity( ), mProperties.mGroundMask );
				logic->fSceneGraph( )->fRayCastAgainstRenderable( ray, cb );
				if( cb.mHit.fHit( ) )
					mCollision = true;

				if( Debug_Airborne_DrawDebug )
				{
					tRayf debugRay = ray;
					debugRay.mExtent *= 10.f;
					logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( debugRay, tVec4f( 1,1,0,1 ) );
				}
			}
		}

		// Rectify the V value back to something sane.
		mV = mTransform.fZAxis( ) * mSpeed;

		// interpolate camera parameters
		f32 signedSpeedBlend = fSignedSpeedBlend( );
		if( signedSpeedBlend > 0.f )
			mIdealZoom = fLerp( 0.f, mProperties.mZoomMax, signedSpeedBlend );
		else
			mIdealZoom = fLerp( 0.f, mProperties.mZoomSlow, -signedSpeedBlend );

	}

	f32 tAirbornePhysics::fSignedSpeedBlend( ) const
	{
		if( mSpeed > mProperties.mCruiseSpeed )
			return fClamp( (mSpeed - mProperties.mCruiseSpeed) / (mProperties.mMaxSpeed - mProperties.mCruiseSpeed), 0.f, 1.f );
		else
			return -(1.0f - fClamp( (mSpeed - mProperties.mSlowSpeed) / (mProperties.mCruiseSpeed - mProperties.mSlowSpeed), 0.f, 1.f ));
	}

	f32 tAirbornePhysics::fCurrentYawRate( ) const
	{
		f32 headingMixing = fAbs(mYawInput.fValue( )) / mProperties.mMaxRoll;
		return mProperties.mYawRate * headingMixing * -fSign( mYawInput.fValue( ) );
	}

	f32 tAirbornePhysics::fCurrentElevRate( ) const
	{
		return mProperties.mElevationRate * mElevationInput.fValue( );
	}

	tVec2f tAirbornePhysics::fComputeStickToReachHeading( const tVec3f& heading, b32& passedTarget ) const
	{
		tMat3f flatXform;
		flatXform.fSetTranslation( mTransform.fGetTranslation( ) );
		flatXform.fOrientYAxis( tVec3f::cYAxis, mTransform.fXAxis( ) );

		tVec3f localHead = flatXform.fInverseXformVector( heading );
		localHead.fNormalize( );

		if( localHead.z < 0 )
			passedTarget = true;

		if( passedTarget )
			localHead.x -= 0.5f; //target is behind us, favor one way or the other so we dont end up flying straight away from it

		f32 rollInput = -localHead.x * 6.f;
		rollInput += fCurrentYawRate( ) * mProperties.mAIYawDamping;

		f32 elevInput = -localHead.y * Gameplay_Airborne_Pathing_ElevSpring;
		elevInput += mV.y * Gameplay_Airborne_Pathing_ElevDamp;
		//elevInput -= fCurrentElevRate( ) * Gameplay_Airborne_Pathing_ElevAngDamp;

		tVec2f result ( fClamp( rollInput, -1.f, 1.f ), fClamp( elevInput, -1.f, 1.f ) );

		return result;
	}

	void tAirbornePhysics::fThinkST( tLogic* logic, f32 dt )
	{
	}

	void tAirbornePhysics::fDrawDebugInfo( tLogic* logicP )
	{
		if( Debug_Airborne_DrawDebug )
		{
			//center of mass
			logicP->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mTransform, 1.0f );			
		}
	}

	//void tAirbornePhysics::fExportScriptInterface( tScriptVm& vm )
	//{
	//	Sqrat::DerivedClass< tAirbornePhysics, Logic::tPhysical, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
	//	vm.fNamespace(_SC("Physics")).Bind(_SC("Airborne"), classDesc);
	//}
}}
