#include "BasePch.hpp"
#include "tMouse.hpp"

namespace Sig { namespace Input
{
	const tMouse tMouse::cNullMouse;

	devvar( u32, Input_Tweak_Mouse_DoubleClickTime, 10 ); ///< in mStateHistory frames
	devvar_clamp( f32, Input_Tweak_Mouse_DragThreshold		, 20.0f, 0.0f, 100.0f, 0 ); ///< in pixels
	devvar_clamp( f32, Input_Tweak_Mouse_DoubleClickDistance, 20.0f, 0.0f, 100.0f, 0 ); ///< in pixels

	namespace
	{
		u32 fMinHistory( )
		{
			return fMax<u32>( 2u, Input_Tweak_Mouse_DoubleClickTime );
		}

		struct tButtonString
		{ 
			tButtonString( ) : mButton( 0 ) { }
			tButtonString( const tStringPtr& name, tMouse::tButton button )
				: mName( name )
				, mButton( button )
			{
			}

			b32 operator==( const tStringPtr& name ) const { return mName == name; }
			b32 operator==( tMouse::tButton button ) const { return mButton == button; }

			tStringPtr mName; 
			tMouse::tButton mButton; 
		};

		static tGrowableArray< tButtonString > gButtonStrings;
		define_static_function( fBuildButtonStrings )
		{
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "L" ), tMouse::cButtonLeft ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "R" ), tMouse::cButtonRight ) );
			gButtonStrings.fPushBack( tButtonString( tStringPtr( "M" ), tMouse::cButtonMiddle ) );
		}
	}

	tMouse::tButton tMouse::fStringToButton( const tStringPtr& name )
	{
		if( const tButtonString* map = gButtonStrings.fFind( name ) )
			return map->mButton;
		return tMouse::cButtonCount;
	}

	const tStringPtr& tMouse::fButtonToString( tButton button )
	{
		if( const tButtonString* map = gButtonStrings.fFind( button ) )
			return map->mName;
		return tStringPtr::cNullPtr;
	}

	tMouse::tMouse( )
		: mStateHistory( 2 )
		, mWindowHandle( 0 )
	{
		tStateData dummyStateData = {};
		dummyStateData.mCursorPosX = dummyStateData.mCursorPosY = -1; // Initialize the mouse position to out-of-bounds
		mStateHistory.fFill( dummyStateData );
	}

	tMouse::~tMouse( )
	{
		fShutdown( );
	}

	void tMouse::fCaptureState( f32 dt )
	{
		if( mStateHistory.fCapacity( ) < fMinHistory( ) )
			fSetHistorySize( fMinHistory( ) );

		tStateData stateData;
		fCaptureStateUnbuffered( stateData, dt );
		mStateHistory.fPushLast( stateData );
		fStepDrag(dt);

		for( u32 b=0 ; b<mInstanceData.fCount( ) ; ++b )
		{
			const tButton button = (tButton)b;

			const b32 dragging = fButtonDragging(button) || ( fButtonHeld(button) && fDragDelta(button).fLengthSquared( ) >= Math::fSquare<f32>(Input_Tweak_Mouse_DragThreshold) );

			mInstanceData[button].mFlags.fSetAll(false);
			mInstanceData[button].mFlags.fSetBit(cFlagDragging,dragging); // preserve

			if( fButtonUp(button) )
			{
				if( !dragging )
				{
					mInstanceData[button].mFlags.fSetBit( cFlagClicked, true );

					u32 beg = fMax( 0, (s32)mStateHistory.fNumItems( ) - (s32)Input_Tweak_Mouse_DoubleClickTime );
					u32 end = fMax( 0, (s32)mStateHistory.fNumItems( ) - 2 );

					b32 downOnce = false;
					b32 upOnce = false;

					for( u32 h=beg ; h<end ; ++h )
					{
						const float dist2 = mInstanceData[button].mDragCurrent.fLengthSquared( );
						const b32 downNow = mStateHistory[h].mFlags.fGetBit(cButtonLeft);

						if( !downOnce && downNow )
							downOnce = true;
						else if( downOnce && !upOnce && !downNow )
							upOnce = true;
						else if( upOnce && downNow && (dist2<Math::fSquare<f32>(Input_Tweak_Mouse_DoubleClickDistance)) )
						{
							mInstanceData[button].mFlags.fSetBit( cFlagDoubleClicked, true );
						}
					}
					// Input_Tweak_Mouse_DoubleClickTime
					// TODO: Check history
				}
			}
			else if( !fButtonUp(button) && ( !fButtonHeld(button) || fButtonDown(button) ) )
			{
				// don't clear until the frame after it went up
				mInstanceData[button].mFlags.fSetBit( cFlagDragging     , false );
				mInstanceData[button].mFlags.fSetBit( cFlagClicked      , false );
				mInstanceData[button].mFlags.fSetBit( cFlagDoubleClicked, false );
			}
		}
	}

	void tMouse::fSetHistorySize( u32 newSize )
	{
		mStateHistory.fResize( fMax( 2u, newSize ) );
	}

	b32 tMouse::fIsNull( ) const
	{
		return this == &cNullMouse;
	}

	const tMouse::tStateDataBuffer& tMouse::fGetStateHistory( ) const
	{
		return mStateHistory;
	}

	const tMouse::tStateData& tMouse::fGetState( ) const
	{
		return mStateHistory[ mStateHistory.fNumItems( ) - 1 ];
	}

	b32 tMouse::fUpdated( ) const
	{
		const tStateData& now  = mStateHistory[ mStateHistory.fNumItems( ) - 1 ];
		const tStateData& prev = mStateHistory[ mStateHistory.fNumItems( ) - 2 ];

		return ( now.mCursorDeltaX != prev.mCursorDeltaX )
			|| ( now.mCursorDeltaY != prev.mCursorDeltaY )
			|| ( now.mCursorPosX   != prev.mCursorPosX   )
			|| ( now.mCursorPosY   != prev.mCursorPosY   )
			|| ( now.mFlags        != prev.mFlags        )
			|| ( now.mWheelDelta   != prev.mWheelDelta   )
			|| ( now.mWheelPos     != prev.mWheelPos     )
			;
	}

	b32 tMouse::fActive( ) const
	{
		if( fUpdated( ) )
			return true;

		for( u32 i=0 ; i<cButtonCount ; ++i )
			if( fButtonHeld((tButton)i) )
				return true;

		return false;
	}

	b32 tMouse::fButtonHeld( tButton button ) const
	{ 
        if( button < cButtonCount )
			return mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mFlags.fGetBit( button );
		return false;
	}

	b32 tMouse::fButtonDown( tButton button ) const
	{
        if( button < cButtonCount )
		{
			return  mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mFlags.fGetBit( button )
				&& !mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mFlags.fGetBit( button );
		}
		
		return false;
	}

	b32 tMouse::fButtonUp( tButton button ) const
	{
        if( button < cButtonCount )
		{
			return !mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mFlags.fGetBit( button )
				&&  mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mFlags.fGetBit( button );
		}

		return false;
	}

	b32 tMouse::fButtonDragging( tButton button ) const
	{
        if( button < cButtonCount )
			return mInstanceData[ button ].mFlags.fGetBit( cFlagDragging );
		return false;
	}

	b32 tMouse::fButtonClicked( tButton button ) const
	{
        if( button < cButtonCount )
			return mInstanceData[ button ].mFlags.fGetBit( cFlagClicked );
		return false;
	}

	b32 tMouse::fButtonDoubleClicked( tButton button ) const
	{
        if( button < cButtonCount )
			return mInstanceData[ button ].mFlags.fGetBit( cFlagDoubleClicked );
		return false;
	}

	Math::tVec2f tMouse::fPosition( ) const
	{
		return Math::tVec2f( mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mCursorPosX, mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mCursorPosY );
	}

	Math::tVec2f tMouse::fDeltaPosition( ) const
	{
		const Math::tVec2f now(  mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mCursorPosX, mStateHistory[ mStateHistory.fNumItems( ) - 1 ].mCursorPosY );
		const Math::tVec2f prev( mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mCursorPosX, mStateHistory[ mStateHistory.fNumItems( ) - 2 ].mCursorPosY );
		return now-prev;
	}

	b32 tMouse::fCursorInClientArea( ) const
	{
		return mStateHistory.fLast( ).mFlags.fGetBit( cFlagCursorInClient );
	}
	
	void tMouse::fStepDrag( f32 dt ) const
	{
		for( u32 i = 0; i < cButtonCount; ++i )
		{
			tButton button = (tButton)i;

			if( !fButtonHeld( button ) && !fButtonUp( button ) )
				mInstanceData[ button ].mDragCurrent = Math::tVec2f::cZeroVector; //reset
			else // button held or just up
			{
				const tStateData& state = fGetState();
				Math::tVec2f currentPos = Math::tVec2f( state.mCursorPosX, state.mCursorPosY );

				if( fButtonDown( button ) )
					mInstanceData[ button ].mDragStart = currentPos; //calibrate
			
				mInstanceData[ button ].mDragCurrent = currentPos - mInstanceData[ button ].mDragStart; //compute
			}
		}
	}

	Math::tVec2f tMouse::fDragDelta( tButton button ) const
	{
		return mInstanceData[ button ].mDragCurrent;
	}

	//------------------------------------------------------------------------------
	void tMouse::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tMouse, Sqrat::DefaultAllocator<tMouse> > classDesc( vm.fSq( ) );

		classDesc.Func( _SC("Position"), &tMouse::fPosition )
		.StaticFunc( _SC("ShowCursor"), &tMouse::fShowCursor )
		.StaticFunc( _SC("CursorHidden"), &tMouse::fCursorHidden);
		
		vm.fNamespace(_SC("Input")).Bind( _SC("Mouse"), classDesc );

		vm.fConstTable( ).Const( _SC("MOUSE_BUTTON_LEFT"), ( int )cButtonLeft );
		vm.fConstTable( ).Const( _SC("MOUSE_BUTTON_MIDDLE"), ( int )cButtonMiddle );
		vm.fConstTable( ).Const( _SC("MOUSE_BUTTON_RIGHT"), ( int )cButtonRight );
	}

}}

