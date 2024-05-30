#ifndef __tRtsCamera__
#define __tRtsCamera__
#include "tUseUnitCamera.hpp"
#include "Math/tIntegratedX.hpp"
#include "FX/tFxGraph.hpp"
#include "tShapeEntity.hpp"
#include "tBombDropOverlay.hpp"

namespace Sig
{
	class tPlayer;
	class tRtsCursorLogic;

	///
	/// \brief Default debug/free-look camera.
	class base_export tRtsCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tRtsCamera, tUseUnitCamera );
	public:
		tRtsCamera( tPlayer& player, b32 bombDropper = false, b32 tiltShift = false );
		virtual ~tRtsCamera( );
		virtual void fOnTick( f32 dt );
		virtual void fOnActivate( b32 active );

		void fHitSpeedBump( );
		void fSetNormalSettings( );
		void fSetOrthogonalSettings( );

		void fAcquirePosition( const Math::tMat3f& xform );
		void fAcquireCurrentCameraPos( );

		// Set this to true if you dont want the camera to acquire the current position on blend in.
		// This will be set to false after the camera blends in.
		// You can set it to false prematurely to cancel it.
		void fSetPreventPositionAcquisition( b32 prevent ) { mPreventPositionAcquisition = prevent; }

		void fLockPosition( const Math::tVec3f& pos, const Math::tVec3f& dir );
		void fFindNewBlocker( );
		void fPanToCursor( f32 panTargetTime );
		void fUpdatePanTarget( f32 dt );
		
		
		// used "internally"
		typedef tFixedArray< f32, 6 > tHeightSamples;
		f32 fSampleHeight( const Math::tVec3f& pos, b32 testAgainstAvoidObjs, b32 testAgainstGroundObjs ) const;

		b32 fIsBombDropper( ) const { return mBombDropper; }

		const Math::tVec3f& fCursorPosition( ) const { return mTripod.mLookAt; }
		Math::tVec3f fCursorDir( ) const;

		b32 fTiltShift( ) const { return mTiltShift; }

		Math::tVec3f fConstrainToBlocker( const Math::tVec3f& lookAt );

	private:

		void fEvaluateStickValuesFromGamepad( f32& motionMag, f32& rotateMag, Math::tVec2f& motionStick, Math::tVec2f& rotateStick );
		void fPan( f32 motionMag, f32 rotateMag, const Math::tVec2f& motionStick, const Math::tVec2f& rotateStick, f32 dt, b32 fastCam );
		void fYaw( f32 motionMag, f32 rotateMag, const Math::tVec2f& motionStick, const Math::tVec2f& rotateStick, f32 dt, b32 fastCam );
		void fZoom( f32 motionMag, f32 rotateMag, const Math::tVec2f& motionStick, const Math::tVec2f& rotateStick, f32 dt, b32 fastCam );

		f32 fDistance( ) const { return mDistance.fX( ); }
		f32 fLookAtHeight( ) const { return mLookAtHeight.fX( ); }
		f32 fEyeHeight( ) const { return mEyeHeight.fValue( ); }

		void fFindBlocker( );

		void fCaptureHeightSamples( const Math::tVec3f& pos, tHeightSamples& samples, b32 testAgainstAvoidObjs, b32 testAgainstGroundObjs ) const;
		f32 fFilterHeightSamples( tHeightSamples& samples ) const;
		void fUpdateGround( const Math::tVec3f& eye, const Math::tVec3f& lookat );

	private:

		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );

		tPlayer&					mPlayer;
		tRefCounterPtr<tRtsCursorLogic> mCursorLogic;
		Gfx::tTripod				mTripod;
		f32							mMovementSpeed;
		f32							mGroundAtLookAt;
		f32							mGroundAtEye;
		f32							mDistanceZoomPercent;
		f32							mLookAtHeightPercent;
		f32							mEyeHeightPercentAtCameraChange;
		f32							mEyeHeightPercent;
		f32							mSpeedBumpTimer;
		f32							mBombDropperAngle;

		b8							mOrthoView;
		b8							mPreventPositionAcquisition;
		b8							mBombDropper;
		b8							mTiltShift;

		b8							mLockPositionButAllowZoom;
		b8							pad0;
		b8							pad1;
		b8							pad2;

		tIntegratedV<f32>			mYawVelocity;
		tIntegratedV<Math::tVec3f>	mPanVelocity;
		tIntegratedX<f32>			mDistance;
		tIntegratedX<f32>			mLookAtHeight;
		Math::tDampedFloat			mEyeHeight;

		FX::tGraphPtr				mTargetDistanceGraph;
		FX::tGraphPtr				mTargetLookAtHeightGraph;
		FX::tGraphPtr				mTargetEyeHeightGraph;

		tFixedArray<Math::tPlanef,4>	mBlockerPlanes;
		tShapeEntityPtr					mBlocker;

		Gui::tBombDropOverlayPtr	mBombDropOverlay;

		Math::tVec3f mPanTarget;
		Math::tVec3f mPanFrom;
		f32 mPanTargetTime;
		f32 mPanTargetTimer;
	};
}


#endif//__tRtsCamera__

