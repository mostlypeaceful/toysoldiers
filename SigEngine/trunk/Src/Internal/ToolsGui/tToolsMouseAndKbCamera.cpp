#include "ToolsGuiPch.hpp"
#include "tToolsMouseAndKbCamera.hpp"

namespace Sig
{

	tToolsMouseAndKbCamera::tToolsMouseAndKbCamera( const Gfx::tViewportPtr& viewport, const Input::tMouse& mouse, const Input::tKeyboard& kb )
		: Gfx::tCameraController( viewport )
		, mMouse( mouse )
		, mKb( kb )
		, mProjType( Gfx::tLens::cProjectionPersp )
		, mPanVelocity( 0.f )
		, mYawVelocity( 0.f )
		, mPitchVelocity( 0.f )
		, mZoomVelocity( 0.f )
		, mFrameInProgress( false )
		, mFrameRateMagicNumber( 1.f )
		, mHandlingInput( true )
		, mCanChangePerspective( true )
		, mDisableRotate( false )
		, mDisableOrthoToggle( true )
	{
		mFrameBox = Math::tAabbf( -0.1f, +0.1f );
		mSavedPerspLens = viewport->fRenderCamera( ).fGetLens( );

		mHandAdjusting = false;
	}

	void tToolsMouseAndKbCamera::fFrame( const Math::tAabbf& frameBox )
	{
		mFrameInProgress = true;
		mFrameBox = frameBox;
		mFrameTimer.fResetElapsedS( );
	}

	void tToolsMouseAndKbCamera::fOnTick( f32 dt )
	{
		Gfx::tCamera camera = fViewport( )->fLogicCamera( );
		const f32 elapsed = mTimer.fGetElapsedS( );
		const f32 curFps = 1.f/elapsed;
		const f32 minFps = 1.f/15.f;
		const f32 maxFps = 1.f/500.f;
		mFrameRateMagicNumber = 1.f - fClamp( ( minFps - curFps ) / ( minFps - maxFps ), 0.f, 1.f );

		if( mMouse.fCursorInClientArea( ) &&
			!mDisableOrthoToggle &&
			mKb.fButtonDown( Input::tKeyboard::cButtonSpace ) &&
			mCanChangePerspective )
		{
			fToggleOrtho( camera );
		}

		if( mFrameInProgress )
		{
			if( !mFrameBox.fIsValid( ) )
			{
				const f32 defFrameBoxHalfEdgeLen = 10.f;
				mFrameBox = Math::tAabbf( Math::tVec3f( -defFrameBoxHalfEdgeLen ), Math::tVec3f( +defFrameBoxHalfEdgeLen ) );
			}

			fFrame( camera, mFrameBox );
		}
		else if( !fReset( camera ) )
		{
			fPan( camera );
			fYaw( camera );
			fPitch( camera );
			fZoom( camera );
		}

		b32 triggerSave = false;

		if( mProjType == Gfx::tLens::cProjectionOrtho )
			fUpdateOrthoLens( camera );

		fViewport( )->fSetCameras( camera );
	}

	void tToolsMouseAndKbCamera::fToggleOrtho( Gfx::tCamera& cameraData )
	{
		if( mProjType == Gfx::tLens::cProjectionOrtho )
		{
			fSetOrtho( cameraData, Gfx::tLens::cProjectionPersp );
		}
		else if( mProjType == Gfx::tLens::cProjectionPersp  )
		{
			fSetOrtho( cameraData, Gfx::tLens::cProjectionOrtho );
		}
	}

	void tToolsMouseAndKbCamera::fSetOrtho( Gfx::tCamera& cameraData, u32 projectionSetting )
	{
		// Usually used to transform a camera's data to the corresponding orthogonal version
		// which is the re-applied to the same camera.
		if( projectionSetting == Gfx::tLens::cProjectionOrtho )
		{
			mSavedPerspLens = cameraData.fGetLens( );
			fUpdateOrthoLens( cameraData );
		}
		else if( projectionSetting == Gfx::tLens::cProjectionPersp )
		{
			cameraData.fSetLens( mSavedPerspLens );
		}
		else
			sigassert( !"invalid camera mode" );

		mProjType = cameraData.fGetLens( ).mProjectionType;
	}

	void tToolsMouseAndKbCamera::fUpdateOrthoLens( Gfx::tCamera& camera )
	{
		// convert current perspective settings to ortho settings
		const f32 distToEye = ( camera.fGetTripod( ).mLookAt - camera.fGetTripod( ).mEye ).fLength( );
		Gfx::tLens lens = camera.fGetLens( );
		f32 aspect = lens.fWidth( ) / lens.fHeight( );

		lens.fSetOrtho( lens.mNearPlane, lens.mFarPlane, distToEye, distToEye/aspect );
		camera.fSetLens( lens );
	}

	void tToolsMouseAndKbCamera::fFrame( Gfx::tCamera& camera, const Math::tAabbf& aabb )
	{
		const Gfx::tTripod& tripod = camera.fGetTripod( );
		const Gfx::tTripod  targetTripod = tripod.fComputeFramedTripod( aabb );

		const f32 lerpAmount = fMin( 0.025f * ( mFrameTimer.fGetElapsedMs( ) / 16.666f ), 1.f );
		const Math::tVec3f newLookAt = Math::fLerp( tripod.mLookAt, targetTripod.mLookAt, lerpAmount );
		const Math::tVec3f newEye = Math::fLerp( tripod.mEye, targetTripod.mEye, lerpAmount );

		if( ( newLookAt - tripod.mLookAt ).fLengthSquared( ) < Math::fSquare( 0.001f ) &&
			( newEye - tripod.mEye ).fLengthSquared( ) < Math::fSquare( 0.001f ) )
		{
			mFrameInProgress = false;
		}

		camera.fSetTripod( Gfx::tTripod( newEye, newLookAt, tripod.mUp ) );
	}

	b32 tToolsMouseAndKbCamera::fReset( Gfx::tCamera& camera )
	{
		if( mMouse.fCursorInClientArea( ) &&
			( mKb.fButtonHeld( Input::tKeyboard::cButtonLAlt ) || mKb.fButtonHeld( Input::tKeyboard::cButtonRAlt ) ) && 
			mKb.fButtonDown( Input::tKeyboard::cButtonEnter ) )
		{
			fSetIsHandlingInput( );
			const Math::tVec3f origin = Math::tVec3f::cZeroVector;
			const Math::tVec3f up = Math::tVec3f( 0.f, 1.f, 0.f );
			fSetTripod( camera, origin, up, up );

			return true;
		}

		return false;
	}

	void tToolsMouseAndKbCamera::fSetTripod( Gfx::tCamera& cameraData, const Math::tVec3f& lookAt, const Math::tVec3f& viewAxis, const Math::tVec3f& up )
	{
		const f32 distToEye = ( cameraData.fGetTripod( ).mLookAt - cameraData.fGetTripod( ).mEye ).fLength( );
		cameraData.fSetTripod( Gfx::tTripod( lookAt + viewAxis*distToEye, lookAt, up ) );
	}

	void tToolsMouseAndKbCamera::fPan( Gfx::tCamera& camera )
	{
		// damp velocity
		mPanVelocity *= Math::fLerp( 0.25f, 0.75f, mFrameRateMagicNumber );

		if( mMouse.fCursorInClientArea( ) 
			&& ( mKb.fButtonHeld( Input::tKeyboard::cButtonLAlt ) || mKb.fButtonHeld( Input::tKeyboard::cButtonRAlt ) )
				&& mMouse.fButtonHeld( Input::tMouse::cButtonMiddle ) )
		{
			fSetIsHandlingInput( );

			const f32 dx = mMouse.fGetState( ).mCursorDeltaX;
			const f32 dy = mMouse.fGetState( ).mCursorDeltaY;

			const b32 noNewMath = false;
			if( !noNewMath && mProjType == Gfx::tLens::cProjectionOrtho )
			{
				const Math::tMat4f projToWorld = camera.fGetWorldToProjection( ).fInverse( );

				Math::tVec2i org( mMouse.fGetState( ).mCursorPosX, mMouse.fGetState( ).mCursorPosY );
				if( org.x < 0 ) org.x = 0;
				else if( org.x > (s32)mWindowRes.x ) org.x = mWindowRes.x;
				if( org.y < 0 ) org.y = 0;
				else if( org.y > (s32)mWindowRes.y ) org.y = mWindowRes.y;

				Math::tVec2i dest( mMouse.fGetState( ).mCursorPosX+dx, mMouse.fGetState( ).mCursorPosY+dy );
				if( dest.x < 0 ) dest.x = 0;
				else if( dest.x > (s32)mWindowRes.x ) dest.x = mWindowRes.x;
				if( dest.y < 0 ) dest.y = 0;
				else if( dest.y > (s32)mWindowRes.y ) dest.y = mWindowRes.y;

				Math::tVec3f origin = camera.fUnproject( Math::tVec2u( org.x, org.y), mWindowRes, projToWorld );
				Math::tVec3f end = camera.fUnproject( Math::tVec2u( dest.x, dest.y), mWindowRes, projToWorld );

				Math::tVec3f worldSpaceMovement = origin - end;
				mPanVelocity = worldSpaceMovement;
			}
			else
			{
				Math::tVec3f moveDir = 
					+ dx * camera.fXAxis( )
					+ dy * camera.fYAxis( );


				if( mHandAdjusting )
				{
					// When framing, move vector shouldn't scale based on look at distance.
					mPanVelocity += moveDir / 50.f;
				}
				else
				{
					const f32 toLookAtLen = ( camera.fGetTripod( ).mLookAt - camera.fGetTripod( ).mEye ).fLength( );

					const f32 incrementSpeedUpEveryNMeters = 50.f;
					const f32 panScaleFactor = toLookAtLen / incrementSpeedUpEveryNMeters;

					mPanVelocity += Math::fLerp( 0.1f, 0.01f, mFrameRateMagicNumber ) * panScaleFactor * moveDir;
				}
			}
		}

		camera.fMoveGlobal( mPanVelocity );
	}

	void tToolsMouseAndKbCamera::fYaw( Gfx::tCamera& camera )
	{
		// damp velocity
		mYawVelocity *= Math::fLerp( 0.25f, 0.75f, mFrameRateMagicNumber );

		if( mMouse.fCursorInClientArea( ) 
			&& !mDisableRotate 
			&& ( mKb.fButtonHeld( Input::tKeyboard::cButtonLAlt ) || mKb.fButtonHeld( Input::tKeyboard::cButtonRAlt ) ) 
				&& mMouse.fButtonHeld( Input::tMouse::cButtonLeft ) )
		{
			fSetIsHandlingInput( );

			const f32 dx = mMouse.fGetState( ).mCursorDeltaX;
			const f32 dy = mMouse.fGetState( ).mCursorDeltaY;

			mYawVelocity += -Math::fLerp( 0.05f, 0.01f, mFrameRateMagicNumber ) * ( dx / 8.f );
		}

		Math::tVec3f toEye = camera.fGetTripod( ).mEye - camera.fGetTripod( ).mLookAt;

		// When hand adjusting, the camera should never be 
		if( mHandAdjusting )
			camera.fSetUp( Math::tVec3f::cYAxis );
		
		toEye = Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, mYawVelocity ) ).fRotate( toEye );
		camera.fSetEye( camera.fGetTripod( ).mLookAt + toEye );
	}

	void tToolsMouseAndKbCamera::fPitch( Gfx::tCamera& camera )
	{
		// damp velocity
		mPitchVelocity *= Math::fLerp( 0.25f, 0.75f, mFrameRateMagicNumber );

		if( mMouse.fCursorInClientArea( ) 
			&& !mDisableRotate 
			&& ( mKb.fButtonHeld( Input::tKeyboard::cButtonLAlt ) || mKb.fButtonHeld( Input::tKeyboard::cButtonRAlt ) ) 
				&& mMouse.fButtonHeld( Input::tMouse::cButtonLeft ) )
		{
			fSetIsHandlingInput( );

			const f32 dx = mMouse.fGetState( ).mCursorDeltaX;
			const f32 dy = mMouse.fGetState( ).mCursorDeltaY;

			mPitchVelocity += Math::fLerp( 0.05f, 0.01f, mFrameRateMagicNumber ) * ( dy / 8.f );
		}

		Math::tVec3f toEye = camera.fGetTripod( ).mEye - camera.fGetTripod( ).mLookAt;
		const Math::tVec3f toEyeNorm = Math::tVec3f( toEye ).fNormalizeSafe( );
		if( ( toEyeNorm.y < 0.99f || mPitchVelocity < 0 ) && ( toEyeNorm.y > -0.99f || mPitchVelocity > 0.f ) )
		{
			toEye = Math::tQuatf( Math::tAxisAnglef( camera.fXAxis( ), mPitchVelocity ) ).fRotate( toEye );
			camera.fSetEye( camera.fGetTripod( ).mLookAt + toEye );
		}
	}

	void tToolsMouseAndKbCamera::fZoom( Gfx::tCamera& camera )
	{
		// damp velocity
		mZoomVelocity *= Math::fLerp( 0.25f, 0.85f, mFrameRateMagicNumber );

		Math::tVec3f toLookAt = camera.fGetTripod( ).mLookAt - camera.fGetTripod( ).mEye;

		f32 toLookAtLen = 0.f;
		const Math::tVec3f toLookAtNorm = Math::tVec3f( toLookAt ).fNormalizeSafe( toLookAtLen );
		if( fAbs( toLookAtLen ) < 0.0001f )
			return;

		const f32 frameBoxSize = mFrameBox.fComputeDiagonal( ).fLength( );
		if( fAbs( frameBoxSize ) < 0.0001f )
			return;

		const f32 speedUpAtThisDistance = 15.f;
		const f32 incrementSpeedUpEveryNMeters = 50.f;

		// check for use of alternate zoom mechanic instead (alt + right mouse button held, instead of mouse wheel)
		s16 wheelDelta = mMouse.fGetState( ).mWheelDelta;
		const bool alt = ( mKb.fButtonHeld( Input::tKeyboard::cButtonLAlt ) || mKb.fButtonHeld( Input::tKeyboard::cButtonRAlt ) );
		const bool shift = ( mKb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || mKb.fButtonHeld( Input::tKeyboard::cButtonRShift ) );
		if( alt && mMouse.fButtonHeld( Input::tMouse::cButtonRight ) )
		{
			const s16 dx = mMouse.fGetState( ).mCursorDeltaX;
			const s16 dy = mMouse.fGetState( ).mCursorDeltaY;
			if( fAbs( dx ) > fAbs( dy ) )	wheelDelta = dx;
			else							wheelDelta = dy;
		}

		f32 zoomScaleFactor = 0.15f * fMax( 0.01f, Math::fSqrt(toLookAtLen));
		if( shift )
			zoomScaleFactor *= 0.2f;

		// Clamp the speed a little bit while hand framing.
		if( mHandAdjusting )
			wheelDelta = fClamp<s32>( wheelDelta, -120, 120 );

		if( mMouse.fCursorInClientArea( ) && wheelDelta )
		{
			fSetIsHandlingInput( );
			mZoomVelocity += Math::fLerp( 0.05f, 0.005f, mFrameRateMagicNumber ) * zoomScaleFactor * wheelDelta;
		}

		if( mHandAdjusting )
		{
			// When framing, zoom actually translates the camera forward to dolly forward.
			const Math::tVec3f newEye = camera.fGetTripod( ).mEye + (toLookAtNorm * mZoomVelocity );
			camera.fSetEye( newEye );
			camera.fSetLookAt( newEye + toLookAtNorm );
		}
		else
		{
			toLookAt -= toLookAtNorm * mZoomVelocity;

			const Math::tVec3f newEye = camera.fGetTripod( ).mLookAt - toLookAt;
			const Math::tVec3f newToLookAt = camera.fGetTripod( ).mLookAt - newEye;

			if( newToLookAt.fDot( toLookAtNorm ) > 0.f && newToLookAt.fLengthSquared( ) > Math::fSquare( fMin( 1.f, 0.035f * frameBoxSize ) ) )
				camera.fSetEye( newEye );
		}
	}

}

