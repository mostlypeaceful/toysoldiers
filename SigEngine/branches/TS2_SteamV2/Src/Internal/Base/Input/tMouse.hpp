#ifndef __tMouse__
#define __tMouse__
#include "tRingBuffer.hpp"
#include "tFixedBitArray.hpp"

namespace Sig { namespace Input
{

	///
	/// \brief Encapsulates a single instance of a mouse hardware input device. Currently
	/// this only supports the idea of a single mouse, as this is the way hardware seems
	/// to behave (i.e., even if you have multiple mice connected, there is really only
	/// a single "logical" mouse and cursor).
	class base_export tMouse
	{
	public:

		typedef u32 tButton;

		static const tButton cButtonLeft = 0;
		static const tButton cButtonMiddle = 1;
		static const tButton cButtonRight = 2;
		static const tButton cButtonCount = 3;

		static tButton fStringToButton( const tStringPtr& name );
		static const tStringPtr& fButtonToString( tButton button );

		static s16 sRestrictCursorPosX;
		static s16 sRestrictCursorPosY;

		/// \brief Also applied to tStateData::mFlags
		enum tStateFlag
		{
			cFlagCursorInClient = cButtonCount ///< cursor is inside the window's "client area" (not window title etc)
		};

		/// \brief Applied to tButtonData::mFlags
		enum tInstanceFlag
		{
			cFlagDragging,		///< set on moving while down, reset on up
			cFlagClicked,		///< set briefly on up when !cFlagDragging similarly to touch "Taps" -- this might be poorly named.  Set when cFlagDoubleClicked as well.
			cFlagDoubleClicked,	///< set briefly on up when !cFlagDragging a second time within Input_Tweak_Mouse_DoubleClickTime.  Implies cFlagClicked
		};

		typedef Math::tVec2f tPosition;

		/// \brief 1 per mStateHistory entry
		struct tStateData
		{
			s16				mCursorPosX;
			s16				mCursorPosY;
			s16				mWheelPos;
			s16				mCursorDeltaX;
			s16				mCursorDeltaY;
			s16				mWheelDelta;
			u16				pad;
			tFlags8			mFlags;
			u8				pad1;
			s16				mCoordWidth;
			s16				mCoordHeight;

			tStateData( ) { fZeroOut( this ); }
			Math::tVec2i	fCursorPos( ) const { return Math::tVec2i( mCursorPosX, mCursorPosY ); }

			template<class tArchive>
			void fSaveLoad( tArchive & archive )
			{
				archive.fSaveLoad( mCursorPosX );
				archive.fSaveLoad( mCursorPosY );
				archive.fSaveLoad( mWheelPos );
				archive.fSaveLoad( mCursorDeltaX );
				archive.fSaveLoad( mCursorDeltaY );
				archive.fSaveLoad( mWheelDelta );
				archive.fSaveLoad( mFlags );
				archive.fSaveLoad( mCoordWidth );
				archive.fSaveLoad( mCoordHeight );
			}
		};

		/// \brief 1 per button.  Might want to shove this into tStateData so we get button history for better double click detection.
		struct tButtonData
		{
			tPosition	mDragStart;
			tPosition	mDragCurrent;
			u32			mLastClickTime;
			tFlags16	mFlags;
			u16			pad;

			tButtonData( )
				: mDragStart( tPosition::cZeroVector )
				, mDragCurrent( tPosition::cZeroVector )
				, mLastClickTime( 0 )
				, mFlags( 0 )
			{ }
		};

		typedef tRingBuffer<tStateData> tStateDataBuffer;
		typedef void* tGenericWindowHandle;

	private:
		mutable tFixedArray< tButtonData, cButtonCount > mInstanceData;
		tStateDataBuffer		mStateHistory;
		tGenericWindowHandle	mWindowHandle;

	public:
		static const tMouse cNullMouse;

	public:

		tMouse( );
		~tMouse( );

		///
		/// \brief Should be called to initialize all mouse input. There's not much reason
		/// to call this more than once, unless you for some reason need to shutdown mouse
		/// input during the game and want to re-initialize it later.
		void fStartup( tGenericWindowHandle winHandle );

		///
		/// \brief Should be called once for each call to fStartup to uninitialize all mouse input.
		void fShutdown( ); 

		///
		/// \brief Capture the current state of the mouse and add it to the history buffer.
		/// \note This should be called once per frame.
		void fCaptureState( f32 dt = 1.f );

		///
		/// \brief Show/Hide the cursor.  NOTE: COUNTS HOW MANY TIMES THE CURSOR HAS BEEN SHOWN/HIDDEN.
		static void fShowCursor( b32 show );

		///
		/// \brief Returns whether or not the cursor is hidden.
		static b32 fCursorHidden( );

		///
		/// \brief set global mouse sensitivity (0..1)
		static void fSetMouseSensitivity( f32 sensitivity );
		static f32 fGetMouseSensitivity( );

		///
		/// \brief restrict the mouse to the center of the client window (and hide it)
		/// \note  use this when you want to use the mouse for gameplay controls
		void fRestrictToClientWindow( b32 enable );

		///
		/// \brief Set the cursor position
		/// \note  This sets the cursor position within the client rect.
		void fSetPosition( u32 x, u32 y );

		///
		/// \brief Capture the current state of the mouse and put it in the output variable,
		/// without putting it in the history buffer. Prefer getting the state history and
		/// accessing the fFront( ) item.
		void fCaptureStateUnbuffered( tStateData& stateData, f32 dt = 1.f );

		///
		/// \brief Set new size of the history buffer. This method will clamp the input
		/// size to a minimum of 2. The mouse starts by default with a history size of two.
		void fSetHistorySize( u32 newSize );

		///
		/// \brief Access the state history buffer.
		const tStateDataBuffer& fGetStateHistory( ) const;

		///
		/// \brief Access the last state in the history buffer.
		const tStateData& fGetState( ) const;

		///
		/// \brief set state into the history buffer.
		void fPutStateData( const tStateData& data );

		/// 
		/// \brief Get whether this mouse is the null mouse.
		b32 fIsNull( ) const;

		///
		/// \brief Query for whether the mouse has moved or otherwise changed in state (between this frame and last).
		b32 fUpdated( ) const;

		///
		/// \brief Query for whether the mouse is updated or has buttons held
		b32 fActive( ) const;

		///
		/// \brief Query for whether the specified button is currently held down (tests latest frame only).
		b32 fButtonHeld( tButton button ) const;

		///
		/// \brief Query for whether the specified button has just been pressed (between this frame and last).
		b32 fButtonDown( tButton button ) const;

		///
		/// \brief Query for whether the specified button has just been let go (between this frame and last).
		b32 fButtonUp( tButton button ) const;

		///
		/// \brief Query for whether the specified button is held, and if the mouse has moved far enough to be considered 'dragging' since then.
		b32 fButtonDragging( tButton button ) const;

		b32 fButtonClicked( tButton button ) const;

		b32 fButtonDoubleClicked( tButton button ) const;

		///
		/// \brief Query for the mouse position
		Math::tVec2f fPosition( ) const;

		///
		/// \brief Query for the ( width, height ) of the coord area that fPosition is within
		Math::tVec2f fCoordRange( ) const;

		///
		/// \brief Query for the change in mouse position between this frame and last.
		Math::tVec2f fDeltaPosition( ) const;

		///
		/// \brief Query for whether the cursor is in the client area.
		b32 fCursorInClientArea( ) const;

		///
		/// \brief If a button is held, this will give you the relative displacement from where they initially touched.
		Math::tVec2f fDragDelta( tButton button ) const;

		///
		/// \brief change in direction of the mouse wheel
		f32 fWheelDelta( ) const;

		///
		/// \brief get the window handle.
		const tGenericWindowHandle	fWindowHandle( ) { return mWindowHandle; };


		///
		/// \brief Capture the current state of the keyboard and store it in the global state data
		/// \note This should be called once per frame under normal circumstances.
		static void fCaptureGlobalState( tGenericWindowHandle windowHandle, f32 dt = 1.f );


	private:

		///
		/// \brief Capture the current state of the keyboard and put it in the output variable,
		/// without putting it storing it in this keyboard object's state data variable.
		/// Prefer using the tKeyboard class directly to query the current state.
		static void fCaptureGlobalStateUnbuffered( const tGenericWindowHandle &windowHandle, tStateData& stateData, tStateData& lastState, f32 dt );


		///
		/// \brief Call this once per frame to utilize the Drag functions below.
		void fStepDrag( f32 dt ) const;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};


}}


#endif//__tMouse__
