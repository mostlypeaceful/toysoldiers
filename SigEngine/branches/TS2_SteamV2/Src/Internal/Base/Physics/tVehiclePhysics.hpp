#ifndef __tVehiclePhysics__
#define __tVehiclePhysics__
#include "tCollidingRigidBody.hpp"
#include "tContactPoint.hpp"
#include "Math/tDamped.hpp"
#include "tProximity.hpp"

namespace Sig { namespace Physics
{

	////////////////////// - Data Structures - /////////////////////////////

	struct tWheelProperties
	{
		Math::tMat3f mPosition;
		Math::tVec3f mScale;
		f32 mRadius;
		f32 mSteer;
		f32 mProbeLength;
		u32 mFollowIndex; //the index of the wheel this wheel should be cloned from

		f32 mWeight;

		tWheelProperties(Math::tMat3f pos = Math::tMat3f::cIdentity, f32 radius = 0, f32 steer = 0, f32 probeLength = 0.6f)
			: mPosition(pos), mRadius(radius), mSteer(steer), mProbeLength(probeLength), mFollowIndex( ~0 ), mWeight( 0.f )
		{ }
	};

	struct tWheelState
	{
		Math::tMat3f mTransform;
		Math::tMat3f mFenderTransform;
		Math::tRayf mRay;
		f32 mAngle;
		f32 mAngVel;
		f32 mCompressionVel;
		f32 mCompression;
		f32 mSuspensionAcc; //in g's
		f32 mTemperature; //1.0 when its been sliding for a while, 0.0 for no slide
		Math::tVec3f mSurfaceVelocity; //world velocity of what ever we're resting on
		Math::tVec3f mSurfaceNormal;
		u32 mSurfaceType; //gameflags surface type enum
		f32 mSag; //how much compression will be caused by gravity

		tWheelState( )
			: mAngle(0.0f)
			, mAngVel(0.0f)
			, mCompressionVel(0.0f)
			, mCompression(0.0f)
			, mSuspensionAcc(0.0f)
			, mTemperature(0.0f)
			, mSurfaceVelocity( Math::tVec3f::cZeroVector )
			, mSurfaceNormal( Math::tVec3f::cYAxis )
			, mSurfaceType( ~0 )
			, mSag( 0.f )
		{ }
	};

	class tVehiclePhysicsCallBack
	{
	public:
		virtual ~tVehiclePhysicsCallBack( ) { }

		virtual b32 fShouldSuspendWheels( tLogic* ) { return true; } //return true if this logic object should suspend the vehicle.
	};

	struct tVehicleProperties
	{
		tVehicleProperties( )
			: mCallback( NULL )
		{ }

		tDynamicArray<tWheelProperties> mWheels;
		tEntityTagMask mGroundMask;
		u32 mSurfaceTypeEnumID;

		Math::tVec3f mHalfExtents; //for inertia
		f32 mMass, mInertiaScale;
		f32 mAcceleration;
		f32 mDecceleration;
		f32 mSpeedDamping;
		f32 mSteerSpeedDamping;
		f32 mAngDamping;
		f32 mForceMixing;
		f32 mDampMixingCompress;
		f32 mDampMixingExtend;
		f32 mMaxSpeed;
		f32 mMaxAIThrottle;
		f32 mOrientationLimit;
		f32 mOrientationSpring;
		f32 mSteerLock;
		f32 mSteerLockMaxSpeed; //percentage of available steer at full speed
		f32 mTurnRoll;
		f32 mAccelerationDive;
		b16 mIsTank;
		b16 mHasFenders;
		f32 mImpactImpulse;
		f32 mDamageImpulse;
		f32 mImpulseLimit;
		f32 mGravity;
		f32 mProbeLength;

		// advanced suspension
		f32 mSpringMin;
		f32 mSpringMax;
		f32 mDampMin;
		f32 mDampMax;

		f32 mTrack;
		f32 mFrontWheelbase, mRearWheelbase;
		f32 mPivotPt;
		f32 mTreadScalar; //tread scalar = 1.f / distance of tread on texture. if the texture represents 2m of tread, this value should be 0.5f

		tVehiclePhysicsCallBack* mCallback;
	};

	struct tVehicleInput
	{
		f32 mSteer;
		f32 mAccelerate;
		f32 mDeccelerate;
		f32 mClutch;
		b16 mUserControl; //false for raw control
		b16 mHandBrake;
		f32 mMaxSpeedMult;
		f32 mBoost;
		f32 mBurnout; //set this from 0 to 1 to do burnouts.

		void fZero( )
		{
			mSteer = 0.0f;
			mAccelerate = 0.0f;
			mDeccelerate = 0.0f;
			mClutch = 1.f;
			mUserControl = false;
			mHandBrake = false;
			mMaxSpeedMult = 1.f;
			mBoost = 0.f;
			mBurnout = 0.f;
		}

		b32 fHasAcc( ) const { return !fEqual( mAccelerate, 0.f ); }
		b32 fHasDec( ) const { return !fEqual( mDeccelerate, 0.f ); }
	};

	////////////////////// - Logic Component - /////////////////////////////
	class tVehiclePhysics : public tCollidingRigidBody
	{
		define_dynamic_cast( tVehiclePhysics, tCollidingRigidBody );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tVehiclePhysics( );
		void fOnDelete( );

		void fSetProperties( const tVehicleProperties& vp );
		const tVehicleProperties& fProperties( ) const { return mProperties; }
		tVehicleProperties& fProperties( ) { return mProperties; }
		void fUpdateWeightDistribution( );

		void fSetInput( const tVehicleInput& input );
		const tVehicleInput& fInput( ) const { return mInput; }
		void fReset( const Math::tMat3f& tm );
		void fSetVelocity( const Math::tVec3f& v );

		void fSet3DSimulationMode( b32 enable ) { m3DPhysicsMode = enable; mImmediateResolve = true; }

		void fSetTransform( const Math::tMat3f& tm );
		const tDynamicArray<tWheelState>& fWheelStates( ) const { return mWheelStates; }

		// set this to a positive value when you get hit with an explosion incase you start flipping.
		//  negative value for disable
		void fSetFullDynamicsTimer( f32 time ) { mFullDynamics = time; }
		b32 fDoFullDynamics( ) const { return mFullDynamics > 0.0f; }
		void fSetDoGroundCheck( b32 enable ) { mDoGroundCheck = enable; }

		// collion stuff
		f32 fMaxCollisionResponse( ) const;
		Math::tVec3f fComputeContactResponse( const Physics::tContactPoint& cp, f32 mass );

		// Add is thread safe
		void fAddCollisionResponse( const Math::tVec3f& response, f32 mass );

		f32 fWheelInfluence( ) const { return mWheelInfluence.fValue( ); }
		b32 fOnGround( ) const { return fWheelInfluence( ) > 0.f; }

		const tFixedArray<f32, 2>& fTankTreadPosition( ) const { return mTankTreadPosition; }
		
		// this information is used for ai
		f32 fSpeed( ) const;
		f32 fMaxSpeed( ) const;
		f32 fSpeedPercent( ) const { return fAbs( fSpeed( ) ) / fMaxSpeed( ); }
		f32 fMaxAIThrottle( ) const;
		f32 fMinSpeed( ) const; //any slower would just be silly slow
		f32 fSteering( ) const;
		f32 fSteeringInput( ) const;
		f32 fMaxSteeringAngle( ) const;
		f32 fMaxSteeringAngleAtSpeed( f32 speed ) const;
		f32 fMinTurningRadiusAtSpeed( f32 speed ) const;
		f32 fComputeSteerValueForRadius( f32 radius, f32 speed ) const;
		Math::tVec3f fGetAxlePivotPoint( ) const; // between rear axle on regular car
		f32 fTimeToReachSpeed( f32 fromSpeed, f32 toSpeed, f32 brakeFactor ) const;
		f32 fDistanceToReachSpeed( f32 fromSpeed, f32 toSpeed, f32 brakeFactor ) const;
		Math::tVec3f fGravityVector( ) const { return Math::tVec3f( 0.f, mProperties.mGravity, 0.f ); }
		f32 fGravity( ) const { return mProperties.mGravity; }

		f32 fSteerReverser( ) const;
		f32 fTotalProbeLength( u32 wheelIndex );

		//this will set the accelerate and deccelerate members of the input structure
		void fComputeAccelerationAndDeccelerationToReachSpeed( f32 desiredSpeed, f32 currentSpeed, f32 dt, tVehicleInput& inputOut, f32 inputSign = 0.0f ) const;
		f32 fComputeSteerInputForTankToReachHeading( const Math::tVec3f& worldHeading ) const;

		void fDrawDebugInfo( tLogic* logicP );


		void fPhysicsMT( tLogic* logic, f32 dt );
		void fCoRenderMT( tLogic* logic, f32 dt );
		void fThinkST( tLogic* logic, f32 dt );

		struct tAxle
		{
			Math::tVec3f mPosition;
			f32 mInertia;
			f32 mMaxAngV;
			f32 mAngVel;
			f32 mAngle;
			f32 mRadius;
			f32 mSteerAngle;
			f32 mDriveTorque;
			f32 mBrakeTorque;
			f32 mBrakeBias;
			f32 mHandBrake;
			f32 mBurnout;

			f32 mStaticFrictionCoef;
			f32 mKineticFrictionCoef;
			b32 mStaticFriction;

			Math::tVec3f mLocalForwardDir;
			Math::tVec3f mLocalConstraintDir;
			Math::tVec3f mSurfaceVelocity; //world velocity of what ever we're resting on
			Math::tVec3f mSurfaceNormal;

			// output variables
			Math::tVec2f mFrictionAcc;
			f32 mSuspensionAcc;
			f32 mSlipMagnitude;
			f32 mSlipRatio;
			f32 mTemperature;
			f32 mCompression;

			tAxle( );

			void fSetup( const Math::tVec3f& pos, f32 radius );

			f32 fPatchSpeed( ) const { return mAngVel * mRadius; }
			f32 fBrakeTorque( ) const { return mBrakeTorque * mBrakeBias + mHandBrake; }

			// Drive torque less brake torque
			f32 fNetTorque( ) const;

			// Compute the ideal linear DV if this torque were unconstrained
			f32 fComputeStaticTorqueDV( f32 maxKineticForce, f32 worldlyAcc, f32 forwardVel, f32 dt );

			// Compute the ideal linear DV if this wheel is sliding
			f32 fComputeKineticTorqueDV( f32 maxKineticForce, f32 forwardVel, f32 maxSpeed, f32 dt );
			
			void fSetSteering( f32 angle );

			f32 fMaxFrictionGs( ) const { return mSuspensionAcc * (mStaticFriction ? mStaticFrictionCoef : mKineticFrictionCoef); }
		};

		const tFixedArray<tAxle, 2>& fAxles( ) const { return mAxles; }
		tAxle& fFrontAxle( ) { return mAxles[ 0 ]; }
		tAxle& fBackAxle( ) { return mAxles[ 1 ]; }
		void  fUpdateSteeringAngle( f32 dt );

	protected:

		// constants, parameters
		tVehicleProperties mProperties;
		Math::tVec3f       mGroundConstraintPt; //local pt which will never penetrate ground
		
		// live dynamic data
		Math::tMat3f		mTransformInv;
		tVehicleInput		mInput;
		Math::tDampedFloat	mCurrentSteer;
		tDynamicArray<tWheelState> mWheelStates;
		f32 mFullDynamics;
		b16 mDoGroundCheck;
		b16 m3DPhysicsMode; // Full 3d simulation


		// live kinematic vehicle stuff
		Math::tVec3f mGroundPosition, mGroundVelocity;
		f32 mTotalWheelbase;
		Math::tPDDampedFloat mWheelInfluence; //how much the wheels are on the ground
		f32 mSpeed;
		tFixedArray<f32, 2> mTankTreadPosition;

		void fComputeAndApplyWheelforcesMT( tLogic* logic, f32 dt );
		void fKinematicVehicleMT( tLogic* logic, f32 dt );

		//debugging
		Math::tVec3f mPivotPoint;
		Math::tRayf mCornerRays[4];

		void fConfigureCachedQuery( );
		tMovingProximity mCachedQuery;

	private:
		tFixedArray<tAxle, 2> mAxles;

		void fComplexPhysics( tLogic* logic, f32 dt );
		void fComplexFriction( tLogic* logic, f32 dt );
		void fApplyWheelConstraint( tLogic* logic, tAxle& axle, const Math::tVec3f& worldGravity, f32 dt );

		void fUpdateAxleInfo( );
		void fUpdateWheelInfo( f32 dt );

		void fSimplePhysics( tLogic* logic, f32 dt );
	};

}}

#endif//__tVehiclePhysics__
