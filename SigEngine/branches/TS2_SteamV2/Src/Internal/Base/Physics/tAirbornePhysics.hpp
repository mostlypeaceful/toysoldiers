#ifndef __tAirbornePhysics__
#define __tAirbornePhysics__

#include "tRigidBody.hpp"
#include "Math/tDamped.hpp"

namespace Sig { namespace Physics
{

	////////////////////// - Data Structures - /////////////////////////////

	struct tAirborneProperties
	{
		tEntityTagMask mGroundMask;

		f32 mMaxRoll;
		f32 mMaxPitch;
		f32 mRollP;
		f32 mRollD;
		f32 mElevationLerp;

		f32 mElevationRate;
		f32 mElevationDamping;
		f32 mYawRate;
		f32 mYawDamping;

		f32 mCruiseSpeed;
		f32 mMaxSpeed;
		f32 mSlowSpeed;
		f32 mAccelerationLerp;

		f32 mZoomMax;
		f32 mZoomSlow;

		f32 mAIYawDamping;
		f32 mAIThrottle;

		tGrowableArray<Math::tVec3f> mCollisionProbes;
	};

	struct tAirborneInput
	{
		Math::tVec2f	mStick; // {roll, elevation}
		f32				mThrottle;
		b32				mActive;
		f32				mBoost;

		void fZero( )
		{
			mStick = Math::tVec2f::cZeroVector;
			mThrottle = 0.0f;
			mActive = false;
			mBoost = 0.f;
		}
	};

	////////////////////// - Logic Component - /////////////////////////////
	class tAirbornePhysics : public tRigidBody
	{
		define_dynamic_cast( tVehiclePhysics, tRigidBody );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tAirbornePhysics( );

		void fSetProperties( const tAirborneProperties& vp );
		const tAirborneProperties& fProperties( ) const { return mProperties; }
		void fSetCollisionMask( u32 mask ) { mProperties.mGroundMask = mask; }

		void fSetInput( const tAirborneInput& input );
		const tAirborneInput& fInput( ) const { return mInput; }

		void fDitch( bool enable ); //crashes plane into ground
		b32 fDitch( ) { return mDitch; }

		void fSetTransform( const Math::tMat3f& tm );
		void fReset( const Math::tMat3f& tm );

		const Math::tMat3f& fGetTransform( ) const { return mTransform; }
		const Math::tVec3f& fVelocity( ) const { return mV; }

		void fDrawDebugInfo( tLogic* logicP );

		f32 fIdealZoom( ) const { return mIdealZoom; }
		f32 fSpeed( ) const { return mSpeed; }
		f32 fSpeedBlend( ) const { return fSpeedBlendImp( mSpeed ); }
		f32 fSpeedBlendImp( f32 speed ) const { return (speed - mProperties.mSlowSpeed)/(mProperties.mMaxSpeed - mProperties.mSlowSpeed); }
		f32 fSignedSpeedBlend( ) const;
		f32 fRollBlend( ) const { return fClamp( mInput.mStick.x, -1.f, 1.f ); }
		f32 fCurrentYawRate( ) const;
		f32 fCurrentElevRate( ) const;
		f32 fLoad( ) const { return mLoad; }
		Math::tVec2f fRollPos( ) const { return Math::tVec2f( mRoll.fValue( ) / mProperties.mMaxRoll, mElevation / mProperties.mMaxPitch ); }

		void fPhysicsMT( tLogic* logic, f32 dt );
		void fThinkST( tLogic* logic, f32 dt );

		b32 fCollided( ) { b32 result = mCollision; mCollision = false; return result; }

		Math::tVec2f fComputeStickToReachHeading( const Math::tVec3f& heading, b32& passedTarget ) const;

	private:
		// constants, parameters
		tAirborneProperties mProperties;
		s32					mDitch; //sign indicates direction to ditch
		b32					mCollision; //true after we hit something
		
		// live dynamic data
		tAirborneInput     mInput;

		Math::tPDDampedFloat mRoll, mYawInput;
		Math::tDampedFloat mElevationInput;

		f32 mSpeed, mHeading, mElevation;
		f32 mIdealZoom;
		f32 mLoad;
	};

}}

#endif//__tAirbornePhysics__
