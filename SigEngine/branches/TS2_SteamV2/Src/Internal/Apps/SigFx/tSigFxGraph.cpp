#include "SigFxPch.hpp"
#include "tSigFxGraph.hpp"
#include "tFxEditorActions.hpp"
#include "tSigFxMainWindow.hpp"
#include "tSigFxGraphline.hpp"

namespace Sig
{

wxFont tSigFxGraph::mFont( 8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL | wxFONTFLAG_ANTIALIASED, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );

	tSigFxGraph::tSigFxGraph( tGraphPtr* graph, tEntityPtr entity, const wxString& name, tSigFxMainWindow* guiApp )
		: mGraph( graph )
		, mOldGraph( 0 )
		, mAssociatedEntity( entity )
		, mGraphName( name )
		, mMainWindow( guiApp )
		, mMouseMovement( cMouseMoveNormal )
		, mDoSelection( false )
		, mMovingKeyframes( false )
		, mCopyKeyframes( false )
		, mMultiSelect( false )
		, mHighlightedNode( 0 )
		, mHeightClamped( false )
		, mAllowScroll( true )
		, mShowScrub( true )
		, mDirty( true )
		, mColorGraph( false )
		, mSpatialGraph( false )
	{
		mMinX = 0.f;
		mMaxX = 1.f;
		mMinY = 0.f;
		mMaxY = 10.f;

		mElevatorOffset = 0.f;
		mGraphTotalMaximumValue = 100000;	//100,000?

		fFrame( );
	}

	tSigFxGraph::~tSigFxGraph( )
	{
		for( u32 i = 0; i < mDrawNodePoints.fCount( ); ++i )
			delete mDrawNodePoints[ i ];
	}

	void tSigFxGraph::fFlipX( )
	{
		for( u32 i = 0; i < fNumKeyframes( ); ++i )
		{
			tKeyframePtr key = fRawKeyframe( i );
			key->fSetX( 1.f - key->fX( ) );
		}
		( *mGraph )->fUpdate( );
	}

	void tSigFxGraph::fFlipY( )
	{
		u32 id = (*mGraph)->fGetID( );
		for( u32 i = 0; i < fNumKeyframes( ); ++i )
		{
			tKeyframePtr key = fRawKeyframe( i );
			if( id == Rtti::fGetClassId< f32 >( ) )
				key->fSetValue< f32 >( key->fValue< f32 >( ) * -1.f );
			else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
				key->fSetValue< Math::tVec2f >( key->fValue< Math::tVec2f >( ) * -1.f );
			else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
				key->fSetValue< Math::tVec3f >( key->fValue< Math::tVec3f >( ) * -1.f );
			else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
				key->fSetValue< Math::tVec4f >( key->fValue< Math::tVec4f >( ) * -1.f );
			else if( id == Rtti::fGetClassId< Math::tQuatf >( ) )
				key->fSetValue< Math::tQuatf >( key->fValue< Math::tQuatf >( ) * -1.f );
		}

		( *mGraph )->fUpdate( );
	}

	void tSigFxGraph::fClear( )
	{
		( *mGraph )->fClear( );
		for( u32 i = 0; i < mDrawNodePoints.fCount( ); ++i )
			delete mDrawNodePoints[ i ];

		mDrawNodePoints.fSetCount( 0 );
	}

	void tSigFxGraph::fAddKeyframe( tKeyframePtr key )
	{
		( *mGraph )->fAddKeyframe( key );
		
		
		//fAddKeyframe
	}

	// fOnMouseWheel is called from within the tSigFxMainWindow classes fOnMouseEvents( )	
	void tSigFxGraph::fOnMouseWheel( wxMouseEvent& event )
	{
		if( mHeightClamped )
			return;

		f32 ychange = fRoundUp< f32 >( fMaximumNodeDifference( ) );
		f32 old = mMaxY;
		mMaxY += ( ychange * 0.2f ) * fSign( event.GetWheelRotation( ) );

		//if( old == 1.f && ychange > 1.f )	mMaxY -= 1.f;
		if( mMaxY < 1.f )	mMaxY = 1.0f;
		if( mMaxY > mGraphTotalMaximumValue )
			mMaxY = mGraphTotalMaximumValue;

		mDirty = true;
	}


	void tSigFxGraph::fMouseMove( wxMouseEvent& event )
	{
		f32 x = event.GetPosition( ).x - mLastMousePos.x;
		f32 y = event.GetPosition( ).y - mLastMousePos.y;

		b32 outOfXBounds( false );
		b32 outOfYBounds( false );

		if( event.GetPosition( ).x < mBounds.GetLeft( ) || event.GetPosition( ).x > mBounds.GetRight( ) )
			outOfXBounds = true;
		if( event.GetPosition( ).y < mBounds.GetTop( ) || event.GetPosition( ).y > mBounds.GetBottom( ) )
			outOfYBounds = true;

		f32 xwidth = mMaxX - mMinX;
		f32 ywidth = mMaxY - mMinY;

		f32 px = x / mBounds.GetSize( ).x;
		f32 py = y / mBounds.GetSize( ).y;
		
		if( mMovingKeyframes && event.LeftIsDown( ) )
		{
			if( event.ShiftDown( ) && mSelectedNodes.fCount( ) && !mCopyKeyframes )
			{
				mCopyKeyframes = true;
				// copy all nodes!
				
				for( u32 i = 0; i < mSelectedNodes.fCount( ); ++i )
				{
					mNewKeys.fFindOrAdd( mSelectedNodes[ i ] );
				}
			}

			tGrowableArray< tKeyframePtr > touchedNodes;

			for( u32 i = 0; i < mSelectedNodes.fCount( ); ++i )
			{
				f32 val = fValue( mSelectedNodes[ i ]->mKey, mSelectedNodes[ i ]->mIdx );
				f32 x = mSelectedNodes[ i ]->mKey->fX( );
				
				x += px * xwidth;

				if( x < 0.f )	x = 0.f;
				else if( x > 1.f )	x = 1.f;

				val -= py * ywidth;

				if( val < mMinY ) val = mMinY;
				else if( val > mMaxY ) val = mMaxY;
				
				if( outOfYBounds )
				{
					f32 epsilon = ywidth * 0.01f;
					if( val > mMaxY - epsilon )	val = mMaxY;
					if( val < mMinY + epsilon )	val = mMinY;
				}
				if( outOfXBounds )
				{
					f32 epsilon = 0.01f;
					if( x < epsilon )	x = 0.f;
					if( x > ( 1.f - epsilon ) )	x = 1.f;
				}

				if( !outOfYBounds || val == mMinY || val == mMaxY )
					fSetValue( mSelectedNodes[ i ]->mKey, val, mSelectedNodes[ i ]->mIdx );

				if( !touchedNodes.fFind( mSelectedNodes[ i ]->mKey ) )
				{
					if( !outOfXBounds || x == 0.f || x == 1.f )
						mSelectedNodes[ i ]->mKey->fSetX( x );
				}

				touchedNodes.fPushBack( mSelectedNodes[ i ]->mKey );
			}

			(*mGraph)->fUpdate( );

			if( mColorGraph )
				fMarkAsDirty( );

			if( mMainWindow->fGuiApp( ).fSceneGraph( )->fIsPaused( ) )
				mMainWindow->fImmediateRefresh( false );
		}
		else if( !mMovingKeyframes )
		{
			mHighlightedNode = 0;

			for( u32 i = 0; i < mDrawNodePoints.fCount( ); ++i )
			{
				wxRect rc( mDrawNodePoints[ i ]->mPos.x, mDrawNodePoints[ i ]->mPos.y, mDrawNodePoints[ i ]->mNodeRadius, mDrawNodePoints[ i ]->mNodeRadius );
				if( rc.Contains( event.GetPosition( ) ) && !mHighlightedNode )
					mHighlightedNode = mDrawNodePoints[ i ];
			}

			if( event.MiddleIsDown( ) && mAllowScroll )
			{
				if( mMouseMovement == cMouseMoveNormal )
				{
					f32 delta = mMaxY - mMinY;
					f32 inc = ( py * delta ) * 3.f;

					mElevatorOffset += inc;

					mMaxY += inc;
					mMinY += inc;

					mDirty = true;
					//if( mMaxY < 1.f )
					//	mMaxY = 1.0f;
				}
				/*else if( mMouseMovement == cMouseMoveVertical )
				{
					mMaxY += ( y );
					if( mMaxY < 1.f )
						mMaxY = 1.0f;
				}
				else if( mMouseMovement == cMouseMoveHorizontal )
				{
					mMinX -= x;
					mMaxX += x;
				}*/
			}
			else if( event.LeftIsDown( ) )
			{
				if( mDoSelection )
				{
					const wxPoint p = event.GetPosition( );
					const u32 width = fAbs( p.x - mSelectionRectangleAnchor.x );
					const u32 height = fAbs( p.y - mSelectionRectangleAnchor.y );

					if( p.x < mSelectionRectangleAnchor.x )
						mSelectionBox.SetX( p.x );
					if( p.y < mSelectionRectangleAnchor.y )
						mSelectionBox.SetY( p.y );

					mSelectionBox.SetSize( wxSize( width, height ) );

					mNewlySelectedNodes.fSetCount( 0 );

					for( u32 i = 0; i < mDrawNodePoints.fCount( ); ++i )
					{
						wxRect rc( mDrawNodePoints[ i ]->mPos.x, mDrawNodePoints[ i ]->mPos.y, mDrawNodePoints[ i ]->mNodeRadius, mDrawNodePoints[ i ]->mNodeRadius );

						if( mSelectionBox.Intersects( rc ) )
							mNewlySelectedNodes.fFindOrAdd( mDrawNodePoints[ i ] );
					}	
				}
			}
		}

		mLastMousePos = event.GetPosition( );
	}
		
	void tSigFxGraph::fOnMouseButtonDoubleClick( wxMouseEvent& event )
	{
		// only if we're clicking within the bounds!
		if( !mBounds.Contains( event.GetPosition( ) ) )
			return;

		fSetOldGraph( *mGraph, &mOldGraph );

		wxPoint pos = mBounds.GetPosition( );
		wxSize size = mBounds.GetSize( );

		f32 breadth = fAbs( mMaxY - mMinY );

		f32 x = ( f32 ) ( event.GetPosition( ).x - pos.x ) / ( f32 ) size.x;
		f32 y = event.GetPosition( ).y - pos.y;
		f32 val = ( 1.f - y / size.y ) * breadth + mElevatorOffset;

		tKeyframePtr key = (*mGraph)->fCopyPreviousKey( x );
		key->fSetX( x );

		u32 id = (*mGraph)->fGetID( );

		//if( !mColorGraph )
		{
			if( id == Rtti::fGetClassId< f32 >( ) )
				key->fSetValue< f32 >( val );
			else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
				key->fSetValue< Math::tVec2f >( Math::tVec2f( val ) );
			else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
				key->fSetValue< Math::tVec3f >( Math::tVec3f( val ) );
			else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
				key->fSetValue< Math::tVec4f >( Math::tVec4f( val ) );
			else if( id == Rtti::fGetClassId< Math::tQuatf >( ) )
				key->fSetValue< Math::tQuatf >( Math::tQuatf( val ) );
		}

		(*mGraph)->fAddKeyframe( key );
		if( mColorGraph )
			fMarkAsDirty( );

		tGraphlineAction* ga = new tGraphlineAction( mGraph, mOldGraph );
		mMainWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( ga ) );
	}

	void tSigFxGraph::fMouseButtonDown( wxMouseEvent& event )
	{
		mMouseMovement = cMouseMoveNormal;

		if( event.LeftIsDown( ) )
		{
			if( event.ShiftDown( ) )
			{
				if( mHighlightedNode )
					mSelectedNodes.fFindOrAdd( mHighlightedNode );
			}

			if( mSelectedNodes.fCount( ) > 1 )
				mMultiSelect = true;

			if( mSelectedNodes.fFind( mHighlightedNode ) )
				mMovingKeyframes = true;
			else
				mMultiSelect = false;
			
			if( mHighlightedNode )
			{
				mMovingKeyframes = true;

				if( !mMultiSelect )
				{
					mSelectedNodes.fSetCount( 0 );
					mNewlySelectedNodes.fSetCount( 0 );
				}

				mSelectedNodes.fFindOrAdd( mHighlightedNode );
			}
			else
			{
				if( !event.ShiftDown( ) )
				{
					mSelectedNodes.fSetCount( 0 );
					mNewlySelectedNodes.fSetCount( 0 );
				}

				for( u32 i = 0; i < mDrawNodePoints.fCount( ); ++i )
				{
					wxRect rc( mDrawNodePoints[ i ]->mPos.x, mDrawNodePoints[ i ]->mPos.y, mDrawNodePoints[ i ]->mNodeRadius, mDrawNodePoints[ i ]->mNodeRadius );
					if( rc.Contains( event.GetPosition( ) ) )
						mSelectedNodes.fFindOrAdd( mDrawNodePoints[ i ] );
				}

				if( !mSelectedNodes.fCount( ) || event.ShiftDown( ) )
				{
					mDoSelection = true;
					mSelectionRectangleAnchor = event.GetPosition( );
					mSelectionBox.SetPosition( event.GetPosition( ) );
					mSelectionBox.SetSize( wxSize( 0, 0 ) );
				}
			}
		}


		if( mMovingKeyframes )
		{
			// okay make a copy of our current graph
			fSetOldGraph( (*mGraph), &mOldGraph );
		}

		/*
		if( event.ShiftDown( ) && !mSelectedKeyframes.fCount( ) )
		{
			u32 x = fAbs( event.GetPosition( ).x - mLastMousePos.x );
			u32 y = fAbs( event.GetPosition( ).y - mLastMousePos.y );

			if( x > y )
				mMouseMovement = cMouseMoveHorizontal;
			else if( y > x )
				mMouseMovement = cMouseMoveVertical;
			else
				mMouseMovement = cMouseMoveVertical;
		}
		*/

		mLastMousePos = event.GetPosition( );
	}

	void tSigFxGraph::fSetOldGraph( tGraphPtr graph, tGraphPtr* old )
	{
		u32 id = graph->fGetID( );
			
		old->fReset( 0 );
		if( id == Rtti::fGetClassId< f32 >( ) )
			old->fReset( new tFxGraphF32( graph.fGetRawPtr( ) ) );
		else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
			old->fReset( new tFxGraphV2f( graph.fGetRawPtr( ) ) );
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
			old->fReset( new tFxGraphV3f( graph.fGetRawPtr( ) ) );
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
			old->fReset( new tFxGraphV4f( graph.fGetRawPtr( ) ) );
	}

	void tSigFxGraph::fMouseButtonUp( wxMouseEvent& event )
	{
		mMultiSelect = false;
		mCopyKeyframes = false;

		if( mDoSelection )
		{
			if( !event.ShiftDown( ) )
			{
				mNewlySelectedNodes.fSetCount( 0 );
				mSelectedNodes.fSetCount( 0 );
			}

			for( u32 i = 0; i < mDrawNodePoints.fCount( ); ++i )
			{
				wxRect rc( mDrawNodePoints[ i ]->mPos.x, mDrawNodePoints[ i ]->mPos.y, mDrawNodePoints[ i ]->mNodeRadius, mDrawNodePoints[ i ]->mNodeRadius );

				if( mSelectionBox.Intersects( rc ) )
					mSelectedNodes.fFindOrAdd( mDrawNodePoints[ i ] );
			}

			if( mSelectedNodes.fCount( ) )
				mMultiSelect = true;
		}
		else
		{
			if( mSelectedNodes.fCount( ) == 1 && !event.ShiftDown( ) )
			{
				mNewlySelectedNodes.fSetCount( 0 );
				mSelectedNodes.fSetCount( 0 );
				mHighlightedNode = 0;
			}

			if( mMovingKeyframes )
			{
				tGraphlineAction* ga = new tGraphlineAction( mGraph, mOldGraph );
				mMainWindow->fGuiApp( ).fActionStack( ).fAddAction( tEditorActionPtr( ga ) );
			}
		}

		mMovingKeyframes = false;
		mDoSelection = false;
	}

	void tSigFxGraph::fRefresh( )
	{
		mSelectedNodes.fSetCount( 0 );
		mNewlySelectedNodes.fSetCount( 0 );

		for( u32 i = 0; i < mDrawNodePoints.fCount( ); ++i )
			delete mDrawNodePoints[ i ];
		mDrawNodePoints.fSetCount( 0 );

		fMarkAsDirty( );
	}

	void tSigFxGraph::fOnKeyDown( wxKeyEvent& event )
	{
		if( event.GetKeyCode( ) == WXK_DELETE )
		{
			// save our graph for undo/redo, duh!
			tEditorActionPtr action( new tSaveParticleSystemGraphsAction( mMainWindow->fGuiApp( ).fSelectionList( ) ) );

			for( u32 i = 0; i < mSelectedNodes.fCount( ) && fNumKeyframes( ) > 1; ++i )
				(*mGraph)->fRemoveKeyframe( mSelectedNodes[ i ]->mKey );

			action->fEnd( );
			mMainWindow->fGuiApp( ).fActionStack( ).fAddAction( action );

			if( mColorGraph )
				fMarkAsDirty( );
			fRefresh( );
		}
		else if( event.GetKeyCode( ) == 'F' )
		{
			fFrame( );
		}
	}

	void tSigFxGraph::fSetValue( tKeyframePtr key, f32 val, u32 idx ) const
	{
		u32 id = key->fGetID( );

		if( id == Rtti::fGetClassId< f32 >( ) )
		{
			key->fSetValue< f32 >( val );
		}
		else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
		{
			Math::tVec2f v = key->fValue< Math::tVec2f >( );
			v[ idx ] = val;
			key->fSetValue< Math::tVec2f >( v );
		}
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
		{
			Math::tVec3f v = key->fValue< Math::tVec3f >( );
			v[ idx ] = val;
			key->fSetValue< Math::tVec3f >( v );
		}
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
		{
			Math::tVec4f v = key->fValue< Math::tVec4f >( );
			v[ idx ] = val;
			key->fSetValue< Math::tVec4f >( v );
		}
	}

	f32 tSigFxGraph::fSampleGraph( f32 x, u32 idx ) const
	{
		const tMersenneGenerator* generator = 0;

		tSigFxParticleSystem* fxps = mAssociatedEntity->fDynamicCast< tSigFxParticleSystem >( );
		tSigFxAttractor* fxa = mAssociatedEntity->fDynamicCast< tSigFxAttractor >( );
		tSigFxMeshSystem* fxms = mAssociatedEntity->fDynamicCast< tSigFxMeshSystem >( );

		if( fxps )
		{
			generator = fxps->fGetParticleSystem( )->fSystemRandomNumberGenerator( );
		}
		else if( fxa )
		{
			generator = fxa->fGetAttractor( )->fRandomNumberGenerator( );
		}
		else if( fxms )
		{
			generator = fxms->fFxMeshSystem( )->fRandomNumberGenerator( );
		}

		f32 val( 0.f );
		u32 id = (*mGraph)->fGetID( );

		if( id == Rtti::fGetClassId< f32 >( ) )
			val = (*mGraph)->fSample< f32 >( generator, x );
		else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
			val = (*mGraph)->fSample< Math::tVec2f >( generator, x )[ idx ];
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
			val = (*mGraph)->fSample< Math::tVec3f >( generator, x )[ idx ];
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
			val = (*mGraph)->fSample< Math::tVec4f >( generator, x )[ idx ];

		return val;
	}

	f32 tSigFxGraph::fValue( tKeyframePtr key, u32 idx ) const
	{
		f32 val( 0.f );
		u32 id = key->fGetID( );

		if( id == Rtti::fGetClassId< f32 >( ) )
			val = key->fValue< f32 >( );
		else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
			val = key->fValue< Math::tVec2f >( )[ idx ];
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
			val = key->fValue< Math::tVec3f >( )[ idx ];
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
			val = key->fValue< Math::tVec4f >( )[ idx ];

		return val;
	}

	namespace
	{
		wxFont cGraphHeaderFont( 72, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL | wxFONTFLAG_ANTIALIASED, wxFONTWEIGHT_NORMAL, false, wxT( "Georgia" ) );
	}

	void tSigFxGraph::fPaint( wxAutoBufferedPaintDC& dc, wxSize& panelSize, f32 KeylineDelta, const tGrowableArray< tGraphButton* >& graphButtons )
	{
		u32 xstart = 175;
		u32 ystart = 8;

		mBounds = wxRect( xstart, ystart, panelSize.x-xstart-10, panelSize.y-ystart-20 );

		dc.SetFont( mFont );
		wxPen backgroundLinesPen( wxColour( 123, 117, 108 ), 1, wxDOT );

		wxPoint pos = mBounds.GetPosition( );
		wxSize size = mBounds.GetSize( );
		f32 w10 = size.x / 10.f;
		f32 h10 = size.y / 5.f;
		f32 breadth = fAbs( mMaxY - mMinY );

		if( mDirty )
		{
			mDirty = false;

			wxMemoryDC backgroundDC;
			wxMemoryDC graphDC;
			wxMemoryDC maskDC;

			mBackgroundBitmap.Create( panelSize.x, panelSize.y );
			backgroundDC.SelectObject( mBackgroundBitmap );
			backgroundDC.SetBrush( dc.GetBrush( ) );
			backgroundDC.DrawRectangle( 0, 0, panelSize.x, panelSize.y );

			mGraphBitmap.Create( size.x, size.y );
			//mGraphBitmap.UseAlpha( );
			graphDC.SelectObject( mGraphBitmap );
			graphDC.SetBrush( dc.GetBrush( ) );
			graphDC.DrawRectangle( 0, 0, size.x, size.y );


			mMaskBitmap.Create( size.x, size.y );
			maskDC.SelectObject( mMaskBitmap );
			//mGraphBitmap.SetMask( new wxMask( mMaskBitmap, *wxWHITE ) );

			if( mColorGraph )
			{
				for( u32 i = 0; i < fNumKeyframes( ); ++i )
				{
					tKeyframePtr key1 = fRawKeyframe( i );
					f32 x1 = size.x * key1->fX( );
					f32 y1 = 0;
					f32 x2 = size.x;
					f32 y2 = size.y;

					Math::tVec4f color1 = key1->fValue< Math::tVec4f >( );
					Math::tVec4f color2 = key1->fValue< Math::tVec4f >( );
					wxColour c1( color1.x * 255.f, color1.y * 255.f, color1.z * 255.f );
					f32 alpha = 255.f - color1.w * 255.f;
					wxColour m1( alpha, alpha, alpha );
					
					if( i < fNumKeyframes( ) - 1 )
					{
						tKeyframePtr key2 = fRawKeyframe( i+1 );
						x2 = size.x * key2->fX( );
						color2 = key2->fValue< Math::tVec4f >( );
					}
					
					wxRect rc( wxPoint( x1, y1 ), wxPoint( x2, y2 ) );
					wxColour c2( color2.x * 255.f, color2.y * 255.f, color2.z * 255.f );
					alpha = 255.f - color2.w * 255.f;
					wxColour m2( alpha, alpha, alpha );
					
					maskDC.GradientFillLinear( rc, m1, m2 );
					graphDC.GradientFillLinear( rc, c1, c2 );
				}
			}
			
			graphDC.SetTextForeground( wxColour( 156, 156, 156 ) );
			graphDC.SetFont( cGraphHeaderFont );			
		
			wxSize textdim;
			graphDC.GetTextExtent( mGraphName, &textdim.x, &textdim.y );
			//mBackgroundDC->DrawText( mGraphName, xstart - textdim.x/2, ystart - textdim.y/2 );
			graphDC.DrawText( mGraphName, (((panelSize.x-xstart-5)/2) - textdim.x/2 ), (((size.y-ystart-5)/2) - textdim.y/2 ) );	// actually draw the text offset from the center of the panel, center it instead in the graph, it looks more visually pleasing that way.

			graphDC.SetFont( mFont );
			graphDC.SetPen( backgroundLinesPen );
			graphDC.SetTextForeground( wxColour( 0, 0, 0 ) );
			
			backgroundDC.SetClippingRegion( mBounds );
			backgroundDC.SetTextForeground( wxColour( 0, 0, 0 ) );
			backgroundDC.SetFont( mFont );

			for( u32 i = 0; i <= 10; ++i )
			{
				u32 x1 = pos.x + w10 * i;
				u32 y1 = pos.y;

				u32 x2 = pos.x + w10 * i;
				u32 y2 = pos.y + size.y;

				f32 val = mMinX + ( mMaxX - mMinX ) * ( f32 )( i / 10.f );
				wxString txt = wxString::Format( "%.1f", val );
				wxSize txtSize;
				backgroundDC.GetTextExtent( txt, &txtSize.x, &txtSize.y);
				backgroundDC.DestroyClippingRegion( ) ;
				backgroundDC.DrawText( txt, x1 - txtSize.x/2, y2 );
				backgroundDC.SetClippingRegion( mBounds );
				if( i > 0 )
					graphDC.DrawLine( w10 * i, 0, w10*i, size.y );
				
				for( u32 j = 0; j <= 5; ++j )
				{
					x1 = pos.x;
					y1 = pos.y + h10 * j;

					x2 = pos.x + size.x;
					y2 = pos.y + h10 * j;

					f32 val = mElevatorOffset + ( breadth * ( 1.f - j / 5.f ) );

					wxString txt = wxString::Format( "%.2f", val );
					if( val > 1000.f )
						txt = wxString::Format( "%.0f", val );
					
					if( val >= 100000.f )	//100,000
						txt.insert( 3, ',' );
					else if( val >= 10000.f )	//10,000
						txt.insert( 2, ',' );
					else if( val >= 1000.f )	//1,000
						txt.insert( 1, ',' );


					wxSize txtSize;
					backgroundDC.GetTextExtent( txt, &txtSize.x, &txtSize.y);
					backgroundDC.DestroyClippingRegion( ) ;
					backgroundDC.DrawText( txt, x1 - txtSize.x, y1 - txtSize.y/2 );
					backgroundDC.SetClippingRegion( mBounds );
					if( j > 0 )
						graphDC.DrawLine( 1, h10*j, size.x-1, h10*j );

					if( i == 0 && j == 0 )	// draw the '0' black line!
					{
						f32 val = 0.f;

						//wxString txt = wxString::Format( "%.1f", val );
						//wxSize txtSize;
						//dc.GetTextExtent( txt, &txtSize.x, &txtSize.y);
						
						x1 = pos.x;
						x2 = pos.x + size.x;

						f32 delta = 1.0f - ( ( ( val - mElevatorOffset ) ) / breadth );
						y1 = y2 = ( pos.y + size.y * delta );
					
						graphDC.SetPen( wxPen( wxColour( 64, 32, 32 ), 1, wxDOT ) );
						//dc.DrawText( txt, x1 - txtSize.x, y1 - txtSize.y/2 );
						graphDC.DrawLine( x1 - pos.x+1, y1 - pos.y, x2 - pos.x, y2 - pos.y );
						graphDC.SetPen( backgroundLinesPen );
					}
				}
			}
		}

		//if( mBackgroundDC )
		{
			wxMemoryDC backgroundDC;
			wxMemoryDC graphDC;
			wxMemoryDC maskDC;

			backgroundDC.SelectObject( mBackgroundBitmap );
			graphDC.SelectObject( mGraphBitmap );
			maskDC.SelectObject( mMaskBitmap );

			dc.Blit( 0, 0, panelSize.x, panelSize.y, &backgroundDC, 0, 0 );
			//dc.Blit( pos.x, pos.y, size.x, size.y, &maskDC, 0, 0 );
			dc.Blit( pos.x, pos.y, size.x, size.y, &graphDC, 0, 0, wxCOPY, true );
		}

		for( u32 i = 0; i < graphButtons.fCount( ); ++i )
			graphButtons[ i ]->fPaint( dc );

		u32 id = ( *mGraph )->fGetID( );
		u32 idxcnt = 1;

		if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
			idxcnt = 2;
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
			idxcnt = 3;
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
			idxcnt = 4;
		else if( id == Rtti::fGetClassId< Math::tQuatf >( ) )
			idxcnt = 4;

		wxPen randomnessLinesPen( wxColour( 72, 72, 72 ), 1, wxDOT );
		dc.SetPen( randomnessLinesPen );
		
		// draw randomness lines if there are any!
		if( ( *mGraph )->fMinRandomness( ) != 0.f || ( *mGraph )->fMaxRandomness( ) != 0.f )
		{
			const u32 N = 128;
			for( u32 i = 0; i < N; ++i )
			{
				f32 f1 = ( f32 ) i / ( f32 ) ( N );
				f32 f2 = ( f32 ) (i+1) / ( f32 ) ( N );

				f32 x1 = pos.x + ( size.x * f1 );
				f32 x2 = pos.x + ( size.x * f2 );
				f32 breadth = mMaxY - mMinY;

				for( s32 idx = idxcnt-1; idx >= 0; --idx )
				{
					f32 val1 = fSampleGraph( f1, idx );
					f32 delta = 1.0f - ( ( ( val1 - mElevatorOffset ) ) / breadth );
					f32 y1 = ( pos.y + size.y * delta );

					f32 val2 = fSampleGraph( f2, idx );
					delta = 1.0f - ( ( ( val2 - mElevatorOffset ) ) / breadth );
					f32 y2 = ( pos.y + size.y * delta );
					//y2 += mNodeHeight * 0.5f;
					dc.DrawLine( x1, y1, x2, y2 );
				}
			}
		}

		dc.SetPen( *wxBLACK_PEN );
		
		f32 minrandomness = (*mGraph)->fMinRandomness( );
		f32 maxrandomness = (*mGraph)->fMaxRandomness( );

		if( mShowScrub )
		{
			f32 dx = pos.x + ( size.x * KeylineDelta );
			dc.DrawLine( dx, pos.y, dx, pos.y + size.y );
		}

		dc.SetBrush( *wxTRANSPARENT_BRUSH );

		if( mNewKeys.fCount( ) )
		{
			tEditorActionPtr action( new tSaveParticleSystemGraphsAction( mMainWindow->fGuiApp( ).fSelectionList( ) ) );

			for( u32 i = 0; i < mNewKeys.fCount( ); ++i )
			{
				if( i > 0 )
				{
					if( mNewKeys[ i ]->mKey->fX( ) == mNewKeys[ i - 1 ]->mKey->fX( ) )
					{
						mNewKeys[ i ]->mKey.fReset( mNewKeys[ i - 1 ]->mKey.fGetRawPtr( ) );
						continue;
					}
				}
			
				mNewKeys[ i ]->mKey = ( *mGraph )->fNewKeyFromKey( mNewKeys[ i ]->mKey );
				( *mGraph )->fAddKeyframe( mNewKeys[ i ]->mKey );
			}
			
			action->fEnd( );
			mMainWindow->fGuiApp( ).fActionStack( ).fAddAction( action );
			mSelectedNodes.fSetCount( 0 );
		}

		static Math::tVec3f col[ 4 ] = { Math::tVec3f( 255.f, 72.f, 72.f ),
										 Math::tVec3f( 72.f, 255.f, 72.f ),
										 Math::tVec3f( 72.f, 128.f, 255.f ),
										 Math::tVec3f( 255.f, 255.f, 72.f ) };

		static wxColour baseColours[ 4 ][ 3 ] = 
		{
			wxColour( col[ 0 ].x * .33f, col[ 0 ].y * .33f, col[ 0 ].z * .33f ),
			wxColour( col[ 0 ].x * .66f, col[ 0 ].y * .66f, col[ 0 ].z * .66f ),
			wxColour( col[ 0 ].x * 1.0f, col[ 0 ].y * 1.0f, col[ 0 ].z * 1.0f ),

			wxColour( col[ 1 ].x * .33f, col[ 1 ].y * .33f, col[ 1 ].z * .33f ),
			wxColour( col[ 1 ].x * .66f, col[ 1 ].y * .66f, col[ 1 ].z * .66f ),
			wxColour( col[ 1 ].x * 1.0f, col[ 1 ].y * 1.0f, col[ 1 ].z * 1.0f ),

			wxColour( col[ 2 ].x * .33f, col[ 2 ].y * .33f, col[ 2 ].z * .33f ),
			wxColour( col[ 2 ].x * .66f, col[ 2 ].y * .66f, col[ 2 ].z * .66f ),
			wxColour( col[ 2 ].x * 1.0f, col[ 2 ].y * 1.0f, col[ 2 ].z * 1.0f ),

			wxColour( col[ 3 ].x * .33f, col[ 3 ].y * .33f, col[ 3 ].z * .33f ),
			wxColour( col[ 3 ].x * .66f, col[ 3 ].y * .66f, col[ 3 ].z * .66f ),
			wxColour( col[ 3 ].x * 1.0f, col[ 3 ].y * 1.0f, col[ 3 ].z * 1.0f )
		};

		// okay time to draw the actual lines of the keyframes!
		for( u32 i = 0; i < fNumKeyframes( ); ++i )
		{
			tKeyframePtr key = fRawKeyframe( i );
			f32 x = pos.x + ( size.x * key->fX( ) );
			f32 breadth = mMaxY - mMinY;

			for( s32 idx = idxcnt-1; idx >= 0; --idx )
			{
				wxPen graphlinePen( baseColours[ idx ][ 0 ] );

				f32 val = fValue( key, idx );
				f32 minval = val + minrandomness;
				f32 maxval = val + maxrandomness;

				f32 delta1 = 1.0f - ( ( ( val - mElevatorOffset ) ) / breadth );
				f32 delta2 = 1.0f - ( ( ( minval - mElevatorOffset ) ) / breadth );
				f32 delta3 = 1.0f - ( ( ( maxval - mElevatorOffset ) ) / breadth );

				f32 y1 = ( pos.y + size.y * delta1 );
				f32 minY1 = ( pos.y + size.y * delta2 );
				f32 maxY1 = ( pos.y + size.y * delta3 );

				if( fNumKeyframes( ) == 1 )
				{
					// randomness area lines
					dc.SetPen( randomnessLinesPen );
					dc.DrawLine( x, minY1, pos.x+size.x, minY1 );
					dc.DrawLine( x, maxY1, pos.x+size.x, maxY1 );

					// main line
					dc.SetPen( graphlinePen );
					dc.DrawLine( x, y1, pos.x+size.x, y1 );
				}
				else if( i > 0 )
				{
					tKeyframePtr keyprev = fRawKeyframe( i-1 );
					f32 x2 = pos.x + ( size.x * keyprev->fX( ) );

					val = fValue( keyprev, idx );
					minval = val + minrandomness;
					maxval = val + maxrandomness;
					
					delta1 = 1.0f - ( ( ( val - mElevatorOffset ) ) / breadth );
					delta2 = 1.0f - ( ( ( minval - mElevatorOffset ) ) / breadth );
					delta3 = 1.0f - ( ( ( maxval - mElevatorOffset ) ) / breadth );

					f32 y2 = ( pos.y + size.y * delta1 );
					f32 minY2 = ( pos.y + size.y * delta2 );
					f32 maxY2 = ( pos.y + size.y * delta3 );

					// randomness area lines
					dc.SetPen( randomnessLinesPen );
					dc.DrawLine( x, minY1, x2, minY2 );
					dc.DrawLine( x, maxY1, x2, maxY2 );

					// main line
					dc.SetPen( graphlinePen );
					dc.DrawLine( x, y1, x2, y2 );
				}
				
				tNode* tFound = 0;
				
				for( u32 j = 0; j < mDrawNodePoints.fCount( ); ++j )
				{
					if( mDrawNodePoints[ j ]->mKey.fGetRawPtr( ) == key.fGetRawPtr( ) )
					{
						if( mDrawNodePoints[ j ]->mIdx == idx )
						{
							tFound = mDrawNodePoints[ j ];
							break;
						}
					}
				}

				if( !tFound )
				{
					tNode *n = new tNode( );
					if( idx == 0 )
						n->mNodeRadius = 9.0f;
					else if( idx == 1 )
						n->mNodeRadius = 13.0f;
					else if( idx == 2 )
						n->mNodeRadius = 17.0f;
					else if( idx == 3 )
						n->mNodeRadius = 21.0f;
					n->mIdx = idx;
					n->mPos = Math::tVec2f( x-n->mNodeRadius*.5f, y1-n->mNodeRadius*.5f );
					n->mKey = key;

					mDrawNodePoints.fPushBack( n );

					for( u32 i = 0; i < mNewKeys.fCount( ); ++i )
					{
						if( n->mIdx == mNewKeys[ i ]->mIdx )
						{
							mSelectedNodes.fFindOrAdd( n );
							mNewKeys.fErase( i );
							break;
						}
					}
				}
				else
				{
					tFound->mPos = Math::tVec2f( x-tFound->mNodeRadius*.5f, y1-tFound->mNodeRadius*.5f );
				}
			}
		}

		mNewKeys.fSetCount( 0 );
		std::sort( mDrawNodePoints.fBegin( ), mDrawNodePoints.fEnd( ), tDrawNodeSort( ) );

		dc.DestroyClippingRegion( );

		wxRect rc = mBounds;
		rc.Inflate( 22.f*.5f, 21.f*.5f );	// just inflate a bit to help encompass the nodes

		dc.SetPen( *wxBLACK_PEN );

		if( mShowScrub )
		{
			f32 dx = pos.x + ( size.x * KeylineDelta );
			f32 dropRadius = 6.f;
			f32 half = dropRadius * .5f;
			for( u32 i = 0; i < idxcnt; ++i )
			{
				f32 val = fSampleGraph( KeylineDelta, i );
				f32 delta = 1.0f - ( ( ( val - mElevatorOffset ) ) / breadth );
				val = ( pos.y + size.y * delta );
				dc.SetBrush( wxBrush( baseColours[ i ][ 2 ] ) );
				dc.DrawEllipse( dx - half, val - half,  dropRadius, dropRadius );
			}
		}

		for( s32 i = mDrawNodePoints.fCount( ) - 1; i >= 0 && mDrawNodePoints.fCount( ); --i )
		{
			tNode* node = mDrawNodePoints[ i ];
			Math::tVec2f dp = node->mPos;
			tKeyframePtr key = node->mKey;

			b32 selected( false );
			if( rc.Contains( wxPoint( dp.x, dp.y ) ) )
			{
				f32 x = dp.x + node->mNodeRadius*.5f;
				f32 y = dp.y + node->mNodeRadius*.5f;

				if( mSelectedNodes.fFind( node ) || mNewlySelectedNodes.fFind( node ) )
				{
					selected = true;
					dc.SetBrush( wxBrush( baseColours[ node->mIdx ][ 2 ] ) );
				}
				else if( mDrawNodePoints[ i ] == mHighlightedNode )
					dc.SetBrush( wxBrush( baseColours[ node->mIdx ][ 1 ] ) );
				else
					dc.SetBrush( wxBrush( baseColours[ node->mIdx ][ 0 ] ) );

				if( selected )
				{					
					// draw a vertical line through the selected keyframes so it's easy
					// to find the 'x' value of the selected node.
					dc.SetPen( wxPen( wxColour( 172, 172, 172) ) );
					dc.DrawLine( x, y, x, pos.y + size.y - 1 );
				}

				dc.SetPen( *wxBLACK_PEN );
				dc.DrawEllipse( dp.x, dp.y, node->mNodeRadius, node->mNodeRadius);
				
				if( selected )
				{
					// draw the textual value over the node if it's selected!
					wxString val = tSigFxKeyframe::fGetKeyframeText( key );
					wxSize txtSize;
					dc.GetTextExtent( val, &txtSize.x, &txtSize.y);

					f32 xtext = x - txtSize.x*.5f;
					f32 ytext = y - 11.f - txtSize.y;

					while( ytext < pos.y )
						ytext += 0.5f;
					
					dc.DrawText( val, xtext, ytext );					
				}
			}
		}

		if( mDoSelection )
		{		
			dc.SetClippingRegion( mBounds );
			dc.SetPen( wxPen( wxColour( *wxRED ), 1, wxDOT ) );
			dc.SetBrush( *wxTRANSPARENT_BRUSH );
			dc.DrawRectangle( mSelectionBox );
			dc.SetPen( *wxBLACK_PEN );

			dc.DestroyClippingRegion( );
		}
	}


	u32 tSigFxGraph::fGetNumberIndices( )
	{
		u32 count( 0 );
		u32 id = (*mGraph)->fGetID( );

		if( id == Rtti::fGetClassId< f32 >( ) )
			count = 1;
		else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
			count = 2;
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
			count = 3;
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
			count = 4;
		else if( id == Rtti::fGetClassId< Math::tQuatf >( ) )
			count = 4;

		return count;
	}

	f32 tSigFxGraph::fMaximumNodeDifference( )
	{
		f32 maxY = 1.f;
		f32 minY = 0.f;

		for( u32 idx = 0; idx < fGetNumberIndices( ); ++idx )
		{
			for( u32 i = 0; i < fNumKeyframes( ); ++i )
			{
				tKeyframePtr key = fRawKeyframe( i );

				f32 val = fValue( key, idx );
				
				if( val < minY )	minY = val;
				if( val > maxY )	maxY = val;
			}
		}

		maxY += (*mGraph)->fMaxRandomness( );
		minY += (*mGraph)->fMinRandomness( );

		return maxY - minY;
	}

	f32 tSigFxGraph::fGraphHeight( )
	{
		return mMaxY - mMinY;
	}

	void tSigFxGraph::fFrame( )
	{
		mMaxY = 1.f;
		mMinY = 0.f;

		for( u32 idx = 0; idx < fGetNumberIndices( ); ++idx )
		{
			for( u32 i = 0; i < fNumKeyframes( ); ++i )
			{
				tKeyframePtr key = fRawKeyframe( i );

				f32 val = fValue( key, idx );
				
				if( val < mMinY )	mMinY = val;
				if( val > mMaxY )	mMaxY = val;
			}
		}

		mMaxY += (*mGraph)->fMaxRandomness( );
		mMinY += (*mGraph)->fMinRandomness( );

		if( mMaxY - mMinY < 1.f )
			mMaxY += 1.f;

		mElevatorOffset = mMinY;
		mDirty = true;
	}

	
}


