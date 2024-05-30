#include "ToolsPch.hpp"
#include "tDAGNode.hpp"
#include "tDAGNodeCanvas.hpp"
#include "Math/tIntersectionRayAabb.hpp"

namespace Sig
{

	namespace
	{
		const wxPen gNodeNoBorderPen( wxColour( 0x00, 0x00, 0x00 ), 0, wxTRANSPARENT );
		const wxPen gNodeOutlinePen( wxColour( 0x00, 0x00, 0x00 ), 1, wxSOLID );
		const wxPen gNodeOutlineSelectedPen( wxColour( 0x00, 0xff, 0x00 ), 1, wxSOLID );
		const wxPen gConnectionSelectedPen( wxColour( 0x00, 0xff, 0x00 ), 3, wxSOLID );

		const wxBrush gNodeShadowBrush( wxColour( 0x11, 0x11, 0x11 ) );
		const wxBrush gNodeShadowSelectedBrush( wxColour( 0x11, 0x99, 0x11 ) );

		const wxBrush gNodeMainBrush( wxColour( 0xcc, 0xcc, 0xcc ) );


		const wxColour gDefaultOutputTextColor = wxColour( 0x11, 0x11, 0x11 );
		const wxFont gNodeTextFont( 8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT );
		const wxFont gConnectionTextFont( 10, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT );
		const wxColour gConnectionTextColor( 255, 255, 255 );

		const wxBrush gNodeCloseBoxBrush( wxColour( 0xff, 0xff, 0xff ) );
		const wxPen gNodeCloseBoxPen( wxColour( 0xff, 0x00, 0x00 ), 2, wxSOLID );
		const u32 cCloseBoxSize = 12;

		static inline int fStringWidth( const wxString& str )
		{
			wxAppBase* app = static_cast<wxAppBase*>(wxAppConsole::GetInstance( ));

			if( app )
			{
				wxClientDC dc( app->GetTopWindow( ) );
				dc.SetFont( gNodeTextFont );
				return dc.GetTextExtent( str ).x;
			}
			else
			{
				// fall-back for tools with no wxApp
				return ( int )str.size( ) * 9;
			}
		}

		static inline int fNodeWidth( const wxString& name )
		{
			return fMax<int>( 50, fStringWidth( name ) + 30 );
		}

		static inline int fNodeWidth( const std::string& name )
		{
			wxString tempStr = name.c_str( );
			return fNodeWidth( tempStr );
		}
	}

	///
	/// \section tDAGNodeOutput
	///

	tDAGNodeOutput::tDAGNodeOutput( const std::string& name, tDAGNode& owner, const wxColour& color, u32 maxConnections, tEdge edge, f32 radius )
		: mOwner( owner )
		, mColor( color )
		, mFillBrush( wxColor( color.Red( ) / 3, color.Green( ) / 3, color.Blue( ) / 3 ) )
		, mFillConnectedBrush( color )
		, mOutlinePen( wxColour( 0x00, 0x00, 0x00 ), 1, wxSOLID )
		, mIO( cOutput )
		, mRadius( radius )
		, mCircleXOffset( -6.f )
		, mAnchorXOffset( radius )
		, mMaxConnections( maxConnections )
		, mEdge( edge )
		, mName( name )
		, mLabelXOffset( 14.f )
		, mTextColor( gDefaultOutputTextColor )
	{
	}
	b32 tDAGNodeOutput::fHasConnection( tDAGNodeConnection* connection ) const
	{
		return mConnections.fFind( connection ) != 0;
	}
	void tDAGNodeOutput::fAddConnection( tDAGNodeConnection* connection )
	{
		if( fHasConnection( connection ) ) return; // already has it
		sigassert( mConnections.fCount( ) < mMaxConnections );
		mConnections.fFindOrAdd( connection );
	}
	void tDAGNodeOutput::fRemoveConnection( tDAGNodeConnection* connection )
	{
		mConnections.fFindAndErase( connection );
	}
	void tDAGNodeOutput::fClearConnections( )
	{
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			mOwner.fOwner( )->fDeleteConnection( tDAGNodeConnectionPtr( mConnections[ i ] ) );

		mConnections.fSetCount( 0 );
	}
	void tDAGNodeOutput::fOnPaint( wxPoint parentTopLeft, f32 offsets[cEdgeCount], wxDC& dc )
	{		
		const b32 connected = mConnections.fCount( ) > 0;
		const int diameter = fRound<int>( mRadius * 2.f );
		const wxSize dimensions = mOwner.fDimensions( );

		const u32 cTopSpacing = 4; //spacing of circles along the top
		const wxPoint cCornerOffset( diameter, -mRadius );

		wxPoint offset( 0,0 );
		f32 circleXOffset = 0;

		switch( mEdge )
		{
		case cTop:
			{
				offset.x = offsets[ cTop ];
				offset.y = 0;
				offset += cCornerOffset;

				offsets[ cTop ] += diameter + cTopSpacing;
				break;
			}
		case cBottom:
			{
				offset.x = offsets[ cBottom ];
				offset.y = dimensions.y - diameter;
				offset.x += cCornerOffset.x;
				offset.y -= cCornerOffset.y;

				offsets[ cBottom ] += diameter + cTopSpacing;
				break;
			}
		case cRight:
			offset.x = dimensions.x;
			offset.y = offsets[ cLeft ];
			offsets[ cLeft ] += fHeight( );
			break;

		case cLeft:
			offset.y = offsets[ cLeft ];
			offsets[ cLeft ] += fHeight( );
			break;
		}

		mParentOffset = offset;
		const wxPoint effectiveOrigin = parentTopLeft + mParentOffset;

		b32 selected = mOwner.fOwner( )->fSelectedOutputs( ).fCount( ) > 0 ? mOwner.fOwner( )->fSelectedOutputs( )[ 0 ].fGetRawPtr( ) == this : false;
		dc.SetPen( selected ? gNodeOutlineSelectedPen : mOutlinePen );
		
		b32 hasData = connected && mOwner.fOwner( )->fLightConnectedConnections( );
		if( mData ) hasData = mData->fHasData( );
		dc.SetBrush( hasData ? mFillConnectedBrush : mFillBrush );

		const wxRect circleRect = fCircleRect( parentTopLeft );
		const b32 halfCircles = false;
		b32 drawText = true;

		switch( mEdge ) 
		{
		case cTop:
			drawText = false;
			if( halfCircles )
			{
				dc.DrawEllipticArc( circleRect.x, circleRect.y, circleRect.width, circleRect.height, 5, 180 );
				break;
			}
		case cBottom:
			drawText = false;
			if( halfCircles )
			{
				dc.DrawEllipticArc( circleRect.x, circleRect.y, circleRect.width, circleRect.height, 190, 355 );
				break;
			}
		case cLeft:
		case cRight:
			dc.DrawEllipse( circleRect );

			if( mName.length( ) > 0 )
			{
				dc.SetTextForeground( mTextColor );

				u32 left = 0;
				if( mIO == cOutput ) 
					left = parentTopLeft.x + dimensions.x - dc.GetTextExtent( mName ).x - mLabelXOffset;
				else
					left = parentTopLeft.x + mLabelXOffset;

				if( drawText ) dc.DrawText( mName, left, effectiveOrigin.y );
			}
			break;
		}
	}
	wxPoint tDAGNodeOutput::fGetAnchorPoint( ) const
	{
		const wxPoint& offset = mOwner.fTopLeft( );
		const wxPoint normal = fGetConnectionNormal( mRadius );

		return offset + mParentOffset + normal + wxPoint( fRound<int>( mCircleXOffset + mRadius ), fRound<int>( mRadius ) );
	}
	wxPoint tDAGNodeOutput::fGetConnectionNormal( f32 length ) const
	{
		int rLen = fRound<int>( length );
		switch( mEdge )
		{
		case cTop: return wxPoint( 0, -rLen );
		case cBottom: return wxPoint( 0, rLen );
		case cLeft: return wxPoint( -rLen, 0 );
		case cRight: return wxPoint( rLen, 0 );
		}

		return wxPoint( 0,0 );
	}
	b32 tDAGNodeOutput::fContains( const wxPoint& p ) const
	{
		const Math::tVec2f fromCenter = Math::tVec2f( p.x, p.y ) - fGetCenterPoint( );
		return fromCenter.fLengthSquared( ) <= Math::fSquare( mRadius );
	}
	b32 tDAGNodeOutput::fIntersects( const wxPoint& parentTopLeft, const wxRect& rect ) const
	{
		return rect.Intersects( fCircleRect( parentTopLeft ) );
	}
	wxRect tDAGNodeOutput::fCircleRect( const wxPoint& parentTopLeft ) const
	{
		const int diameter = fRound<int>( mRadius * 2.f );
		const wxPoint effectiveOrigin = parentTopLeft + mParentOffset;
		return wxRect( effectiveOrigin.x + mCircleXOffset, effectiveOrigin.y, diameter, diameter ); 
	}
	u32 tDAGNodeOutput::fHeight( ) const 
	{ 
		return mName.length( ) > 0 && (mEdge != cTop && mEdge != cBottom) ? tDAGNode::cLineSpacing : 0;
	}

	u32 tDAGNodeOutput::fIndex( ) const 
	{ 
		return mOwner.fOutputIndex( this ); 
	}

	tDAGNodeInput::tDAGNodeInput( const std::string& name, tDAGNode& owner, const wxColour& color, u32 maxConnections, tEdge edge, f32 radius )
		: tDAGNodeOutput( name, owner, color, maxConnections, edge, radius )
	{ 
		mAnchorXOffset = -mRadius;
		mIO	= cInput;
	}

	u32 tDAGNodeInput::fIndex( ) const 
	{ 
		return mOwner.fInputIndex( this ); 
	}


	tDAGNodeText::tDAGNodeText( const std::string& name, const std::string& value, tDAGNode& owner, b32 hideIfNullText )
		: mOwner( owner )
		, mName( name )
		, mValue( value )
		, mLabelXOffset( 14.f )
		, mHideIfNullText( hideIfNullText )
	{
	}

	void tDAGNodeText::fOnPaint( wxPoint parentTopLeft, f32 offsets[tDAGNodeOutput::cEdgeCount], wxDC& dc )
	{
		if( !mHideIfNullText || mValue.length( ) > 0 )
		{
			const wxSize dimensions = mOwner.fDimensions( );

			wxPoint offset( 0,0 );
			offset.x = 0;
			offset.y = offsets[ tDAGNodeOutput::cLeft ];
			offsets[ tDAGNodeOutput::cLeft ] += fHeight( );

			mParentOffset = offset;
			const wxPoint effectiveOrigin = parentTopLeft + mParentOffset;

			dc.SetTextForeground( wxColour( 0x11, 0x11, 0x11 ) );

			u32 left = parentTopLeft.x + mLabelXOffset;

			dc.DrawText( fTotalString( ), left, effectiveOrigin.y );
		}
	}

	u32 tDAGNodeText::fHeight( ) const
	{
		if( mHideIfNullText && mValue.length( ) == 0 )
			return 0;
		else
			return tDAGNode::cLineSpacing;
	}

	wxString tDAGNodeText::fTotalString( ) const
	{
		wxString tempStr;

		if( mName.length( ) )
		{
			tempStr = mName.c_str( );
			tempStr += " - ";
		}

		return tempStr + mValue.c_str( );
	}


	///
	/// \section tDAGNode
	///

	tDAGNode::tDAGNode( const std::string& name, const wxColour& titleBarColor, const wxColour& titleBarTextColor, const wxPoint& topLeft )
		: mOwner( NULL )
		, mName( name )
		, mTopLeft( topLeft )
		, mDimensions( fNodeWidth( name ), 160 )
		, mTitleBarBrush( titleBarColor )
		, mTitleBarTextColor( titleBarTextColor )
		, mRoundedRadius( 12 )
		, mTitleBarHeight( 12 )
		, mTitleBarTextXOffset( mRoundedRadius )
		, mTitleBarTextYOffset( 2 )
		, mDropShadowOffset( 2 )
		, mCloseBoxWidth( cCloseBoxSize )
		, mCloseBoxHeight( mCloseBoxWidth )
		, mCloseBoxXOffset( mDimensions.x - mRoundedRadius - mCloseBoxWidth + 2 )
		, mCloseBoxYOffset( 3 )
		, mBaseConnectionOffset( mTitleBarHeight + 2)
	{
	}
	tDAGNode::~tDAGNode( )
	{
	}

	void tDAGNode::fSetName( const std::string& name )
	{
		mName = name;
		fComputeDimensions( );
	}

	void tDAGNode::fOnPaint( const wxPoint& topLeftOrigin, const wxSize& clipDims, wxDC& dc, tRenderStyle renderStyle )
	{
		const wxPoint effectiveTopLeft = mTopLeft - topLeftOrigin;

		//dc.SetLogicalScale( 0.5f, 0.5f );
		//dc.SetUserScale( 0.5f, 0.5f );
		//dc.SetSystemScale( 0.5f, 0.5f );

		// drop shadow
		dc.SetPen( gNodeNoBorderPen );
		dc.SetBrush( mSelected ? gNodeShadowSelectedBrush : gNodeShadowBrush );
		dc.DrawRoundedRectangle( effectiveTopLeft.x + mDropShadowOffset, effectiveTopLeft.y + mDropShadowOffset, mDimensions.x, mDimensions.y, mRoundedRadius );

		const wxPen& outlinePen = mSelected ? gNodeOutlineSelectedPen : gNodeOutlinePen;

		// window title bar
		dc.SetPen( outlinePen );
		dc.SetBrush( mTitleBarBrush );
		dc.DrawRoundedRectangle( effectiveTopLeft.x, effectiveTopLeft.y, mDimensions.x, mDimensions.y, mRoundedRadius );

		// window title text
		dc.SetTextForeground( mTitleBarTextColor );
		dc.SetFont( gNodeTextFont );
		wxSize textSize = dc.GetTextExtent( fDisplayName( ) );
		dc.DrawText( fDisplayName( ), effectiveTopLeft.x + mTitleBarTextXOffset, effectiveTopLeft.y + mTitleBarHeight - textSize.y - mTitleBarTextYOffset );

		// close box
		const wxPoint closeBoxTL( effectiveTopLeft.x + mCloseBoxXOffset, effectiveTopLeft.y + mCloseBoxYOffset );
		dc.SetBrush( gNodeCloseBoxBrush );
		dc.DrawRectangle( closeBoxTL.x, closeBoxTL.y, mCloseBoxWidth, mCloseBoxHeight );
		dc.SetPen( gNodeCloseBoxPen );
		const wxPoint closeBoxLineTL = wxPoint( closeBoxTL.x + 2, closeBoxTL.y + 2 );
		const wxPoint closeBoxLineBR = wxPoint( closeBoxTL.x + 2 + mCloseBoxWidth - 5, closeBoxTL.y + 2 + mCloseBoxHeight - 5 );
		dc.DrawLine( closeBoxLineTL, closeBoxLineBR );
		dc.DrawLine( wxPoint( closeBoxLineBR.x, closeBoxLineTL.y ), wxPoint( closeBoxLineTL.x, closeBoxLineBR.y ) );

		// main panel
		dc.SetClippingRegion( effectiveTopLeft.x, effectiveTopLeft.y + mTitleBarHeight, mDimensions.x, mDimensions.y - mTitleBarHeight );
		dc.SetPen( outlinePen );
		dc.SetBrush( gNodeMainBrush );
		dc.DrawRoundedRectangle( effectiveTopLeft.x, effectiveTopLeft.y, mDimensions.x, mDimensions.y, mRoundedRadius );
		dc.DestroyClippingRegion( );

		// draw inputs and output in main panel
		f32 offsets[ tDAGNodeOutput::cEdgeCount ] = { 0 };
		offsets[ tDAGNodeOutput::cLeft ] = mBaseConnectionOffset;

		for( u32 i = 0; i < mTexts.fCount( ); ++i )
			mTexts[ i ]->fOnPaint( effectiveTopLeft, offsets, dc );

		switch( renderStyle )
		{
		case cRenderOutputsBeforeInputs:
			{
				for( u32 i = 0; i < mOutputs.fCount( ); ++i )
					mOutputs[ i ]->fOnPaint( effectiveTopLeft, offsets, dc );

				for( u32 i = 0; i < mInputs.fCount( ); ++i )
					mInputs[ i ]->fOnPaint( effectiveTopLeft, offsets, dc );
			}
		break;
		case cRenderInputsBeforeOutputs:
			{
			for( u32 i = 0; i < mInputs.fCount( ); ++i )
				mInputs[ i ]->fOnPaint( effectiveTopLeft, offsets, dc );

			for( u32 i = 0; i < mOutputs.fCount( ); ++i )
				mOutputs[ i ]->fOnPaint( effectiveTopLeft, offsets, dc );
			}
			break;
		case cRenderOutputsNextToInputs:
			{
				f32 store[ tDAGNodeOutput::cEdgeCount ];
				memcpy( store, offsets, sizeof( f32 ) * array_length( store ) );

				for( u32 i = 0; i < mInputs.fCount( ); ++i )
					mInputs[ i ]->fOnPaint( effectiveTopLeft, offsets, dc );

				// restore the offsets, things could potentially overlap, but this is what the user requested
				memcpy( offsets, store, sizeof( f32 ) * array_length( store ) );
				for( u32 i = 0; i < mOutputs.fCount( ); ++i )
					mOutputs[ i ]->fOnPaint( effectiveTopLeft, offsets, dc );
			}
			break;
		}

	}
	void tDAGNode::fSetPosition( const wxPoint& pos )
	{
		mTopLeft = pos;
	}
	void tDAGNode::fMove( const wxPoint& delta )
	{
		mTopLeft += delta;
	}
	tDAGNode::tAction tDAGNode::fOnLeftButtonDown( const wxPoint& canvasP, wxMouseEvent& event, tActionContext& contextOut )
	{
		const wxPoint relativeP = canvasP - mTopLeft;

		const wxPoint closeBoxTL( mCloseBoxXOffset, mCloseBoxYOffset );
		const wxPoint closeBoxBR( mCloseBoxXOffset + mCloseBoxWidth, mCloseBoxYOffset + mCloseBoxHeight );
		if( fInBounds( relativeP.x, closeBoxTL.x, closeBoxBR.x ) && fInBounds( relativeP.y, closeBoxTL.y, closeBoxBR.y ) )
			return cActionDelete;

		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
		{
			if( mOutputs[ i ]->fContains( relativeP ) )
			{
				contextOut.mOutput = mOutputs[ i ];
				if( !mOutputs[ i ]->fMaxConnectionsReached( ) )
					return cActionBeginConnection;
				else
					return cActionOutputSelect;
			}
		}

		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			if( !mInputs[ i ]->fContains( relativeP ) )
				continue;
			if( mInputs[ i ]->fConnectionList( ).fCount( ) > 0 )
			{
				contextOut.mInput = mInputs[ i ];
				contextOut.mConnection.fReset( mInputs[ i ]->fConnectionList( ).fFront( ) );
				return cActionRedoConnection;
			}
			else
			{
				contextOut.mInput = mInputs[ i ];
				return cActionBeginConnection;
			}
		}

		if( fContains( canvasP ) )
		{
			return cActionBeginDrag;
		}
			
		return cActionNone;
	}
	tDAGNode::tAction tDAGNode::fOnRightButtonUp( const wxPoint& canvasP, wxMouseEvent& event, tActionContext& contextOut )
	{
		const wxPoint relativeP = canvasP - mTopLeft;

		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
		{
			if( mOutputs[ i ]->fContains( relativeP ) )
			{
				contextOut.mOutput = mOutputs[ i ];
				return cActionUserOutput;
			}
		}

		if( fContains( canvasP ) )
		{
			return cActionUserNode;
		}

		return cActionNone;
	}
	b32 tDAGNode::fTryToCompleteConnection( const wxPoint& p, tDAGNodeConnection& connection, b32 preventCycles )
	{
		const wxPoint relativeP = p - mTopLeft;
		if( connection.fOutput( ) )
		{
			// already have an output, need to try to connect with inputs
			for( u32 i = 0; i < mInputs.fCount( ); ++i )
			{
				if( !mInputs[ i ]->fMaxConnectionsReached( ) && mInputs[ i ]->fContains( relativeP ) )
				{
					if( connection.fComplete( mInputs[ i ], preventCycles ) )
						return true;
				}
			}
		}
		else
		{
			sigassert( connection.fInput( ) );
			// already have an input, need to try to connect with output
			for( u32 i = 0; i < mOutputs.fCount( ); ++i )
			{
				if( !mOutputs[ i ]->fMaxConnectionsReached( ) && mOutputs[ i ]->fContains( relativeP ) )
				{
					if( connection.fComplete( mOutputs[ i ], preventCycles ) )
						return true;
				}
			}
		}

		return false;
	}
	void tDAGNode::fCollectAllConnections( tDAGNodeOutput::tConnectionList& connections ) const
	{
		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
		{
			for( u32 j = 0; j < mOutputs[ i ]->fConnectionList( ).fCount( ); ++j )
				connections.fFindOrAdd( mOutputs[ i ]->fConnectionList( )[ j ] );
		}

		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			for( u32 j = 0; j < mInputs[ i ]->fConnectionList( ).fCount( ); ++j )
				connections.fFindOrAdd( mInputs[ i ]->fConnectionList( )[ j ] );
		}
	}
	b32 tDAGNode::fConnectsTo( const tDAGNode& node ) const
	{
		if( this == &node )
			return true;

		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
		{
			const tDAGNodeOutput::tConnectionList& outputConnex = mOutputs[ i ]->fConnectionList( );
			for( u32 i = 0; i < outputConnex.fCount( ); ++i )
			{
				if( !outputConnex[ i ]->fInput( ) )
					continue;
				if(  outputConnex[ i ]->fInput( )->fOwner( ).fConnectsTo( node ) )
					return true;
			}
		}

		return false;
	}
	b32 tDAGNode::fContains( const wxPoint& canvasP ) const
	{
		return ( fInBounds( canvasP.x, mTopLeft.x, mTopLeft.x + mDimensions.x ) &&
			fInBounds( canvasP.y, mTopLeft.y, mTopLeft.y + mDimensions.y ) );
	}
	b32 tDAGNode::fIntersects( const wxRect& rect ) const
	{
		return rect.Intersects( wxRect( mTopLeft, mTopLeft + mDimensions ) );
	}
	void tDAGNode::fComputeDimensions( )
	{
		s32 height = 0;
		s32 width = fNodeWidth( mName ) + cCloseBoxSize;
		b32 inputsOnTop = false;
		
		for( u32 i = 0; i < mTexts.fCount( ); ++i )
		{
			height += mTexts[ i ]->fHeight( );
			width = fMax( width, fNodeWidth( mTexts[ i ]->fTotalString( ) ) );
		}

		for( u32 i = 0; i < mInputs.fCount( ); ++i )
		{
			switch( mInputs[ i ]->fEdge( ) ) 
			{
			case tDAGNodeOutput::cTop:
				inputsOnTop = true;
				break;
			case tDAGNodeOutput::cLeft:
			case tDAGNodeOutput::cRight:
				height += mInputs[ i ]->fHeight( );
				width = fMax( width, fNodeWidth( mInputs[ i ]->fName( ) ) );
				break;
			}
		}

		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
		{
			switch( mOutputs[ i ]->fEdge( ) ) 
			{
			case tDAGNodeOutput::cLeft:
			case tDAGNodeOutput::cRight:
				height += mOutputs[ i ]->fHeight( );
				width = fMax( width, fNodeWidth( mOutputs[ i ]->fName( ) ) );
				break;
			}
		}

		if( height == 0 ) 
			height += mRoundedRadius + cLineSpacing / 2; //default size, no items
		else 
			height += mRoundedRadius / 2 + 2; //some items, + little extra space along the bottom

		mTitleBarHeight = inputsOnTop ? 22 : 20;
		mBaseConnectionOffset = mTitleBarHeight + 2;
		height += mBaseConnectionOffset;

		mDimensions.x = width;
		mDimensions.y = height;

		mCloseBoxXOffset = mDimensions.x - mRoundedRadius - mCloseBoxWidth + 2;
		mCloseBoxYOffset = (mTitleBarHeight - mCloseBoxHeight) / 2;
	}

	const u32 tDAGNode::fInputIndex( const tDAGNodeInput* of ) const
	{
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
			if( mInputs[ i ] == of ) return i;

		return ~0;
	}

	const u32 tDAGNode::fOutputIndex( const tDAGNodeOutput* of ) const
	{
		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
			if( mOutputs[ i ] == of ) return i;

		return ~0;
	}

	tDAGNodeInputPtr tDAGNode::fInput( const std::string& name ) const
	{
		for( u32 i = 0; i < mInputs.fCount( ); ++i )
			if( mInputs[ i ]->fName( ) == name ) return mInputs[ i ];

		return tDAGNodeInputPtr();
	}

	tDAGNodeOutputPtr tDAGNode::fOutput( const std::string& name ) const
	{
		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
			if( mOutputs[ i ]->fName( ) == name ) return mOutputs[ i ];

		return tDAGNodeOutputPtr();
	}

	void tDAGNode::fClearOutputs( )
	{
		for( u32 i = 0; i < mOutputs.fCount( ); ++i )
			mOutputs[ i ]->fClearConnections( );

		mOutputs.fSetCount( 0 );
	}


	///
	/// \section tDAGNodeConnection
	///

	const wxColor tDAGNodeConnection::gDefaultConnectionColor(0x00, 0x00, 0x00 );

	tDAGNodeConnection::tDAGNodeConnection( const tDAGNodeInputPtr& start, const wxPoint& currentPos )
		: mInput( start )
		, mStartPos( start->fGetAnchorPoint( ) )
		, mText( "" )
		, mTextPosition( 30.f )
		, mColor( gDefaultConnectionColor )
	{
		mEndPos = mStartPos;
		mInput->fAddConnection( this );
	}
	tDAGNodeConnection::tDAGNodeConnection( const tDAGNodeOutputPtr& start, const wxPoint& currentPos )
		: mOutput( start )
		, mStartPos( start->fGetAnchorPoint( ) )
		, mText( "" )
		, mTextPosition( 30.f )
		, mColor( gDefaultConnectionColor )
	{
		mEndPos = mStartPos;
		mOutput->fAddConnection( this );
	}
	tDAGNodeConnection::tDAGNodeConnection( const tDAGNodeInputPtr& i, const tDAGNodeOutputPtr& o, const wxString& text, f32 textPos )
		: mInput( i )
		, mOutput( o )
		, mText( text )
		, mTextPosition( textPos )
		, mColor( gDefaultConnectionColor )
	{
		mInput->fAddConnection( this );
		mOutput->fAddConnection( this );
		mStartPos = mOutput->fGetAnchorPoint( );
		mEndPos = mInput->fGetAnchorPoint( );
	}
	tDAGNodeConnection::~tDAGNodeConnection( )
	{
		fUnhook( );
	}
	void tDAGNodeConnection::fUpdate( const wxPoint& currentPos )
	{
		mEndPos = currentPos;
	}
	void tDAGNodeConnection::fOnPaint( const wxPoint& topLeftOrigin, const wxSize& clipDims, b32 curved, wxDC& dc )
	{
		const b32 doSpline = curved; //!( GetAsyncKeyState( VK_SPACE ) & 0x8000 );

		tGrowableArray<wxPoint> points;
		fGetPoints( topLeftOrigin, points, doSpline );

		//const wxPen& pen = mSelected ? gConnectionSelectedPen : wxPen( mColor, 1, wxSOLID );
		const wxPen& pen = wxPen( mColor, mSelected ? 3 : 1, wxSOLID );
		dc.SetPen( pen );

		if( doSpline )
			dc.DrawSpline( points.fCount( ), points.fBegin( ) );
		else
			dc.DrawLines( points.fCount( ), points.fBegin( ) );

		// Render lable text
		dc.SetTextForeground( gConnectionTextColor );
		dc.SetFont( gConnectionTextFont );
		
		Math::tVec2f delta = Math::tVec2f(points.fFront( ).x,points.fFront( ).y) - Math::tVec2f(points.fBack( ).x,points.fBack( ).y);
		delta.fSetLength( mTextPosition );

		wxPoint start = mEndPos;
		if( mInput && !mOutput ) start = mStartPos;

		f32 width = dc.GetTextExtent( mText ).x;
		wxPoint textStart = wxPoint( delta.x - width, delta.y ) + start - topLeftOrigin;
		dc.DrawText( mText, textStart );
	}
	void tDAGNodeConnection::fGetPoints( const wxPoint& topLeftOrigin, tGrowableArray<wxPoint>& points, b32 doSpline )
	{
		const u32 pushOut = 6;
		wxPoint endPushOut(0, 0);
		wxPoint startPushOut(0, 0);

		if( mOutput && mInput )
		{
			mStartPos = mOutput->fGetAnchorPoint( );
			startPushOut = mOutput->fGetConnectionNormal( pushOut );

			mEndPos = mInput->fGetAnchorPoint( );
			endPushOut = mInput->fGetConnectionNormal( pushOut );
		}
		else if( mInput )
		{
			mStartPos = mInput->fGetAnchorPoint( );
			startPushOut = mInput->fGetConnectionNormal( pushOut );
		}
		else if( mOutput )
		{
			mStartPos = mOutput->fGetAnchorPoint( );
			startPushOut = mOutput->fGetConnectionNormal( pushOut );
		}

		const wxPoint delta = mEndPos - mStartPos;
		wxPoint midPoint0 = mStartPos + wxPoint( delta.x / 2, delta.y / 2 );
		wxPoint midPoint1 = midPoint0;
		midPoint0.y -= delta.y / 4;
		midPoint1.y += delta.y / 4;
		midPoint0.x -= delta.x / 3;
		midPoint1.x += delta.x / 3;

		points.fPushBack( mStartPos );
		points.fPushBack( mStartPos + startPushOut );
		if( doSpline )
		{
			points.fPushBack( midPoint0 );
			points.fPushBack( midPoint1 );
		}
		points.fPushBack( mEndPos + endPushOut );
		points.fPushBack( mEndPos );

		for( u32 i = 0; i < points.fCount( ); ++i )
			points[ i ] -= topLeftOrigin;
	}
	void tDAGNodeConnection::fRehook( )
	{
		if( mInput )
			mInput->fAddConnection( this );
		if( mOutput )
			mOutput->fAddConnection( this );
	}
	void tDAGNodeConnection::fUnhook( )
	{
		if( mInput )
			mInput->fRemoveConnection( this );
		if( mOutput )
			mOutput->fRemoveConnection( this );
	}
	void tDAGNodeConnection::fRemoveInput( )
	{
		sigassert( mInput && mOutput );
		mStartPos = mOutput->fGetAnchorPoint( );
		mEndPos = mInput->fGetAnchorPoint( );
		mInput->fRemoveConnection( this );
		mInput.fRelease( );
	}
	b32 tDAGNodeConnection::fComplete( const tDAGNodeInputPtr& input, b32 preventCycles )
	{
		sigassert( mOutput && !mInput );

		// before completing connection, check for cycle in node connections and reject
		if( preventCycles && input->fOwner( ).fConnectsTo( mOutput->fOwner( ) ) )
			return false;

		mInput = input;
		mInput->fAddConnection( this );
		return true;
	}
	b32 tDAGNodeConnection::fComplete( const tDAGNodeOutputPtr& output, b32 preventCycles )
	{
		sigassert( !mOutput && mInput );

		// before completing connection, check for cycle in node connections and reject
		if( preventCycles && mInput->fOwner( ).fConnectsTo( output->fOwner( ) ) )
			return false;

		mOutput = output;
		mOutput->fAddConnection( this );
		return true;
	}

	b32 tDAGNodeConnection::fIntersects( const wxRect& rect )
	{
		tGrowableArray<wxPoint> points;
		fGetPoints( wxPoint( 0,0 ), points, false );

		const f32 buffer = 8;
		Math::tAabbf abb( Math::tVec3f( rect.x, rect.y, -1 ), Math::tVec3f( rect.x + rect.width, rect.y + rect.height, 1 ) );
		abb = abb.fInflate( buffer );


		for( u32 i = 0; i < points.fCount()-2; ++i )
		{
			wxPoint& a = points[ i ];
			wxPoint& b = points[ i + 1 ];

			Math::tRayf ray;
			ray.mOrigin = Math::tVec3f( a.x, a.y, 0 );
			ray.mExtent = Math::tVec3f( b.x, b.y, 0 ) - ray.mOrigin;

			if( Math::tIntersectionRayAabb<f32>( abb, ray ).fIntersects( ) )
				return true;
		}

		return false;
	}

}

