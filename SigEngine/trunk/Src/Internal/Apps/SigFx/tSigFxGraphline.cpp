#include "SigFxPch.hpp"
#include "tSigFxGraphline.hpp"
#include "FxEditor/tSigFxAttractor.hpp"
#include "FxEditor/tSigFxMeshSystem.hpp"
#include "tFxEditorActions.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{

u32 tGenerateSpinMenu::mSpinCounts[ tGenerateSpinMenu::cSpinOptions ] = { 1, 2, 3, 4, 5, 10, 12, 15, 18, 20 };

wxFont tGraphButton::mFontNom( 8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL | wxFONTFLAG_ANTIALIASED, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
wxFont tGraphButton::mFontHil( 8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL | wxFONTFLAG_ANTIALIASED, wxFONTWEIGHT_NORMAL, true, wxT( "Georgia" ) );
wxFont tGraphButton::mFontSel( 10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL | wxFONTFLAG_ANTIALIASED, wxFONTWEIGHT_BOLD, true, wxT( "Georgia" ) );

	tGraphButton::tGraphButton( )
	{

	}

	tGraphButton::tGraphButton( wxPoint pos, wxSize size, wxString text, wxColour col, wxColour hil, wxColour sel )
		: mMouseOver( false )
		, mSelected( false )
		, mPosition( pos )
		, mSize( size )
		, mText( text )
		, mColor( col )
		, mHighlight( hil )
		, mSelectedCol( sel )
	{

	}

	void tGraphButton::fPaint( wxAutoBufferedPaintDC& dc )
	{
		wxRect bounds( mPosition.x, mPosition.y, mSize.x, mSize.y );
		u32 cx = mPosition.x + mSize.x * 0.5f;
		u32 cy = mPosition.y + mSize.y * 0.5f;

		u32 radius = 6;
		if( mSelected )
		{
			dc.SetFont( mFontSel );
			dc.SetBrush( wxBrush( mSelectedCol ) );
			radius = 7;
		}
		else if( mMouseOver )
		{
			dc.SetFont( mFontHil );
			dc.SetBrush( wxBrush( mHighlight ) );
			radius = 7;
		}
		else
		{
			dc.SetFont( mFontNom );
			dc.SetBrush( wxBrush( mColor ) );
		}

		dc.DrawCircle( mPosition.x + 12, cy, radius );
		
		wxSize textdim;
		dc.GetTextExtent( mText, &textdim.x, &textdim.y);
		
		if( mSelected )
		{
			//wxBrush br( wxColour( 255, 128, 64, 128 ) );
			dc.DrawRectangle( mPosition.x+23, cy - textdim.y/2 - 2, textdim.x+5, textdim.y + 6);
		}
		
		//dc.DrawText( mText, cx - textdim.x/2, cy - textdim.y/2 );
		dc.DrawText( mText, mPosition.x + 25, cy - textdim.y/2 );
	}
		
	b32 tGraphButton::fMouseOver( wxMouseEvent& event )
	{
		mMouseOver = false;

		wxRect bounds( mPosition.x, mPosition.y, mSize.x, mSize.y );
		
		if( bounds.Contains( event.GetPosition( ) ) )
			mMouseOver = true;

		return mMouseOver;
	}

	b32 tGraphButton::fMouseClick( wxMouseEvent& event )
	{
		mSelected = false;

		if( event.LeftIsDown( ) )
		{
			wxRect bounds( mPosition.x, mPosition.y, mSize.x, mSize.y );
			
			if( bounds.Contains( event.GetPosition( ) ) )
			{
				mSelected = true;
				return true;
			}
		}

		return false;
	}


	tGraphlineContextMenu::tGraphlineContextMenu( tSigFxGraph* graph, wxPoint clickPos )
		: wxMenu( wxString( "" ) )
		, mGraph( graph )
		, mMouseClick( clickPos )
	{
		Append( cFrame, wxString( "Frame" ) );
		Append( cSetRandomness, wxString( "Set Randomness" ) );

		u32 id = ( *mGraph->fRawGraph( ) )->fGetID( );

		Append( cAllOnes, wxString( "All Ones" ) );
		Append( cAllZeroes, wxString( "All Zeroes" ) );
		Append( cEvenSpacing, wxString( "Even Spacing" ) );

		if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
		{
			Append( cXAxis, wxString( "Make X Axis" ) );
			Append( cYAxis, wxString( "Make Y Axis" ) );
		}
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
		{
			Append( cXAxis, wxString( "Make X Axis" ) );
			Append( cYAxis, wxString( "Make Y Axis" ) );
			Append( cZAxis, wxString( "Make Z Axis" ) );
			Append( cNormalize, wxString( "Normalize" ) );
		}
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
		{
			Append( cXAxis, wxString( "Make X Axis" ) );
			Append( cYAxis, wxString( "Make Y Axis" ) );
			Append( cZAxis, wxString( "Make Z Axis" ) );
			Append( cWAxis, wxString( "Make W Axis" ) );
			Append( cNormalize, wxString( "Normalize" ) );
		}
	
		if( graph->fSpatialGraph( ) )
		{
			wxMenu* submenu = new wxMenu( "" );
			wxMenu* spin = new wxMenu( "" );
			
			tGenerateSpinMenu* xSpinMenu = new tGenerateSpinMenu( graph, cXSpin );
			tGenerateSpinMenu* ySpinMenu = new tGenerateSpinMenu( graph, cYSpin );
			tGenerateSpinMenu* zSpinMenu = new tGenerateSpinMenu( graph, cZSpin );

			spin->AppendSubMenu( xSpinMenu, "Around The X-Axis" );
			spin->AppendSubMenu( ySpinMenu, "Around The Y-Axis" );
			spin->AppendSubMenu( zSpinMenu, "Around The Z-Axis" );

			wxMenu* line = new wxMenu( "" );
			line->Append( cGenerateXLine, "Along The X-Axis" );
			line->Append( cGenerateYLine, "Along The Y-Axis" );
			line->Append( cGenerateZLine, "Along The Z-Axis" );

			submenu->AppendSubMenu( spin, "Spin" );
			submenu->AppendSubMenu( line, "Line" );
			AppendSubMenu( submenu, "Generate" );

			Append( cFlipX, wxString( "Flip X" ) );
			Append( cFlipY, wxString( "Flip Y" ) );
		}


		Append( cCopyGraph, wxString( "Copy Graph" ) );
		wxMenuItem* pasteItem = Append( cPasteGraph, wxString( "Paste Graph" ) );
		pasteItem->Enable( graph->fSigFxMainWindow( )->fSigFxClipboard( )->fCanPaste( graph ) == true );

		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tGraphlineContextMenu::fOnMenuItemSelected ) );
	}

	void tGraphlineContextMenu::fOnMenuItemSelected( wxCommandEvent& event )
	{
		u32 id = event.GetId( );
		u32 graphID = ( *mGraph->fRawGraph( ) )->fGetID( );

		if( id == cFrame )
		{
			mGraph->fFrame( );
		}
		else if( id == cSetRandomness )
		{
			tRandomnessWindow* window = new tRandomnessWindow( 0, mGraph, mMouseClick );
			window->ShowModal( );
		}
		else if( id == cEvenSpacing )
		{
			f32 numKeys = ( f32 ) mGraph->fNumKeyframes( );
			if( numKeys > 1 )
			{
				f32 delta = 1.f / ( numKeys - 1.f );
				for( f32 i = 0; i < numKeys; ++i )
					mGraph->fRawKeyframe( i )->fSetX( delta * i );
			}
		}
		else if( id >= cAllOnes && id <= cNormalize && id != cEvenSpacing )
		{
			for( u32 i = 0; i < mGraph->fNumKeyframes( ); ++i )
			{
				tKeyframePtr key = mGraph->fRawKeyframe( i );

				if( graphID == Rtti::fGetClassId< f32 >( ) )
				{
					if( id == cAllOnes )
						key->fSetValue< f32 >( 1.f );
					else if( id == cAllZeroes )
						key->fSetValue< f32 >( 0.f );
				}
				else if( graphID == Rtti::fGetClassId< Math::tVec2f >( ) )
				{
					if( id == cAllOnes )
						key->fSetValue< Math::tVec2f >( Math::tVec2f( 1.f ) );
					else if( id == cAllZeroes )
						key->fSetValue< Math::tVec2f >( Math::tVec2f( 0.f ) );
					else if( id == cXAxis )
						key->fSetValue< Math::tVec2f >( Math::tVec2f( 1.f, 0.f ) );
					else if( id == cYAxis )
						key->fSetValue< Math::tVec2f >( Math::tVec2f( 0.f, 1.f ) );
				}
				else if( graphID == Rtti::fGetClassId< Math::tVec3f >( ) )
				{
					Math::tVec3f val = key->fValue< Math::tVec3f >( );

					if( id == cAllOnes )
						key->fSetValue< Math::tVec3f >( Math::tVec3f( 1.f ) );
					else if( id == cAllZeroes )
						key->fSetValue< Math::tVec3f >( Math::tVec3f( 0.f ) );
					else if( id == cXAxis )
						key->fSetValue< Math::tVec3f >( Math::tVec3f( 1.f, val.y, val.z) );
					else if( id == cYAxis )
						key->fSetValue< Math::tVec3f >( Math::tVec3f( val.x, 1.f, val.z ) );
					else if( id == cZAxis )
						key->fSetValue< Math::tVec3f >( Math::tVec3f( val.x, val.y, 1.f ) );
					else if( id == cNormalize )
						key->fSetValue< Math::tVec3f >( val.fNormalizeSafe( ) ); 
				}
				else if( graphID == Rtti::fGetClassId< Math::tVec4f >( ) )
				{
					Math::tVec4f val = key->fValue< Math::tVec4f >( );

					if( id == cAllOnes )
						key->fSetValue< Math::tVec4f >( Math::tVec4f( 1.f ) );
					else if( id == cAllZeroes )
						key->fSetValue< Math::tVec4f >( Math::tVec4f( 0.f ) );
					else if( id == cXAxis )
						key->fSetValue< Math::tVec4f >( Math::tVec4f( 1.f, val.y, val.z, val.w ) );
					else if( id == cYAxis )
						key->fSetValue< Math::tVec4f >( Math::tVec4f( val.x, 1.f, val.z, val.w ) );
					else if( id == cZAxis )
						key->fSetValue< Math::tVec4f >( Math::tVec4f( val.x, val.y, 1.f, val.w ) );
					else if( id == cWAxis )
						key->fSetValue< Math::tVec4f >( Math::tVec4f( val.x, val.y, val.z, 1.f ) );
					else if( id == cNormalize )
						key->fSetValue< Math::tVec4f >( val.fNormalizeSafe( ) ); 
				}
			}
		}
		else if( id >= cGenerateXLine && id <= cGenerateZLine )
		{
			sigassert( graphID == Rtti::fGetClassId< Math::tVec3f >( ) );
			
			f32 length = mGraph->fGraphHeight( );
			u32 numKeyframes = mGraph->fNumKeyframes( );
			
			for( u32 i = 0 ; i < numKeyframes; ++i )
			{
				tKeyframePtr key = mGraph->fRawKeyframe( i );
				Math::tVec3f value = key->fValue< Math::tVec3f >( );
				f32 curLength = length;
				if( numKeyframes > 1 )
					curLength *= ( ( f32 ) i / ( f32 )( numKeyframes - 1 ) );

				if( id == cGenerateXLine )
					value.x = curLength;
				else if( id == cGenerateYLine )
					value.y = curLength;
				else if( id == cGenerateZLine )
					value.z = curLength;

				key->fSetValue< Math::tVec3f >( value );
			}

			mGraph->fFrame( );
		}
		else if( id >= cFlipX && id <= cFlipY )
		{
			sigassert( graphID == Rtti::fGetClassId< Math::tVec3f >( ) );
			if( id == cFlipX )
				mGraph->fFlipX( );
			else if( id == cFlipY )
				mGraph->fFlipY( );
		}
		else if( id >= cXSpin && id < cZSpin+tGenerateSpinMenu::cSpinOptions )
		{
			sigassert( graphID == Rtti::fGetClassId< Math::tVec3f >( ) );
			
			u32 idx = id - cXSpin;
			u32 spin = cXSpin;
			if( id >= cYSpin )
			{
				idx -= tGenerateSpinMenu::cSpinOptions;
				spin = cYSpin;
			}
			if( id >= cZSpin )
			{
				idx -= tGenerateSpinMenu::cSpinOptions;
				spin = cZSpin;
			}
				
			const u32 wraps = tGenerateSpinMenu::mSpinCounts[ idx ];
			f32 radius = fAbs( mGraph->fMaxY( ) );//fGraphHeight( );
			u32 curNodes = mGraph->fNumKeyframes( );
			const u32 numSegments = 128;

			b32 clearGraph = curNodes == numSegments ? false : true;
			if( clearGraph )
				mGraph->fClear( );

			for( u32 i = 0; i < numSegments; ++i )
			{
				const f32 delta = ( ( f32 ) i / (numSegments-1.f) );
				const f32 phi = delta * Math::c2Pi * wraps;

				f32 x, y, z;
				if( spin == cXSpin )
				{
					x = std::cos( phi );
					y = 0.f;
					z = std::sin( phi );
				}
				else if( spin == cYSpin )
				{
					x = std::cos( phi );
					y = std::sin( phi );
					z = 0.f;
				}
				else// if( spin == cZSpin )
				{
					x = 0.f;
					y = std::cos( phi );
					z = std::sin( phi );
				}

				if( clearGraph )
				{
					tKeyframePtr key( new tFxKeyframeV3f( delta, Math::tVec3f( x, y, z ) * radius ) );
					mGraph->fAddKeyframe( key );
				}
				else
				{
					tKeyframePtr key = mGraph->fRawKeyframe( i );
					Math::tVec3f curValue = key->fValue< Math::tVec3f >( );
					curValue += ( Math::tVec3f( x, y, z ) * radius );
					key->fSetValue< Math::tVec3f >( curValue );
					key->fSetX( delta );
				}
			}

			mGraph->fFrame( );
		}
		else if( id == cCopyGraph )
		{
			mGraph->fSigFxMainWindow( )->fSigFxClipboard( )->fSetCopiedGraph( mGraph );
		}
		else if( id == cPasteGraph )
		{
			mGraph->fSigFxMainWindow( )->fSigFxClipboard( )->fPasteGraph( mGraph );
		}

		mGraph->fBuildValues( );
	}



	tSigFxGraphline::tSigFxGraphline( wxNotebook* parent, tSigFxKeyline* Keyline )
		: tTabPanel( parent )
		, mTheKeyline( Keyline )
		, mSelectedEntity( 0 )
		, mSelectedGraphButton( 0 )
		, mActiveGraph( 0 )
		, mForceMouseInput( false )
		, mContinueMouseInput( false )
		, mPreviousGraphIndexMode( true )
		, mPreviousGraphIndex( 0 )
	{
		SetMinSize( wxSize( 0, 260 ) );
		//wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL );
		//sizer->Add( this, 0, wxEXPAND );
			
		Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( tSigFxGraphline::fOnKeyDown ) );

		Connect( wxEVT_PAINT, wxPaintEventHandler( tSigFxGraphline::fOnPaint ) );
		Connect( wxEVT_SIZE, wxSizeEventHandler( tSigFxGraphline::fOnSize ) );
		Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( tSigFxGraphline::fOnEraseBackground ) );
		Connect( wxEVT_MOTION, wxMouseEventHandler( tSigFxGraphline::fOnMouseMove ) );
		Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( tSigFxGraphline::fOnMouseButtonDown ) );
		Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( tSigFxGraphline::fOnMouseButtonDoubleClick ) );
		Connect( wxEVT_LEFT_UP, wxMouseEventHandler( tSigFxGraphline::fOnMouseButtonUp ) );
		
		Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tSigFxGraphline::fOnRightClick ), NULL, this);

		Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( tSigFxGraphline::fOnMouseLeaveWindow ) );
		Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( tSigFxGraphline::fOnMouseEnterWindow ) );

		SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	}


	tSigFxGraphline::~tSigFxGraphline( )
	{
		fErasePreviousGraphs( );
	}

	void tSigFxGraphline::fOnMouseLeaveWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		if( mContinueMouseInput )
			mForceMouseInput = true;
	}

	void tSigFxGraphline::fOnMouseEnterWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		SetFocus( );
	}

	void tSigFxGraphline::fOnRightClick( wxMouseEvent& event )
	{
		if( mActiveGraph )
		{
			tEditorActionPtr action( new tSaveParticleSystemGraphsAction( mTheKeyline->fMainWindow( )->fGuiApp( ).fSelectionList( ) ) );
			mTheKeyline->fMainWindow( )->fGuiApp( ).fActionStack( ).fAddAction( action );

			tGraphlineContextMenu* menu = new tGraphlineContextMenu( mActiveGraph, ClientToScreen( event.GetPosition( ) ) );
			PopupMenu( menu, event.GetPosition( ).x, event.GetPosition( ).y );

			action->fEnd( );
		}
		//tEditorContextAction::fDisplayContextMenuOnRightClick( this, event, fMainWindow( )->fContextActions( ) );
	}

	void tSigFxGraphline::fOnTick( f32 dt )
	{
		mTheKeyline->fAdvanceTime( dt );

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
				fOnMouseButtonUp( event );
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
	}

	void tSigFxGraphline::fOnMouseMove( wxMouseEvent& event )
	{
		for( u32 i = 0; i < mGraphButtons.fCount( ); ++i )
			mGraphButtons[ i ]->fMouseOver( event );

		if( mActiveGraph )
		{
			mActiveGraph->fMouseMove( event );
		}

		event.Skip( );
	}

	void tSigFxGraphline::fOnMouseWheel( wxMouseEvent& event )
	{
		if( IsShown( ) && mActiveGraph )
		{
			if( IsMouseInWindow( ) )
				mActiveGraph->fOnMouseWheel( event );
		}
		event.Skip( );
	}

	void tSigFxGraphline::fOnMouseButtonUp( wxMouseEvent& event )
	{
		mContinueMouseInput = false;

		if( mActiveGraph )
			mActiveGraph->fMouseButtonUp( event );

		// if the scene graph is paused, only then force a full update.
		if( mTheKeyline->fMainWindow( )->fGuiApp( ).fSceneGraph( )->fIsPaused( ) )
			mTheKeyline->fForceWholeSceneRefresh( );
	}

	void tSigFxGraphline::fOnKeyDown( wxKeyEvent& event )
	{
		if( mActiveGraph )
			mActiveGraph->fOnKeyDown( event );
		
		if( mActiveGraph )
		{
			s32 idx = fGraphIndex( *mActiveGraph->fRawGraph( ) );
			if( idx >= 0 )
			{
				if( event.GetKeyCode( ) == WXK_DOWN )
					idx += 1;
				if( event.GetKeyCode( ) == WXK_UP )
					idx -= 1;
				
				s32 graphCount = ( s32 )mGraphs.fCount( );
				if( idx < 0 )
					idx = graphCount - 1;
				if( idx >= graphCount)
					idx = 0;

				if( idx >= 0 && idx < graphCount )
				{
					fSetActiveGraph( *mGraphs[ idx ]->fRawGraph( ) );
				}
			}
		}

		//if( event.GetKeyCode( ) != WXK_SPACE )
		//	mTheKeyline->fForceWholeSceneRefresh( );
	}

	void tSigFxGraphline::fOnMouseButtonDoubleClick( wxMouseEvent& event )
	{
		if( mActiveGraph )
			mActiveGraph->fOnMouseButtonDoubleClick( event );

		//mTheKeyline->fForceWholeSceneRefresh( );
	}

	s32 tSigFxGraphline::fGraphIndex( const tGraphPtr& graph )
	{
		s32 idx = -1;
		for( u32 i = 0; i < mGraphs.fCount( ); ++i )
		{
			if( mGraphs[ i ]->fRawGraph( )->fGetRawPtr( ) == graph.fGetRawPtr( ) )
			{
				idx = i;
				break;
			}
		}
		return idx;
	}

	void tSigFxGraphline::fSetActiveGraph( tGraphPtr graph )
	{
		if( mActiveGraph && mActiveGraph->fRawGraph( )->fGetRawPtr( ) == graph.fGetRawPtr( ) )
			return;
		
		s32 idx = fGraphIndex( graph );		

		if( mSelectedGraphButton )
			mSelectedGraphButton->fSetSelected( false );

		mSelectedGraphButton = mGraphButtons[ idx ];
		sigassert( mSelectedGraphButton );
		mSelectedGraphButton->fSetSelected( true );

		if( mSelectedEntity )
		{
			mActiveGraph = mGraphs[ idx ];

			tSigFxParticleSystem* fxps = mSelectedEntity->fDynamicCast< tSigFxParticleSystem > ( );
			tSigFxAttractor* fxa = mSelectedEntity->fDynamicCast< tSigFxAttractor > ( );
			tSigFxMeshSystem* fxms = mSelectedEntity->fDynamicCast< tSigFxMeshSystem > ( );
			tSigFxLight* fxl = mSelectedEntity->fDynamicCast< tSigFxLight > ( );
			if( fxps )
				fxps->mLastOpenGraphIdx = idx;
			else if( fxa )
				fxa->mLastOpenGraphIdx = idx;
			else if( fxms )
				fxms->mLastOpenGraphIdx = idx;
			else if( fxl )
				fxl->mLastOpenGraphIdx = idx;
		}
	}

	void tSigFxGraphline::fOnMouseButtonDown( wxMouseEvent& event )
	{
		mContinueMouseInput = true;

		if( mActiveGraph )
		{
			mActiveGraph->fMouseButtonDown( event );
		}

		b32 oversomething = false;
		for( u32 i = 0; i < mGraphButtons.fCount( ) && !oversomething; ++i )
			oversomething = mGraphButtons[ i ]->fMouseOver( event );

		if( !oversomething )
			return;		// early!'

		mSelectedGraphButton = 0;
		mActiveGraph = 0;

		for( u32 i = 0; i < mGraphButtons.fCount( ); ++i )
		{
			if( mGraphButtons[ i ]->fMouseClick( event ) )
				fSetActiveGraph( *mGraphButtons[ i ]->fGraph( )->fRawGraph( ) );
		}
	}

	void tSigFxGraphline::fOnSize( wxSizeEvent& event )
	{
		if( mActiveGraph )
			mActiveGraph->fMarkAsDirty( );
	}

	void tSigFxGraphline::fOnPaint( wxPaintEvent& event )
	{
		wxAutoBufferedPaintDC dc( this );

		wxSize size = GetSize( );

		// this SetBrush call needs to happen before mActiveGraph->fPaint( )
		dc.SetBrush( wxBrush( GetParent( )->GetBackgroundColour( ) ) );	

		if( mSelectedEntity && mActiveGraph )
		{
			mActiveGraph->fPaint( dc, size, mTheKeyline->fDelta( ), mGraphButtons );
		}
		else
		{
			dc.DrawRectangle( 0, 0, size.x, size.y );

			static const wxFont headerFont( 72, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL | wxFONTFLAG_ANTIALIASED, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
			wxString title( "SigFx" );
			wxSize textdim;

			dc.SetTextForeground( wxColour( 156, 156, 156 ) );
			dc.SetFont( headerFont);

			dc.GetTextExtent( title, &textdim.x, &textdim.y );
			dc.DrawText( title, size.x/2-textdim.x/2, size.y/2-textdim.y/2 );

			for( u32 i = 0; i < mGraphButtons.fCount( ); ++i )
				mGraphButtons[ i ]->fPaint( dc );
		}
	}

	void tSigFxGraphline::fOnEraseBackground( wxEraseEvent& event )
	{

	}

	void tSigFxGraphline::fRefreshNodes( )
	{
		if( mActiveGraph )
			mActiveGraph->fRefresh( );
	}

	void tSigFxGraphline::fErasePreviousGraphs( )
	{
		for( u32 i = 0; i < mGraphButtons.fCount( ); ++i )
			delete mGraphButtons[ i ];
		mGraphButtons.fSetCount( 0 );

		for( u32 i = 0; i < mGraphs.fCount( ); ++i )
			delete mGraphs[ i ];
		mGraphs.fSetCount( 0 );

		mSelectedGraphButton = 0;
		mActiveGraph = 0;
	}

	void tSigFxGraphline::fBuildPageFromEntities( tEditorSelectionList& selected )
	{
		fErasePreviousGraphs( );

		if( mPreviousGraphIndexMode && mSelectedEntity )
		{
			tSigFxParticleSystem* previousfxps = mSelectedEntity->fDynamicCast< tSigFxParticleSystem > ( );
			tSigFxAttractor* previousfxa = mSelectedEntity->fDynamicCast< tSigFxAttractor > ( );
			tSigFxMeshSystem* previousfxms = mSelectedEntity->fDynamicCast< tSigFxMeshSystem > ( );
			tSigFxLight* previousfxl = mSelectedEntity->fDynamicCast< tSigFxLight > ( );
			
			if( previousfxps )
				mPreviousGraphIndex = previousfxps->mLastOpenGraphIdx;
			else if( previousfxa )
				mPreviousGraphIndex = previousfxa->mLastOpenGraphIdx;
			else if( previousfxms )
				mPreviousGraphIndex = previousfxms->mLastOpenGraphIdx;
			else if( previousfxl )
				mPreviousGraphIndex = previousfxl->mLastOpenGraphIdx;
			else
				mPreviousGraphIndex = 0;
		}

		mSelectedEntity.fReset( 0 );

		if( selected.fCount( ) )
			mSelectedEntity = selected[ 0 ];

		if( mSelectedEntity.fNull( ) )
			return;

		u32 gbIdx = 0;
		wxPoint pos( 1, 5 );
		wxSize size( 150, 17 );
		f32 delta = 255.0f / 160.0f;

		tSigFxParticleSystem* fxps = mSelectedEntity->fDynamicCast< tSigFxParticleSystem > ( );
		tSigFxAttractor* fxa = mSelectedEntity->fDynamicCast< tSigFxAttractor > ( );
		tSigFxMeshSystem* fxms = mSelectedEntity->fDynamicCast< tSigFxMeshSystem > ( );
		tSigFxLight* fxl = mSelectedEntity->fDynamicCast< tSigFxLight > ( );

		if( fxps )	// Fill out Particle Systems
		{
			for( u32 i = 0; i < cEmissionGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mEmissionColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mEmissionHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mEmissionSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );
								
				tGraphButton* gb = new tGraphButton( pos, size, wxString( tToolParticleSystemState::mEmissionGraphNames[ i ].fCStr( ) ), nom, hil, sel );
				mGraphButtons.fPushBack( gb );
				pos.y += size.y;
			}
			
			for( u32 i = 0; i < cParticleGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mParticleColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mParticleHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mParticleSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );

				tGraphButton* gb = new tGraphButton( pos, size, wxString( tToolParticleSystemState::mPerParticleGraphNames[ i ].fCStr( ) ), nom, hil, sel );
				mGraphButtons.fPushBack( gb );
				pos.y += size.y;
			}

			for( u32 i = 0; i < cMeshGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mMeshColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mMeshHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mMeshSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );

				tGraphButton* gb = new tGraphButton( pos, size, wxString( tToolParticleSystemState::mMeshGraphNames[ i ].fCStr( ) ), nom, hil, sel );
				mGraphButtons.fPushBack( gb );
				pos.y += size.y;
			}

			for( u32 i = 0; i < cEmissionGraphCount; ++i )
			{
				tSigFxGraph* graph = new tSigFxGraph( &fxps->fGetToolState( )->mEmissionGraphs[ i ], mSelectedEntity, wxString( tToolParticleSystemState::mEmissionGraphNames[ i ].fCStr( ) ), mTheKeyline->fMainWindow( ) );
				mGraphs.fPushBack( graph );
				mGraphButtons[ gbIdx++ ]->fSetGraph( graph ); 

				if( i == cEmitterTranslationGraph )
					graph->fSetAsSpatialGraph( );
				if( i == cEmissionRateGraph )
					graph->fSetGraphTotalMaximumValue( 10000 );
			}
			
			for( u32 i = 0; i < cParticleGraphCount; ++i )
			{
				// since these graphs x-axis are determined per-particle we add an asterick to the end of the name to denote that
				tSigFxGraph* graph = new tSigFxGraph( &fxps->fGetToolState( )->mPerParticleGraphs[ i ], mSelectedEntity, wxString( tToolParticleSystemState::mPerParticleGraphNames[ i ].fCStr( ) ) + "*", mTheKeyline->fMainWindow( ) );
				mGraphs.fPushBack( graph );
				mGraphButtons[ gbIdx++ ]->fSetGraph( graph );

				graph->fShowScrub( true );

				if( i == cColorGraph )
				{
					graph->fClampHeightAtOne( true );
					graph->fAllowScrolling( false );
					graph->fSetAsColorGraph( );
				}
				else if( i == cCutoutGraph )
				{
					graph->fClampHeightAtOne( true );
					graph->fAllowScrolling( false );
				}
			}

			for( u32 i = 0; i < cMeshGraphCount; ++i )
			{
				// since these graphs x-axis are determined per-particle we add an asterick to the end of the name to denote that
				tSigFxGraph* graph = new tSigFxGraph( &fxps->fGetToolState( )->mMeshGraphs[ i ], mSelectedEntity, wxString( tToolParticleSystemState::mMeshGraphNames[ i ].fCStr( ) ) + "*", mTheKeyline->fMainWindow( ) );
				mGraphs.fPushBack( graph );
				mGraphButtons[ gbIdx++ ]->fSetGraph( graph );

				graph->fShowScrub( true );
			}

			u32 graphIdx = mPreviousGraphIndexMode ? mPreviousGraphIndex : fxps->mLastOpenGraphIdx;
			if( graphIdx > mGraphButtons.fCount( )-1 )
				graphIdx = 0;
			mSelectedGraphButton = mGraphButtons[ graphIdx ];
			fxps->mLastOpenGraphIdx = graphIdx;
		}
		else if( fxa )	// Fill out any attractors
		{
			for( u32 i = 0; i < cAttractorGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mEmissionColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mEmissionHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mEmissionSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );

				tGraphButton* gb = new tGraphButton( pos, size, wxString( tToolAttractorData::mGraphNames[ i ].fCStr( ) ), nom, hil, sel );
				mGraphButtons.fPushBack( gb );
				pos.y += size.y;

				tSigFxGraph* graph = new tSigFxGraph( &fxa->fGetToolData( )->fGraph( i ), mSelectedEntity, wxString( tToolAttractorData::mGraphNames[ i ].fCStr( ) ), mTheKeyline->fMainWindow( ) );
				mGraphs.fPushBack( graph );
				mGraphButtons[ gbIdx++ ]->fSetGraph( graph );

				if( i == cAttractorPositionGraph )
					graph->fSetAsSpatialGraph( );
				if( i == cAttractorParticleColorGraph )
					graph->fSetAsColorGraph( );
			}

			u32 graphIdx = mPreviousGraphIndexMode ? mPreviousGraphIndex : fxa->mLastOpenGraphIdx;
			if( graphIdx > mGraphButtons.fCount( )-1 )
				graphIdx = 0;
			mSelectedGraphButton = mGraphButtons[ graphIdx ];
			fxa->mLastOpenGraphIdx = graphIdx;
		}
		else if( fxms )
		{
			FX::tMeshSystemPtr ms = fxms->fFxMeshSystem( );
			for( u32 i = 0; i < cFxMeshSystemGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mEmissionColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mEmissionHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mEmissionSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );

				tGraphButton* gb = new tGraphButton( pos, size, wxString( tToolFxMeshSystemData::mGraphNames[ i ].fCStr( ) ), nom, hil, sel );
				mGraphButtons.fPushBack( gb );
				pos.y += size.y;

				wxString graphName( tToolFxMeshSystemData::mGraphNames[ i ].fCStr( ) );
				if( i >= cMeshScale )
					graphName += "*";	// since these graphs x-axis are determined per-particle we add an asterick to the end of the name to denote that

				tSigFxGraph* graph = new tSigFxGraph( &ms->fFxMeshSystemData( )->fGraph( i ), mSelectedEntity, graphName, mTheKeyline->fMainWindow( ) );
				mGraphs.fPushBack( graph );
				mGraphButtons[ gbIdx++ ]->fSetGraph( graph );

				if( i == cMeshTint )
				{
					graph->fClampHeightAtOne( true );
					graph->fAllowScrolling( false );
					graph->fSetAsColorGraph( );
				}
			}
			u32 graphIdx = mPreviousGraphIndexMode ? mPreviousGraphIndex : fxms->mLastOpenGraphIdx;
			if( graphIdx > mGraphButtons.fCount( )-1 )
				graphIdx = 0;
			mSelectedGraphButton = mGraphButtons[ graphIdx ];
			fxms->mLastOpenGraphIdx = graphIdx;
		}
		else if( fxl )
		{
			FX::tAnimatedLightEntityPtr al = fxl->fGetLight( );

			for( u32 i = 0; i < cLightGraphCount; ++i )
			{
				// It seems dangerous and stupid to use the emission colors when maybe a different type
				// will have more graphs in it than particles someday.
				const Math::tVec3f& n = tToolParticleSystemState::mEmissionColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mEmissionHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mEmissionSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );

				tGraphButton* gb = new tGraphButton( pos, size, wxString( tToolAnimatedLightData::mGraphNames[ i ].fCStr( ) ), nom, hil, sel );
				mGraphButtons.fPushBack( gb );
				pos.y += size.y;

				tSigFxGraph* graph = new tSigFxGraph( &fxl->mToolData->mGraphs[ i ], mSelectedEntity, wxString( tToolAnimatedLightData::mGraphNames[ i ].fCStr( ) ), mTheKeyline->fMainWindow( ) );
				mGraphs.fPushBack( graph );
				mGraphButtons[ gbIdx++ ]->fSetGraph( graph );

				if( i == cLightColorGraph )
				{
					graph->fClampHeightAtOne( true );
					graph->fAllowScrolling( false );
					graph->fSetAsColorGraph( );
				}
			}

			u32 graphIdx = mPreviousGraphIndexMode ? mPreviousGraphIndex : fxl->mLastOpenGraphIdx;
			if( graphIdx > mGraphButtons.fCount( )-1 )
				graphIdx = 0;
			mSelectedGraphButton = mGraphButtons[ graphIdx ];
			fxl->mLastOpenGraphIdx = graphIdx;
		}

		if( mSelectedGraphButton )
		{
			mSelectedGraphButton->fSetSelected( true );
			mActiveGraph = mSelectedGraphButton->fGraph( );
		}
	}


	void tSigFxGraphline::fFrame( )
	{
		if( mActiveGraph )
			mActiveGraph->fFrame( );
	}
}

