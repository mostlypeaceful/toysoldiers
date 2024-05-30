#include "GameAppPch.hpp"
#include "tRtsCamera.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tMeshEntity.hpp"
#include "tShapeEntity.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tPlayer.hpp"
#include "tRtsCursorLogic.hpp"
#include "tVehicleLogic.hpp"
#include "tSync.hpp"
#include "Wwise_IDs.h"

namespace Sig
{
	devvar_clamp( f32, Cameras_RTSCam_LeftStickScale, 1.f, 0.f, 100.f, 2 );
	devvar_clamp( f32, Cameras_RTSCam_RightStickScale, 1.f, 0.f, 100.f, 2 );
//
	devvar_clamp( f32, Cameras_RTSCam_Distance, 100.f, 0.1f, 1000.f, 1 );
// devvar_clamp( f32, Cameras_RTSCam_Distance, 100.f, 0.1f, 1000.f, 1 );
	devvar_clamp( f32, Cameras_RTSCam_EyeHeight, 50.0f, -100.f, 1000.f, 1 );
// devvar_clamp( f32, Cameras_RTSCam_EyeHeight, 50.0f, -100.f, 1000.f, 1 );
	devvar_clamp( f32, Cameras_RTSCam_LookAtHeight, 0.f, -100.f, 1000.f, 1 );
// devvar_clamp( f32, Cameras_RTSCam_LookAtHeight, 0.f, -100.f, 1000.f, 1 );
	devvar_clamp( f32, Cameras_RTSCam_SpeedBumpTime, 0.25f, 0.f, 1.f, 2 );
	devvar_clamp( f32, Cameras_RTSCam_SpeedBumpScale, 0.00f, 0.f, 1.f, 2 );
	devvar_clamp( f32, Cameras_RTSCam_SuperSpeedScale, 3.00f, 1.f, 10.f, 2 );
//
	devvar_clamp( f32, Cameras_RTSCam_TiltShiftSpeedScale, 0.5f, 0.f, 10.f, 2 );
//
	devvar( bool, Cameras_RTSCam_SouthPaw, false );
	devvar( bool, Cameras_RTSCam_UseDistributedFor, false );
//
	devvar( f32, Gameplay_DisplayCase_Zoom, 0.0f );
	devvar_clamp( f32, Gameplay_DisplayCase_ZoomShift, -17.26f, -30.f, 10.f, 2 );
	devvar( f32, Gameplay_DisplayCase_Height, 0.091f );
	devvar( f32, Gameplay_DisplayCase_ExtraHeightLook, 3.f );
//

	namespace
	{
		struct tRtsCameraGroundRayCastCallback
		{
			mutable Math::tRayCastHit	mHit;
			mutable tEntity*			mFirstEntity;
			tEntity*					mIgnoreEntity;

			explicit tRtsCameraGroundRayCastCallback( tEntity* ignore )
				: mFirstEntity( 0 ), mIgnoreEntity( ignore )
			{
			}
			inline void operator( )( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );
				if( !spatial->fHasGameTagsAny( GameFlags::cFLAG_GROUND ) )
					return;
				if( spatial->fHasGameTagsAny( GameFlags::cFLAG_DUMMY ) )
					return;
				if( spatial == mIgnoreEntity || spatial->fIsAncestorOfMine( *mIgnoreEntity ) )
					return;
				tEntity* logicEnt = spatial->fFirstAncestorWithLogic( );
				if( logicEnt )
				{
					if( logicEnt->fLogicDerived<tVehicleLogic>( ) )
						return;
				}
				if( i->fQuickRejectByBox( ray ) )
					return;


				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( hit.fHit( ) && hit.mT < mHit.mT )
				{
					mHit			= hit;
					mFirstEntity	= spatial;
				}
			}
		};

		static const f32 cRayStart = 50.0f;
		static const f32 cRayLength = 1000.0f;
	}


	tRtsCamera::tRtsCamera( tPlayer& player, b32 bombDropper, b32 tiltShift )
		: tUseUnitCamera( player, NULL, false )
		, mPlayer( player )
		, mCursorLogic( 0 )
		, mMovementSpeed( 10.f )
		, mGroundAtLookAt( 0.f )
		, mGroundAtEye( 0.f )
		, mDistanceZoomPercent( 1.0f )
		, mLookAtHeightPercent( 0.f )
		, mEyeHeightPercentAtCameraChange( 0.5f )
		, mEyeHeightPercent( 0.f )
		, mSpeedBumpTimer( 0.f )
		, mBombDropperAngle( Math::cPiOver2 )
		, mOrthoView( false )
		, mPreventPositionAcquisition( false )
		, mBombDropper( bombDropper )
		, mTiltShift( tiltShift )
		, mLockPositionButAllowZoom( false )
		, mPanTargetTimer( 0.0f )
		, mPanTargetTime( 0.0f )
	{
		mCursorLogic.fReset( NEW tRtsCursorLogic( player, *this ) );

		mBlendType = cBlendPureLerp;
		mBlendLerpStart = 0.02f;
		mBlendLerpEnd = 0.2f;
		mHasBlendedInPercentage = 0.5f;
		mStepWhenPaused = tiltShift;

		fAddUser( player.fUser( ) );

		mTargetDistanceGraph.fReset( NEW FX::tFxGraphF32( ) );
		mTargetLookAtHeightGraph.fReset( NEW FX::tFxGraphF32( ) );
		mTargetEyeHeightGraph.fReset( NEW FX::tFxGraphF32( ) );

		mTargetDistanceGraph->fSetKeepLastKeyValue( true );
		mTargetLookAtHeightGraph->fSetKeepLastKeyValue( true );
		mTargetEyeHeightGraph->fSetKeepLastKeyValue( true );

		if( mBombDropper )
			fSetOrthogonalSettings( );
		else
			fSetNormalSettings( );

		mDistance = tIntegratedX< f32 >( mTargetDistanceGraph->fSample< f32 >( 0, mDistanceZoomPercent ) );
		mLookAtHeight = tIntegratedX< f32 >( mTargetLookAtHeightGraph->fSample< f32 >( 0, mLookAtHeightPercent ) );
		mEyeHeight = Math::tDampedFloat( mTargetEyeHeightGraph->fSample< f32 >( 0, mEyeHeightPercent ), 0.15f, 0.25f, 4.f );

		mCursorLogic->fInsertStandAloneToSceneGraph( *tApplication::fInstance( ).fSceneGraph( ) );

		if( !mBombDropper && !mTiltShift )
			player.fSetCursorLogic( mCursorLogic );

		if( mBombDropper )
		{
			mBombDropOverlay.fReset( NEW Gui::tBombDropOverlay( player.fUser( ) ) );
			mBombDropOverlay->fSetAngle( mBombDropperAngle );
		}
	}
	tRtsCamera::~tRtsCamera( )
	{
		mCursorLogic->fRemoveStandAloneFromSceneGraph( );
		mCursorLogic.fRelease( );

		if( !mBombDropper && !mTiltShift )
			mPlayer.fSetCursorLogic( tRtsCursorLogicPtr( ) );

		if( mBombDropOverlay )
		{
			mBombDropOverlay->fCanvas( ).fDeleteSelf( );
			mBombDropOverlay.fRelease( );
		}
	}
	void tRtsCamera::fLockPosition( const Math::tVec3f& pos, const Math::tVec3f& dir )
	{
		f32 originalHeight = mTripod.mEye.y - pos.y;
		f32 originalDist = (mTripod.mEye - mTripod.mLookAt).fProjectToXZ( ).fLength( );

		mTripod.mLookAt = pos;
		mTripod.mEye = mTripod.mLookAt - dir * originalDist;
		mTripod.mEye.y = originalHeight;
		mTripod.mUp = Math::tVec3f::cYAxis;
		mDistanceZoomPercent = Gameplay_DisplayCase_Zoom;
		mEyeHeightPercent = Gameplay_DisplayCase_Height;
		mLockPositionButAllowZoom = true;
	}
	void tRtsCamera::fHitSpeedBump( )
	{
		if( !mTiltShift )
			mSpeedBumpTimer = Cameras_RTSCam_SpeedBumpTime;
	}
	void tRtsCamera::fSetNormalSettings( )
	{
		mEyeHeightPercent = mEyeHeightPercentAtCameraChange;
		mLookAtHeightPercent = 1.0f;
		mOrthoView = false;
		mTargetDistanceGraph->fClear( );
		mTargetLookAtHeightGraph->fClear( );
		mTargetEyeHeightGraph->fClear( );

		mTargetDOF = tGameApp::fInstance( ).fPostEffectsManager( )->fDefaultGameCamData( mPlayer.fUser( )->fViewportIndex( ) ).mDof;
		mTargetZoom = mPlayer.fDefaultZoom( );

		const u32 intervals = 2;
		for( u32 i = 0; i < intervals; ++i )
		{
			f32 x = ( f32 ) i / ( f32 )( intervals - 1 );
			f32 xupside = ( 1.f - ( 1.f-x ) * ( 1.f-x ) );

			f32 value = Math::fLerp( 35.f, ( f32 )Cameras_RTSCam_Distance, xupside );
			mTargetDistanceGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( x, value ) );

			value = Math::fLerp( 5.f, ( f32 )Cameras_RTSCam_LookAtHeight, Math::fSqrt( x ) );
			mTargetLookAtHeightGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( x, value ) );

			value = Math::fLerp( 5.f, ( f32 )Cameras_RTSCam_EyeHeight, xupside );
			mTargetEyeHeightGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( x, value ) );
		}

		mPlayer.fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_CAMERA_MVMT_IN );
	}
	void tRtsCamera::fSetOrthogonalSettings( )
	{
		mEyeHeightPercentAtCameraChange = mEyeHeightPercent;
		mEyeHeightPercent = 1.f;
		mOrthoView = true;
		mTargetDistanceGraph->fClear( );
		mTargetLookAtHeightGraph->fClear( );
		mTargetEyeHeightGraph->fClear( );

		mTargetDistanceGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( 0.f, 35.f ) );
		mTargetDistanceGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( 1.f, 35.f ) );

		mTargetLookAtHeightGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( 0.f, 0.f ) );
		mTargetLookAtHeightGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( 1.f, 0.f ) );

		mTargetEyeHeightGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( 0.f, 70.f ) );
		mTargetEyeHeightGraph->fAddKeyframe( NEW FX::tFxKeyframeF32( 1.f, 325.f ) );

		mPlayer.fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_CAMERA_MVMT_OUT );
	}
	void tRtsCamera::fAcquirePosition( const Math::tMat3f& xform )
	{
		Math::tVec3f currentLook = mTripod.fLookDelta( );
		Math::tVec3f flat = currentLook;
		flat.fProjectToXZ( );
		f32 flatLen = flat.fLength( );

		Math::tVec3f newLook = xform.fZAxis( ) * flatLen;
		newLook.y = currentLook.y;

		mTripod.mLookAt = xform.fGetTranslation( );
		mTripod.mEye = mTripod.mLookAt - newLook;
	}
	void tRtsCamera::fAcquireCurrentCameraPos( )
	{
		mTripod = fViewport( )->fLogicCamera( ).fGetTripod( );
	}
	void tRtsCamera::fOnTick( f32 dt )
	{
		if( !fIsActive( ) || (tGameApp::fInstance( ).fSceneGraph( )->fIsPaused( ) && !mStepWhenPaused) )
			return;

		sigassert( mUsers.fCount( ) == 1 );

		Math::tVec3f oldPos = mTripod.mEye;

		f32 motionMag, rotateMag;
		Math::tVec2f motionStick, rotateStick;
		fEvaluateStickValuesFromGamepad( motionMag, rotateMag, motionStick, rotateStick );

		if( mStepWhenPaused )
			motionMag *= Cameras_RTSCam_TiltShiftSpeedScale;

		fPan( motionMag, rotateMag, motionStick, rotateStick, dt, false );
		fYaw( motionMag, rotateMag, motionStick, rotateStick, dt, false );
		fZoom(motionMag, rotateMag, motionStick, rotateStick, dt, false );

		// Update eye position
		Math::tVec3f lookDir = mTripod.fLookDelta( );
		mTripod.mEye = mTripod.mLookAt - fDistance( ) * lookDir.fProjectToXZAndNormalize( );
		mTripod.mLookAt.y = mLookAtHeight.fX( );
		mTripod.mEye.y = fMax( mTripod.mLookAt.y, mEyeHeight.fValue( ) );

		fFindBlocker( );
		fUpdateGround( mTripod.mEye, mTripod.mLookAt );

		const Math::tVec3f blockerPushVec = fConstrainToBlocker( mTripod.mLookAt );
		mTripod.fTranslate( blockerPushVec );

		if( tGameApp::fInstance( ).fIsDisplayCase( ) )
			mTripod.mLookAt.y += Gameplay_DisplayCase_ExtraHeightLook;

		mSpeedBumpTimer -= dt;


		fUpdatePanTarget( dt );

		// compute resultant velocity and apply to audio
		Math::tVec3f newPos = mTripod.mEye;
		f32 cameraVel = (newPos - oldPos).fLength( ) / dt;
		tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( )->fSetGameParam( AK::GAME_PARAMETERS::SPEED, cameraVel );

		tUseUnitCamera::fOnTick( dt );

		sync_d_event_c( "Eye", mTripod.mEye, tSync::cSCCamera );
		sync_d_event_c( "Up", mTripod.mUp, tSync::cSCCamera );
		sync_d_event_c( "LookAt", mTripod.mLookAt, tSync::cSCCamera );
	}

	void tRtsCamera::fUserBlendIn( f32 dt, Gfx::tTripod& tripod )
	{
		fUserTick( dt, tripod );
	}

	void tRtsCamera::fUserTick( f32 dt, Gfx::tTripod& tripod )
	{
		mTripod.mUp = Math::tVec3f::cYAxis;
		tripod = mTripod;
	}

	void tRtsCamera::fOnActivate( b32 active )
	{
		tUseUnitCamera::fOnActivate( active );

		if( active )
		{
			if( !mPreventPositionAcquisition && !mLockPositionButAllowZoom )
				fAcquireCurrentCameraPos( );
			mPreventPositionAcquisition = false;

			//// When blending out of a path camera, need to fix up the tripod so it doesnt have a weird blend issue
			//Math::tVec3f delta = mTripod.mLookAt - mTripod.mEye;
			//f32 len;
			//delta.fNormalizeSafe( len );

			//delta.fSetLength( mTargetDistanceGraph->fSample< f32 >( 0, mDistanceZoomPercent ) );
			//
			//mTripod.mLookAt = mTripod.mEye + delta;
			//fUpdateGround( mTripod.mEye, mTripod.mLookAt );
			//fOnTick( 0.1f );

			if( mPlayer.fQuickSwitchDisabled( ) && !mPlayer.fSwitchingUseTurret( ) ) // means we just exited a turret
				mPlayer.fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_CAMERA_MVMT_OUT );

			if(mPlayer.fUser( )->fIsLocal( ) )
				mPlayer.fGameController( )->fDisableMouseCursorAutoRestrict( true );
		}
		else
		{
			if( mPlayer.fCurrentUnit( ) && !mPlayer.fSwitchingUseTurret( ) )
				mPlayer.fSoundSource( )->fHandleEvent( AK::EVENTS::PLAY_CAMERA_MVMT_IN );

			if(mPlayer.fUser( )->fIsLocal( ) )
				mPlayer.fGameController( )->fDisableMouseCursorAutoRestrict( false );

		}

		if( mBombDropper && mBombDropOverlay )
			mBombDropOverlay->fShow( active, &mPlayer );

		mCursorLogic->fOnCameraActivate( active );
		Gfx::tCameraController::fOnActivate( active );
	}

	void tRtsCamera::fEvaluateStickValuesFromGamepad( f32& motionMag, f32& rotateMag, Math::tVec2f& motionStick, Math::tVec2f& rotateStick )
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		u32 inputFilter = ( mStepWhenPaused || ( level && level->fLockPlaceMenuUpUntilBuild( ) ) ) ? mPlayer.fUser( )->fInputFilterLevel( ) : 0;
		const tGameControllerPtr gc = mPlayer.fGameController( );

		rotateMag	= gc->fAimStickMagnitude( tUserProfile::cProfileCamera, inputFilter );
		motionMag	= gc->fMoveStickMagnitude( tUserProfile::cProfileCamera, inputFilter );
		rotateStick	= gc->fAimStick( tUserProfile::cProfileCamera, inputFilter );
		motionStick	= gc->fMoveStick( tUserProfile::cProfileCamera, inputFilter );

		const b32 inverted = ( gc->fMode( ) == tGameController::GamePad ) ? mPlayer.fProfile( )->fInversion( tUserProfile::cProfileCamera ) : gc->fMouseInverted( tUserProfile::cProfileCamera );
		if( inverted )
			rotateStick.y *= -1.0f;

		sync_event_c( "(moMag,rotMag)", Math::tVec2f( motionMag, rotateMag ), tSync::cSCCamera );
		sync_event_v_c( motionStick, tSync::cSCCamera );
		sync_event_v_c( rotateStick, tSync::cSCCamera );

		motionMag = std::powf( motionMag, 4.f );

		if( mSpeedBumpTimer > 0.f )
			motionMag *= Cameras_RTSCam_SpeedBumpScale;

		if( gc->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_CAMERA_SPEED, inputFilter ) )
			motionMag *= Cameras_RTSCam_SuperSpeedScale;


		b32 rotateButtonPressed = true;
		b32 isKeyboardMouse =  gc->fMode( ) == tGameController::KeyboardMouse;

		//hold rotate button for keyboard/mouse
		if( isKeyboardMouse )
		{
			rotateButtonPressed = gc->fButtonHeld( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_TRIGGER_SECONDARY);

			//restrict mouse while holding rotate button
			if( rotateButtonPressed )
			{
				gc->fDisableMouseCursorAutoRestrict( false );
			}
			else
			{
				gc->fDisableMouseCursorAutoRestrict( true );
			}
		}


		const b32 blendingInNoMove = !fHasBlendedIn( );
		const b32 motionDisabled = blendingInNoMove || mLockPositionButAllowZoom || ( rotateButtonPressed && isKeyboardMouse );
		const b32 rotateDisabled = blendingInNoMove || mLockPositionButAllowZoom || mBombDropper || !rotateButtonPressed;

		if( mBombDropper && mBombDropOverlay )
		{
			const f32 rotationSpeed = Math::cPi / 32.0f;
			mBombDropperAngle -= rotateStick.x * rotationSpeed;
			mBombDropOverlay->fSetAngle( mBombDropperAngle );
		}

		if( motionDisabled )
		{
			motionMag = 0.f;
			motionStick = Math::tVec2f::cZeroVector;
		}

		if( rotateDisabled )
		{
			rotateMag = 0.f;
			f32 safeZoom = rotateStick.y;
			rotateStick = Math::tVec2f::cZeroVector;

			if( isKeyboardMouse )
			{
				rotateMag = fAbs( safeZoom );
				rotateStick.y = safeZoom;
			}
		}

		const b32 rthumbDown = !mBombDropper && gc->fButtonDown( tUserProfile::cProfileCamera, GameFlags::cGAME_CONTROLS_CAMERA_CYCLE, inputFilter );
		if( rthumbDown && !mOrthoView )
			fSetOrthogonalSettings( );
		else if( rthumbDown && mOrthoView )
			fSetNormalSettings( );
	}

	void tRtsCamera::fPan( f32 motionMag, f32 rotateMag, const Math::tVec2f& motionStick, const Math::tVec2f& rotateStick, f32 dt, b32 fastCam )
	{
		Math::tMat3f basis;
		mTripod.fConstructWorldMatrix( basis );

		if( motionMag > 0.0001f )
		{
			Math::tVec3f moveDir =
				- motionStick.x * basis.fXAxis( ).fProjectToXZAndNormalize( )
				+ motionStick.y * basis.fZAxis( ).fProjectToXZAndNormalize( );
			moveDir.fNormalizeSafe( Math::tVec3f::cZeroVector );

			const f32 baseScale = 2000.f;
			const f32 panScale = Math::fLerp( 0.33f, 1.f, mDistanceZoomPercent );
			mPanVelocity.fAddA( ( panScale * baseScale * motionMag * ( fastCam ? 4.f : 1.f ) ) * moveDir );

			mCursorLogic->fClearPlacementLock( );
		}

		mPanVelocity.fStep( dt, 0.25f );
		mTripod.fTranslate( mPanVelocity.fV( ) * dt );
	}

	void tRtsCamera::fYaw( f32 motionMag, f32 rotateMag, const Math::tVec2f& motionStick, const Math::tVec2f& rotateStick, f32 dt, b32 fastCam )
	{
		Math::tVec3f camZproj;

		camZproj = mTripod.fLookDelta( ).fProjectToXZAndNormalize( );
		const f32 camZprojX = camZproj.x;

		const f32 baseScale = -30.f;
		const f32 yawScale = Math::fLerp( 0.666f, 1.f, mDistanceZoomPercent );
		mYawVelocity.fAddA( yawScale * baseScale * ( fastCam ? 2.f : 1.f ) * rotateStick.x );
		mYawVelocity.fStep( dt, 0.15f );

		Math::tVec3f toEye = mTripod.mEye - mTripod.mLookAt;
		toEye = Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, mYawVelocity.fV( ) * dt ) ).fRotate( toEye );
		mTripod.mEye = mTripod.mLookAt + toEye;
	}

	void tRtsCamera::fZoom( f32 motionMag, f32 rotateMag, const Math::tVec2f& motionStick, const Math::tVec2f& rotateStick, f32 dt, b32 fastCam )
	{
		const f32 deadZone = 0.0001f;
		if( rotateMag > deadZone || mLockPositionButAllowZoom )
		{
			f32 zoomSpeed = ( fastCam ? 2.f : 1.f );

			if( !mOrthoView )
			{
				mDistanceZoomPercent -= rotateStick.y * dt * zoomSpeed;
				mDistanceZoomPercent = fClamp( mDistanceZoomPercent, 0.f, 1.f );

				mLookAtHeightPercent -= rotateStick.y * dt * zoomSpeed;
				mLookAtHeightPercent = fClamp( mLookAtHeightPercent, 0.f, 1.f );
			}

			mEyeHeightPercent -= rotateStick.y * dt * zoomSpeed;
			mEyeHeightPercent = fClamp( mEyeHeightPercent, 0.f, 1.f );
		}

		const f32 baseLH = ( mGroundAtLookAt == -Math::cInfinity ) ? 0.f : mGroundAtLookAt;
		const f32 baseEH = ( mGroundAtEye == -Math::cInfinity ) ? 0.f : mGroundAtEye;

		const f32 currentD = fDistance( );
		const f32 currentLH = fLookAtHeight( );
		const f32 currentEH = fEyeHeight( );
		const f32 targetLH = baseLH + mTargetLookAtHeightGraph->fSample< f32 >( 0, mLookAtHeightPercent );
		const f32 targetEH = baseEH + mTargetEyeHeightGraph->fSample< f32 >( 0, mEyeHeightPercent );

		f32 targetD = mTargetDistanceGraph->fSample< f32 >( 0, mDistanceZoomPercent );
		if( tGameApp::fInstance( ).fIsDisplayCase( ) )
			targetD += Gameplay_DisplayCase_ZoomShift;

		mDistance.fAddV( mMovementSpeed * ( targetD - currentD ) );
		mLookAtHeight.fAddV( mMovementSpeed * ( targetLH - currentLH ) );

		mDistance.fStep( dt );
		mLookAtHeight.fStep( dt );
		mEyeHeight.fStep( targetEH, dt );
	}
	f32 tRtsCamera::fSampleHeight( const Math::tVec3f& pos, b32 testAgainstAvoidObjs, b32 testAgainstGroundObjs ) const
	{
		const Math::tRayf ray = Math::tRayf( pos + cRayStart * Math::tVec3f::cYAxis, -cRayLength * Math::tVec3f::cYAxis );
		Math::tRayCastHit bestHit;

		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		if( levelLogic )
		{
			if( testAgainstAvoidObjs )
			{
				const u32 count = levelLogic->fCameraBlockerCount( );
				for( u32 i = 0; i < count; ++i )
				{
					tShapeEntity* ent = levelLogic->fCameraBlocker( i );
					Math::tRayCastHit hit;
					ent->fRayCast( ray, hit );
					if( hit.mT < bestHit.mT )
						bestHit = hit;
				}
			}
			if( testAgainstGroundObjs )
			{
				const u32 count = levelLogic->fCameraGroundCount( );
				for( u32 i = 0; i < count; ++i )
				{
					tShapeEntity* ent = levelLogic->fCameraGround( i );
					Math::tRayCastHit hit;
					ent->fRayCast( ray, hit );
					if( hit.mT < bestHit.mT )
						bestHit = hit;
				}
			}
		}

		tEntity* ignore = 0;
		tRtsCameraGroundRayCastCallback rayCastCallback( ignore );

		//using a pre-cached proximity query for this raycast seemed to show worse performance
		tGameApp::fInstance( ).fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastCallback );

		if( rayCastCallback.mHit.mT < bestHit.mT )
			bestHit = rayCastCallback.mHit;

		if( !bestHit.fHit( ) )
			return 0.f;
		return ray.fEvaluate( bestHit.mT ).y;
	}



	class tTerrainSampler
	{
	public:
		tTerrainSampler( )
			: mCamera( NULL )
			, mProx( NULL )
			, mSamples( NULL )
		{ }

		b32 fDoIt( u32 index )
		{
			(*mSamples)[ index ] = mCamera->fSampleHeight( mSamplePoints[ index ], mTestAvoidObjs, mTestGroundObjs );

			return true;
		}

		void fSetup( const tRtsCamera& camera
			, tRtsCamera::tHeightSamples& samples
			, const tGrowableArray< Math::tVec3f >& samplePoints
			//, const tProximity& prox
			, b32 testAvoidObjs
			, b32 testGroundObjs )
		{
			mCamera = &camera;
			mSamples = &samples;
			//mProx = &prox;

			mTestAvoidObjs = testAvoidObjs;
			mTestGroundObjs = testGroundObjs;

			mSamplePoints = samplePoints;
		}

		Threads::tDistributedForLoopCallback fMakeCallback( )
		{
			return make_delegate_memfn(  Threads::tDistributedForLoopCallback, tTerrainSampler, fDoIt );
		}

	private:
		const tRtsCamera* mCamera;
		const tProximity* mProx;
		b16 mTestAvoidObjs;
		b16 mTestGroundObjs;

		tGrowableArray<Math::tVec3f> mSamplePoints;
		tRtsCamera::tHeightSamples* mSamples;
	};

	void tRtsCamera::fCaptureHeightSamples( const Math::tVec3f& pos, tHeightSamples& samples, b32 testAgainstAvoidObjs, b32 testAgainstGroundObjs ) const
	{
		const f32 sampleDist = 1.f;

		if( Cameras_RTSCam_UseDistributedFor )
		{
			tGrowableArray< Math::tVec3f > points;
			points.fSetCount( 5 );

			points[ 0 ] = pos;
			points[ 1 ] = pos + Math::tVec3f::cXAxis * sampleDist;
			points[ 2 ] = pos - Math::tVec3f::cXAxis * sampleDist;
			points[ 3 ] = pos + Math::tVec3f::cZAxis * sampleDist;
			points[ 4 ] = pos - Math::tVec3f::cZAxis * sampleDist;

			tTerrainSampler sampler;
			sampler.fSetup( *this, samples, points, testAgainstAvoidObjs, testAgainstGroundObjs );

			Threads::tDistributedForLoopCallback callback = sampler.fMakeCallback( );
			tGameApp::fInstance( ).fCurrentLevel( )->fRootEntity( )
				->fSceneGraph( )->fDistributeForLoop( callback, 5 );
		}
		else
		{
			samples[ 0 ] = fSampleHeight( pos, testAgainstAvoidObjs, testAgainstGroundObjs );
			samples[ 1 ] = fSampleHeight( pos + Math::tVec3f::cXAxis * sampleDist, testAgainstAvoidObjs, testAgainstGroundObjs );
			samples[ 2 ] = fSampleHeight( pos - Math::tVec3f::cXAxis * sampleDist, testAgainstAvoidObjs, testAgainstGroundObjs);
			samples[ 3 ] = fSampleHeight( pos + Math::tVec3f::cZAxis * sampleDist, testAgainstAvoidObjs, testAgainstGroundObjs );
			samples[ 4 ] = fSampleHeight( pos - Math::tVec3f::cZAxis * sampleDist, testAgainstAvoidObjs, testAgainstGroundObjs );
		}
	}

	f32 tRtsCamera::fFilterHeightSamples( tHeightSamples& h ) const
	{
		h.fBack( ) = h.fFront( ); // count the primary position as two hits

		f32 hsum = 0.f;
		u32 numHits = 0;
		for( u32 i = 0; i < h.fCount( ); ++i )
		{
			if( h[ i ] != -Math::cInfinity )
			{
				hsum += h[ i ];
				++numHits;
			}
		}

		if( numHits == 0 )
			return -Math::cInfinity;
		return hsum / numHits;
	}

	void tRtsCamera::fUpdateGround( const Math::tVec3f& eye, const Math::tVec3f& lookat )
	{
		profile( cProfilePerfRTSCam );

		tHeightSamples hLookAt, hEye;
		fCaptureHeightSamples( lookat, hLookAt, false, true );
		fCaptureHeightSamples( eye, hEye, true, true );
		mGroundAtLookAt = fFilterHeightSamples( hLookAt );
		mGroundAtEye = fFilterHeightSamples( hEye );
	}

	void tRtsCamera::fFindNewBlocker( )
	{
		mBlocker.fRelease( );
		fFindBlocker( );
	}

	Math::tVec3f tRtsCamera::fCursorDir( ) const
	{
		Math::tVec3f dir = mTripod.mLookAt - mTripod.mEye;
		dir.fProjectToXZAndNormalize( );

		if( mBombDropper )
			dir = Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, mBombDropperAngle ) ).fRotate( dir );

		return dir;
	}

	void tRtsCamera::fFindBlocker( )
	{
		tLevelLogic* levelLogic = tGameApp::fInstance( ).fCurrentLevel( );
		if( !levelLogic )
			return;

		tShapeEntity* cameraBox = NULL;

		if( mBombDropper )
			cameraBox = levelLogic->fBombDropperBox( mPlayer.fTeam( ) );
		else
			cameraBox = levelLogic->fCameraBox( mPlayer.fTeam( ) );

		if( cameraBox  )
		{
			if( mBlocker != cameraBox )
			{
				mBlocker.fReset( cameraBox );

				const Math::tObbf box = cameraBox->fBox( );
				mBlockerPlanes[ 0 ] = Math::tPlanef( -box.fAxis( 0 ), box.fCenter( ) + box.fAxis( 0 ) * box.fExtents( ).x );
				mBlockerPlanes[ 1 ] = Math::tPlanef(  box.fAxis( 0 ), box.fCenter( ) - box.fAxis( 0 ) * box.fExtents( ).x );
				mBlockerPlanes[ 2 ] = Math::tPlanef( -box.fAxis( 2 ), box.fCenter( ) + box.fAxis( 2 ) * box.fExtents( ).z );
				mBlockerPlanes[ 3 ] = Math::tPlanef(  box.fAxis( 2 ), box.fCenter( ) - box.fAxis( 2 ) * box.fExtents( ).z );
			}
		}
		else
			mBlocker.fRelease( );
	}

	Math::tVec3f tRtsCamera::fConstrainToBlocker( const Math::tVec3f& lookAt )
	{
		if( mBlocker.fNull( ) )
			return Math::tVec3f::cZeroVector; // no blocker

		Math::tVec3f pushVector = Math::tVec3f::cZeroVector;
		for( u32 i = 0; i < mBlockerPlanes.fCount( ); ++i )
		{
			const f32 sd = mBlockerPlanes[ i ].fSignedDistance( lookAt );
			if( sd < 0.f )
				pushVector += mBlockerPlanes[ i ].fGetNormal( ) * -sd;
		}
		return pushVector;
	}


	void tRtsCamera::fPanToCursor( f32 panTargetTime )
	{
		mPanFrom = mTripod.mLookAt;
		mPanTarget = mCursorLogic->fCurrentPosition();
		mPanTargetTimer = mPanTargetTime = panTargetTime;

	}

	void tRtsCamera::fUpdatePanTarget( f32 dt )
	{
		if( mPanTargetTimer <= 0.0f )
			return;

		mPanTargetTimer = fMax( 0.0f, mPanTargetTimer - dt );

		f32 panScale = 1.0f - ( mPanTargetTimer / mPanTargetTime );

		Math::tVec3f lastPos =	mTripod.mLookAt;
		Math::tVec3f panPos = mPanFrom + ((mPanTarget - mPanFrom) * panScale);

		mTripod.mLookAt = panPos;
		mTripod.mEye += (panPos - lastPos);


	}


}
