#ifndef __tKeyboard__
#define __tKeyboard__

namespace Sig { namespace Input
{

	///
	/// \brief Encapsulates a single instance of a keyboard hardware input device. Currently
	/// this only supports the idea of a single keyboard, as this is the way hardware seems
	/// to behave (i.e., even if you have multiple keyboards connected, there is really only
	/// a single "logical" keyboard).
	class base_export tKeyboard
	{
		declare_singleton_define_own_ctor_dtor( tKeyboard );

	public:

		typedef u32 tButton;

		struct tStateData
		{
			typedef tFixedArray<u8,256> tKeyBuffer;

			tKeyBuffer mKeysOn;
			tKeyBuffer mPrevKeysOn;
			tKeyBuffer mFramesHeld;
		};

		typedef void* tGenericWindowHandle;

	public:

		static const tKeyboard cNullKeyboard;

	private:

		tStateData				mStateData;
		tGenericWindowHandle	mWindow;

	public:

		tKeyboard( );
		~tKeyboard( );

		///
		/// \brief Should be called to initialize all keyboard input. There's not much reason
		/// to call this more than once, unless you for some reason need to shutdown keyboard
		/// input during the game and want to re-initialize it later.
		void fStartup( tGenericWindowHandle window );

		///
		/// \brief Should be called once for each call to fStartup to uninitialize all keyboard input.
		void fShutdown( );

		///
		/// \brief Capture the current state of the keyboard and store it (this will overwrite the
		/// previous 'keys on' array in the state data buffer).
		/// \note This should be called once per frame under normal circumstances.
		void fCaptureState( f32 dt = 1.f );

		///
		/// \brief Capture the current state of the keyboard and put it in the output variable,
		/// without putting it storing it in this keyboard object's state data variable.
		/// Prefer using the tKeyboard class directly to query the current state.
		void fCaptureStateUnbuffered( tStateData& stateData, f32 dt = 1.f );

		///
		/// \brief Unmediated access to the state buffer.
		const tStateData& fGetStateData( ) const;

		///
		/// \brief Query for the number of frames a key has been held.
		u8 fNumFramesHeld( tButton button ) const;

		///
		/// \brief Query for whether the specified button is currently held.
		b32 fButtonHeld( tButton button ) const;

		///
		/// \brief Query for whether the specified button has just been pressed (between this frame and last).
		b32 fButtonDown( tButton button ) const;

		///
		/// \brief Query for whether the specified button has just been released (between this frame and last).
		b32 fButtonUp( tButton button ) const;

	public:

		///
		/// Button ids...

		static tButton fStringToButton( const tStringPtr& name );
		const tStringPtr& fButtonToString( tButton button );

		static const tButton cButton0;
		static const tButton cButton1;
		static const tButton cButton2;
		static const tButton cButton3;
		static const tButton cButton4;
		static const tButton cButton5;
		static const tButton cButton6;
		static const tButton cButton7;
		static const tButton cButton8;
		static const tButton cButton9;

		static const tButton cButtonA;
		static const tButton cButtonB;
		static const tButton cButtonC;
		static const tButton cButtonD;
		static const tButton cButtonE;
		static const tButton cButtonF;
		static const tButton cButtonG;
		static const tButton cButtonH;
		static const tButton cButtonI;
		static const tButton cButtonJ;
		static const tButton cButtonK;
		static const tButton cButtonL;
		static const tButton cButtonM;
		static const tButton cButtonN;
		static const tButton cButtonO;
		static const tButton cButtonP;
		static const tButton cButtonQ;
		static const tButton cButtonR;
		static const tButton cButtonS;
		static const tButton cButtonT;
		static const tButton cButtonU;
		static const tButton cButtonV;
		static const tButton cButtonW;
		static const tButton cButtonX;
		static const tButton cButtonY;
		static const tButton cButtonZ;

		static const tButton cButtonUp;
		static const tButton cButtonLeft;
		static const tButton cButtonRight;
		static const tButton cButtonDown;

		static const tButton cButtonEscape;
		static const tButton cButtonMinus;
		static const tButton cButtonEquals;
		static const tButton cButtonBackspace;
		static const tButton cButtonTab;
		static const tButton cButtonLBracket;
		static const tButton cButtonRBracket;
		static const tButton cButtonEnter;
		static const tButton cButtonLCtrl;
		static const tButton cButtonRCtrl;
		static const tButton cButtonLAlt;
		static const tButton cButtonRAlt;
		static const tButton cButtonLShift;
		static const tButton cButtonRShift;
		static const tButton cButtonSpace;
		static const tButton cButtonCapsLock;
		static const tButton cButtonNumLock;
		static const tButton cButtonScrollLock;
		static const tButton cButtonHome;
		static const tButton cButtonPrior;
		static const tButton cButtonEnd;
		static const tButton cButtonNext;
		static const tButton cButtonInsert;
		static const tButton cButtonDelete;
		static const tButton cButtonLWin;
		static const tButton cButtonRWin;
		static const tButton cButtonPause;

		static const tButton cButtonNumPad0;
		static const tButton cButtonNumPad1;
		static const tButton cButtonNumPad2;
		static const tButton cButtonNumPad3;
		static const tButton cButtonNumPad4;
		static const tButton cButtonNumPad5;
		static const tButton cButtonNumPad6;
		static const tButton cButtonNumPad7;
		static const tButton cButtonNumPad8;
		static const tButton cButtonNumPad9;
		static const tButton cButtonNumPadDec;
		static const tButton cButtonNumPadSub;
		static const tButton cButtonNumPadAdd;
		static const tButton cButtonNumPadMul;
		static const tButton cButtonNumPadDiv;

		static const tButton cButtonF1;
		static const tButton cButtonF2;
		static const tButton cButtonF3;
		static const tButton cButtonF4;
		static const tButton cButtonF5;
		static const tButton cButtonF6;
		static const tButton cButtonF7;
		static const tButton cButtonF8;
		static const tButton cButtonF9;
		static const tButton cButtonF10;
		static const tButton cButtonF11;
		static const tButton cButtonF12;
		static const tButton cButtonF13;
		static const tButton cButtonF14;
		static const tButton cButtonF15;
	};

}}

#endif//__tKeyboard__
