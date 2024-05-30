#include "SigAnimPch.hpp"
#include "tSigAnimTimeline.hpp"
#include "tSigAnimMainWindow.hpp"
#include "tEntityControlPanel.hpp"
#include "tProjectFile.hpp"

namespace Sig
{
	namespace
	{
		// Duplicated in tSigAnimEdDialog.cpp
		std::string fKeyframeEventStringFromKey( u32 key )
		{
			std::string eventType = "Error";
			const tProjectFile::tEvent* event = tProjectFile::fInstance( ).fFindKeyframeEventByKey( key );
			if( event )
				eventType = event->mName;
			else
				log_warning( "Could not find key frame event. key: " << key );
			return eventType;
		}
	}


	tSigAnimTimeline::tCutCopyPasteAction::tCutCopyPasteAction( tSigAnimTimeline* timeline )
		: mTimeline( timeline )
		, mCut( fNextUniqueActionId() )
		, mCopy( fNextUniqueActionId() )
		, mPaste( fNextUniqueActionId() )
		, mDelete( fNextUniqueActionId() )
	{

	}

	b32 tSigAnimTimeline::tCutCopyPasteAction::fAddToContextMenu( wxMenu& menu )
	{
		if( mTimeline->fRangeIsSelected() && mTimeline->mMultiselectedEvents.fCount() > 0 )
		{
			menu.Append( mCut, _T("Cut") );
			menu.Append( mCopy, _T("Copy") );
			
		}
		if( mTimeline->mEventClipboard.fCount() > 0 )
			menu.Append( mPaste, _T("Paste") );

		// Put delete at the end and separate it so people don't accidentally delete stuff.
		if( mTimeline->fRangeIsSelected() && mTimeline->mMultiselectedEvents.fCount() > 0 )
		{
			menu.AppendSeparator();
			menu.Append( mDelete, _T("Delete") );
		}
		return true;
	}

	b32	tSigAnimTimeline::tCutCopyPasteAction::fHandleAction( u32 actionId )
	{
		if( actionId == mCopy )
		{
			mTimeline->fCopySelectedEvents();
			return true;
		}
		else if( actionId == mCut )
		{
			// remove
			mTimeline->fCopySelectedEvents();
			mTimeline->fDeleteSelectedEvents();
			return true;
		}
		else if( actionId == mPaste )
		{
			mTimeline->fPasteEvents( fRightClickPos().x );
			return true;
		}
		else if( actionId == mDelete )
		{
			mTimeline->fDeleteSelectedEvents();
			return true;
		}

		return false;
	}


	tSigAnimTimeline::tSigAnimTimeline( wxPanel* parent, tEntityControlPanel* entityControlPanel )
		: wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.x, 36 ) )
		, mEntityControlPanel( entityControlPanel )
		, mScrub( 0 )
		, mForceMouseInput( false )
		, mContinueMouseInput( false )
		, mUpdateSkeletonToScrub( false )
		, mBackgroundDC( 0 )
		, mDirty( true )
		, mLastTimelineLength( -1.f )
		, mCurrentTimelineLength( 10.f )
		, mStartingPosition( 128, 0 )
		, mTimelineDrawSize( 1024, 64 )
		, mAnimMetaData( 0 )
		, mShiftHeld( false )
		, mSelectingNewRange( false )
		, mExpandingLeft( false )
		, mExpandingRight( false )
		, mMovingRange( false )
		, mSelectionRange( Math::tVec2f::cZeroVector )
		, mSingleSelectedEvent( -1 )
		, mHoveredEvent( -1 )
	{
		Connect( wxEVT_PAINT, wxPaintEventHandler( tSigAnimTimeline::fOnPaint ) );
		Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( tSigAnimTimeline::fOnEraseBackground ) );
		
		Connect( wxEVT_MOTION, wxMouseEventHandler( tSigAnimTimeline::fOnMouseMove ) );
		Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( tSigAnimTimeline::fOnMouseLeftButtonDown ) );
		Connect( wxEVT_LEFT_UP, wxMouseEventHandler( tSigAnimTimeline::fOnMouseLeftButtonUp ) );
		Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tSigAnimTimeline::fOnRightClick ) );
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tSigAnimTimeline::fOnAction ) );

		Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( tSigAnimTimeline::fOnMouseLeaveWindow ) );
		Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( tSigAnimTimeline::fOnMouseEnterWindow ) );
		
		Connect( wxEVT_SIZE, wxSizeEventHandler( tSigAnimTimeline::fOnSize ), NULL, this );

		mContextActions.fPushBack( tEditorContextActionPtr( new tCutCopyPasteAction( this ) ) );

		mPlay = new wxButton( this, wxID_ANY, "Playing", wxDefaultPosition, wxSize( 92, 20 ), 0 );
		mPlay->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigAnimTimeline::fOnPlayButtonClicked ), NULL, this );
		mPlay->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( tSigAnimTimeline::fOnPlayButtonFocused ), NULL, this );

		wxBoxSizer* top = new wxBoxSizer( wxVERTICAL );
		top->AddSpacer( 22 );
		top->Add( mPlay, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT, 8 );
		top->AddSpacer( 22 );

		wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL);
		sizer->Add( top, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxEXPAND, 1 );
		SetSizer( sizer );

		SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	}

	tSigAnimTimeline::~tSigAnimTimeline( )
	{
		delete mScrub;
	}

	void tSigAnimTimeline::fDrawTickPos( wxDC* dc, f32 x, f32 height, f32 yOffset )
	{
		const f32 y1 = mStartingPosition.y + fTickSpace( ) - yOffset;
		const f32 y2 = y1 - height;
		dc->DrawLine( x, y1, x, y2 );
	}

	void tSigAnimTimeline::fDrawTickTime( wxDC* dc, f32 t, f32 height )
	{
		const f32 x = mStartingPosition.x + f32(mTimelineDrawSize.x) * t;
		fDrawTickPos( dc, x, height );
	}

	void tSigAnimTimeline::fDrawTextPos( wxDC* dc, f32 x, f32 t, f32 height )
	{
		const f32 y1 = mStartingPosition.y + fTickSpace( ) - height;
		const f32 time = fLifetime( ) * t;
		wxString s = wxString::Format( "%.2f", time ).Trim( );

		wxPoint tickFontSize;
		dc->GetTextExtent( s, &tickFontSize.x, &tickFontSize.y);
		dc->DrawText( s, x - tickFontSize.x - 1, y1 - tickFontSize.y );
	}

	f32 tSigAnimTimeline::fLifetime( )
	{
		return mCurrentTimelineLength;
	}

	void tSigAnimTimeline::fSetLifetime( const f32 lifetime )
	{
		mCurrentTimelineLength = lifetime;
	}

	void tSigAnimTimeline::fOnTick( f32 dt )
	{
		if( mContinueMouseInput && mForceMouseInput )
		{
			wxMouseState ms = wxGetMouseState( );
			wxPoint p = ScreenToClient( wxPoint( ms.GetX( ), ms.GetY( ) ) );

			if( !ms.LeftDown( ) )
			{
				mContinueMouseInput = mForceMouseInput = false;
				wxMouseEvent event( wxEVT_LEFT_UP );
				event.m_x = p.x;
				event.m_y = p.y;
				event.m_leftDown = ms.LeftDown( );
				event.m_rightDown = ms.RightDown( );
				event.m_shiftDown = ms.ShiftDown( );
				fOnMouseLeftButtonUp( event );
			}
			else
			{
				wxMouseEvent event( wxEVT_MOTION );
				event.m_x = p.x;
				event.m_y = p.y;
				event.m_leftDown = ms.LeftDown( );
				event.m_rightDown = ms.RightDown( );
				event.m_shiftDown = ms.ShiftDown( );
				fOnMouseMove( wxMouseEvent( event ) );
			}
		}

		Refresh( );
		mUpdateSkeletonToScrub = false;

		// Ensure the pause button is set properly.
		fSyncPauseButton( );
	}	

	void tSigAnimTimeline::fSetAnimEvents( const std::string& animPkLabel, const std::string& animName, Anipk::tFile& anipkFile, const tKeyFrameAnimation* kfAnim )
	{
		// Clear before assigning new since clear acts on the currently existed tags.
		fClearSelectionRange( );

		mAnimMetaData = &anipkFile.fFindOrAddAnimMetaData( animName );
		fMarkTimelineDirty( );
	}

	namespace
	{
		static const wxFont cMedTickFont( 7.25, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
		static const wxFont cMedTickFontBold( 7.25, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxT( "Georgia" ) );
		static const wxFont cBigTickFont( 8.5, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );

		static const f32 cSmallTickHeight = 6.f;
		static const f32 cMedTickHeight = 18.f;

		static const f32 cHandlePixelRadius = 4.f;
	}

	f32 tSigAnimTimeline::fToT( f32 timelinePos ) const
	{
		return (timelinePos - (f32)mStartingPosition.x) / (f32)mTimelineDrawSize.x;
	}

	void tSigAnimTimeline::fOnPaint( wxPaintEvent& event )
	{
		if( !mScrub )
		{
			mScrub = new tSigAnimTimelineScrub( this );
		}

		wxAutoBufferedPaintDC dc( this );

		mTimelineDrawSize = wxSize( GetSize( ).x - 155, GetSize( ).y - 8 );
		const wxPoint pos = fGetStartingPosition( );
		const wxSize size = fGetTimelineDrawSize( );

		b32 inupperdeck = false;
		f32 upperx = 0.f;

		f32 lifetime = fLifetime( );
		if( mLastTimelineLength != lifetime )
			fMarkTimelineDirty( );

		if( !mBackgroundDC || mDirty )
		{
			mDirty = false;
			if( mBackgroundDC )
				delete mBackgroundDC;

			wxBitmap bitmap(  GetSize( ).x, GetSize( ).y );
			mBackgroundDC = new wxMemoryDC( bitmap );

			mBackgroundDC->SetBrush( wxBrush( wxColour( 145, 138, 127 ) ) );
			mBackgroundDC->DrawRectangle( 0, 0, GetSize( ).x, GetSize( ).y );
			mLastTimelineLength = lifetime;

			if( mAnimMetaData && fRangeIsSelected() )
			{
				mBackgroundDC->SetPen( *wxBLACK_PEN );
				mBackgroundDC->SetBrush( *wxRED_BRUSH );
				Math::tVec2f orderedRange = fGetOrderedRange();
				orderedRange *= (f32)size.x;
				orderedRange += (f32)pos.x;
				mBackgroundDC->DrawRectangle( orderedRange.x, pos.y, orderedRange.y - orderedRange.x, fTickSpace( ) + 1 );

				mBackgroundDC->SetBrush( *wxBLUE_BRUSH );
				const f32 centerX = (orderedRange.x + orderedRange.y) / 2.f;
				mBackgroundDC->DrawCircle( centerX, 5, cHandlePixelRadius );
			}

			// Draw the small ticks.
			mBackgroundDC->SetPen( wxPen( wxColour( 68, 68, 72 ) ) );

			for( u32 i = 1; i <= 100; ++i )
			{
				const f32 t = ( f32 ) i / 100.f;
				fDrawTickTime( mBackgroundDC, t, cSmallTickHeight );
			}

			// Draw the medium ticks.
			mBackgroundDC->SetPen( wxPen( wxColour( 52, 52, 56 ) ) );
			mBackgroundDC->SetTextForeground( wxColour( 52, 52, 56 ) );
			mBackgroundDC->SetFont( cMedTickFont );

			for( u32 i = 1; i <= 20; ++i )
			{
				const f32 t = ( f32 ) i / 20.f;
				const f32 x = pos.x + ( f32 ) size.x * t;
				fDrawTickPos( mBackgroundDC, x, cMedTickHeight );

				if( i % 4 != 0 )
					fDrawTextPos( mBackgroundDC, x, t, 8 );
			}

			// Draw the large ticks.
			mBackgroundDC->SetPen( wxPen( wxColour( 24, 24, 30 ) ) );
			mBackgroundDC->SetTextForeground( wxColour( 24, 24, 30 ) );
			mBackgroundDC->SetFont( cBigTickFont );

			for( u32 i = 1; i <= 5; ++i )
			{
				const f32 t = ( f32 ) i / 5.f;
				const f32 x = pos.x + ( f32 ) size.x * t;
				fDrawTickPos( mBackgroundDC, x, fTickSpace() - 5 );
				fDrawTextPos( mBackgroundDC, x, t, 20 );
			}

			mBackgroundDC->SetBrush( *wxTRANSPARENT_BRUSH );
			mBackgroundDC->DrawRectangle( pos.x, pos.y, size.x + 1, fTickSpace( ) + 1 );
		}

		// Copy in the static background ticks.
		if( mBackgroundDC )
			dc.Blit( 0, 0, GetSize( ).x, GetSize( ).y, mBackgroundDC, 0, 0 );

		// Render the animation data events.
		if( mAnimMetaData )
		{
			// Clear and determine hovered.
			mHoveredEvent = mSingleSelectedEvent; // mSingleSelectedFrame is either -1 or a selection.
			for( u32 i = 0; i < mAnimMetaData->mKeyFrameTags.fCount() && mHoveredEvent == -1; ++i )
			{
				const f32 t = mAnimMetaData->mKeyFrameTags[ i ].mTime / fLifetime( );
				f32 x = pos.x + ( f32 ) size.x * t;

				// Determine which frame is hovered/draw a selected frame.
				if( (mLastMousePosition.x-4) <= x && x <= (mLastMousePosition.x+4) && mHoveredEvent == -1 )
				{
					mHoveredEvent = i;
					break;
				}
			}

			for( u32 i = 0; i < mAnimMetaData->mKeyFrameTags.fCount( ); ++i )
			{
				switch( mAnimMetaData->mKeyFrameTags[ i ].mEventTypeKey )
				{
				case 0: dc.SetPen( wxPen( wxColour( 67, 120, 210 ), 2 ) ); break;
				case 1: dc.SetPen( wxPen( wxColour( 255, 255, 0 ), 2 ) ); break;
				case 2: dc.SetPen( wxPen( wxColour( 0, 255, 255 ), 2 ) ); break;
				case 3: dc.SetPen( wxPen( wxColour( 0, 128, 255 ), 2 ) ); break;
				case 4: dc.SetPen( wxPen( wxColour( 128, 255, 0 ), 2 ) ); break;
				case 5: dc.SetPen( wxPen( wxColour( 0, 64, 128 ), 2 ) ); break;
				case 6: dc.SetPen( wxPen( wxColour( 95, 32, 255 ), 2 ) ); break;
				case 7: dc.SetPen( wxPen( wxColour( 190, 55, 150 ), 2 ) ); break;
				case 8: dc.SetPen( wxPen( wxColour( 255, 0, 0 ), 2 ) ); break;
				case 9: dc.SetPen( wxPen( wxColour( 25, 15, 255 ), 2 ) ); break;
				default:
					dc.SetPen( wxPen( wxColour( 0, 255, 0 ), 2 ) );
				}

				dc.SetFont( cMedTickFont );
				dc.SetTextForeground( wxColour( 24, 24, 30 ) );

				// These ts may go outside the range of the actual timeline as part of Multiselect movement. However,
				// these events will get clamped for real when the Multiselect is cleared.
				const f32 t = fClamp( mAnimMetaData->mKeyFrameTags[ i ].mTime / fLifetime( ), 0.f, 1.f );
				f32 x = pos.x + ( f32 ) size.x * t;
				f32 tickHeight = 13.f;

				// Determine which frame is hovered/draw a selected frame.
				const b32 multiSelected = fRangeIsSelected() && mMultiselectedEvents.fFind( i ) != NULL;

				if( mHoveredEvent == i || multiSelected )
				{
					wxColour epc = dc.GetPen().GetColour();
					dc.SetPen( wxPen( epc, 5 ) );
					tickHeight += 15.f;

					dc.SetFont( cMedTickFontBold );
				}

				// Determine pop.
				static const f32 cMaxHeight = 10.f;
				static const f32 cMaxRange = 40.f;

				const f32 signDiff = (mLastMousePosition.x - x) / cMaxRange;
				const f32 absDiff = fAbs( signDiff );

				if( absDiff <= 1.f )
				{
					const f32 diffAmount = 1.f - absDiff;
					tickHeight += cMaxHeight * diffAmount;
				}

				fDrawTickPos( &dc, x, tickHeight, 1.f );

				// Ghost out all non-selected/hovered frames if there is one.
				if( mHoveredEvent == i || mHoveredEvent == -1 )
				{
					const f32 y1 = pos.y + fTickSpace( ) - 1;
					wxString s = wxString( fKeyframeEventStringFromKey( mAnimMetaData->mKeyFrameTags[i].mEventTypeKey ) );
					s += ": ";
					s += wxString( mAnimMetaData->mKeyFrameTags[i].mTag );
						
					wxPoint tickFontSize;
					dc.GetTextExtent( s, &tickFontSize.x, &tickFontSize.y);
					dc.DrawText( s, x - (tickFontSize.x /2), y1 + (tickFontSize.y/2) - 2 );
				}
			}
		}

		// draw a little triangle thingie to indicate our current time in the Keyline...
		mScrub->fPaintScrub( &dc );
	}

	void tSigAnimTimeline::fOnEraseBackground( wxEraseEvent& event )
	{
	}

	void tSigAnimTimeline::fOnMouseMove( wxMouseEvent& event )
	{
		if( mAnimMetaData && mSingleSelectedEvent != -1 )
		{
			// Moving around a single event.
			const wxPoint pos = event.GetPosition();

			f32 t = fToT( pos.x );
			t = fClamp( t, 0.f, 1.f );

			const f32 time = t * fLifetime();
			mAnimMetaData->fSetTagTime( mSingleSelectedEvent, time );
		}
		else if( mMovingRange )
		{
			// Moving the multiselect range and all contained events.
			const f32 delta = (f32)(event.GetPosition().x - mLastMousePosition.x) / (f32)mTimelineDrawSize.x;
			mSelectionRange += delta;

			for( u32 i = 0; i < mMultiselectedEvents.fCount(); ++i )
			{
				const u32 eventNum = mMultiselectedEvents[i];
				const f32 existingTime = mAnimMetaData->mKeyFrameTags[ eventNum ].mTime;
				mAnimMetaData->fSetTagTime( eventNum, existingTime + delta * fLifetime() );
			}

			mDirty = true;
		}
		else if( mShiftHeld )
		{
			mDirty = true;

			// Either selecting a new range or adding to an existing range.
			if( mSelectingNewRange )
			{
				// Determining a new range from the anchor point.
				mSelectionRange.y = fToT( event.GetPosition().x );
				fSetSelectionRange(  mSelectionRange.x, mSelectionRange.y );
			}
			else
			{
				// Expanding the range.
				const f32 t = fToT( event.GetPosition().x );
				const Math::tVec2f ordered = fGetOrderedRange();
				if( mExpandingLeft )
					fSetSelectionRange( t, ordered.y );
				else if( mExpandingRight )
					fSetSelectionRange( ordered.x, t );
			}
		}
		// Standard scrubbing.
		else if( mScrub )
			mScrub->fOnMouseMove( event );
		mLastMousePosition = event.GetPosition( );
	}

	void tSigAnimTimeline::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		mContinueMouseInput = true;

		if( mAnimMetaData && event.ShiftDown() )
		{
			// Testing if we're selecting new or adding to existing.
			mShiftHeld = true;
			mSelectingNewRange = !fRangeIsSelected( );

			if( mSelectingNewRange )
			{
				const f32 t = fToT( event.GetPosition( ).x );
				fSetSelectionRange( t, t );
			}
			else
			{
				// add more or whatever who cares everything sucks
				const f32 t = fToT( event.GetPosition( ).x );
				const Math::tVec2f ordered = fGetOrderedRange();
				mSelectionRange = ordered;
				if( t < ordered.x )
				{
					fSetSelectionRange( t, ordered.y );
					mExpandingLeft = true;
				}
				else if( t > ordered.y )
				{
					fSetSelectionRange( ordered.x, t );
					mExpandingRight = true;
				}

				mDirty = true;
			}
		}
		else if( mAnimMetaData && fRangeIsSelected() )
		{
			// Testing if we're moving a range.
			const Math::tVec2f orderedRange = fGetOrderedRange();
			const f32 centerX = (orderedRange.x + orderedRange.y) / 2.f;

			const f32 t = fToT( event.GetPosition( ).x );
			const f32 halfRad = cHandlePixelRadius / (f32)mTimelineDrawSize.x;
			mMovingRange = (centerX - halfRad) <= t && t <= (centerX + halfRad);

			if( !mMovingRange )
				fClearSelectionRange();
		}
		else if( mAnimMetaData && mHoveredEvent != -1 && mSingleSelectedEvent == -1 && !fRangeIsSelected() )
		{
			// Single guy selected.
			mSingleSelectedEvent = mHoveredEvent;
		}
		else if( mScrub )
		{
			fClearSelectionRange( );
			mScrub->fOnMouseLeftButtonDown( event );
		}
	}

	void tSigAnimTimeline::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		mContinueMouseInput = false;

		if( mAnimMetaData && mSingleSelectedEvent != -1 )
		{
			mSingleSelectedEvent = -1;
		}
		else if( mShiftHeld )
		{
			// SHIFT HELD: no special upkeep but trap the event.
		}
		else if( mScrub )
			mScrub->fOnMouseLeftButtonUp( event );

		mShiftHeld = mExpandingLeft = mExpandingRight = mMovingRange = false;
	}

	void tSigAnimTimeline::fOnRightClick( wxMouseEvent& event )
	{
		tEditorContextAction::fDisplayContextMenuOnRightClick( this, event, mContextActions );
	}

	void tSigAnimTimeline::fOnAction( wxCommandEvent& event )
	{
		tEditorContextAction::fHandleContextActionFromRightClick( this, event, tEditorContextAction::fLastActionList() );
	}

	void tSigAnimTimeline::fOnPlayButtonClicked( wxCommandEvent& event )
	{
		if( mEntityControlPanel )
			mEntityControlPanel->fSetPaused( mEntityControlPanel->fPaused() );
	}

	void tSigAnimTimeline::fOnPlayButtonFocused( wxFocusEvent& event )
	{
	}

	void tSigAnimTimeline::fOnSize( wxSizeEvent& event )
	{
		fMarkTimelineDirty( );
		event.Skip( );
	}

	void tSigAnimTimeline::fOnMouseLeaveWindow( wxMouseEvent& event )
	{
		mForceMouseInput = mContinueMouseInput;
		if( !mContinueMouseInput )
			mLastMousePosition = wxPoint( 0, 0 );
	}

	void tSigAnimTimeline::fOnMouseEnterWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		SetFocus( );
	}

	void tSigAnimTimeline::fSyncPauseButton( )
	{
		if( mEntityControlPanel )
		{
			// What? Why does fPaused report the opposite?
			if( mEntityControlPanel->fPaused() )
				mPlay->SetLabel( "Playing" );
			else
				mPlay->SetLabel( "Paused" );
		}
	}

	void tSigAnimTimeline::fClearSelectionRange( )
	{
		// If any events are selected, this is when they are clamped to valid values to facilitate sliding
		// to and away from the end of the timeline.
		for( u32 i = 0; i < mMultiselectedEvents.fCount(); ++i )
		{
			const u32 eventNum = mMultiselectedEvents[i];
			const f32 clampedTime = fClamp( mAnimMetaData->mKeyFrameTags[ eventNum ].mTime, 0.f, fLifetime() );
			mAnimMetaData->fSetTagTime( eventNum, clampedTime );
		}

		mDirty = true;
		mSelectionRange = Math::tVec2f::cZeroVector;
		mMultiselectedEvents.fDeleteArray();
	}

	Math::tVec2f tSigAnimTimeline::fGetOrderedRange( ) const
	{
		Math::tVec2f ret = mSelectionRange;
		if( ret.x > ret.y )
			fSwap( ret.x, ret.y );

		ret.x = fClamp( ret.x, 0.f, 1.f );
		ret.y = fClamp( ret.y, 0.f, 1.f );

		return ret;
	}

	void tSigAnimTimeline::fSetSelectionRange( f32 l, f32 r )
	{
		mSelectionRange.x = l;
		mSelectionRange.y = r;

		mMultiselectedEvents.fSetCount( 0 );
		for( u32 i = 0; i < mAnimMetaData->mKeyFrameTags.fCount( ); ++i )
		{
			const f32 t = mAnimMetaData->mKeyFrameTags[ i ].mTime / fLifetime( );
			const Math::tVec2f range = fGetOrderedRange( );
			if( fInBounds( t, range.x, range.y ) )
				mMultiselectedEvents.fPushBack( i );
		}
	}

	void tSigAnimTimeline::fCopySelectedEvents( )
	{
		mEventClipboard.fDeleteArray();
		mEventClipboard.fSetCount( mMultiselectedEvents.fCount() );

		// Determine the earliest tag and all other tags will be stored relative to that one.
		f32 clipAnchor = fLifetime();

		for( u32 i = 0; i < mMultiselectedEvents.fCount(); ++i )
		{
			const u32 eIdx = mMultiselectedEvents[i];
			clipAnchor = fMin( clipAnchor, mAnimMetaData->mKeyFrameTags[eIdx].mTime );
		}

		for( u32 i = 0; i < mMultiselectedEvents.fCount(); ++i )
		{
			const u32 eIdx = mMultiselectedEvents[i];
			mEventClipboard[i] = mAnimMetaData->mKeyFrameTags[eIdx];
			mEventClipboard[i].mTime -= clipAnchor;
		}
	}

	void tSigAnimTimeline::fDeleteSelectedEvents( )
	{
		tGrowableArray< Anipk::tKeyFrameTag > tagsCopy( mAnimMetaData->mKeyFrameTags );
		mAnimMetaData->mKeyFrameTags.fDeleteArray();

		for( u32 i = 0; i < tagsCopy.fCount(); ++i )
		{
			if( mMultiselectedEvents.fFind(i) )
				continue;

			mAnimMetaData->mKeyFrameTags.fPushBack( tagsCopy[i] );
		}

		mMultiselectedEvents.fDeleteArray();

		fMarkTimelineDirty();
		mAnimMetaData->fSetDirty();
	}

	void tSigAnimTimeline::fPasteEvents( f32 pasteAnchorPos )
	{
		// Paste these events in based on the right click pos or where the range is selected.
		const f32 t = fRangeIsSelected() ? fGetOrderedRange().x : fToT( pasteAnchorPos );
		const f32 pasteAnchorTime = t * fLifetime();

		for( u32 i = 0; i < mEventClipboard.fCount(); ++i )
		{
			mAnimMetaData->mKeyFrameTags.fPushBack( mEventClipboard[i] );
			mAnimMetaData->mKeyFrameTags.fBack().mTime += pasteAnchorTime;
		}
		
		mEventClipboard.fDeleteArray();
		fMarkTimelineDirty();
		mAnimMetaData->fSetDirty();
	}

	tSigAnimTimelineScrub::tSigAnimTimelineScrub( tSigAnimTimeline* timeline )
		: mParentTimeline( timeline )
		, mX( 0.f )
		, mTime( 0.f )
		, mMouseOver( false )
		, mMouseHookedOn( false )
		, mLastMousePosition( 0, 0 )
	{

	}

	tSigAnimTimelineScrub::~tSigAnimTimelineScrub( )
	{
	}


	f32 tSigAnimTimelineScrub::fGetTime( ) const
	{
		return mParentTimeline->fLifetime( ) * mX;
	}


	void tSigAnimTimelineScrub::fSetXThruTime( const f32 time )
	{
		mX = time / mParentTimeline->fLifetime( );
		mX = fClamp( mX, 0.f, 1.f );
	}

	void tSigAnimTimelineScrub::fPaintScrub( wxAutoBufferedPaintDC* dc )
	{
		const u32 keylinexpos = mParentTimeline->fGetStartingPosition( ).x;
		const u32 x = keylinexpos + mParentTimeline->fGetTimelineDrawSize( ).x * mX;
		const u32 y1 = mParentTimeline->fGetTimelineDrawSize( ).y - 19;		//subtracting '19' is arbitrary and was chosen because it just makes the things line up right!
		const u32 y2 = y1 + 11;

		mPoints[ 0 ] = wxPoint( x-6, y2 );
		mPoints[ 1 ] = wxPoint( x+6, y2 );
		mPoints[ 2 ] = wxPoint( x, y1 );

		wxRect rc( mPoints[ 0 ].x, mPoints[ 2 ].y, 12, 11 );
		if( rc.Contains( mLastMousePosition ) )
			mMouseOver = true;
		else
			mMouseOver = false;

		if( mMouseOver || mMouseHookedOn )
		{
			dc->SetBrush( *wxRED_BRUSH );
		}
		else
		{
			dc->SetBrush( wxBrush( wxColour( 200, 100, 100 ) ) );
		}

		dc->SetPen( wxPen( wxColour( 24, 24, 30 ) ) );

		dc->DrawPolygon( 3, mPoints );
	}

	b32 tSigAnimTimelineScrub::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		// Anytime you click in the timeline area you are setting the scrub to that location.
		mMouseHookedOn = true;
		fOnMouseMove( event );
		return true;
	}

	void tSigAnimTimelineScrub::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		mMouseHookedOn = false;
	}

	void tSigAnimTimelineScrub::fOnMouseMove( wxMouseEvent& event )
	{
		if( event.LeftIsDown( ) )
		{
			if( mMouseHookedOn )
			{
				f32 x = ( f32 )( event.GetPosition( ).x - mParentTimeline->fGetStartingPosition( ).x ) / ( f32 )mParentTimeline->fGetTimelineDrawSize( ).x;
				x = fClamp( x, 0.f, 1.f );
				
				mX = x;
				mParentTimeline->fSetUpdateSkeletonToScrub( );
			}
		}

		mLastMousePosition = event.GetPosition( );
	}

}

