#include "SigFxPch.hpp"
#include "tSigFxKeyframe.hpp"
#include "tSigFxKeyline.hpp"
#include "tSigFxKeylineTrack.hpp"

namespace Sig
{
	tGrowableArray< tSigFxKeyframe* > tSigFxKeyframe::mSelectedKeyframes;
	tGrowableArray< tSigFxKeyframe* > tSigFxKeyframe::mHighlightedKeyframes;

	tSigFxKeyframe::tSigFxKeyframe( tKeyframePtr key
		, tSigFxKeyline* parent
		, tSigFxKeylineTrack* track
		, tGraphPtr graph
		, const u32 ypos
		, const wxString& text
		, const wxColour& normal
		, const wxColour& highlite
		, const wxColour& selected )
			: mKeyframe( key )
			, mFrame( 0 )
			, mTheKeyline( parent )
			, mParentTrack( track )
			, mGraph( graph )
			, mLocked( false )
			, mSelected( false )
			, mHighlighted( false )
			, mLastX( 0.f )
			, mShrink( false )
			, mTopY( ypos )
			, mText( text )
			, mNormalColour( normal )
			, mHighlightedColour( highlite)
			, mSelectedColour( selected )
			, mFont( 8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT("Courier 10 Pitch") )
			, mSolidPen( wxColour( 0, 0, 0 ), 1, wxSOLID )
			, mDashedLinePen( mNormalColour, 1, wxSOLID )
			, mSolidBrush( mNormalColour )
	{
		//fSetSize( wxSize( 0, 0 ) );
	}

	tSigFxKeyframe::tSigFxKeyframe( const tSigFxKeyframe* keyframe, tKeyframePtr rawkey )
	{
		mKeyframe = rawkey;

		mLocked = keyframe->mLocked;
		mSelected = keyframe->mSelected;
		mHighlighted = keyframe->mHighlighted;
		mShrink = keyframe->mShrink;

		mTopY = keyframe->mTopY;
		mMiddleY = keyframe->mMiddleY;

		mText = keyframe->mText;
		mTextSize = keyframe->mTextSize;
		mNormalSize = keyframe->mNormalSize;
		mExtraSize = keyframe->mExtraSize;
		mBounds = keyframe->mBounds;
		mTheKeyline = keyframe->mTheKeyline;
		mParentTrack = keyframe->mParentTrack;
		mGraph = keyframe->mGraph;

		mNormalColour = keyframe->mNormalColour;
		mHighlightedColour = keyframe->mHighlightedColour;
		mSelectedColour = keyframe->mSelectedColour;

		mFont = keyframe->mFont;
		mSolidPen = keyframe->mSolidPen;
		mDashedLinePen = keyframe->mDashedLinePen;
		mSolidBrush = keyframe->mSolidBrush;
	}

	tSigFxKeyframe::~tSigFxKeyframe( )
	{

	}

	void tSigFxKeyframe::fOnPaint( wxAutoBufferedPaintDC* dc )
	{
		++mFrame;
		dc->SetFont( mFont );

		const wxPoint position( mBounds.GetX( ), mBounds.GetY( ) );
		const wxSize size = mBounds.GetSize( );

		dc->SetClippingRegion( wxRect( position.x, position.y, size.x, size.y ) );

		//if( mFrame % 2 == 0 )
		{
			if( mShrink )
			{
				wxSize newSize = size;
				if( size.y > 0 )
				{
					newSize.y -= 1;
					fSetSize( newSize );
					fSetPosition( wxPoint( position.x, mMiddleY - size.y / 2 ) );
				}
			}
			else
			{
				wxSize newSize = size;
				wxSize topSize = fNormalSize( );
				if( fSelected( ) )
					topSize = fExtraSize( );

				if( size.y < topSize.y )
				{
					newSize.y += 1;
					fSetSize( newSize );
					fSetPosition( wxPoint( position.x, mMiddleY - size.y / 2 ) );
				}
			}
		}


		dc->SetPen( mSolidPen );
		dc->SetBrush( mSolidBrush );
		dc->DrawRectangle( position.x, position.y, size.x, size.y );

		dc->GetTextExtent( mText, &mTextSize.x, &mTextSize.y);

		{
			dc->DrawText( mText, 4 + position.x, position.y );
		
			if( fLocked( ) )
			{
				dc->SetBrush( *wxBLACK_BRUSH );
				dc->DrawRectangle( position.x + size.x - 10, position.y + size.y - 10, 6, 6 );
			}
		}

		// check to see if the parent Keyline's trackbar is over this keyframe
		// if so select ourselves white!
		s32 x = mTheKeyline->fGetCurrentBarPos( );
		if( x >= position.x && x <= position.x + size.x )
		{
			dc->SetPen( wxPen( wxColour( 255, 255, 255 ), 1 ) );
			dc->SetBrush( *wxTRANSPARENT_BRUSH );
			
			const wxPoint position( fGetBounds( ).GetX( ), fGetBounds( ).GetY( ) );
			const wxSize size = fGetBounds( ).GetSize( );

			dc->DrawRectangle( position.x, position.y, size.x, size.y );
		}


		if( fSelected( ) )
		{
			wxString val = fGetKeyframeText( mKeyframe );
			wxSize valueSize;
			dc->GetTextExtent( val, &valueSize.x, &valueSize.y);
			dc->DrawText( val, 4 + position.x  + fGetTextSize( ).x + 4, position.y );

			dc->DestroyClippingRegion( );

			dc->SetPen( wxPen( wxColour( 255, 0, 0 ), 2 ) );
			const u32 x = position.x + size.x / 2;
			const u32 y = mTheKeyline->fGetInnerBorder( ).y + mTheKeyline->fTickSpace( );
			dc->DrawLine( x, y, x, y - 13 );

			/*
			f32 curTime = fX( ) * mTheKeyline->fKeylineLength( );
			const u32 minutes = curTime / 60.f;
			const u32 seconds = ( curTime - 60.f * minutes );
			const u32 tens = fAbs( 100.f * ( curTime - ( u32 ) curTime ) );

			wxString min( wxString::Format( "%02d", minutes ) );
			wxString second( wxString::Format( "%02d", seconds ) );
			wxString ten( wxString::Format( "%.2d", tens ) );

			wxString txt = wxString::Format( "%s:%s.%s", min, second, ten );

			wxPoint txtSize;
			dc->GetTextExtent( wxString( "00:00.00" ), &txtSize.x, &txtSize.y);
			dc->DrawText( txt, position.x + size.x/2 - txtSize.x/2, position.y - txtSize.y + 2 );
			*/
		}
		else
			dc->DestroyClippingRegion( );
	}


	b32 tSigFxKeyframe::fOnMouseLeftButtonDoubleClick( wxMouseEvent& event )
	{
		if( mBounds.Contains( event.GetPosition( ) ) )
			return true;
		return false;
	}

	b32 tSigFxKeyframe::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		mLastX = fX( );

		if( mBounds.Contains( event.GetPosition( ) ) )
		{
			if( event.CmdDown( ) )
			{
				if( fSelected( ) )		// already
				{
					fDeselect( );
				}
				else
					fSelect( );
			}
			else if( event.ShiftDown( ) )
			{
				fSelect( );
			}
			else
			{
				if( !fSelected( ) )
				{
					fRemoveAllSelected( );
					fSelect( );
				}	
			}

			return true;
		}
		else
		{
			if( !fSelected( ) )
				mSolidBrush = wxBrush( mNormalColour );
		}
		return false;
	}

	void tSigFxKeyframe::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		if( !mTheKeyline->fRectangleSelectionInProcess( ) )
		{
			if( event.GetPosition( ) == tSigFxKeyline::mLastMouseClickPosition )
			{
				if( !event.CmdDown( ) && mBounds.Contains( event.GetPosition( ) ) )
				{	
					fRemoveAllSelected( );
					fSelect( );
				}
			}
		}
	}

	b32 tSigFxKeyframe::fOnMouseMove( wxMouseEvent& event, b32 resetHighlight, b32 oneHighlight )
	{
		const b32 highlighted = fHighlighted( );
		const b32 selected = fSelected( );

		if( selected )
		{
			//if( mBounds.Contains( event.GetPosition( ) ) )
			{
				//if( !highlighted )
				//	mHighlightedKeyframes.fPushBack( this );
			}

			// check to see if the cursor is in our rectangle...
			wxPoint cursor = event.GetPosition( );

			const wxSize tsize = mTheKeyline->fGetKeylineSize( );
			const wxPoint tpos = mTheKeyline->fGetKeylinePosition( );
			
			b32 outRight( false );
			b32 outLeft( false );
			
			if( cursor.x < tpos.x )	outLeft = true;
			else if ( cursor.x > tpos.x + tsize.x )	outRight = true;

			if( event.LeftIsDown( ) && !mLocked )
			{	
				u32 hw = mBounds.GetSize( ).x / 2;
			
				f32 diff = cursor.x - tSigFxKeyline::mLastMousePosition.x;
				f32 delta = diff / ( f32 ) tsize.x;

				s32 border = tpos.x - hw;

				f32 x = fX( ) + delta;
				if( x < 0.f )	x = 0.f;
				if( x > 1.f )	x = 1.f;

				if( outLeft || outRight )
				{
					if( x < 0.03f )		// snap to zero
						x = 0.f;
					else if( x > 0.97f )	// snap to one
						x = 1.f;
					else
						x = fX( );		// stay current then
				}
					
				fSetX( x );

				return true;
			}
		}
		else
		{
			if( mBounds.Contains( event.GetPosition( ) ) )
			{
				if( oneHighlight )
				{
					if( !mHighlightedKeyframes.fCount( )  )
					{
						fHighlight( );
					}
				}
				else
				{
					fHighlight( );
				}
			}
			else
			{
				fUnHighlight( );
			}
		}
		return false;
	}

	void tSigFxKeyframe::fOnParentResize( wxSizeEvent& event, const u32 ypos )
	{
		mTopY = ypos;
		fCalculateSizes( );
	}

	void tSigFxKeyframe::fRemoveAllSelected( )
	{
		for( u32 i = 0; i < mSelectedKeyframes.fCount( ); ++i )
			mSelectedKeyframes[ i ]->fDeselect( false );

		mSelectedKeyframes.fSetCount( 0 );
	}

	void tSigFxKeyframe::fRemoveAllHighlighted( )
	{
		for( u32 i = 0; i < mHighlightedKeyframes.fCount( ); ++i )
			mHighlightedKeyframes[ i ]->fUnHighlight( false );

		mHighlightedKeyframes.fSetCount( 0 );
	}

	void tSigFxKeyframe::fSelect( )
	{
		if( fSelected( ) )
			return;

		fSetSelected( true );
		mSelectedKeyframes.fPushBack( this );
		mSolidBrush = wxBrush( mSelectedColour );
		mDashedLinePen.SetColour( mSelectedColour );
		fCalculateSizes( );
	}

	void tSigFxKeyframe::fDeselect( b32 remove )
	{
		if( !fSelected( ) )
			return;

		//fSetHighlighted( true );
		fSetSelected( false );
		fCalculateSizes( );
		mSolidBrush = wxBrush( mNormalColour );
		mDashedLinePen.SetColour( mNormalColour );
		if( remove )
			mSelectedKeyframes.fFindAndErase( this );
	}

	void tSigFxKeyframe::fHighlight( )
	{
		if( fHighlighted( ) || fSelected( ) )
			return;

		fSetHighlighted( true );
		mHighlightedKeyframes.fPushBack( this );
		mSolidBrush = wxBrush( mHighlightedColour );
		mDashedLinePen.SetColour( mHighlightedColour );
	}

	void tSigFxKeyframe::fUnHighlight( b32 remove )
	{
		if( !fHighlighted( ) || fSelected( ) )
			return;

		fSetHighlighted( false );
		mSolidBrush = wxBrush( mNormalColour );
		mDashedLinePen.SetColour( mNormalColour );
		if( remove )
			mHighlightedKeyframes.fFindAndErase( this );
	}

	wxString tSigFxKeyframe::fGetKeyframeText( tKeyframePtr key )
	{
		wxString result;

		if( key->fGetID( ) == Rtti::fGetClassId< f32 >( ) )
		{
			result = wxString::Format( "%.2f", key->fValue< f32 >( ) );
		}
		else if( key->fGetID( ) == Rtti::fGetClassId< Math::tVec2f >( ) )
		{
			const Math::tVec2f val = key->fValue< Math::tVec2f >( );
			result = wxString::Format( "%.2f, %.2f", val.x, val.y );
		}
		else if( key->fGetID( ) == Rtti::fGetClassId< Math::tVec3f >( ) )
		{
			const Math::tVec3f val = key->fValue< Math::tVec3f >( );
			result = wxString::Format( "%.2f, %.2f, %.2f", val.x, val.y, val.z );
		}
		else if( key->fGetID( ) == Rtti::fGetClassId< Math::tVec4f >( ) )
		{
			const Math::tVec4f val = key->fValue< Math::tVec4f >( );
			result = wxString::Format( "%.2f, %.2f, %.2f, %.2f", val.x, val.y, val.z, val.w );
		}
		else if( key->fGetID( ) == Rtti::fGetClassId< Math::tQuatf >( ) )
		{
			const Math::tQuatf val = key->fValue< Math::tQuatf >( );
			result = wxString::Format( "%.2f, %.2f, %.2f, %.2f", val.x, val.y, val.z, val.x );
		}

		return result;
	}

	void tSigFxKeyframe::fFastShrink( )
	{
		const wxPoint position( mBounds.GetX( ), mBounds.GetY( ) );
		const wxSize size = mBounds.GetSize( );
		wxSize newSize = size;
		wxSize topSize = fNormalSize( );
		if( fSelected( ) )
			topSize = fExtraSize( );

		newSize.y = 0;
		fSetSize( newSize );
		fSetPosition( wxPoint( position.x, mMiddleY - size.y / 2 ) );
	}

	void tSigFxKeyframe::fCalculateSizes( )
	{
		if( fShrunk( ) || !mTheKeyline )
		{
			return;
		}

		wxClientDC dc( mTheKeyline );
		wxFont font( fFont( ) );
		dc.SetFont( font );

		wxString& txt = fText( );
		wxSize textSize;
		dc.GetTextExtent( txt, &textSize.x, &textSize.y );
		
		u32 height = textSize.y + 1;
		u32 width = 0;
	
		u32 extra = 0;
		if( fLocked( ) )
			extra = 12;

		wxPoint valueSize;
		dc.GetTextExtent( fGetKeyframeText( mKeyframe ), &valueSize.x, &valueSize.y);
		width = 4 + textSize.x + valueSize.x + 4 + 4 + extra;
		fExtraSize( wxSize( width, height ) );
		
		width = 4 + textSize.x + 4 + extra;
		fNormalSize( wxSize( width, height ) );
		
		wxSize size;
		if( fSelected( ) )
			size = fExtraSize( );
		else
			size = fNormalSize( );
				
		const u32 hw = size.x / 2;
		const u32 innerX = mTheKeyline->fGetKeylinePosition( ).x;
		const u32 xpos = innerX + ( mTheKeyline->fGetKeylineSize( ).x * fX( ) );
		
		fSetSize( size );
		fSetPosition( wxPoint( xpos - hw, mTopY ) );

		mMiddleY = mTopY + size.y / 2;
	}

	void tSigFxKeyframe::fSetX( f32 x )
	{
		mKeyframe->fSetX( x );

		const u32 hw = mBounds.GetSize( ).x / 2;
		const u32 innerX = mTheKeyline->fGetKeylinePosition( ).x;
		const u32 xpos = innerX + ( mTheKeyline->fGetKeylineSize( ).x * fX( ) );
		fSetPosition( wxPoint( xpos - hw, mTopY ) );
		mGraph->fUpdate( );
	}
}

