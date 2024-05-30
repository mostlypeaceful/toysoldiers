#ifndef __tGamepad__
#define __tGamepad__
#include "tRingBuffer.hpp"
#include "tRumbleManager.hpp"

namespace Sig { namespace Input
{
	class base_export tGamepad
	{
	public:

		typedef u32 tButton;

		static const tButton cButtonStart;
		static const tButton cButtonSelect;
		static const tButton cButtonA;
		static const tButton cButtonB;
		static const tButton cButtonX;
		static const tButton cButtonY;
		static const tButton cButtonDPadRight;
		static const tButton cButtonDPadUp;
		static const tButton cButtonDPadLeft;
		static const tButton cButtonDPadDown;
		static const tButton cButtonLShoulder;
		static const tButton cButtonLThumb;
		static const tButton cButtonLTrigger;
		static const tButton cButtonLThumbMaxMag; //"down" if thumb stick is fully displaced
		static const tButton cButtonLThumbMinMag; //"down" if thumb stick is displaced at all
		static const tButton cButtonRShoulder;
		static const tButton cButtonRThumb;
		static const tButton cButtonRTrigger;
		static const tButton cButtonRThumbMaxMag;
		static const tButton cButtonRThumbMinMag;
		static const tButton cButtonLStickRight;
		static const tButton cButtonLStickUp;
		static const tButton cButtonLStickLeft;
		static const tButton cButtonLStickDown;
		static const tButton cButtonRStickRight;
		static const tButton cButtonRStickUp;
		static const tButton cButtonRStickLeft;
		static const tButton cButtonRStickDown;

		static tButton fStringToButton( const tStringPtr& name );
		const tStringPtr& fButtonToString( tButton button );

		enum tDirection
		{
			cDirectionNone = 0,
			cDirectionLeft,
			cDirectionRight,
			cDirectionUp,
			cDirectionDown
		};

		struct tStateData
		{
			Math::tVec2f	mLeftStick;
			Math::tVec2f	mRightStick;
			f32				mLeftStickAngle;
			f32				mRightStickAngle;
			f32				mLeftStickStrength;
			f32				mRightStickStrength;
			u32				mButtonsDown; // combined tButton flags of the buttons that were down
			u8				mLeftTrigger; // strength in [0-255]
			u8				mRightTrigger; // strength in [0-255]
			tStateData( ) { fZeroOut( this ); }

			template<class tArchive>
			void fSaveLoad( tArchive & archive )
			{
				archive.fSaveLoad( mLeftStick );
				archive.fSaveLoad( mRightStick );
				archive.fSaveLoad( mLeftStickAngle );
				archive.fSaveLoad( mRightStickAngle );
				archive.fSaveLoad( mLeftStickStrength );
				archive.fSaveLoad( mRightStickStrength );
				archive.fSaveLoad( mButtonsDown );
				archive.fSaveLoad( mLeftTrigger );
				archive.fSaveLoad( mRightTrigger );
			}
		};

		typedef tRingBuffer<tStateData> tStateDataBuffer;

	public:
		static const tGamepad	cNullGamepad;

	private:

		tStateDataBuffer		mStateHistory;
		mutable tRumbleManager	mRumble;

		u64						mInputDevicePtr;
		b8						mConnected;
		b8						pad0;
		b8						pad1;
		b8						pad2;

	public:
		explicit tGamepad( u32 historyBufferSize = 12 );
		~tGamepad( );

		///
		/// \brief Should be called to initialize all gamepad input. There's not much reason
		/// to call this more than once, unless you for some reason need to shutdown gamepad
		/// input during the game and want to re-initialize it later.
		void fStartup( );

		///
		/// \brief Should be called once for each call to fStartup to uninitialize all gamepad input.
		void fShutdown( );

		///
		/// \brief A real, connected gamepad.
		b32 fConnected( ) const { return mConnected; }

		///
		/// \brief Capture the current state of the gamepad and store it (this will overwrite the
		/// current state).
		/// \note This should be called once per frame under normal circumstances.
		void fCaptureState( u32 userIndex = 0, f32 dt = 1.f );

		///
		/// \brief Capture the current state of the gamepad and put it in the output variable,
		/// without putting it storing it in this gamepad object's state data variable.
		/// Prefer using the tGamepad class directly to query the current state.
		void fCaptureStateUnbuffered( tStateData& stateData, u32 userIndex = 0, f32 dt = 1.f );

		///
		/// \brief Set new size of the history buffer. This method will clamp the input
		/// size to a minimum of 2. The gamepad starts by default with a history size of two.
		void fSetHistorySize( u32 newSize );

		///
		/// \brief Access the state history buffer.
		const tStateDataBuffer& fGetStateHistory( ) const { return mStateHistory; }

		///
		/// \brief Query for whether the specified button is currently held down (tests latest frame only).
		b32 fButtonHeld( tButton button ) const;

		///
		/// \brief Query for whether the specified button is currently held down (tests last 'repeat' + 1 frames).
		/// \note Calling fButtonHeldRepeat( button, 0 ) is equivalent to calling fButtonHeld( button ).
		b32 fButtonHeldRepeat( tButton button, u32 repeat ) const;

		///
		/// \brief Query for whether the specified button has just been pressed (between this frame and last).
		b32 fButtonDown( tButton button ) const;

		///
		/// \brief Ensures that subsiquent calls to fButtonDown during this frame will return false.
		///  Useful if you dont want two things responding to the same event.
		/// *You should be very careful about what you call this on. You dont want to accidentally set this on the null gamepad. (meant to remail totally blank.)*
		void fClearButtonDown( tButton button ) const;

		///
		/// \brief Query for whether the specified button has just been let go (between this frame and last).
		b32 fButtonUp( tButton button ) const;

		///
		/// \brief Get the normalized pressure of the button [0,1].
		f32 fLeftTriggerPressure( ) const;

		///
		/// \brief Is the trigger held?
		b32 fLeftTriggerHeld( ) const;

		///
		/// \brief Was the trigger just depressed?
		b32 fLeftTriggerDown( ) const;

		///
		/// \brief Was the trigger just let go of?
		b32 fLeftTriggerUp( ) const;

		///
		/// \brief Get the normalized pressure of the button [0,1].
		f32 fRightTriggerPressure( ) const;

		///
		/// \brief Is the trigger held?
		b32 fRightTriggerHeld( ) const;

		///
		/// \brief Was the trigger just depressed?
		b32 fRightTriggerDown( ) const;

		///
		/// \brief Was the trigger just let go of?
		b32 fRightTriggerUp( ) const;

		///
		/// \brief Get the specified stick direction.
		inline Math::tVec2f fLeftStick( ) const { return fLeftStick( 0 ); }
		Math::tVec2f fLeftStick( u32 numFramesBack ) const;

		///
		/// \brief Get the magnitude of the specified stick.
		f32 fLeftStickMagnitude( u32 numFramesBack = 0 ) const;

		///
		/// \brief Get the 2D angle of the specified stick.
		f32 fLeftStickAngle( u32 numFramesBack = 0 ) const;

		///
		/// \brief Get the specified stick direction/magnitude.
		inline Math::tVec2f fRightStick( ) const { return fRightStick( 0 ); }
		Math::tVec2f fRightStick( u32 numFramesBack ) const;

		///
		/// \brief Get the magnitude of the specified stick.
		f32 fRightStickMagnitude( u32 numFramesBack = 0 ) const;

		///
		/// \brief Get the 2D angle of the specified stick.
		f32 fRightStickAngle( u32 numFramesBack = 0 ) const;

		///
		/// \brief Check D-pad and then l-stick direction
		/// \returns tDirection values
		u32 fGetDirection( ) const;

		///
		/// \brief Map the tVec2 unit circle stick position to unit rectangle
		static Math::tVec2f fMapStickCircleToRectangle( const Math::tVec2f &stick, u32 innerRoundness = 1 );
		
		///
		/// \brief Return mutable rumble manager.
		tRumbleManager& fRumble( ) const { return mRumble; }

		///
		/// \brief Puts state data into the buffer
		void fPutStateData( const tStateData& data );

		///
		/// \brief Gets the current state data
		const tStateData& fGetStateData( ) const;

		///
		/// \brief Query for whether the gamepad has had it's sticks moved or otherwise changed in state (between this frame and last).
		b32 fUpdated( ) const;

		///
		/// \brief Query for wheither the gamepad is active (updated, buttons held, etc)
		b32 fActive( ) const;
	private:
		
		static tGamepad * fNullGamepadFromScript( );

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	

	};

}}

#endif//__tGamepad__
