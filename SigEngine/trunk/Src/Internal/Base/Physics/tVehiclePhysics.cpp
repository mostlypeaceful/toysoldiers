#include "BasePch.hpp"
#include "tVehiclePhysics.hpp"
#include "tPhysicsWorld.hpp"
#include "tGroundRayCastCallback.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tProfiler.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	devvar( bool, Physics_Vehicle_DrawDebug, false );

	devvar( bool, Physics_Vehicle_Enables_ApplyDynamics, true ); 
	devvar( f32, Physics_Vehicle_CollisionStop, 8.0f ); 
	devvar( bool, Physics_Vehicle_Enables_DoAngleLock, true ); 
	devvar( bool, Physics_Vehicle_Enables_DoHeightLock, false ); 
	devvar( bool, Physics_Vehicle_Enables_FullDynamics, false ); 
	devvar( bool, Physics_Vehicle_Enables_ReverseReverseSteering, false ); 
	devvar( f32, Physics_Vehicle_NormalBlend, 0.0f ); 

	devvar( bool, Perf_VehicleGroundCheck, true ); 
	devvar( f32, Physics_Vehicle_HeightLockMix, 0.125f );
	devvar( f32, Physics_Vehicle_TerrainSlowDown, 0.75f );
	devvar( bool, Physics_Vehicle_3D_Enable, true );
	devvar( f32, Physics_Vehicle_3D_MaxSuspensionAcc, 0.318f );
	devvar( f32, Physics_Vehicle_3D_SlipScale, 0.636f );
	devvar( f32, Physics_Vehicle_3D_TorqueMult, 1.0f );

	devvar( f32, Physics_Vehicle_3D_Friction_StaticScale, 2.0f );
	devvar( f32, Physics_Vehicle_3D_Friction_KineticScale, 1.9f );
	devvar( f32, Physics_Vehicle_3D_Friction_FrontScale, 1.0f );
	devvar( f32, Physics_Vehicle_3D_Friction_RearScale, 1.155f );
	devvar( f32, Physics_Vehicle_3D_AccRollDamping, 0.1f );
	devvar( f32, Physics_Vehicle_3D_AccYawDamping, 0.005f );
	devvar( f32, Physics_Vehicle_3D_SteerRate, 1.3f );
	devvar( f32, Physics_Vehicle_3D_SteerRateFast, 0.8f );
	devvar( f32, Physics_Vehicle_3D_SteerReturnRate, 1.0f );
	devvar( bool, Physics_Vehicle_3D_AWD, true );
	devvar( f32, Physics_Vehicle_3D_AWDFrontTorque, 1.3f );
	devvar( f32, Physics_Vehicle_3D_BrakeBiasFront, 0.75f );
	devvar( f32, Physics_Vehicle_3D_Friction_TempDecay, 1.0f );
	devvar( f32, Physics_Vehicle_3D_Friction_TempRise, 3.0f );

	devvar( f32, Physics_Vehicle_ExtraGravity, 0.0f );

	devvar( bool, Physics_Vehicle_ForceUseDevValues, false );
	devvar( f32, Physics_Vehicle_ForceMaxCompress, 1.68f );
	devvar( f32, Physics_Vehicle_ForceMinCompress, 0.275f );
	devvar( f32, Physics_Vehicle_DampingMaxCompress, 0.40f );
	devvar( f32, Physics_Vehicle_DampingMinCompress, 0.15f );
	devvar( f32, Physics_Vehicle_DampingExtend, 0.03f );
	devvar( bool, Physics_Vehicle_RollInfluence_Enable, false );
	devvar( f32, Physics_Vehicle_RollInfluence_Value, 0.0f );
	devvar( f32, Physics_Vehicle_HandBrakeKick, 0.38f );

	devvar( f32, Physics_Vehicle_AngleLockStart, 0.2f ); //multiples of pi
	devvar( f32, Physics_Vehicle_AngleLockEnd, 0.4f );


	namespace
	{
		static Threads::tCriticalSection gVehicleCollisionCS;

		struct tVehicleGroundRayCastCallback
		{
		public:
			mutable Math::tRayCastHit	mHit;
			mutable tEntity*			mFirstEntity;
			tGrowableArray<tEntity*>	mIgnoreEntities;
			const tEntityTagMask		mGroundMask;
			mutable tVehiclePhysics*	mPhysics;

			explicit tVehicleGroundRayCastCallback( tEntityTagMask groundMask, tVehiclePhysics* physics ) 
				: mFirstEntity( 0 )
				, mGroundMask( groundMask )
				, mPhysics( physics )
			{
			}

			inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i->fOwner( ) );
				if( !spatial->fHasGameTagsAny( mGroundMask ) )
					return;

				for( u32 ig = 0; ig < mIgnoreEntities.fCount( ); ++ig )
				{
					if( spatial == mIgnoreEntities[ ig ] || spatial->fIsAncestorOfMine( *mIgnoreEntities[ ig ] ) )
						return;
				}

				if( i->fQuickRejectByBox( ray ) )
					return;

				if( mPhysics->fProperties( ).mCallback )
				{
					tEntity* logicEnt = spatial->fFirstAncestorWithLogic( );
					if( logicEnt && !mPhysics->fProperties( ).mCallback->fShouldSuspendWheels( logicEnt->fLogic( ) ) )
						return;
				}

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( hit.fHit( ) && hit.mT < mHit.mT )
				{
					mHit			= hit;
					mFirstEntity	= spatial;
				}
			}
			void fRayCast( tSceneGraph& sg, const Math::tRayf& ray );
		};
	}


	tVehiclePhysics::tAxle::tAxle( )
		: mInertia( 0.025f )
		, mMaxAngV( 100.f )
		, mAngVel( 0.f )
		, mAngle( 0.f )
		, mRadius( 0.5f )
		, mSteerAngle( 0.f )
		, mDriveTorque( 0.f )
		, mBrakeTorque( 0.f )
		, mBrakeBias( 1.f )
		, mHandBrake( 0.f )
		, mBurnout( 0.f )
		, mStaticFrictionCoef( 1.f )
		, mKineticFrictionCoef( 1.f )
		, mStaticFriction( true )
		, mSuspensionAcc( 0.f )
		, mSlipMagnitude( 0.f )
		, mSlipRatio( 0.f )
		, mTemperature( 0.f )
		, mCompression( 0.f )
		, mLocalForwardDir( tVec3f::cZAxis )
		, mLocalConstraintDir( tVec3f::cXAxis )
	{ 
	}

	void tVehiclePhysics::tAxle::fSetup( const tVec3f& pos, f32 radius )
	{
		mPosition = pos;
		mRadius = radius;
	}

	f32 tVehiclePhysics::tAxle::fNetTorque( ) const
	{
		// diminish torque by brakes
		f32 absTorque = fAbs( mDriveTorque );
		f32 brakeTorque = fClamp( fSign( mDriveTorque ) * fBrakeTorque( ), -absTorque, absTorque );

		f32 torque = mDriveTorque - brakeTorque;
		if (fAbs(mAngVel) > mMaxAngV) 
			torque = 0.0f;

		return torque;
	}

	// Compute the ideal linear DV if this torque were unconstrained
	f32 tVehiclePhysics::tAxle::fComputeStaticTorqueDV( f32 parentMass, f32 worldlyAcc, f32 forwardVel, f32 dt )
	{
		f32 futureForwardVel = forwardVel + worldlyAcc * dt;
		f32 maxTorque = fAbs( futureForwardVel / dt ) * mRadius * parentMass;
		f32 brakeTorque = fSign( futureForwardVel ) * fBrakeTorque( );
		brakeTorque = fClamp( brakeTorque, -maxTorque, maxTorque );

		f32 patchForce = (fNetTorque( ) + brakeTorque) / mRadius;
		f32 patchAcc = patchForce / parentMass + worldlyAcc;
		f32 patchDV = patchAcc * dt;

		mAngVel = forwardVel / mRadius;
		mAngle += mAngVel * dt;

		return patchDV;
	}

	// Compute the ideal linear DV if this wheel is sliding
	f32 tVehiclePhysics::tAxle::fComputeKineticTorqueDV( f32 maxKineticForce, f32 forwardVel, f32 maxSpeed, f32 dt )
	{
		f32 kineticTorqueMax = maxKineticForce * mRadius;

		// ground torque is how much torque the force from sliding is inflicting on the wheel.
		f32 velDiff = forwardVel - fPatchSpeed( );
		f32 torqueToCorrectVel = velDiff * mInertia / dt;
		f32 groundTorque = fClamp( torqueToCorrectVel, -kineticTorqueMax, kineticTorqueMax );

		f32 driveTorque = fNetTorque( );

		// Determine how much brake torque we'd actually use up stopping
		f32 absGT = fAbs( groundTorque );
		f32 absAngVel = fAbs( mAngVel ) * mInertia / dt;
		f32 absTorque = fAbs( driveTorque );
		f32 maxTorqueToBrake = absGT + absAngVel + absTorque;

		// compute the brake torque possible to 
		f32 bt = fSign( mAngVel ) * fClamp(fBrakeTorque( ), -maxTorqueToBrake, maxTorqueToBrake);

		f32 appliedTorque = groundTorque + driveTorque;
		f32 deltaAV = appliedTorque / mInertia * dt;
		mAngVel = fClamp( mAngVel + deltaAV, -mMaxAngV, mMaxAngV );
		mAngle += mAngVel * dt;

		if( fAbs( forwardVel ) > maxSpeed && fSign( mDriveTorque ) == fSign( forwardVel ) ) 
			driveTorque = 0.f;

		f32 torque = fClamp(groundTorque - bt - driveTorque, -kineticTorqueMax, kineticTorqueMax);
		return torque / mRadius;
	}

	void tVehiclePhysics::tAxle::fSetSteering( f32 angle )
	{
		tMat3f rotate( tQuatf( tAxisAnglef( tVec3f::cYAxis, angle ) ) );
		mLocalConstraintDir = rotate.fXAxis( );
		mLocalForwardDir = rotate.fZAxis( );
		mSteerAngle = angle;
	}

	
	tVehiclePhysics::tVehiclePhysics( tEntity* owner )
		: mOwner( owner )
		, mCurrentSteer( 0.0f, 0.5f, 0.5f, 4.0f )
		, mWheelInfluence( 0.0f, 20.0f, 10.f )
		, mFullDynamics( -1.0f )
		, mDoGroundCheck( true )
	{
		mBody.fReset( NEW tRigidBody( ) );
		mBodyA = mBody;
	}

	void tVehiclePhysics::fStepConstraintInternal( f32 dt, f32 percentage )
	{	
		log_assert( !fBody( ).fTransform( ).fIsNan( ), "Nan transform!" );

		fIslandData( ).fAwaken( );

		fUpdateSteeringAngle( dt );

		fBody( ).fSetGravity( 0.f ); //gravity will be applied through constraints
		fComplexPhysics( dt );

		//update transforms
		for (u32 i = 0; i < mWheelStates.fCount( ); ++i)
			mWheelStates[i].mTransform = fBody( ).fTransform( ) * mWheelStates[i].mTransform;

		if(  mProperties.mHasFenders )
		{
			for (u32 i = 0; i < mWheelStates.fCount( ); ++i)
				mWheelStates[i].mFenderTransform = tMat3f::cIdentity; //mTransform * mWheelStates[i].mFenderTransform;
		}


		log_assert( !fBody( ).fTransform( ).fIsNan( ), "Nan transform!" );
	}

	void tVehiclePhysics::fComplexPhysics( f32 dt )
	{
		fComputeAndApplyWheelforcesMT( dt );
		fComplexFriction( dt );
	}

	void tVehiclePhysics::fComplexFriction( f32 dt )
	{
		fUpdateAxleInfo( );

		for( u32 i = 0; i < mAxles.fCount( ); ++i )
			fApplyWheelConstraint( mAxles[ i ], tVec3f::cZeroVector, dt );

		fUpdateWheelInfo( dt );
	}

	void tVehiclePhysics::fUpdateAxleInfo( )
	{
		// Steering
		f32 maxSteerAngle = fMaxSteeringAngleAtSpeed( mSpeed );
		fFrontAxle( ).fSetSteering( mProperties.mWheels[0].mSteer * mCurrentSteer.fValue( ) * maxSteerAngle );
		fBackAxle( ).fSetSteering( mProperties.mWheels[2].mSteer * mCurrentSteer.fValue( ) * maxSteerAngle );

		f32 maxAngV = fMaxSpeed( ) / mProperties.mWheels[0].mRadius;
		fBackAxle( ).mMaxAngV = maxAngV;
		fFrontAxle( ).mMaxAngV = maxAngV;

		// Torque
		f32 torqueMultiplier = Physics_Vehicle_3D_TorqueMult * mInput.mClutch;
		b32 awd = Physics_Vehicle_3D_AWD && fEqual( mInput.mBurnout, 0.f );
		if( awd ) 
			torqueMultiplier *= 0.5f;

		if( mInput.mBurnout )
			torqueMultiplier *= 2.f;

		fBackAxle( ).mDriveTorque = torqueMultiplier * mInput.mAccelerate * mProperties.mAcceleration * mProperties.mMass * fBackAxle( ).mRadius;
		if( awd )
			fFrontAxle( ).mDriveTorque = Physics_Vehicle_3D_AWDFrontTorque * fBackAxle( ).mDriveTorque;

		fBackAxle( ).mBurnout = mInput.mBurnout;
		
		f32 brakeTorque = -mProperties.mDecceleration * mProperties.mMass * fBackAxle( ).mRadius;
		fBackAxle( ).mBrakeTorque = mInput.mDeccelerate * brakeTorque;
		fFrontAxle( ).mBrakeTorque = fBackAxle( ).mBrakeTorque;
		fBackAxle( ).mHandBrake = -300000.0f * mInput.mHandBrake;
		//fFrontAxle( ).mHandBrake = brakeTorque * 0.25f * mInput.mHandBrake;

		f32 frontBias = Physics_Vehicle_3D_BrakeBiasFront;
		// bias all forward if both gas and brake held. burn out.
		if( !fEqual( mInput.mDeccelerate, 0.f ) && !fEqual( mInput.mAccelerate, 0.f )/* && !mInput.mHandBrake*/ )
			frontBias = 1.f;

		fFrontAxle( ).mBrakeBias = frontBias * 2.0f; // *2 because it's split, avg to 1.0
		fBackAxle( ).mBrakeBias = (1.0f - frontBias) * 2.0f;

		// Suspension
		fFrontAxle( ).mCompression = (mWheelStates[ 0 ].mCompression + mWheelStates[ 1 ].mCompression) * 0.5f;
		fBackAxle( ).mCompression = (mWheelStates[ 2 ].mCompression + mWheelStates[ 3 ].mCompression) * 0.5f;
		fFrontAxle( ).mSurfaceVelocity = (mWheelStates[ 0 ].mSurfaceVelocity + mWheelStates[ 1 ].mSurfaceVelocity) * 0.5f;
		fBackAxle( ).mSurfaceVelocity = (mWheelStates[ 2 ].mSurfaceVelocity + mWheelStates[ 3 ].mSurfaceVelocity) * 0.5f;
		fFrontAxle( ).mSurfaceNormal = (mWheelStates[ 0 ].mSurfaceNormal + mWheelStates[ 1 ].mSurfaceNormal).fNormalizeSafe( tVec3f::cZeroVector );
		fBackAxle( ).mSurfaceNormal = (mWheelStates[ 2 ].mSurfaceNormal + mWheelStates[ 3 ].mSurfaceNormal).fNormalizeSafe( tVec3f::cZeroVector );
		fFrontAxle( ).mSuspensionAcc = (mWheelStates[ 0 ].mSuspensionAcc + mWheelStates[ 1 ].mSuspensionAcc);
		fBackAxle( ).mSuspensionAcc = (mWheelStates[ 2 ].mSuspensionAcc + mWheelStates[ 3 ].mSuspensionAcc);

		if( fFrontAxle( ).mSuspensionAcc > 0.0f )
		{
			f32 ratio = mWheelStates[ 0 ].mSuspensionAcc / fFrontAxle( ).mSuspensionAcc;
			fFrontAxle( ).mPosition = mProperties.mWheels[ 0 ].mPosition.fGetTranslation( ) * ratio + mProperties.mWheels[ 1 ].mPosition.fGetTranslation( ) * (1.0f - ratio);
			fFrontAxle( ).mPosition.y = 0;
		}
		if( fBackAxle( ).mSuspensionAcc > 0.0f )
		{
			f32 ratio = mWheelStates[ 2 ].mSuspensionAcc / fBackAxle( ).mSuspensionAcc;
			fBackAxle( ).mPosition = mProperties.mWheels[ 2 ].mPosition.fGetTranslation( ) * ratio + mProperties.mWheels[ 3 ].mPosition.fGetTranslation( ) * (1.0f - ratio);
			fBackAxle( ).mPosition.y = 0;
		}

		fFrontAxle( ).mSuspensionAcc *= 2.0f;
		fBackAxle( ).mSuspensionAcc *= 2.0f;

		// Friction
		fFrontAxle( ).mStaticFrictionCoef = 1.0f * Physics_Vehicle_3D_Friction_StaticScale * Physics_Vehicle_3D_Friction_FrontScale;
		fFrontAxle( ).mKineticFrictionCoef = 0.85f * Physics_Vehicle_3D_Friction_KineticScale * Physics_Vehicle_3D_Friction_FrontScale;
		fBackAxle( ).mStaticFrictionCoef = 1.0f * Physics_Vehicle_3D_Friction_StaticScale * Physics_Vehicle_3D_Friction_RearScale;
		fBackAxle( ).mKineticFrictionCoef = 0.85f * Physics_Vehicle_3D_Friction_KineticScale * Physics_Vehicle_3D_Friction_RearScale;
	}

	void tVehiclePhysics::fUpdateWheelInfo( f32 dt )
	{
		mSpeed = fBackAxle( ).fPatchSpeed( ); //mV.fDot( mTransform.fZAxis( ) );
		
		mWheelStates[ 0 ].mAngle = fFrontAxle( ).mAngle;
		mWheelStates[ 0 ].mAngVel = fFrontAxle( ).mAngVel;
		mWheelStates[ 0 ].mTemperature = fFrontAxle( ).mTemperature;
		mWheelStates[ 1 ].mAngle = fFrontAxle( ).mAngle;
		mWheelStates[ 1 ].mAngVel = fFrontAxle( ).mAngVel;
		mWheelStates[ 1 ].mTemperature = fFrontAxle( ).mTemperature;
		mWheelStates[ 2 ].mAngle = fBackAxle( ).mAngle;
		mWheelStates[ 2 ].mAngVel = fBackAxle( ).mAngVel;
		mWheelStates[ 2 ].mTemperature = fBackAxle( ).mTemperature;
		mWheelStates[ 3 ].mAngle = fBackAxle( ).mAngle;
		mWheelStates[ 3 ].mAngVel = fBackAxle( ).mAngVel;
		mWheelStates[ 3 ].mTemperature = fBackAxle( ).mTemperature;
	}

	void tVehiclePhysics::fApplyWheelConstraint( tAxle& axle, const tVec3f& worldGravity, f32 dt )
	{
		tVec3f gravityAccTotal = fGravityVector( ) * 0.5f; // half because this will happen again on the other axle
		tVec3f gravityAcc = gravityAccTotal;
		f32 previousAngle = axle.mAngle;

		if( axle.mSuspensionAcc == 0.f ) 
		{
			axle.mFrictionAcc = tVec2f::cZeroVector;
			axle.mSlipMagnitude = 0.f;
			axle.mSlipRatio = 1.0f;
			axle.mAngle += axle.mAngVel * dt; //TODO integrate torques from engine and brakes
		}
		else
		{
			//log_line( Log::cFlagPhysics, suspensionAcc << " " << fGravity( ) );

			// Gather a lot of data
			f32 mass = fBody( ).fMass( );
			f32 suspensionAcc = fGravity( ) * axle.mSuspensionAcc;
			f32 maxStaticAcc = fAbs( suspensionAcc * axle.mStaticFrictionCoef );
			f32 maxKineticAcc = fAbs( suspensionAcc * axle.mKineticFrictionCoef );
			f32 maxKineticForce = maxKineticAcc * mass;
			f32 maxKineticDV = maxKineticAcc * dt;

			tVec3f constraintPt = fBody( ).fTransform( ).fXformPoint( axle.mPosition );
			tVec3f forwardDir = fBody( ).fToWorld( axle.mLocalForwardDir );
			tVec3f constraintDir = fBody( ).fToWorld( axle.mLocalConstraintDir );

			//const tVec3f debugOffset( 0,2,0 );
			//logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( constraintPt + debugOffset, constraintPt + debugOffset + constraintDir, tVec4f( 1,0,0,1) );

			tVec3f groundVel = fBody( ).fPointVelocity( constraintPt ) - axle.mSurfaceVelocity;// +gravityAccTotal * dt;
			f32 constraintVelMag = groundVel.fDot( constraintDir );
			tVec3f constraintVel = constraintDir * constraintVelMag;
			f32 forwardVelMag = groundVel.fDot( forwardDir );
			tVec3f patchVel = forwardDir * axle.fPatchSpeed( );
			tVec3f patchVelDiff = groundVel - patchVel;
			f32 patchVelDiffMag = patchVelDiff.fLength( );

			tVec3f torqueDV = tVec3f::cZeroVector;

			if (axle.mStaticFriction)
			{
				f32 gravityAcc = gravityAccTotal.fDot( forwardDir );
				torqueDV = forwardDir * axle.fComputeStaticTorqueDV( mass, gravityAcc, forwardVelMag, dt );
			}
			else
			{
				f32 forwardVelDiff = axle.fComputeKineticTorqueDV( maxKineticForce, forwardVelMag, mProperties.mMaxSpeed, dt ) / mass * dt;
				forwardVelDiff = fClamp( forwardVelDiff, -maxKineticDV, maxKineticDV );
				torqueDV = forwardDir * -forwardVelDiff;
			}

			// Compute the corrective impulse and its resultant acceleration
			tVec3f sideImpulseAcc = fBody( ).fComputeImpulseToChangePointVel( dt, constraintPt, -constraintVel ) / mass / dt;
			tVec3f forwardImpulseAcc = fBody( ).fComputeImpulseToChangePointVel( dt, constraintPt, torqueDV ) / mass / dt;

			// Determine whether to switch to kinetic or static friction
			tVec3f frictionAcc =  sideImpulseAcc + forwardImpulseAcc;
			f32 accMag = frictionAcc.fLength( );

			if( !axle.mStaticFriction && accMag < maxKineticAcc * 0.9f && patchVelDiffMag < 1.f )
				axle.mStaticFriction = true;

			// compute slip mag and ratio even during static friction.
			f32 continuousSlipMag = 0.f;
			f32 continuousSlipRatio = 0.f;

			if (fAbs(accMag) > 0.1f)
			{
				// most fun slip formula
				continuousSlipMag = fAbs( frictionAcc.fDot( constraintDir ) / accMag ); //axle.mSlipMagnitude; //
				continuousSlipMag *= fRemapMinimum( fClamp( fAbs( forwardImpulseAcc.fLength( ) / maxKineticAcc ), 0.0f, 1.0f ), 0.25f );

				// more accurate slip formula
				//continuousSlipMag = fMin( fAbs( forwardImpulseAcc.fLength( ) / maxKineticAcc ), 1.0f );

				continuousSlipMag += mInput.mBurnout;

				continuousSlipRatio = 1.0f - fClamp( continuousSlipMag, 0.0f, 1.0f );

				f32 slipScale = Physics_Vehicle_3D_SlipScale;
				continuousSlipRatio *= slipScale;
				continuousSlipRatio += 1.0f - slipScale;
			}

			if( axle.mStaticFriction )
			{
				if (accMag > maxStaticAcc * axle.mSlipRatio)
					axle.mStaticFriction = false;
			}

			if( !axle.mStaticFriction )
			{
				// Kinetic friction, clamp acceleration
				sideImpulseAcc.fClampLength( maxKineticAcc * axle.mSlipRatio );
				forwardImpulseAcc.fClampLength( maxKineticAcc );
				frictionAcc = sideImpulseAcc + forwardImpulseAcc;
				frictionAcc.fClampLength( maxKineticAcc * axle.mSlipRatio );
				axle.mTemperature = fClamp( axle.mTemperature + continuousSlipMag * Physics_Vehicle_3D_Friction_TempRise * dt, 0.f, 1.f );

				axle.mSlipRatio = continuousSlipRatio;
				axle.mSlipMagnitude = continuousSlipMag;
			}
			else
			{
				// static friction
				sideImpulseAcc *= axle.mSlipRatio;
				frictionAcc = sideImpulseAcc + forwardImpulseAcc;

				// remove gravity in friction directions
				tVec3f frictionGravity = gravityAccTotal - gravityAccTotal.fDot( axle.mSurfaceNormal ) * axle.mSurfaceNormal;
				gravityAcc -= frictionGravity;

				axle.mSlipRatio = 1.0f;
				axle.mSlipMagnitude = 0.0f;
			}

			// Boost
			if( fBody( ).fVelocity( ).fLength( ) < fMaxSpeed( ) )
				frictionAcc += forwardDir * mInput.mBoost * mass * axle.mCompression;

			// Handbrake kick
			if( !fEqual( axle.mHandBrake, 0.f ) && fWheelInfluence( ) > 0.75f && fSpeedPercent( ) > 0.25f )
			{
				f32 kickAcc = fGravity( ) * axle.mStaticFrictionCoef * Physics_Vehicle_HandBrakeKick * axle.mSlipRatio;
				frictionAcc += fSteeringInput( ) * kickAcc * constraintDir; // * fSign( constraintVelMag );
			}

			// Output variables
			axle.mFrictionAcc = Math::tVec2f( frictionAcc.fDot( constraintDir ), frictionAcc.fDot( forwardDir ) ) / fGravity( );

			// Remove forward force if burning out
			frictionAcc -= forwardDir * forwardDir.fDot( frictionAcc ) * axle.mBurnout;

			// Apply result
			tVec3f totalImpulse = frictionAcc * dt * mass;

			f32 rollInfluence = mProperties.mTurnRoll;
			f32 pitchInfluence = mProperties.mAccelerationDive;

			if( Physics_Vehicle_RollInfluence_Enable )
			{
				rollInfluence = Physics_Vehicle_RollInfluence_Value;
				pitchInfluence = 0.f;
			}
			
			{
				//Roll
				tVec3f rollDir = fBody( ).fTransform( ).fXAxis( );
				tVec3f rollImpulse = rollDir * totalImpulse.fDot( rollDir );
				tVec3f hardPoint = constraintPt;
				hardPoint -= fBody( ).fTransform( ).fYAxis( ) * rollInfluence;
				fBody( ).fAddImpulse( rollImpulse, hardPoint );
			}
			{
				//pitch
				tVec3f pitchDir = fBody( ).fTransform( ).fZAxis( );
				tVec3f pitchImpulse = pitchDir * totalImpulse.fDot( pitchDir );
				tVec3f hardPoint = constraintPt;
				hardPoint -= fBody( ).fTransform( ).fYAxis( ) * pitchInfluence;
				fBody( ).fAddImpulse( pitchImpulse, hardPoint );
			}

			// Damp body rotation under high acceleration
			f32 frictionLen = fMin( axle.mFrictionAcc.fLength( ), 1.f );
			f32 rollDamping = 1.0f - frictionLen * Physics_Vehicle_3D_AccRollDamping;
			f32 yawDamping = 1.0f - frictionLen * Physics_Vehicle_3D_AccYawDamping;
			fBody( ).fSetAngularDamping( tVec3f( rollDamping, yawDamping, rollDamping ) );
		}

		fBody( ).fSetVelocity( fBody( ).fVelocity( ) + gravityAcc * dt );

		if( axle.mHandBrake < 0.f ) 
		{
			axle.mAngle = previousAngle;
		}

		axle.mTemperature -= Physics_Vehicle_3D_Friction_TempDecay * dt;
		axle.mTemperature = fClamp( axle.mTemperature, 0.f, 1.f );
	}

	void tVehiclePhysics::fSetProperties( const tVehicleProperties& vp )
	{
		mProperties = vp;

		fBody( ).fSetMassProperties( mProperties.mHalfExtents, mProperties.mMass, mProperties.mInertiaScale );

		u32 wCnt = vp.mWheels.fCount( );

		// dont re-initialize the wheels due to the scale extraction
		if( wCnt != mWheelStates.fCount( ) )
		{
			mWheelStates.fResize( wCnt );

			mTotalWheelbase = fAbs( mProperties.mFrontWheelbase ) + fAbs( mProperties.mRearWheelbase );
			
			for( u32 i = 0; i < wCnt; ++i )
			{
				tWheelProperties& wheel = mProperties.mWheels[ i ];
				wheel.mScale = wheel.mPosition.fGetScale();
				wheel.mPosition.fScaleLocal( 1.0f / wheel.mScale );
			}

			fUpdateWeightDistribution( );
		}

		fFrontAxle( ).fSetup( tVec3f( 0,0, mProperties.mFrontWheelbase ), mProperties.mWheels[ 0 ].mRadius );
		fBackAxle( ).fSetup( tVec3f( 0,0, mProperties.mRearWheelbase ), mProperties.mWheels[ 2 ].mRadius );
	}

	void tVehiclePhysics::fUpdateWeightDistribution( )
	{
		f32 totalWheelBase = fAbs( mProperties.mFrontWheelbase ) + fAbs( mProperties.mRearWheelbase );

		for( u32 i = 0; i < mProperties.mWheels.fCount( ); ++i )
		{
			f32 dist = fAbs( mProperties.mWheels[ i ].mPosition.fGetTranslation( ).z );
			mProperties.mWheels[ i ].mWeight = (1.f - dist / totalWheelBase) * -fGravity( ) * fBody( ).fMass( ) * 0.5f; //expecting two wheels at each end
		}
	}

	void tVehiclePhysics::fUpdateSteeringAngle( f32 dt )
	{
		// New way rotate towards new angle, done in radians
		f32 newAngle = mInput.mSteer * mProperties.mSteerLock * fSteerReverser( );
		f32 oldAngle = mCurrentSteer.fValue( ) * mProperties.mSteerLock;
		f32 delta = newAngle - oldAngle;
		f32 absDelta = fAbs(delta);

		f32 steerRate = (!fEqual( mInput.mSteer, 0.0f )) ? Physics_Vehicle_3D_SteerReturnRate : fLerp( (f32)Physics_Vehicle_3D_SteerRate, (f32)Physics_Vehicle_3D_SteerRateFast, fAbs( fSpeed( ) ) / fMaxSpeed( ) );
		f32 change = fSign(delta) * steerRate * dt;
		change = fClamp(change, -absDelta, absDelta);

		// conver to zero to 1
		mCurrentSteer.fSetValue( (oldAngle + change) / mProperties.mSteerLock );
	}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	void tVehiclePhysics::fSetInput( const tVehicleInput& input )
	{
		mInput = input;
	}

	void tVehiclePhysics::fReset( const Math::tMat3f& tm )
	{
		fBody( ).fReset( tm );
		mWheelInfluence.fSetValue( 0.0f );
		mSpeed = 0.0f;
		mInput.fZero( );
		mCollisionDeltaV = tVec3f::cZeroVector;
		mTankTreadPosition.fFill( 0.f );
	}

	void tVehiclePhysics::fSetVelocity( const Math::tVec3f& v )
	{
		fBody( ).fSetVelocity( v );
		mSpeed = v.fDot( fBody( ).fTransform( ).fZAxis( ) );
	}

	void tVehiclePhysics::fComputeAndApplyWheelforcesMT( f32 dt )
	{
		profile( cProfilePerfVehiclePhysicsDynamics );

		log_assert( !fBody( ).fTransform( ).fIsNan( ), "Nan transform!" );

		if( fBody( ).fTransform( ).fIsNan( ) )
		{
			fBody( ).fSetTransform( tMat3f::cIdentity );
			log_warning( "Killed vehicle because of NAN" );
		}

		if( Physics_Vehicle_ForceUseDevValues )
		{
			mProperties.mSpringMax = Physics_Vehicle_ForceMaxCompress;
			mProperties.mSpringMin = Physics_Vehicle_ForceMinCompress;
			mProperties.mDampMax = Physics_Vehicle_DampingMaxCompress;
			mProperties.mDampMin = Physics_Vehicle_DampingMinCompress;
			mProperties.mDampMixingExtend = Physics_Vehicle_DampingExtend;
		}

		const tVec3f localForward = fBody( ).fTransform( ).fZAxis( );
		tVec3f localForwardFlat = localForward;
		localForwardFlat.fProjectToXZ( );
		tVec3f forces[4] = { tVec3f::cZeroVector, tVec3f::cZeroVector, tVec3f::cZeroVector, tVec3f::cZeroVector };
		tVec3f forcePoints[4];

		tVec3f velDir = fBody( ).fVelocity( );
		velDir.fNormalizeSafe( tVec3f::cZeroVector );
		
		f32 maxSteerAngle = fMaxSteeringAngleAtSpeed( mSpeed );
		tGrowableArray<tEntity*> ignoreList;
		//ignoreList.fSetCapacity( fBody( ).mConstraints.fCount( ) + 1 );
		//ignoreList.fPushBack( logic->fOwnerEntity( ) );
		//for( u32 i = 0; i < fBody( ).mConstraints.fCount( ); ++i )
		//	ignoreList.fPushBack( fBody( ).mConstraints[ i ]->fOtherRigidBody( this )->fOwnerEntity( ) );

		u32 hit = 0;

		for (u32 i = 0; i < mProperties.mWheels.fCount(); ++i)
		{
			tWheelProperties& wheelProps = mProperties.mWheels[ i ];
			tWheelState& wheelState = mWheelStates[ i ];
			const tVec3f sagOffset = tVec3f(0, -wheelState.mSag, 0);

			if( wheelProps.mFollowIndex == ~0 )
			{
				tVec3f localForcePt = wheelProps.mPosition.fGetTranslation( );
				forcePoints[i] = fBody( ).fTransform( ).fXformPoint( localForcePt );
				
				f32 totalProbeLength = fTotalProbeLength( i );
				const f32 upperTLimit = wheelProps.mProbeLength / totalProbeLength;

				tVec3f localProbeDir = tVec3f( 0, -totalProbeLength, 0 );
				tVec3f probeDir = fBody( ).fTransform( ).fXformVector( localProbeDir );
				f32 radiusT = wheelProps.mRadius / totalProbeLength;

				//offset the ray down by radiusT so ray ends at tire patch instead of tire center
				wheelState.mRay = Math::tRayf( forcePoints[i] - probeDir * (1.f - radiusT) + sagOffset, probeDir );


				if( mDoGroundCheck )
				{
					log_assert( !wheelState.mRay.mExtent.fIsNan( ) && !wheelState.mRay.mExtent.fIsZero( ), "Nans in Vehicle!" );
					
					tVehicleGroundRayCastCallback rayCastCallback( mProperties.mGroundMask, this );
					rayCastCallback.mIgnoreEntities = ignoreList;

					{
						profile( cProfilePerfVehiclePhysicsRayCast );
						mOwner->fSceneGraph( )->fRayCastAgainstRenderable( wheelState.mRay, rayCastCallback );
					}

					if( rayCastCallback.mHit.fHit( ) )
					{
						++hit;

						const tVec3f contactPt = wheelState.mRay.fPointAtTime( rayCastCallback.mHit.mT );
						tEntity* logicEnt = rayCastCallback.mFirstEntity->fFirstAncestorWithLogic( );
						tRigidBody* surfaceBody = logicEnt->fLogic( )->fQueryPhysicalDerived<tRigidBody>( );
						if( surfaceBody )
							wheelState.mSurfaceVelocity = surfaceBody->fPointVelocity( contactPt );
						else 
							wheelState.mSurfaceVelocity = tVec3f::cZeroVector;

						tVec3f normal = fLerp( rayCastCallback.mHit.mN, tVec3f::cYAxis, (f32)Physics_Vehicle_NormalBlend ).fNormalize( );
						wheelState.mSurfaceNormal = normal;
						
						if( rayCastCallback.mFirstEntity )
							wheelState.mSurfaceType = rayCastCallback.mFirstEntity->fQueryEnumValue( mProperties.mSurfaceTypeEnumID );

						//old model - turns out i like this one better
						//tVec3f normal = rayCastCallback.mHit.mN;
						//f32 lenSqr = normal.fLengthSquared( ); 
						//if( !fEqual( lenSqr, 1.0f ) && !fEqual( lenSqr, 0.0f ) ) normal.fNormalize( );

						//<strike> need to retune vehicles to this model! </strike>
						//tVec3f normal = -probeDir; 
						//normal.fNormalize( );

						wheelState.mCompression = 1.0f - rayCastCallback.mHit.mT;
						wheelState.mCompression = fClamp( wheelState.mCompression / upperTLimit, 0.0f, 1.0f );
						f32 clampedCompression = fMin( wheelState.mCompression, 0.8f );

						//damping force
						tVec3f normalVel = fBody( ).fPointVelocity( forcePoints[i] ) - wheelState.mSurfaceVelocity;
						wheelState.mCompressionVel = normal.fDot(normalVel);

						//spring force
						f32 deltaV = 0;
						f32 springForce = wheelProps.mWeight * fLerp( mProperties.mSpringMin, mProperties.mSpringMax, wheelState.mCompression);
						deltaV = springForce / fBody( ).fMass( ) * dt;

						f32 dampCompression = clampedCompression;
						mProperties.mDampMixingCompress = fLerp( mProperties.mDampMin, mProperties.mDampMax, wheelState.mCompression);
						dampCompression = wheelState.mCompression;

						f32 sagCompression = (1.f - mProperties.mSpringMin) / (mProperties.mSpringMax - mProperties.mSpringMin);
						wheelState.mSag = sagCompression * wheelProps.mProbeLength;
						
						if( wheelState.mCompressionVel < 0 )
							deltaV += -mProperties.mDampMixingCompress * wheelState.mCompressionVel * dampCompression;
						else
							deltaV += -mProperties.mDampMixingExtend * wheelState.mCompressionVel * dampCompression;

						wheelState.mSuspensionAcc = deltaV / dt;
						forces[ i ] = wheelState.mSuspensionAcc * fBody( ).fMass( ) * normal;
						//Clamp for niceness, no asplosions, convert to Gs
						wheelState.mSuspensionAcc = fMin( wheelState.mSuspensionAcc / -fGravity( ), (f32)Physics_Vehicle_3D_MaxSuspensionAcc ) ;
					}
					else
					{
						wheelState.mCompression = 0.0f;
						wheelState.mCompressionVel = 0.f;
						wheelState.mSuspensionAcc = 0.0f;
						wheelState.mSurfaceVelocity = tVec3f::cZeroVector;
						wheelState.mSurfaceNormal = tVec3f::cZeroVector;
					}
				}

				wheelState.mAngle += wheelState.mAngVel * dt;

				//compute wheel transform
				f32 steerAngle = mProperties.mWheels[i].mSteer * mCurrentSteer.fValue( ) * maxSteerAngle;

				tQuatf wheelRotation( tAxisAnglef( wheelProps.mPosition.fXAxis( ), wheelState.mAngle ) );
				tQuatf wheelSteer( tAxisAnglef( wheelProps.mPosition.fYAxis( ), steerAngle ) );
				tMat3f scale(wheelProps.mScale);

				//displacement is relative to entity starting position
				tVec3f pos = localProbeDir * (-wheelState.mCompression * upperTLimit) + sagOffset;
				tMat3f displacement( wheelSteer * wheelRotation, pos );

				wheelState.mTransform = wheelProps.mPosition * displacement * scale;

				if( mProperties.mHasFenders )
				{
					tMat3f displacementFender( wheelSteer, pos );
					wheelState.mFenderTransform = wheelProps.mPosition * displacementFender * scale;
				}
			}
			else
			{
				//dummy wheel
				const tWheelProperties& master = mProperties.mWheels[ wheelProps.mFollowIndex ];
				tVec3f pDelta = wheelProps.mPosition.fGetTranslation( ) - master.mPosition.fGetTranslation( );

				wheelState.mTransform = mWheelStates[ wheelProps.mFollowIndex ].mTransform;
				wheelState.mTransform.fTranslateGlobal( pDelta );
			}
		}

		f32 wheelFluence = 0.0f;

		if( hit > 0 )
		{
			for (u32 i = 0; i < 4; ++i)
			{
				fBody( ).fAddForce( forces[i], forcePoints[i] );
			}

			wheelFluence = (f32)hit / 4.0f;
		}
		
		//this seems to furk shit up
		//mWheelInfluence.fStep( wheelFluence, dt );
		mWheelInfluence.fSetValue( wheelFluence );

		if( Physics_Vehicle_Enables_DoAngleLock && !fDoFullDynamics( ) )
		{
			//apply torque to keep vehicle upright
			tVec3f error = fBody( ).fTransform( ).fYAxis( ).fCross( tVec3f::cYAxis );
			f32 angle;
			
			error.fNormalizeSafe( angle );
			angle = fAsin( angle );

			f32 start = cPi * Physics_Vehicle_AngleLockStart;
			f32 end = cPi * Physics_Vehicle_AngleLockEnd;
			f32 range = end - start;
			f32 damp = (angle - start) / range;
			if( damp > 0.0f )
			{
				// dampen violation
				tVec3f w = fBody( ).fAngularVelocity( );
				tVec3f deltaW = error * -w.fDot( error ) * damp;
				w += deltaW;

				// add some torque to go back to center
				fBody( ).fAddTorque( error * damp * fBody( ).fMass( ) );
			}
		}
	}

	f32 tVehiclePhysics::fSpeed( ) const
	{
		return mSpeed;
	}
	
	f32 tVehiclePhysics::fMaxSpeed( ) const
	{
		return mProperties.mMaxSpeed * mInput.mMaxSpeedMult;
	}

	f32 tVehiclePhysics::fMaxAIThrottle( ) const
	{
		return mProperties.mMaxAIThrottle;
	}
	
	f32 tVehiclePhysics::fMinSpeed( ) const
	{
		return fMaxSpeed( ) * 0.1f;
	}
	
	f32 tVehiclePhysics::fSteering( ) const
	{
		return mCurrentSteer.fValue( );
	}
	
	f32 tVehiclePhysics::fSteeringInput( ) const
	{
		return mInput.mSteer;
	}

	f32 tVehiclePhysics::fMaxSteeringAngle( ) const
	{
		return mProperties.mSteerLock;
	}

	f32 tVehiclePhysics::fMaxSteeringAngleAtSpeed( f32 speed ) const
	{
		return fMaxSteeringAngle( );
	}

	f32 tVehiclePhysics::fMinTurningRadiusAtSpeed( f32 speed ) const
	{
		if( mProperties.mIsTank )
		{
			f32 turnRate = fMaxSteeringAngleAtSpeed( speed );
			return speed / turnRate;
		}
		else
			return mTotalWheelbase / tan( fMaxSteeringAngleAtSpeed( speed ) );
	}
	
	f32 tVehiclePhysics::fComputeSteerValueForRadius( f32 radius, f32 speed ) const
	{
		log_assert( !fEqual(radius, 0.0f), "Zero radius!" );

		if( mProperties.mIsTank )
		{
			f32 angle = speed / radius;
			return fClamp( angle, -1.0f, 1.0f );
		}
		else
		{
			f32 angle = atan( mTotalWheelbase / radius );
			f32 max = fMaxSteeringAngleAtSpeed( speed );
			return fClamp( angle / max, -1.0f, 1.0f );
		}
	}

	void tVehiclePhysics::fComputeAccelerationAndDeccelerationToReachSpeed( f32 desiredSpeed, f32 currentSpeed, f32 dt, tVehicleInput& inputOut, f32 inputSign ) const
	{
		if( desiredSpeed == 0.0f || (fSign(desiredSpeed) != fSign(currentSpeed) && currentSpeed != 0.0f) )
		{
			inputOut.mDeccelerate = 1.0f;
			if( fAbs(inputSign) > 0.5f && fSign(currentSpeed) != fSign(inputSign) )
				inputOut.mDeccelerate = 2.0f;
		}
		else
		{
			f32 speedDiff = desiredSpeed - currentSpeed;

			if( fAbs(desiredSpeed) > fAbs(currentSpeed) )
			{
				//accelerating
				f32 rate = mProperties.mAcceleration;
				f32 instaRate = speedDiff / dt;
				
				inputOut.mAccelerate = fSign(desiredSpeed);

				if( fAbs(instaRate) < fAbs(rate) )
					inputOut.mAccelerate *= fAbs(instaRate)/rate;
			}
			else
			{ 
				//deccelerating
				f32 rate = mProperties.mDecceleration;
				f32 instaRate = speedDiff / dt;
				
				//this constant is for how hard to apply brakes to slow down
				inputOut.mDeccelerate = 0.5f;

				if( fAbs(instaRate) < rate )
					inputOut.mDeccelerate = fAbs(instaRate)/rate;
			}
		}
	}
	
	f32 tVehiclePhysics::fComputeSteerInputForTankToReachHeading( const Math::tVec3f& worldHeading ) const
	{
		f32 intendedHeading = worldHeading.fXZHeading( );
		f32 currentHeading = fBody( ).fTransform( ).fZAxis( ).fXZHeading( );
		f32 steerInput = fShortestWayAround( currentHeading, intendedHeading );
		steerInput = fClamp( steerInput, -1.f, 1.f );
		return steerInput;
	}

	tVec3f tVehiclePhysics::fGetAxlePivotPoint( ) const
	{
		if( mProperties.mIsTank )
			return tVec3f::cZeroVector;
		else
			return tVec3f(0,0,mProperties.mPivotPt);
	}
	
	f32 tVehiclePhysics::fTimeToReachSpeed( f32 fromSpeed, f32 toSpeed, f32 brakeFactor ) const
	{
		return fAbs(fromSpeed - toSpeed) / (mProperties.mDecceleration * brakeFactor);
	}
	
	f32 tVehiclePhysics::fDistanceToReachSpeed( f32 fromSpeed, f32 toSpeed, f32 brakeFactor ) const
	{
		f32 tts = fTimeToReachSpeed( fromSpeed, toSpeed, brakeFactor );
		return fromSpeed * tts + 0.5f * -mProperties.mDecceleration * brakeFactor * pow(tts, 2.0f);
	}

	f32 tVehiclePhysics::fSteerReverser( ) const
	{
		return ( mInput.mUserControl && Physics_Vehicle_Enables_ReverseReverseSteering && !mProperties.mIsTank
				&& mSpeed < 0.0f ) ? -1.0f : 1.0f;
	}

	f32 tVehiclePhysics::fTotalProbeLength( u32 wheelIndex )
	{
		const f32 extraProbeLength = 2.0f;
		return mProperties.mWheels[wheelIndex].mProbeLength + extraProbeLength;
	}

	void tVehiclePhysics::fDebugDraw( tPhysicsWorld& world )
	{
		if( Physics_Vehicle_DrawDebug )
		{
			Math::tVec4f color( 1.0f, 0.0f, 0.0f, 0.4f );

			for (u32 i = 0; i < mProperties.mWheels.fCount(); ++i)
			{
				Sig::tPair<tVec3f,tVec3f> line(mWheelStates[i].mRay.mOrigin
					, mWheelStates[i].mRay.mOrigin + mWheelStates[i].mRay.mExtent );

				world.fDebugGeometry( ).fRenderOnce( line, color );
				world.fDebugGeometry( ).fRenderOnce( tSpheref(mWheelStates[i].mTransform.fGetTranslation( ), mProperties.mWheels[i].mRadius), color );
			}	

			for (u32 i = 0; i < 4; ++i)
			{
				world.fDebugGeometry( ).fRenderOnce( mCornerRays[i].mOrigin, mCornerRays[i].mOrigin + mCornerRays[i].mExtent, tVec4f( 0,1,0,1 ) );
				world.fDebugGeometry( ).fRenderOnce( mCornerRays[i].fPointAtTime( 0.5f ), mCornerRays[i].fPointAtTime( 0.5f ) + tVec3f(2,0,0), tVec4f( 0,1,0,1 ) );
			}

			//// draw point momentum for physics debugging
			//tVec3f point(0,0,1);
			//point = mTransform.fXformPoint(point);
			//Sig::tPair<tVec3f,tVec3f> line( point
			//	, point + tRigidBody::fPointMomentum(point) );

			//logicP->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( line, color );

			//draw turning pivot point

			//center of mass
			world.fDebugGeometry( ).fRenderOnce( fBody( ).fTransform( ), 1.0f );
			
		}
	}

	void tVehiclePhysics::fSetWorld( tPhysicsWorld* world )
	{
		if( mInWorld )
		{
			tConstraint::fSetWorld( NULL );
			mBody->fRemoveFromWorld( );
		}

		if( world )
		{
			world->fAddObject( mBody.fGetRawPtr( ) );
			tConstraint::fSetWorld( world );
		}
	}

}}
