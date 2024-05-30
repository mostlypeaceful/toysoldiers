#ifndef __tHoverPhysics__
#define __tHoverPhysics__

#include "tRigidBody.hpp"
#include "tContactPoint.hpp"
#include "Math/tDamped.hpp"

namespace Sig { namespace Physics
{

	////////////////////// - Data Structures - /////////////////////////////

	struct tHoverProperties
	{
		tEntityTagMask mGroundMask;

		f32 mMaxRoll;
		f32 mYawAcc;
		f32 mYawDamping;
		f32 mYawAccAI;
		f32 mYawDampingAI;

		f32 mMaxSpeed;
		f32 mAIMaxThrottle; //percentage of max speed
		f32 mSpeedAcc;
		f32 mSpeedDamping;

		f32 mRollP;
		f32 mRollD;
		f32 mYawRoll;
		f32 mPreYaw;
		f32 mMinStrafeInfluence; ///[0,1] increases strafe responsiveness

		f32 mMinGroundHeight; //In user control
		f32 mLandingHeight;
		f32 mElevationVelMax;
		f32 mElevationVelAcc; 
		f32 mElevationVelDamping; 
	};

	struct tHoverInput
	{
		f32				mHeightAdjustment; //[-1 full down, 1 full up]
		f32				mIntendedHeading;
		Math::tVec2f	mWorldStrafe; // {x, z}
		b8				mActive;
		b8				mStartingUp;
		b8				mStayAboveGround;
		b8				mUserControl;

		void fZero( )
		{
			mHeightAdjustment = 0.f;
			mIntendedHeading = 0.f;
			mWorldStrafe = Math::tVec2f::cZeroVector;
			mActive = false;
			mStartingUp = false;
			mStayAboveGround = true;
			mUserControl = false;
		}
	};

	////////////////////// - Logic Component - /////////////////////////////
	class tHoverPhysics : public tStandardPhysics
	{
		define_dynamic_cast( tHoverPhysics, tStandardPhysics );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tHoverPhysics( );

		void fSetProperties( const tHoverProperties& vp );
		const tHoverProperties& fProperties( ) const { return mProperties; }

		void fSetInput( const tHoverInput& input );
		void fDitch( bool enable ); //crashes plane into ground

		void fSetTransform( const Math::tMat3f& tm );
		void fReset( const Math::tMat3f& tm );

		void fDrawDebugInfo( tLogic* logicP );

		f32 fGetSpeed( ) const { return mSpeed; }
		f32 fGetMaxSpeed( ) const { return mProperties.mMaxSpeed; }
		f32 fGetMaxAIThrottle( ) const { return mProperties.mAIMaxThrottle; }
		f32 fCurrentHoverHeight( ) const { return mCurrentHoverHeight; }
		f32 fComputeHeightAdjustment( f32 newHeight ) const;
		f32 fLoad( ) const { return mLoad; }
		const Math::tVec3f& fDeltaV( ) const { return mLoadVec; }
		const Math::tVec2f& fRollPos( ) const { return mRollPos.fValue( ); }

		void fPhysicsMT( tLogic* logic, f32 dt );
		void fThinkST( tLogic* logic, f32 dt );

		// collion stuff
		void fSetCollisionShape( const Math::tSpheref& collisionShape ) { mCollisionShape = collisionShape; }
		void fClearContacts( );
		void fAddContactPt( f32 dt, const Physics::tContactPoint& cp );
		Math::tVec3f fComputeContactResponse( const Physics::tContactPoint& cp, f32 mass );
		void fAddCollisionResponse( const Math::tVec3f& response, f32 mass );

	private:
		// constants, parameters
		tHoverProperties	mProperties;
		s8					mDitch; //sign indicates direction to ditch
		b8					pad0;
		b8					pad1;
		b8					pad2;
		
		// live dynamic data
		Math::tMat3f	mFlatTransform;
		tHoverInput		mInput;
		f32				mHeading;
		f32				mSpeed;
		f32				mGroundOffset; //how high above the terrain we should be
		f32				mCurrentHoverHeight; //how high we really are above the terrain
		f32				mLoad; //0 for no acceleration, 1 for max acceleration
		Math::tVec3f	mLoadVec;

		// Input buffers
		f32				mYawRate;
		Math::tVec2f	mStrafeRate;
		Math::tVec3f	mOldV;
		//Math::tDampedFloat mStrafeInput; //1.f if input present, 0.f if not
		Math::tPDDampedVec2f mRollPos;
		Math::tDampedFloat mStartupBlendOut;

		// collision handling
		Math::tSpheref mCollisionShape;
		tGrowableArray<tContactPoint> mContactPoints, mPrevCPs;
		void fResolveCollisionMT( tLogic* logic, f32 dt );

		f32 fYawAcc( ) const;
		f32 fYawDamp( ) const;
	};

}}

#endif//__tHoverPhysics__
