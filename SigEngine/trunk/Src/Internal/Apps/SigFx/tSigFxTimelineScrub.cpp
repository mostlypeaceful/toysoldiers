#include "SigFxPch.hpp"
#include "tSigFxTimelineScrub.hpp"
#include "tSigFxKeyline.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{


	tSigFxTimeline::tSigFxTimeline( wxPanel* parent )
		: wxPanel( parent, wxID_ANY, wxDefaultPosition, wxSize( wxDefaultSize.x, 36 ) )
		, mKeyline( 0 )
		, mScrub( 0 )
		, mForceMouseInput( false )
		, mContinueMouseInput( false )
		, mBackgroundDC( 0 )
		, mDirty( true )
		, mLastTimelineLength( 0.f )
	{
		Connect( wxEVT_PAINT, wxPaintEventHandler( tSigFxTimeline::fOnPaint ) );
		Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( tSigFxTimeline::fOnEraseBackground ) );
		
		Connect( wxEVT_MOTION, wxMouseEventHandler( tSigFxTimeline::fOnMouseMove ) );
		Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( tSigFxTimeline::fOnMouseLeftButtonDown ) );
		Connect( wxEVT_LEFT_UP, wxMouseEventHandler( tSigFxTimeline::fOnMouseLeftButtonUp ) );

		Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( tSigFxTimeline::fOnMouseLeaveWindow ) );
		Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( tSigFxTimeline::fOnMouseEnterWindow ) );
		
		Connect( wxEVT_SIZE, wxSizeEventHandler( tSigFxTimeline::fOnSize ), NULL, this );

		wxString times[ ] = 
		{
			wxString( "0.25" ),
			wxString( "0.5" ),
			wxString( "1.0" ),
			wxString( "2.5" ),
			wxString( "3.0" ),
			wxString( "5.0" ),
			wxString( "10.0" ),
			wxString( "30.0" ),
			wxString( "60.0" )
		};

		mPlay = new wxButton( this, wxID_ANY, "Paused", wxDefaultPosition, wxSize( 92, 20 ), 0 );
		mPlay->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigFxTimeline::fOnPlayButtonClicked ), NULL, this );
		mPlay->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( tSigFxTimeline::fOnPlayButtonFocused ), NULL, this );
		
		if( mKeyline )
		{
			mLastTimelineState = mKeyline->fPaused( );
			if( mKeyline->fPaused( ) )
				mPlay->SetLabel( wxString( "Play" ) );
		}

		mChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 92, 20 ), 9, times );
		mChoice->SetSelection( 5 );
		mChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tSigFxTimeline::fOnTimelineLengthChanged ), NULL, this );

		wxBoxSizer* top = new wxBoxSizer( wxVERTICAL );
		top->Add( mPlay, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxTOP, 4 );
		top->Add( mChoice, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM | wxTOP, 4 );

		wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL);
		sizer->Add( top, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxEXPAND, 1 );
		SetSizer( sizer );

		SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	}

	tSigFxTimeline::~tSigFxTimeline( )
	{
		delete mScrub;
	}

	void tSigFxTimeline::fOnSize( wxSizeEvent& event )
	{
		mDirty = true;
		event.Skip( );
	}

	void tSigFxTimeline::fOnMouseLeaveWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		if( mContinueMouseInput )
			mForceMouseInput = true;
	}

	void tSigFxTimeline::fOnMouseEnterWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		SetFocus( );
	}

	void tSigFxTimeline::fOnPlayButtonClicked( wxCommandEvent& event )
	{
		if( mKeyline )
		{
			mKeyline->fSetHotKeyPaused( !mKeyline->fPaused( ) );
		}
	}
	
	void tSigFxTimeline::fOnPlayButtonFocused( wxFocusEvent& event )
	{
		
	}

	void tSigFxTimeline::fOnTimelineLengthChanged( wxCommandEvent& event )
	{
		u32 idx = mChoice->GetSelection( );

		f32 times[ ] = 
		{
			0.25f,
			0.5f,
			1.0f,
			2.5f,
			3.0f,
			5.0f,
			10.0f,
			30.0f,
			60.0f
		};

		mKeyline->fSetLifetime( times[ idx ] );
		mKeyline->fMainWindow( )->fGuiApp( ).fActionStack( ).fForceSetDirty( );
	}

	void tSigFxTimeline::fOnMouseMove( wxMouseEvent& event )
	{
		if( mScrub )
			mScrub->fOnMouseMove( event );
		mLastMousePosition = event.GetPosition( );
	}

	void tSigFxTimeline::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		mContinueMouseInput = true;

		if( mScrub )
			mScrub->fOnMouseLeftButtonDown( event );
	}

	void tSigFxTimeline::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		mContinueMouseInput = false;

		if( mScrub )
			mScrub->fOnMouseLeftButtonUp( event );
	}

	void tSigFxTimeline::fOnTick( f32 dt )
	{
		if( mLastTimelineState != mKeyline->fPaused( ) )
		{
			mLastTimelineState = mKeyline->fPaused( );
			if( mKeyline->fPaused( ) )
				mPlay->SetLabel( wxString( "Play" ) );
			else
				mPlay->SetLabel( wxString( "Pause" ) );
		}

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

		mKeyline->fClearHotKeyPausedState( );
		Refresh( );
	}

	void tSigFxTimeline::fOnEraseBackground( wxEraseEvent& event )
	{
		
	}

	namespace
	{
		static const wxFont cMedTickFont( 7.25, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
		static const wxFont cBigTickFont( 8.5, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
	}

	void tSigFxTimeline::fOnPaint( wxPaintEvent& event )
	{
		if( !mKeyline )	return;
		if( !mScrub )
		{
			mScrub = new tSigFxTimelineScrub( this, mKeyline );
			mKeyline->fSetScrub( mScrub );
		}

		wxAutoBufferedPaintDC dc( this );

		const wxPoint pos = mKeyline->fGetKeylinePosition( ) + wxPoint( 4, 0 );
		const wxSize size = mKeyline->fGetKeylineSize( );

		b32 inupperdeck = false;
		f32 upperx = 0.f;

		/*if( IsMouseInWindow( ) )
		{
			inupperdeck = true;
			upperx = ( f32 ) ( mLastMousePosition.x - pos.x )  / ( f32 ) size.x;
		}*/

		//dc.SetFont( tickFontSmall );
		//f32 increment = mKeyline->fLifetime( ) * 0.0125f;

		f32 lifetime = mKeyline->fLifetime( );
		if( mLastTimelineLength != lifetime )
			mDirty = true;

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

			mBackgroundDC->SetPen( wxPen( wxColour( 68, 68, 72 ) ) );
			// draw the small ticks
			const f32 spacer = 0.0125f / 2.f;
			for( u32 i = 1; i <= 100; ++i )
			{
				f32 delta = ( f32 ) i / 100.f;
				const f32 x = pos.x + ( f32 ) size.x * delta;
				const f32 y1 = pos.y + fTickSpace( );
				const f32 y2 = y1 - 6;
				mBackgroundDC->DrawLine( x, y1, x, y2 );
			}

			mBackgroundDC->SetPen( wxPen( wxColour( 52, 52, 56 ) ) );
			
			// draw the medium ticks
			for( u32 i = 1; i <= 20; ++i )
			{
				const f32 delta = ( f32 ) i / 20.f;
				const f32 x = pos.x + ( f32 ) size.x * delta;
				f32 y1 = pos.y + fTickSpace( );
				f32 y2 = y1 - 18;	//12
				mBackgroundDC->DrawLine( x, y1, x, y2 );

				y1 -= 8;
				f32 time = mKeyline->fLifetime( ) * delta;
				wxString s = wxString::Format( "%.2f", time );
				s = s.Trim( );

				if( i % 4 == 0 )
				{
					mBackgroundDC->SetTextForeground( wxColour( 24, 24, 30 ) );
					mBackgroundDC->SetFont( cBigTickFont );
					y1 -= 12;
				}
				else
				{
					mBackgroundDC->SetTextForeground( wxColour( 52, 52, 56 ) );
					mBackgroundDC->SetFont( cMedTickFont );
				}

				wxPoint tickFontSize;
				mBackgroundDC->GetTextExtent( s, &tickFontSize.x, &tickFontSize.y);
				mBackgroundDC->DrawText( s, x - tickFontSize.x - 1, y1 - tickFontSize.y );
			}

			mBackgroundDC->SetPen( wxPen( wxColour( 24, 24, 30 ) ) );
			
			for( u32 i = 1; i <= 5; ++i )
			{
				const f32 delta = ( f32 ) i / 5.f;
				const f32 x = pos.x + ( f32 ) size.x * delta;
			
				const f32 y1 = pos.y + fTickSpace( );
				const f32 y2 = y1 - fTickSpace( ) + 5;

				f32 time = mKeyline->fLifetime( ) * delta;
				wxString s = wxString::Format( "%.1f", time );
				wxPoint tickFontSize;
				mBackgroundDC->GetTextExtent( s, &tickFontSize.x, &tickFontSize.y);
				//dc.DrawText( s, x - tickFontSize.x / 2, y2 );// - tickFontSize.y * 1.0f );

				mBackgroundDC->DrawLine( x, y1, x, y2 );
			}

			mBackgroundDC->SetBrush( *wxTRANSPARENT_BRUSH );
			mBackgroundDC->DrawRectangle( pos.x, pos.y, size.x + 1, fTickSpace( ) + 1 );
		}

		if( mBackgroundDC )
			dc.Blit( 0, 0, GetSize( ).x, GetSize( ).y, mBackgroundDC, 0, 0 );

		// draw a little triangle thingie to indicate our current time in the Keyline...
		mScrub->fPaintScrub( &dc );
	}

	f32 tSigFxTimeline::fLifetime( )
	{
		if( mKeyline )
			return mKeyline->fLifetime( );
		return 0.f;
	}

	void tSigFxTimeline::fSetLifetime( const f32 lifetime )
	{
		if( mKeyline )
			mKeyline->fSetLifetime( lifetime );

		f32 times[ ] = 
		{
			0.25f,
			0.5f,
			1.0f,
			2.5f,
			3.0f,
			5.0f,
			10.0f,
			30.0f,
			60.0f
		};

		u32 idx = 8;

		for( u32 i = 0; i < 9; ++i )
		{
			if( times[ i ] >= lifetime )
			{
				idx = i;
				break;
			}
		}

		mChoice->SetSelection( idx );
	}


	tSigFxTimelineScrub::tSigFxTimelineScrub( tSigFxTimeline* timeline, tSigFxKeyline* parent )
		: mParentTimeline( timeline )
		, mParentKeyline( parent )
		, mX( 0.f )
		, mTime( 0.f )
		, mMouseOver( false )
		, mMouseHookedOn( false )
		, mLastMousePosition( 0, 0 )
	{

	}

	tSigFxTimelineScrub::~tSigFxTimelineScrub( )
	{

	}


	f32 tSigFxTimelineScrub::fGetTime( ) const
	{
		return mParentKeyline->fLifetime( ) * mX;
	}
	
	void tSigFxTimelineScrub::fSetX( f32 x )
	{
		mX = x;
		const f32 time = mParentKeyline->fLifetime( ) * x;
		mParentKeyline->fSetCurrentTime( time );

	}
	void tSigFxTimelineScrub::fSetXThruTime( const f32 time )
	{
		mX = time / mParentKeyline->fLifetime( );
	}

	void tSigFxTimelineScrub::fPaintLine( wxAutoBufferedPaintDC* dc )
	{
		if( mMouseOver || mMouseHookedOn )
			dc->SetPen( *wxRED_PEN );
		else
			dc->SetPen( *wxBLACK_PEN );

		const u32 x = mParentKeyline->fGetKeylinePosition( ).x + mParentKeyline->fGetKeylineSize( ).x * mX;
		const u32 y1 = mParentKeyline->fGetKeylinePosition( ).y;
		dc->DrawLine( x, y1, x, y1 + mParentKeyline->fGetKeylineSize( ).y );
	}

	void tSigFxTimelineScrub::fPaintScrub( wxAutoBufferedPaintDC* dc )
	{
		const u32 keylinexpos = mParentKeyline->fGetKeylinePosition( ).x + 4;
		const u32 x = keylinexpos + mParentKeyline->fGetKeylineSize( ).x * mX;
		const u32 y1 = mParentKeyline->fGetKeylinePosition( ).y + mParentTimeline->fTickSpace( ) - 2;
		const u32 y2 = y1 - 11;

		mPoints[ 0 ] = wxPoint( x-6, y2 );
		mPoints[ 1 ] = wxPoint( x+6, y2 );
		mPoints[ 2 ] = wxPoint( x, y1 );

		wxRect rc( mPoints[ 0 ].x, mPoints[ 0 ].y, 12, 11 );
		if( rc.Contains( mLastMousePosition ) )
			mMouseOver = true;
		else
			mMouseOver = false;

		if( mMouseOver || mMouseHookedOn )
		{
			dc->SetBrush( *wxRED_BRUSH );
			//dc->SetPen( *wxBLACK_PEN );
		}
		else
		{
			dc->SetBrush( *wxTRANSPARENT_BRUSH );
			//dc->SetPen( *wxRED_PEN );
		}

		//dc->SetBrush( *wxTRANSPARENT_BRUSH );
		dc->DrawPolygon( 3, mPoints );
	}

	b32 tSigFxTimelineScrub::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		//if( mMouseOver )		// disable this so anytime you click in the timeline area you are setting the scrup to that location.
		{
			mMouseHookedOn = true;
			mParentKeyline->fSetPaused( true );
			mParentKeyline->fDeselectAllKeyframes( );
			fOnMouseMove( event );
			return true;
		}
		
		return false;
	}

	void tSigFxTimelineScrub::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		mMouseHookedOn = false;
		if( mMouseOver )
		{

		}
	}

	void tSigFxTimelineScrub::fOnMouseMove( wxMouseEvent& event )
	{
		if( event.LeftIsDown( ) )
		{
			if( mMouseHookedOn )//&& mParentKeyline->fPaused( ) )
			{
				f32 x = ( f32 )( event.GetPosition( ).x - mParentKeyline->fGetKeylinePosition( ).x ) / ( f32 )mParentKeyline->fGetKeylineSize( ).x;
				if( x < 0.f )
					x = 0.f;
				if( x > 1.f )
					x = 1.f;
				fSetX( x );
			}
		}

		mLastMousePosition = event.GetPosition( );
	}

}

