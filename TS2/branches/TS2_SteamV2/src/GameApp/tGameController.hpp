#pragma once
#ifndef __tGameController__
#define __tGameController__

#include "Input/tGamepad.hpp"
#include "Input/tKeyboard.hpp"
#include "Input/tMouse.hpp"
#include "tUser.hpp"
#include "tUserProfile.hpp"

namespace Sig 
{
	class tXmlFile;

	class tGameController : public tRefCounter
	{
	public:
		static void fExportScriptInterface( tScriptVm& vm );

		tGameController( void );
		tGameController( const tUserPtr& user, const tUserProfilePtr& profile );
		~tGameController( void );

		enum Mode
		{
			GamePad,
			KeyboardMouse,

			ModeCount
		};

		void fOnTick( f32 dt, u32 inputFilter = 0 );

		// Check if the game controller has any input this frame.
		b32 fIsActive( ) const;

		//void fSetUser( const tUserPtr& user ) { mUser = user; }
		//void fSetProfile( const tUserProfilePtr& profile ) { mProfile = profile; }

        b32 fLoadKeyBindingsFromFile( tXmlFile* xmlFile );
		b32 fSaveKeyBindingsLocalAppData( );
		
		template< class tArchive >
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mBindings );
			archive.fSaveLoad( mMouseInverted );
			archive.fSaveLoad( mMouseSensitivity );
		}

        b32 fButtonHeld( u32 controlProfile, u32 gameControlsFlag ) const;
        b32 fButtonHeld( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const;
        b32 fButtonDown( u32 controlProfile, u32 gameControlsFlag ) const;
        b32 fButtonDown( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const;
        b32 fButtonUp( u32 controlProfile, u32 gameControlsFlag ) const;
        b32 fButtonUp( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter ) const;

		Math::tVec2f fAimStick( u32 controlProfile ) const;
		Math::tVec2f fAimStick( u32 controlProfile, u32 inputFilter ) const;
		f32 fAimStickMagnitude( u32 controlProfile, u32 inputFilter = 0 ) const;

		Math::tVec2f fMoveStick( u32 controlProfile ) const;
		Math::tVec2f fMoveStick( u32 controlProfile, u32 inputFilter ) const;
		f32 fMoveStickMagnitude( u32 controlProfile, u32 inputFilter = 0 ) const;

		f32 fGetAcceleration( u32 controlProfile, u32 inputFilter = 0 ) const;
		f32 fGetDeceleration( u32 controlProfile, u32 inputFilter = 0 ) const;

		Math::tVec2f fMenuStick( ) const;
		Math::tVec2f fMenuStick( u32 inputFilter ) const;
		Math::tVec2f fMousePosForRadialMenu( u32 controlProfile, u32 inputFilter = 0 ) const;

		void fClearBindings( );
		void fSetGamepadBinding( u32 controlProfile, u32 gameControlsFlag, Input::tGamepad::tButton button );
		void fSetKeyboardBinding( u32 controlProfile, u32 gameControlsFlag, Input::tKeyboard::tButton button );
		void fSetMouseBinding( u32 controlProfile, u32 gameControlsFlag, Input::tMouse::tButton button );

		Input::tGamepad::tButton fGetGamepadBinding( u32 controlProfile, u32 gameControlsFlag ) const;
		Input::tKeyboard::tButton fGetKeyboardBinding( u32 controlProfile, u32 gameControlsFlag ) const;
		Input::tMouse::tButton fGetMouseBinding( u32 controlProfile, u32 gameControlsFlag ) const;

		b32 fGamepadConnected() const;

		void fDisableMouseCursorAutoRestrict( b32 restrictDisabled );
		void fPushMouseCursorRestrictFilterLevel( u32 filterLevel );
		void fPopMouseCursorRestrictFilterLevel( u32 filterLevel );
		u32  fMouseCursorRestrictFilterLevel( );

		Math::tVec2f fUIMousePos( u32 inputFilter = 0 ) const;
		b32 fUIMouseClicked( u32 button, u32 inputFilter = 0 ) const;
		Math::tVec2f fUIMouseDelta( u32 inputFilter = 0 ) const;
		b32 fUIMouseHeld( u32 button, u32 inputFilter = 0 ) const;
		s32 fUIMouseWheelDelta( u32 inputFilter = 0 ) const;
		void fUISetMouseCursorPos( const Math::tVec2f &uiCoordsCursorPos );

		const tLocalizedString& fGetGameControlsLocString( u32 controlProfile, u32 gameControl ) const;
		const tLocalizedString& fGetGameControlsKeyboardBinding( u32 controlProfile, u32 gameControl );
		const tLocalizedString& fKeyboardtButtonToAscii( Input::tKeyboard::tButton button );
		const tLocalizedString& fGetGameControlsMouseBinding( u32 controlProfile, u32 gameControl );

		b32 fCaptureNextKeyPress( );
		b32 fCancelCaptureNextKeyPress( );
		u32 fGetKeyPressCaptured( );
		b32 fLoadDefaultGameControlsBindings( );
		b32 fRevertGameControlsBindingsChanges( );

		u32 fMode() {  return mMode; };
		void fSetMode( u32 mode );
		b32 fCanUseGamePad( ) const;
		b32 fCanUseKeyboardMouse( ) const;
		void fSetMouseCursorPos( const Math::tVec2f &cursorPos );

		f32 fMouseSensitivity( u32 controlProfile ) const;
		void fSetMouseSensitivity( u32 controlProfile, f32 sensitivity );

		b32 fMouseInverted( u32 controlProfile ) const;
		void fSetMouseInverted( u32 controlProfile, b32 inverted );

		u32 fCurrentBindingsVersion( ) { return mCurrentBindingsVersion; }
		void fSetCurrentBindingsVersion( u32 version ) { mCurrentBindingsVersion = version; }

	public:
		static const tGameController cNullGameController;

	private:
		static tGameController* fNullGameControllerFromScript( );
		
		Math::tVec2f fMouseToStick( u32 controlProfile, u32 inputFilter = 0 ) const;
		b32 fMouseAsAimStickHeld( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter = 0 ) const;
		b32 fMouseAsAimStickDown( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter = 0 ) const;
		b32 fMouseAsAimStickUp( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter = 0 ) const;
		b32 fMouseAsMoveStickHeld( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter = 0 ) const;
		b32 fMouseAsMoveStickDown( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter = 0 ) const;
		b32 fMouseAsMoveStickUp( u32 controlProfile, u32 gameControlsFlag, u32 inputFilter = 0 ) const;
		b32 fMouseTouchScreenEdgeToMoveStick( u32 controlProfile, u32 inputFilter, Math::tVec2f &stick ) const;
		
		Math::tVec2f fKeysToStick( u32 controlProfile, u32 inputFilter = 0 ) const;

	private:
		struct ControlProfileBindings
		{
			tFixedArray<Input::tKeyboard::tButton, GameFlags::cGAME_CONTROLS_COUNT> mKeyboardBindings;
			tFixedArray<Input::tMouse::tButton, GameFlags::cGAME_CONTROLS_COUNT> mMouseBindings;
			tFixedArray<Input::tGamepad::tButton, GameFlags::cGAME_CONTROLS_COUNT> mGamepadBindings;

			template< class tArchive >
			void fSaveLoad( tArchive& archive )
			{
				archive.fSaveLoad( mKeyboardBindings );
				archive.fSaveLoad( mMouseBindings );
				archive.fSaveLoad( mGamepadBindings );
			}
		};
		tFixedArray< ControlProfileBindings, tUserProfile::cProfileCount > mBindings;
		const tUserPtr& mUser;
		const tUserProfilePtr& mProfile;
		b32 mDisableMouseCursorAutoRestrict;
		tGrowableArray<u32> mMouseCursorRestrictInputFilterStack;

		tLocalizedString mControlBindingLocalizedString;
		b32 mCaptureNextKeyPress;
		Input::tKeyboard::tButton mCaptureNextKeyPressResult;

		b32 mLastGamepadConnected;
		b32 mBindingsChanged;

		Mode mMode;

		f32 mMouseSensitivity;
		u32 mMouseInverted;
		f32 mMouseWheelDeltaAccumulated;

		u32 mCurrentBindingsVersion;
	};

	define_smart_ptr( /*No export type*/, tRefCounterPtr, tGameController );

} // namespace Sig

#endif __tGameController__
