#ifndef __tToolsMouseAndKbCamera__
#define __tToolsMouseAndKbCamera__
#include "Gfx/tCameraController.hpp"
#include "Input/tMouse.hpp"
#include "Input/tKeyboard.hpp"

namespace Sig
{
	///
	/// \brief RTS-style camera, primary camera for the game.
	class toolsgui_export tToolsMouseAndKbCamera : public Gfx::tCameraController
	{
		implement_rtti_base_class( tToolsMouseAndKbCamera );
	private:
		const Input::tMouse& mMouse;
		const Input::tKeyboard& mKb;
		Gfx::tLens::tProjectionType mProjType;
		Gfx::tLens mSavedPerspLens;
		Math::tVec3f mPanVelocity;
		f32 mYawVelocity;
		f32 mPitchVelocity;
		f32 mZoomVelocity;
		Time::tStopWatch mFrameTimer;
		b32 mFrameInProgress;
		Math::tAabbf mFrameBox;
		Time::tStopWatch mTimer;
		f32 mFrameRateMagicNumber;
		b32	mHandlingInput;
		b32 mCanChangePerspective;
		b32 mDisableRotate;
		b32 mDisableOrthoToggle;
		Math::tVec2u mWindowRes;
	public:
		tToolsMouseAndKbCamera( const Gfx::tViewportPtr& viewport, const Input::tMouse& mouse, const Input::tKeyboard& kb );
		void fSetIsHandlingInput( b32 isHandling = true ) { mHandlingInput = isHandling; }
		b32 fGetIsHandlingInput( ) const { return mHandlingInput; }
		void fDisablePerspectiveChanges( ) { mCanChangePerspective = false; }
		void fFrame( const Math::tAabbf& frameBox );
		virtual void fOnTick( f32 dt );
		void fToggleOrtho( Gfx::tCamera& cameraData );
		void fSetOrtho( Gfx::tCamera& cameraData, Gfx::tLens::tProjectionType newSetting );
		void fSetTripod( Gfx::tCamera& cameraData, const Math::tVec3f& lookAt, const Math::tVec3f& viewAxis, const Math::tVec3f& up );
		void fDisableRotation( b32 disable = true ) { mDisableRotate = disable; }
		void fDisableOrthoToggle( b32 disable = true ) { mDisableOrthoToggle = disable; }
		void fSetWindowRes( const Math::tVec2u& windowRes ) { mWindowRes = windowRes; }
	private:
		void fFrame( Gfx::tCamera& camera, const Math::tAabbf& aabb );
		b32 fReset( Gfx::tCamera& camera );
		void fPan( Gfx::tCamera& camera );
		void fYaw( Gfx::tCamera& camera );
		void fPitch( Gfx::tCamera& camera );
		void fZoom( Gfx::tCamera& camera );
	};

}



#endif//__tToolsMouseAndKbCamera__
