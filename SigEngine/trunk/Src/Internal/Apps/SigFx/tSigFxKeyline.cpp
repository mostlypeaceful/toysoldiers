#include "SigFxPch.hpp"
#include "FileSystem.hpp"
#include "tSigFxKeyline.hpp"
#include "tSigFxMainWindow.hpp"
#include "WxUtil.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tWxRenderPanel.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tSceneGraphFile.hpp"
#include "tAssetGenScanner.hpp"
#include "FX/tFxKeyframe.hpp"
#include "tRandom.hpp"
#include "tSigFxKeylineTrack.hpp"
#include "FxEditor/tSigFxAttractor.hpp"
#include "FxEditor/tSigFxMeshSystem.hpp"
#include "wx/dcbuffer.h"
#include "tFxEditorActions.hpp"
#include "tSigFxGraphline.hpp"

namespace Sig
{
	wxPoint	tSigFxKeyline::mLastMouseClickPosition( 0, 0 );
	wxPoint	tSigFxKeyline::mLastMousePosition( 0, 0 );

	tSigFxKeyline::tSigFxKeyline( wxNotebook* parent, tSigFxMainWindow* mainWindow, FX::tSigFxSystem* scene, wxPanel* KeylineScrubPanel )
		: tTabPanel( parent )
		, mGraphline( 0 )
		, mMainWindow( mainWindow )
		, mFxScene( scene )
		, mKeylineScrubPanel( KeylineScrubPanel )
		, mPaused( false )
		, mPreviouslyPaused( false )
		, mPausedByKeyframe( false )
		, mKeyframesMoved( false )
		, mDoRectangleSelection( false )
		, mCopyingKeyframes( false )
		, mHotKeyPaused( false )
		, mSelectedEntity( 0 )
		, mForceMouseInput( false )
		, mContinueMouseInput( false )
		, mUndoAction( 0 )
		, mScrub( 0 )
	{
		fSetPaused( false );
		mInnerBorder = wxSize( 10, 4 );

		//SetBackgroundColour( wxColour( 145, 138, 127 ) );

		Connect( wxEVT_PAINT, wxPaintEventHandler( tSigFxKeyline::fOnPaint ) );
		Connect( wxEVT_SIZE, wxSizeEventHandler( tSigFxKeyline::fOnSize ) );
		Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( tSigFxKeyline::fOnEraseBackground ) );
		Connect( wxEVT_MOTION, wxMouseEventHandler( tSigFxKeyline::fOnMouseMove ) );
		Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler( tSigFxKeyline::fOnMouseLeftDoubleClick ) );
		Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( tSigFxKeyline::fOnMouseLeftButtonDown ) );
		Connect( wxEVT_LEFT_UP, wxMouseEventHandler( tSigFxKeyline::fOnMouseLeftButtonUp ) );
		Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( tSigFxKeyline::fOnKeyDown ) );
		Connect( wxEVT_KEY_UP, wxKeyEventHandler( tSigFxKeyline::fOnKeyUp ) );
		Connect( wxEVT_LEAVE_WINDOW, wxMouseEventHandler( tSigFxKeyline::fOnMouseLeaveWindow ) );
		Connect( wxEVT_ENTER_WINDOW, wxMouseEventHandler( tSigFxKeyline::fOnMouseEnterWindow ) );

		/*play_normal = wxBitmap( "IconPlayNormal" );
		play_hover = wxBitmap( "IconPlayHover" );
		play_selected = wxBitmap( "IconPlaySelected" );
		
		pause_normal = wxBitmap( "IconOauseNormal" );
		pause_hover = wxBitmap( "IconPauseHover" );
		pause_selected = wxBitmap( "IconPauseSelected" );

		mPlayButton.Create( this, wxID_ANY, play_normal, wxPoint( 2, 4 ), wxDefaultSize, 0 );
		mPlayButton.Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigFxKeyline::fOnPlayButtonClicked ), NULL, this );
		mPlayButton.Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( tSigFxKeyline::fOnEraseBackground ) );

		mPlayButton.SetBitmapLabel( pause_normal );
		mPlayButton.SetBitmapSelected( pause_selected );
		mPlayButton.SetBitmapHover( pause_hover );*/

		SetMinSize( wxSize( 0, 260 ) );
		//wxBoxSizer* sizer = new wxBoxSizer( wxHORIZONTAL );
		//sizer->Add( this, 0, wxEXPAND );
		SetBackgroundStyle( wxBG_STYLE_CUSTOM );
	}

	void tSigFxKeyline::fSetHotKeyPaused( const b32 pause )
	{
		if( !mHotKeyPaused )
		{
			mHotKeyPaused = true;
			fSetPaused( pause );
		}
	}

	void tSigFxKeyline::fSetPaused( const b32 pause )
	{
		mPausedByKeyframe = false;
		mPreviouslyPaused = mPaused;
		mPaused = pause;
		fMainWindow( )->fGuiApp( ).fSceneGraph( )->fPauseNextFrame( mPaused  );

		if( mPaused )
		{
			/*mPlayButton.SetBitmapLabel( play_normal );
			mPlayButton.SetBitmapSelected( play_selected );
			mPlayButton.SetBitmapHover( play_hover );*/
		}
		else
		{
			/*mPlayButton.SetBitmapLabel( pause_normal );
			mPlayButton.SetBitmapSelected( pause_selected );
			mPlayButton.SetBitmapHover( pause_hover );*/
		}
	}

	void tSigFxKeyline::fOnPlayButtonClicked( wxCommandEvent& event )
	{
		if( fPaused( ) )
			fSetPaused( false );
		else			
			fSetPaused( true );
	}

	tSigFxKeyline::~tSigFxKeyline( )
	{
		fSave( );

		fClearKeyline( );
	}

	void tSigFxKeyline::fClearKeyline( )
	{
		tSigFxKeyframe::fRemoveAllHighlighted( );
		tSigFxKeyframe::fRemoveAllSelected( );

		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
			delete mKeylineTracks[ i ];

		mKeylineTracks.fSetCount( 0 );
	}

	void tSigFxKeyline::fBuildKeyline( )
	{
		fClearKeyline( );

		if( !mSelectedEntity )
			return;

		u32 trackHeight = fTickSpace( ) + 0;
		f32 delta = 255.0f / 156.0f;

		u32 trackSpace = 18;

		tSigFxParticleSystem* fxps = mSelectedEntity->fDynamicCast< tSigFxParticleSystem > ( );
		tSigFxAttractor* fxa = mSelectedEntity->fDynamicCast< tSigFxAttractor > ( );
		tSigFxMeshSystem* fxms = mSelectedEntity->fDynamicCast< tSigFxMeshSystem > ( );

		if( fxps )
		{
			for( u32 i = 0; i < cEmissionGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mEmissionColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mEmissionHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mEmissionSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );

				tGraphPtr graph = fxps->fGetToolState( )->mEmissionGraphs[ i ];
				tSigFxKeylineTrack* track = new tSigFxKeylineTrack( this, graph, wxString( tToolParticleSystemState::mEmissionGraphNames[ i ].fCStr( ) ), trackHeight, nom, hil, sel );
				trackHeight += trackSpace;
				mKeylineTracks.fPushBack( track );
			}
			for( u32 i = 0; i < cParticleGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mParticleColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mParticleHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mParticleSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );
				tGraphPtr graph = fxps->fGetToolState( )->mPerParticleGraphs[ i ];
				tSigFxKeylineTrack* track = new tSigFxKeylineTrack( this, graph, wxString( tToolParticleSystemState::mPerParticleGraphNames[ i ].fCStr( ) ), trackHeight, nom, hil, sel );
				trackHeight += trackSpace;
				mKeylineTracks.fPushBack( track );
			}
			for( u32 i = 0; i < cMeshGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mMeshColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mMeshHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mMeshSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );
				tGraphPtr graph = fxps->fGetToolState( )->mMeshGraphs[ i ];
				tSigFxKeylineTrack* track = new tSigFxKeylineTrack( this, graph, wxString( tToolParticleSystemState::mMeshGraphNames[ i ].fCStr( ) ), trackHeight, nom, hil, sel );
				trackHeight += trackSpace;
				mKeylineTracks.fPushBack( track );
			}
		}
		else if( fxa )
		{
			for( u32 i = 0; i < cAttractorGraphCount; ++i )
			{
				const Math::tVec3f& n = tToolParticleSystemState::mEmissionColours[ i ];
				const Math::tVec3f& h = tToolParticleSystemState::mEmissionHighlights[ i ];
				const Math::tVec3f& s = tToolParticleSystemState::mEmissionSelects[ i ];

				const wxColour nom( (u32)n.x, (u32)n.y, (u32)n.z );
				const wxColour hil( (u32)h.x, (u32)h.y, (u32)h.z );
				const wxColour sel( (u32)s.x, (u32)s.y, (u32)s.z );

				tGraphPtr graph = fxa->fGetToolData( )->fGraph( i );
				tSigFxKeylineTrack* track = new tSigFxKeylineTrack( this, graph, wxString( tToolAttractorData::mGraphNames[ i ].fCStr( ) ), trackHeight, nom, hil, sel );
				trackHeight += trackSpace;
				mKeylineTracks.fPushBack( track );
			}
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

				tGraphPtr graph = ms->fFxMeshSystemData( )->fGraph( i );
				tSigFxKeylineTrack* track = new tSigFxKeylineTrack( this, graph, wxString( tToolFxMeshSystemData::mGraphNames[ i ].fCStr( ) ), trackHeight, nom, hil, sel );
				trackHeight += trackSpace;
				mKeylineTracks.fPushBack( track );
			}
		}		
	}

	void tSigFxKeyline::fBuildPageFromEntities( tEditorSelectionList& selected )
	{
		if( selected.fCount( ) )
			mSelectedEntity = selected[ 0 ];
		else
			mSelectedEntity.fReset( 0 );
		
		fBuildKeyline( );
	}

	void tSigFxKeyline::fOnMouseLeaveWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		if( mContinueMouseInput )
			mForceMouseInput = true;
	}

	void tSigFxKeyline::fOnMouseEnterWindow( wxMouseEvent& event )
	{
		mForceMouseInput = false;
		SetFocus( );
	}

	void tSigFxKeyline::fOnKeyDown( wxKeyEvent& event )
	{
		if( event.GetKeyCode( ) == WXK_DELETE )
		{
			fDeleteAllSelectedKeyframes( );
		}
		else if( event.GetKeyCode( ) == 'G' )
		{
			//for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
			//	mKeylineTracks[ i ]->fToggleShrink( );
		}
		
		event.Skip( );
	}

	void tSigFxKeyline::fOnKeyUp( wxKeyEvent& event )
	{
		
	}


	void tSigFxKeyline::fDeleteAllSelectedKeyframes( )
	{
		mUndoAction.fReset( new tSaveParticleSystemGraphsAction( mMainWindow->fGuiApp( ).fSelectionList( ) ) );

		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
		{
			tSigFxKeylineTrack* track = mKeylineTracks[ i ];
			track->fDeleteAllSelectedKeyframes( );
		}

		mUndoAction->fEnd( );
		mMainWindow->fGuiApp( ).fActionStack( ).fAddAction( mUndoAction );
		mUndoAction.fReset( 0 );

		fForceWholeSceneRefresh( );
	}


	void tSigFxKeyline::fCopyAllSelectedKeyframes( )
	{
		tSaveParticleSystemGraphsAction* action = new tSaveParticleSystemGraphsAction( mMainWindow->fGuiApp( ).fSelectionList( ) );
		action->fSetDirtyingAction( false );
		mUndoAction.fReset( action );

		tGrowableArray< tSigFxKeyframe* > freshKeys;

		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
		{
			tSigFxKeylineTrack* track = mKeylineTracks[ i ];

			tGrowableArray< tSigFxKeyframe* > copyKeys;
			for( u32 keyIdx = 0; keyIdx < track->fNumKeyframes( ); ++keyIdx )
			{
				tSigFxKeyframe *fxkey = track->fKeyframe( keyIdx );
				if( fxkey->fSelected( ) )
					copyKeys.fPushBack( fxkey );
			}

			for( u32 idx = 0; idx < copyKeys.fCount( ); ++idx )
			{
				tSigFxKeyframe *fxkey = copyKeys[ idx ];
				tKeyframePtr rawkey = track->fNewRawKeyframe( fxkey );
				tSigFxKeyframe* newkey = new tSigFxKeyframe( fxkey, rawkey );

				track->fAddNewKeyframe( newkey );
				fxkey->fDeselect( );
				fxkey->fUnHighlight( );

				freshKeys.fPushBack( newkey );
			}
		}

		mUndoAction->fEnd( );
		mMainWindow->fGuiApp( ).fActionStack( ).fClearDirty( );
		mMainWindow->fGuiApp( ).fActionStack( ).fAddAction( mUndoAction );
		mUndoAction.fReset( 0 );
		fForceWholeSceneRefresh( );
		
		// re-select all freshly copied keys
		for( u32 i = 0; i < freshKeys.fCount( ); ++i )
		{
			freshKeys[ i ]->fDeselect( );
			freshKeys[ i ]->fSelect( );
		}
	}

	void tSigFxKeyline::fOnMouseMove( wxMouseEvent& event )
	{
		if( mDoRectangleSelection )
		{
			if( !event.LeftIsDown( ) )
			{
				mDoRectangleSelection = false;
			}
			else
			{
				// check against all keyframes for intersecting the
				// selection rectangle...
		
				const wxPoint p = event.GetPosition( );
				const u32 width = fAbs( p.x - mSelectionRectangleAnchor.x );
				const u32 height = fAbs( p.y - mSelectionRectangleAnchor.y );

				if( p.x < mSelectionRectangleAnchor.x )
					mSelectionRectangle.SetX( p.x );
				if( p.y < mSelectionRectangleAnchor.y )
					mSelectionRectangle.SetY( p.y );

				mSelectionRectangle.SetSize( wxSize( width, height ) );
							
				for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
					mKeylineTracks[ i ]->fSelectKeyframesWithinBounds( mSelectionRectangle );
			}
		}
		else
		{
			//tSigFxKeyframe::fRemoveAllHighlighted( );
			mKeyframesMoved = false;
			for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
			{
				if( mKeylineTracks[ i ]->fOnMouseMove( event ) )
					mKeyframesMoved = true;
			}

			if( mKeyframesMoved )
			{
				// controls updating-moving the Keyline
				// in coherence with the currently selected
				// keyframe. Wanna do this but not now...
				// problem is it's too slow, makes the program choppy :(
				/*if( fPaused( ) )
				{
					if( tSigFxKeyframe::mSelectedKeyframes.fCount( ) )
					{
						f32 x = tSigFxKeyframe::mSelectedKeyframes[ 0 ]->fX( ) * fKeylineLength( );
						fSetCurrentTime( x );
						fForceWholeSceneRefresh( );
					}
				}*/
			}
		}

		mLastMousePosition = event.GetPosition( );
	}

	void tSigFxKeyline::fOnSize( wxSizeEvent& event )
	{
		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
			mKeylineTracks[ i ]->fOnSize( event );
	}
	
	void tSigFxKeyline::fOnMouseLeftDoubleClick( wxMouseEvent& event )
	{
		mDoRectangleSelection = false;
		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
		{
			if( mKeylineTracks[ i ]->fOnMouseLeftDoubleClick( event ) )
			{
				if( tSigFxKeyframe::mSelectedKeyframes.fCount( ) )
				{
					fSetPaused( true );
					mPausedByKeyframe = true;

					f32 x = tSigFxKeyframe::mSelectedKeyframes[ 0 ]->fX( ) * fLifetime( );
					fSetCurrentTime( x );
					fForceWholeSceneRefresh( );
				}
			}
		}
	}


	void tSigFxKeyline::fDeselectAllKeyframes( )
	{
		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
			mKeylineTracks[ i ]->fDeselectAll( );
	}

	void tSigFxKeyline::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		// Save the state of our particle systems before they're changed!
		mUndoAction.fReset( new tSaveParticleSystemGraphsAction( mMainWindow->fGuiApp( ).fSelectionList( ) ) );

		mContinueMouseInput = true;
		mLastMouseClickPosition = event.GetPosition( );
		mCopyingKeyframes = false;

		event.Skip( true );
		// try to find a way not to have to do this 'noHit' calculation
		// in this class' fOnMouseLeftButton. I dunno if it can tho...
		
		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
			mKeylineTracks[ i ]->fOnMouseLeftButtonDown( event );

		if( event.GetSkipped( ) && !event.LeftDClick( ) )		// meaning we didn't hit any FxKeys...
		{
			mDoRectangleSelection = true;
			mSelectionRectangleAnchor = event.GetPosition( );
			mSelectionRectangle.SetPosition( event.GetPosition( ) );
			mSelectionRectangle.SetSize( wxSize( 0, 0 ) );

			if( mPausedByKeyframe )
				fSetPaused( false );
		}
		else
		{
			// we hit a key, but we need to check here if shift was down
			// so we can copy all the keyframes in one move...
			if( event.ShiftDown( ) )
			{
				mCopyingKeyframes = true;
				fCopyAllSelectedKeyframes( );
			}
		}
	}

	void tSigFxKeyline::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		mContinueMouseInput = false;

		if( mDoRectangleSelection )
		{
			mDoRectangleSelection = false;
			for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
				mKeylineTracks[ i ]->fSelectKeyframesWithinBounds( mSelectionRectangle );
		}
		else
		{
			for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
				mKeylineTracks[ i ]->fOnMouseLeftButtonUp( event );

			if( mKeyframesMoved && !mCopyingKeyframes )
			{
				if( mUndoAction )
				{
					mUndoAction->fEnd( );
					mMainWindow->fGuiApp( ).fActionStack( ).fAddAction( mUndoAction );
					mUndoAction.fReset( 0 );
				}

				//tMultipleActionsAction* movementAction = new tMultipleActionsAction( );

				//for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
				//{
				//	for( u32 keyIdx = 0; keyIdx < mKeylineTracks[ i ]->fNumKeyframes( ); ++keyIdx )
				//	{
				//		tSigFxKeyframe* key = mKeylineTracks[ i ]->fKeyframe( keyIdx );
				//		tKeyframeMovedAction* action = new tKeyframeMovedAction( key, key->fOldX( ), key->fX( ) );
				//		movementAction->fAddAction( tEditorActionPtr( action ) );
				//	}
				//}

				//mMainWindow.fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( movementAction ) );
			}
		}

		if( fMainWindow( )->fGuiApp( ).fSceneGraph( )->fIsPaused( ) )
			fForceWholeSceneRefresh( );
	}
		
	void tSigFxKeyline::fOnEraseBackground( wxEraseEvent& event )
	{
		
	}

	const u32 tSigFxKeyline::fGetCurrentBarPos( )
	{
		//wxSize size = GetSize( );
		//const u32 width = size.GetWidth( ) - mInnerBorder.x * 2;

		return fGetKeylinePosition( ).x + fGetKeylineSize( ).x * fDelta( );
	}

	wxSize tSigFxKeyline::fGetKeylineSize( )
	{
		u32 variableWidth = 0; 

		u32 width = GetSize( ).x - fGetKeylinePosition( ).x;
		width -= mInnerBorder.x;
		width -= variableWidth;

		wxSize size( width, GetSize( ).y - fGetKeylinePosition( ).y * 2 );
		return size;
	}

	wxPoint tSigFxKeyline::fGetKeylinePosition( )
	{
		return wxPoint( mInnerBorder.x + 165, mInnerBorder.y );
	}


	void tSigFxKeyline::fOnPaint( wxPaintEvent& event )
	{
		wxAutoBufferedPaintDC dc( this );
		
		const wxPoint pos = fGetKeylinePosition( );
		const wxSize size = fGetKeylineSize( );
		
		//dc.SetPen( *wxTRANSPARENT_PEN );
		dc.SetBrush( wxBrush( GetParent( )->GetBackgroundColour( ) ) );
		dc.DrawRectangle( 0, 0, GetSize( ).x, GetSize( ).y );

		//dc.SetPen( *wxBLACK_PEN );
		u32 x = fGetCurrentBarPos( );

		//dc.SetBrush( wxBrush( wxColour( 0, 0, 0 ) ) );
		//dc.SetBrush( wxBrush( wxColour( 145, 138, 127 ) ) );
		
		// okay draw the tickspace!

		static const wxFont tickFontSmall( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
		static const wxFont tickFontMedium( 96, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
		static const wxFont tickFontLarge( 20, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
		
		dc.SetFont( tickFontMedium );
		//wxString title( "s i x t e e n   s e c o n d s" );

		const f32 currentTime = fCurrentTime( );
		const u32 minutes = currentTime / 60.f;
		const u32 seconds = ( currentTime - 60.f * minutes );
		const u32 tens = fAbs( 100.f * ( currentTime - ( u32 ) currentTime ) );
		
		wxString min( wxString::Format( "%02d", minutes ) );
		wxString second( wxString::Format( "%02d", seconds ) );
		wxString ten( wxString::Format( "%.2d", tens ) );

		wxString title = wxString::Format( "%s:%s.%s", min, second, ten );	

		wxSize titleSize;
		dc.GetTextExtent( wxString( "00:00.00" ), &titleSize.x, &titleSize.y);
		
		dc.SetTextForeground( wxColour( 156, 156, 156 ) );
		//dc.DrawText( title, pos.x / 2 - titleSize.x / 2, pos.y + ( fTickSpace( ) - titleSize.y  ) / 2 + 1 );
		
		dc.SetClippingRegion( wxRect( pos.x + 1, pos.y, size.x - 2, size.y ) );		
		dc.DrawText( title, pos.x + size.x / 2 - titleSize.x / 2, pos.y + fTickSpace( ) + size.y / 2 - titleSize.y / 2 - fTickSpace( ) );
		
		dc.SetPen( *wxBLACK_PEN );

		dc.DestroyClippingRegion( );
		
		dc.SetTextForeground( *wxBLACK );
		dc.SetFont( tickFontSmall );

		dc.SetBrush( *wxTRANSPARENT_BRUSH );
		dc.DrawRectangle( pos.x, pos.y, size.x + 1, size.y + 1 );

		dc.SetTextForeground( wxColour( 0, 0, 0 ) );
			
		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
			mKeylineTracks[ i ]->fOnPaint1( &dc );

		// draw a little triangle thingie to indicate our current time in the Keyline...
		if( mScrub )
			mScrub->fPaintLine( &dc );		

		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
			mKeylineTracks[ i ]->fOnPaint2( &dc );
		
		// draw the selection rectangle if there is one!
		if( mDoRectangleSelection )
		{
			dc.SetBrush( *wxTRANSPARENT_BRUSH);
			dc.SetPen( wxPen( wxColour( 0, 0, 0 ), 1, wxDOT  ) );
			dc.DrawRectangle( mSelectionRectangle  );
		}
	}

	void tSigFxKeyline::fImmediateUpdate( b32 forcekeyframeupdate )
	{
		if( mSelectedEntity && forcekeyframeupdate )
			fBuildKeyline( );
		fSetCurrentTime( fCurrentTime( ) );	
	}

	void tSigFxKeyline::fSelectKeyframes( tKeyframePtr rawkey )
	{
		for( u32 i = 0; i < mKeylineTracks.fCount( ); ++i )
		{
			if( mKeylineTracks[ i ]->fHasKeyframe( rawkey ) )
				mKeylineTracks[ i ]->fSelectKeyframe( rawkey );
		}
	}

	void tSigFxKeyline::fSetCurrentTime( const f32 time )
	{	
		const f32 dt = time - fCurrentTime( );
		if( dt > 0.f )
			mMainWindow->fGuiApp( ).fSceneGraph( )->fIncrementTimeCounters( dt );
		
		mFxScene->fSetCurrentTime( time );
	}

	void tSigFxKeyline::fForceWholeSceneRefresh( )
	{
		mFxScene->fRefreshScene( );
	}

	void tSigFxKeyline::fAdvanceTime( f32 dt )
	{
		if( mScrub )
			mScrub->fSetXThruTime( fCurrentTime( ) );
	}

	void tSigFxKeyline::fOnTick( f32 dt )
	{
		fAdvanceTime( dt );

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
	}
}

