#include "SigAnimPch.hpp"
#include "tSigAnimTimeline.hpp"
#include "tSigAnimMainWindow.hpp"
#include "tEntityControlPanel.hpp"

namespace Sig
{


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
	{
		Connect( wxEVT_PAINT, wxPaintEventHandler( tSigAnimTimeline::fOnPaint ) );
		Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( tSigAnimTimeline::fOnEraseBackground ) );
		
		Connect( wxEVT_MOTION, wxMouseEventHandler( tSigAnimTimeline::fOnMouseMove ) );
		Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( tSigAnimTimeline::fOnMouseLeftButtonDown ) );
		Connect( wxEVT_LEFT_UP, wxMouseEventHandler( tSigAnimTimeline::fOnMouseLeftButtonUp ) );

		Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( tSigAnimTimeline::fOnMouseLeaveWindow ) );
		Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( tSigAnimTimeline::fOnMouseEnterWindow ) );
		
		Connect( wxEVT_SIZE, wxSizeEventHandler( tSigAnimTimeline::fOnSize ), NULL, this );

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
		mPlay->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigAnimTimeline::fOnPlayButtonClicked ), NULL, this );
		mPlay->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( tSigAnimTimeline::fOnPlayButtonFocused ), NULL, this );
		
		mChoice = new wxChoice( this, wxID_ANY, wxDefaultPosition, wxSize( 92, 20 ), 9, times );
		mChoice->SetSelection( 5 );
		mChoice->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tSigAnimTimeline::fOnTimelineLengthChanged ), NULL, this );

		wxBoxSizer* top = new wxBoxSizer( wxVERTICAL );
		top->Add( mPlay, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxTOP, 8 );
		top->Add( mChoice, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT | wxBOTTOM | wxTOP, 8 );

		wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL);
		sizer->Add( top, 0, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxEXPAND, 1 );
		SetSizer( sizer );

		SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	}

	tSigAnimTimeline::~tSigAnimTimeline( )
	{
		delete mScrub;
	}

	void tSigAnimTimeline::fOnSize( wxSizeEvent& event )
	{
		fMarkTimelineDirty( );
		event.Skip( );
	}

	void tSigAnimTimeline::fOnMouseLeaveWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		if( mContinueMouseInput )
			mForceMouseInput = true;
	}

	void tSigAnimTimeline::fOnMouseEnterWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		SetFocus( );
	}

	void tSigAnimTimeline::fOnPlayButtonClicked( wxCommandEvent& event )
	{
		if( mEntityControlPanel )
			mEntityControlPanel->fSetPaused( mEntityControlPanel->fPaused( ) );
	}
	
	void tSigAnimTimeline::fOnPlayButtonFocused( wxFocusEvent& event )
	{
		
	}

	void tSigAnimTimeline::fOnTimelineLengthChanged( wxCommandEvent& event )
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

		//mKeyline->fSetLifetime( times[ idx ] );
		//mKeyline->fMainWindow( )->fGuiApp( ).fActionStack( ).fForceSetDirty( );
	}

	void tSigAnimTimeline::fOnMouseMove( wxMouseEvent& event )
	{
		if( mScrub )
			mScrub->fOnMouseMove( event );
		mLastMousePosition = event.GetPosition( );
	}

	void tSigAnimTimeline::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		mContinueMouseInput = true;

		if( mScrub )
			mScrub->fOnMouseLeftButtonDown( event );
	}

	void tSigAnimTimeline::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		mContinueMouseInput = false;

		if( mScrub )
			mScrub->fOnMouseLeftButtonUp( event );
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
	}

	void tSigAnimTimeline::fOnEraseBackground( wxEraseEvent& event )
	{
		
	}

	namespace
	{
		static const wxFont cTickFontSmall( 7.25, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
		static const wxFont cTickFontMedium( 8.5, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
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
				f32 time = fLifetime( ) * delta;
				wxString s = wxString::Format( "%.2f", time );
				s = s.Trim( );

				if( i % 4 == 0 )
				{
					mBackgroundDC->SetTextForeground( wxColour( 24, 24, 30 ) );
					mBackgroundDC->SetFont( cTickFontMedium );
					y1 -= 12;
				}
				else
				{
					mBackgroundDC->SetTextForeground( wxColour( 52, 52, 56 ) );
					mBackgroundDC->SetFont( cTickFontSmall );
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

				f32 time = fLifetime( ) * delta;
				wxString s = wxString::Format( "%.1f", time );
				wxPoint tickFontSize;
				mBackgroundDC->GetTextExtent( s, &tickFontSize.x, &tickFontSize.y);
				//dc.DrawText( s, x - tickFontSize.x / 2, y2 );// - tickFontSize.y * 1.0f );

				mBackgroundDC->DrawLine( x, y1, x, y2 );
			}

			mBackgroundDC->SetBrush( *wxTRANSPARENT_BRUSH );
			mBackgroundDC->DrawRectangle( pos.x, pos.y, size.x + 1, fTickSpace( ) + 1 );


			if( mAnimMetaData )		// draw lines for any animation events on the current anim!!!
			{
				for( u32 i = 0; i < mAnimMetaData->mKeyFrameTags.fCount( ); ++i )
				{
					switch( mAnimMetaData->mKeyFrameTags[ i ].mEventTypeKey )
					{
					case 0: mBackgroundDC->SetPen( wxPen( wxColour( 67, 120, 210 ), 2 ) ); break;
					case 1: mBackgroundDC->SetPen( wxPen( wxColour( 255, 255, 0 ), 2 ) ); break;
					case 2: mBackgroundDC->SetPen( wxPen( wxColour( 0, 255, 255 ), 2 ) ); break;
					case 3: mBackgroundDC->SetPen( wxPen( wxColour( 0, 128, 255 ), 2 ) ); break;
					case 4: mBackgroundDC->SetPen( wxPen( wxColour( 128, 255, 0 ), 2 ) ); break;
					case 5: mBackgroundDC->SetPen( wxPen( wxColour( 0, 64, 128 ), 2 ) ); break;
					case 6: mBackgroundDC->SetPen( wxPen( wxColour( 95, 32, 255 ), 2 ) ); break;
					case 7: mBackgroundDC->SetPen( wxPen( wxColour( 190, 55, 150 ), 2 ) ); break;
					case 8: mBackgroundDC->SetPen( wxPen( wxColour( 255, 0, 0 ), 2 ) ); break;
					case 9: mBackgroundDC->SetPen( wxPen( wxColour( 25, 15, 255 ), 2 ) ); break;
					default:
						mBackgroundDC->SetPen( wxPen( wxColour( 0, 255, 0 ), 2 ) );
					}

					f32 time = mAnimMetaData->mKeyFrameTags[ i ].mTime;
					f32 delta = time / fLifetime( );
					const f32 x = pos.x + ( f32 ) size.x * delta;
					f32 y1 = pos.y + fTickSpace( ) - 1;
					f32 y2 = y1 - 13;
					mBackgroundDC->DrawLine( x, y1, x, y2 );

					mBackgroundDC->SetFont( cTickFontSmall );
					wxString s = wxString( mAnimMetaData->mKeyFrameTags[ i ].mTag );
					wxPoint tickFontSize;
					mBackgroundDC->GetTextExtent( s, &tickFontSize.x, &tickFontSize.y);
					mBackgroundDC->DrawText( s, x - (tickFontSize.x /2), y1 + (tickFontSize.y/2) - 2 );

					//fAddKeyframeEventToListBox( mAnimMetaData->mKeyFrameTags[ i ].mTime, fKeyframeEventStringFromKey( mAnimMetaData->mKeyFrameTags[ i ].mEventTypeKey ), mAnimMetaData->mKeyFrameTags[ i ].mTag );
				}
			}
		}

		if( mBackgroundDC )
			dc.Blit( 0, 0, GetSize( ).x, GetSize( ).y, mBackgroundDC, 0, 0 );

		// draw a little triangle thingie to indicate our current time in the Keyline...
		mScrub->fPaintScrub( &dc );
	}

	f32 tSigAnimTimeline::fLifetime( )
	{
		return mCurrentTimelineLength;
	}

	void tSigAnimTimeline::fSetLifetime( const f32 lifetime )
	{
		mCurrentTimelineLength = lifetime;
	}


	void tSigAnimTimeline::fSetAnimEvents( const std::string& animPkLabel, const std::string& animName, Anipk::tFile& anipkFile, const tKeyFrameAnimation* kfAnim )
	{
		mAnimMetaData = &anipkFile.fFindOrAddAnimMetaData( animName );
		fMarkTimelineDirty( );
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
		if( mX > 1.f )
			mX = 0.f;
		if( mX < 0.f )
			mX = 1.f;
	}

	void tSigAnimTimelineScrub::fPaintLine( wxAutoBufferedPaintDC* dc )
	{
		/*
		if( mMouseOver || mMouseHookedOn )
			dc->SetPen( *wxRED_PEN );
		else
			dc->SetPen( *wxBLACK_PEN );

		const u32 x = mParentKeyline->fGetKeylinePosition( ).x + mParentKeyline->fGetKeylineSize( ).x * mX;
		const u32 y1 = mParentKeyline->fGetKeylinePosition( ).y;
		dc->DrawLine( x, y1, x, y1 + mParentKeyline->fGetKeylineSize( ).y );
		*/
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
			//dc->SetBrush( *wxTRANSPARENT_BRUSH );
		}

		dc->DrawPolygon( 3, mPoints );
	}

	b32 tSigAnimTimelineScrub::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		//if( mMouseOver )		// disable this so anytime you click in the timeline area you are setting the scrup to that location.
		{
			mMouseHookedOn = true;
			fOnMouseMove( event );
			return true;
		}
		
		return false;
	}

	void tSigAnimTimelineScrub::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		mMouseHookedOn = false;
		if( mMouseOver )
		{

		}
	}

	void tSigAnimTimelineScrub::fOnMouseMove( wxMouseEvent& event )
	{
		if( event.LeftIsDown( ) )
		{
			if( mMouseHookedOn )
			{
				f32 x = ( f32 )( event.GetPosition( ).x - mParentTimeline->fGetStartingPosition( ).x ) / ( f32 )mParentTimeline->fGetTimelineDrawSize( ).x;
				if( x < 0.f )
					x = 0.f;
				if( x > 1.f )
					x = 1.f;
				
				mX = x;
				mParentTimeline->fSetUpdateSkeletonToScrub( );
			}
		}

		mLastMousePosition = event.GetPosition( );
	}

}

