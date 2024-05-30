#include "SigFxPch.hpp"
#include "tSigFxKeylineTrack.hpp"
#include "tSigFxKeyframe.hpp"
#include "tFxEditorActions.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{
	
	tSigFxKeylineTrack::tSigFxKeylineTrack( tSigFxKeyline* Keyline, tGraphPtr graph,
		const wxString& keyframeText, const f32 trackHeight, const wxColour& normal,
		const wxColour& highlight, const wxColour& selected )
		: mTheKeyline( Keyline )
		, mGraph( graph )
		, mTrackHeight( trackHeight )
		, mTrackSelected( false )
		, mPlacingNewKeyframe( false )
		, mKeyframesShrunk( false )
		, mKeyframeText( keyframeText )
		, mNormalColour( normal )
		, mHighlightedColour( highlight )
		, mSelectedColour( selected )
	{
		for( u32 i = 0; i < mGraph->fNumKeyframes( ); ++i )
		{
			tSigFxKeyframe* key = new tSigFxKeyframe( mGraph->fKeyframe( i ), mTheKeyline, this, mGraph, mTrackHeight, mKeyframeText, mNormalColour, mHighlightedColour, mSelectedColour );
			key->fCalculateSizes( );
			//key->fFastShrink( );
			mKeyframes.fPushBack( key );
		}
	}

	tSigFxKeylineTrack::~tSigFxKeylineTrack( )
	{
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			delete mKeyframes[ i ];
		mKeyframes.fSetCount( 0 );
	}

	void tSigFxKeylineTrack::fToggleShrink( )
	{
		mKeyframesShrunk = !mKeyframesShrunk;
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			mKeyframes[ i ]->fSetShrink( mKeyframesShrunk );
	}

	tSigFxKeyframe* tSigFxKeylineTrack::fCreateNewKeyframe( u32 xpos )
	{
		// Save the state of our particle systems before they're changed!
		tEditorActionPtr action( new tSaveParticleSystemGraphsAction( mTheKeyline->fMainWindow( )->fGuiApp( ).fSelectionList( ) ) );
		
		const wxPoint pos = mTheKeyline->fGetKeylinePosition( );
		const wxSize size = mTheKeyline->fGetKeylineSize( );

		const f32 x = ( f32 ) ( xpos - pos.x ) / ( f32 )size.x;
		tSigFxKeyframe* previous = 0;
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
		{
			if( x < mKeyframes[ i ]->fX( ) && i == 0 )
				previous = mKeyframes[ i ];
			else if( x > mKeyframes[ i ]->fX( ) )
				previous = mKeyframes[ i ];
		}

		if( previous )
		{
			tKeyframePtr rawkey = fNewRawKeyframe( previous );
			tSigFxKeyframe* key = new tSigFxKeyframe( previous, rawkey );
			key->fSetX( x );
			key->fCalculateSizes( );

			fAddNewKeyframe( key );

			key->fSelect( );
			key->fSetSize( wxSize( key->fGetBounds( ).GetSize( ).x, 0 ) );
			return key;
		}

		action->fEnd( );
		mTheKeyline->fMainWindow( )->fGuiApp( ).fActionStack( ).fAddAction( action );

		return 0;
	}

	void tSigFxKeylineTrack::fDeselectAll( )
	{
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			mKeyframes[ i ]->fDeselect( );
	}

	b32 tSigFxKeylineTrack::fHasKeyframe( tSigFxKeyframe* key ) const
	{
		return mKeyframes.fFind( key ) != 0;
	}		
	
	b32 tSigFxKeylineTrack::fHasKeyframe( tKeyframePtr key ) const
	{
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
		{
			if( mKeyframes[ i ]->fGetRawKeyframe( ) == key.fGetRawPtr( ) )
				return true;
		}
		return false;
	}

	tKeyframePtr tSigFxKeylineTrack::fGetRawKeyframe( u32 idx ) const
	{
		return mGraph->fKeyframe( idx );
	}
	
	tKeyframePtr tSigFxKeylineTrack::fNewRawKeyframe( tSigFxKeyframe* key )
	{	
		sigassert( key );
		return mGraph->fNewKeyFromKey( key->fGetRawKeyframe( ) );
	}

	void tSigFxKeylineTrack::fAddNewKeyframe( tSigFxKeyframe* key )
	{
		mGraph->fAddKeyframe( key->fGetRawKeyframe( ) );
		mDelayedAddList.fPushBack( key );
	}

	void tSigFxKeylineTrack::fRemoveKeyframe( tSigFxKeyframe* key )
	{
		mGraph->fRemoveKeyframe( key->fGetRawKeyframe( ) );
		mDelayedRemoveList.fPushBack( key );
	}
	
	void tSigFxKeylineTrack::fUpdateGraphValues( )
	{
		sigassert( mGraph );
		mGraph->fBuildValues( );
	}

	void tSigFxKeylineTrack::fDeleteKeyframe( tSigFxKeyframe* key )
	{
		fRemoveKeyframe( key );
		delete key;
	}
	
	void tSigFxKeylineTrack::fDeleteAllSelectedKeyframes( )
	{
		u32 keyframes = fNumKeyframes( );
		for( u32 i = 0; i < keyframes; ++i )
		{
			tSigFxKeyframe *fxkey = fKeyframe( i );
			if( fxkey->fSelected( ) && fNumKeyframes( ) > 1 )
			{
				fRemoveKeyframe( fxkey );
			}
		}
	}

	u32 tSigFxKeylineTrack::fNumKeyframes( ) const
	{
		return mKeyframes.fCount( ) - mDelayedRemoveList.fCount( );
	}

	tSigFxKeyframe* tSigFxKeylineTrack::fKeyframe( const u32 idx )
	{
		return mKeyframes[ idx ];
	}

	void tSigFxKeylineTrack::fOnSize( wxSizeEvent& event )
	{
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			mKeyframes[ i ]->fOnParentResize( event, mTrackHeight );
	}

	void tSigFxKeylineTrack::fOnPaint1( wxAutoBufferedPaintDC* dc )
	{
		for( u32 i = 0; i < mDelayedRemoveList.fCount( ) && fNumKeyframes( ) > 1; ++i )
		{
			mDelayedRemoveList[ i ]->fDeselect( );
			mDelayedRemoveList[ i ]->fUnHighlight( );
			mKeyframes.fFindAndErase( mDelayedRemoveList[ i ] );
		}

		mDelayedRemoveList.fSetCount( 0 );
		
		for( u32 i = 0; i < mDelayedAddList.fCount( ); ++i )
			mKeyframes.fPushBack( mDelayedAddList[ i ] );
		mDelayedAddList.fSetCount( 0 );
		
		if( mKeyframes.fCount( ) )
		{
			mY.x = mKeyframes[ 0 ]->fGetBounds( ).GetY( );
			mY.y = mKeyframes[ 0 ]->fNormalSize( ).y;
		}

		const wxPoint pos = mTheKeyline->fGetKeylinePosition( );
		const wxSize size = mTheKeyline->fGetKeylineSize( );

		// draw the track. just 2 lines going horizontal down the length of the Keyline...
		if( mTrackSelected )
		{
			dc->SetPen( wxPen( mHighlightedColour ) );
			dc->DrawLine( pos.x, mY.x, pos.x + size.x, mY.x );
			dc->DrawLine( pos.x, mY.x + mY.y - 1, pos.x + size.x, mY.x + mY.y - 1);
		}

		std::sort( mKeyframes.fBegin( ), mKeyframes.fEnd( ), tSigFxKeyframe::tSortKeyframeByXOnly( ) );

		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
		{
			tSigFxKeyframe* keyc = 0;
			tSigFxKeyframe* keyl = 0;
			
			keyc = mKeyframes[ i ];

			f32 height = keyc->fNormalSize( ).y / 2;	//keyc->fGetBounds( ).GetSize( ).y / 2;	//

			if( keyc->fShrunk( ) )
				continue;

			wxPoint pc = keyc->fGetBounds( ).GetPosition( );	
			wxSize sc = keyc->fGetBounds( ).GetSize( );

			wxPen dottedBlack( wxColour( 64, 64, 64 ), 1, wxDOT );
			dc->SetPen( keyc->fDashedLinePen( ) );

			if( i > 0 )
			{
				keyl = mKeyframes[ i-1 ];
				if( keyl->fShrunk( ) )
					continue;

				wxPoint pl;
				wxSize sl;	

				if( keyl )
				{
					if( keyl->fSelected( ) || keyl->fHighlighted( ) )
						dc->SetPen( keyl->fDashedLinePen( ) );

					pl = keyl->fGetBounds( ).GetPosition( );
					sl = keyl->fGetBounds( ).GetSize( );

					dc->DrawLine( pl.x + sl.x, mTrackHeight + height ,pc.x , mTrackHeight + height );
					dc->DrawLine( pc.x, mTrackHeight + height, pl.x + sl.x, mTrackHeight + height );
				}
			}
			
			if( i == 0 )
			{
				if( i == mKeyframes.fCount( ) - 1)
				{
					dc->DrawLine( pc.x + sc.x, mTrackHeight + height, pos.x + size.x, mTrackHeight + height );
				}

				if( mKeyframes[ i ]->fX( ) > 0.f )
				{
					dc->SetPen( dottedBlack );
					dc->DrawLine( pos.x, mTrackHeight + height, pc.x, mTrackHeight + height );
				}
			}
			else if( i == mKeyframes.fCount( ) - 1 )
			{
				if( mKeyframes[ i ]->fX( ) < 1.f )
				{
					dc->SetPen( dottedBlack );
					dc->DrawLine( pc.x + sc.x / 2, mTrackHeight + height, pos.x + size.x, mTrackHeight + height );
				}
			}
		}
	}
	
	void tSigFxKeylineTrack::fOnPaint2( wxAutoBufferedPaintDC* dc )
	{
		const wxPoint pos = mTheKeyline->fGetKeylinePosition( );
		const wxSize size = mTheKeyline->fGetKeylineSize( );

		std::sort( mKeyframes.fBegin( ), mKeyframes.fEnd( ), tSigFxKeyframe::tSortKeyframe( ) );
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			mKeyframes[ i ]->fOnPaint( dc );

		if( mPlacingNewKeyframe )
		{
			s32 x = mLastMousePos.x;
			s32 y = mY.x + mY.y/2;
			s32 radius = 7;

			if( x < pos.x )
				x = pos.x;
			if( x > pos.x + size.x )
				x = pos.x + size.x;

			f32 curTime = ( ( f32 ) ( x - pos.x ) / ( f32 )size.x ) * mTheKeyline->fLifetime( );
			const u32 minutes = curTime / 60.f;
			const u32 seconds = ( curTime - 60.f * minutes );
			const u32 tens = fAbs( 100.f * ( curTime - ( u32 ) curTime ) );

			wxString min( wxString::Format( "%02d", minutes ) );
			wxString second( wxString::Format( "%02d", seconds ) );
			wxString ten( wxString::Format( "%.2d", tens ) );

			wxString txt = wxString::Format( "%s:%s.%s", min, second, ten );

			wxPoint txtSize;
			dc->GetTextExtent( wxString( "00:00.00" ), &txtSize.x, &txtSize.y);

			dc->DrawText( txt, x - txtSize.x/2, y - mY.y/2 - txtSize.y + 2 );

			dc->SetPen( *wxBLACK_PEN );
			dc->SetBrush( wxBrush( mSelectedColour ) );
			dc->DrawCircle( x, y, radius ); 
		}
	}


	b32 tSigFxKeylineTrack::fOnMouseMove( wxMouseEvent& event )
	{
		b32 returnval = false;
		if( !mPlacingNewKeyframe )
		{
			for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			{
				if( mKeyframes[ mKeyframes.fCount( ) - i - 1 ]->fOnMouseMove( event, i == 0, true ) )
					returnval = true;
			}
		}

		mLastMousePos = event.GetPosition( );
		fUpdateGraphValues( );
		return returnval;
	}

	b32 tSigFxKeylineTrack::fOnMouseLeftDoubleClick ( wxMouseEvent& event )
	{
		if( !mTrackSelected )
		{
			// if we're currently over any keyframes then don't look for this
			// double click event....

			b32 hover = false;
			for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			{
				if( mKeyframes[ i ]->fOnMouseLeftButtonDoubleClick( event ) )
					return true;

				if( !hover )
				{
					if( mKeyframes[ i ]->fGetBounds( ).Contains( event.GetPosition( ) ) )
						hover = true;
				}
			}

			if( !hover )
			{
				// check to see if we're clicking on this "track-line"
				if( fAbs( ( mY.x + mY.y/2 ) - event.GetPosition( ).y ) < mY.y / 2)
				{
					mTrackSelected = true;
					mPlacingNewKeyframe = true;
				}
			}
		}

		return false;
	}

	void tSigFxKeylineTrack::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		mTrackSelected = false;

		if( mPlacingNewKeyframe )
		{
			// spawn the new keyframe dummy!
			s32 x = mLastMousePos.x;
			const wxPoint pos = mTheKeyline->fGetKeylinePosition( );
			const wxSize size = mTheKeyline->fGetKeylineSize( );

			if( x < pos.x )
				x = pos.x;
			if( x > pos.x + size.x )
				x = pos.x + size.x;
			
			fCreateNewKeyframe( x );

			mPlacingNewKeyframe = false;
			event.Skip( false );
		}
		else
		{
			for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			{
				if ( mKeyframes[ i ]->fOnMouseLeftButtonDown( event ) )
				{
					//mTrackSelected = true;
					event.Skip( false );
				}
			}
		}

		fUpdateGraphValues( );
	}

	void tSigFxKeylineTrack::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
			mKeyframes[ i ]->fOnMouseLeftButtonUp( event );
		fUpdateGraphValues( );
	}

	void tSigFxKeylineTrack::fSelectKeyframesWithinBounds( const wxRect& selectionRectangle )
	{
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
		{
			if( selectionRectangle.Intersects( mKeyframes[ i ]->fGetBounds( ) ) )
				mKeyframes[ i ]->fSelect( );
			else
				mKeyframes[ i ]->fDeselect( );
		}
	}

	void tSigFxKeylineTrack::fSelectKeyframe( tKeyframePtr rawkey )
	{
		for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
		{
			if( mKeyframes[ i ]->fGetRawKeyframe( ) == rawkey )
				mKeyframes[ i ]->fSelect( );
		}
	}
}

