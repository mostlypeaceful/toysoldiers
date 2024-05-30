#include "ToolsPch.hpp"
#include "tDAGNodeCanvas.hpp"
#include "wx/dcbuffer.h"


namespace Sig
{

	tAddDeleteDAGNodeAction::tAddDeleteDAGNodeAction( tDAGNodeCanvas& canvas, const tDAGNodePtr& node, b32 add )
		: mCanvas( canvas )
		, mSelected( canvas.fSelectedNodes( ) )
		, mAdd( add )
	{
		mNodes.fPushBack( node );
		fRedo( );
	}
	tAddDeleteDAGNodeAction::tAddDeleteDAGNodeAction( tDAGNodeCanvas& canvas, const tDAGNodeCanvas::tDAGNodeList& nodes, b32 add )
		: mCanvas( canvas )
		, mSelected( canvas.fSelectedNodes( ) )
		, mNodes( nodes )
		, mAdd( add )
	{
		fRedo( );
	}
	void tAddDeleteDAGNodeAction::fUndo( ) { mAdd ? fDeleteNodes( ) : fAddNodes( ); }
	void tAddDeleteDAGNodeAction::fRedo( ) { mAdd ? fAddNodes( ) : fDeleteNodes( ); }
	void tAddDeleteDAGNodeAction::fAddNodes( )
	{
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
			mCanvas.fAddNode( mNodes[ i ] );
		mCanvas.fSetSelectedNodes( mNodes );
		mCanvas.Refresh( );
	}
	void tAddDeleteDAGNodeAction::fDeleteNodes( )
	{
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
			mCanvas.fDeleteNode( mNodes[ i ] );
		mCanvas.fSetSelectedNodes( mSelected );
		mCanvas.Refresh( );
	}

	class tools_export tSelectNodesAction : public tEditorAction
	{
		tDAGNodeCanvas& mCanvas;
		tDAGNodeCanvas::tDAGNodeList mPrevSelected, mNewSelected;
		tDAGNodeCanvas::tDAGNodeConnectionList mPrevSelectedConn, mNewSelectedConn;
		tDAGNodeCanvas::tDAGNodeOutputList mPrevSelectedOuts, mNewSelectedOuts;
		b32 mAddTo;
	public:
		tSelectNodesAction( tDAGNodeCanvas& canvas, const tDAGNodeCanvas::tDAGNodeList& newSelected = tDAGNodeCanvas::tDAGNodeList( ), const tDAGNodeCanvas::tDAGNodeConnectionList& newSelectedConn = tDAGNodeCanvas::tDAGNodeConnectionList( ), const tDAGNodeCanvas::tDAGNodeOutputList& newSelectedOuts = tDAGNodeCanvas::tDAGNodeOutputList( ), b32 addTo = false )
			: mCanvas( canvas )
			, mPrevSelected( canvas.fSelectedNodes( ) )
			, mNewSelected( newSelected )
			, mPrevSelectedConn( canvas.fSelectedConnections( ) )
			, mNewSelectedConn( newSelectedConn )
			, mPrevSelectedOuts( canvas.fSelectedOutputs( ) )
			, mNewSelectedOuts( newSelectedOuts )
			, mAddTo( addTo )
		{
			fRedo( );
		}
		virtual void fUndo( ) 
		{ 
			mCanvas.fSetSelectedNodes( mPrevSelected ); 
			mCanvas.fSetSelectedConnections( mPrevSelectedConn );
			mCanvas.fSetSelectedOutput( mPrevSelectedOuts );
		}
		virtual void fRedo( ) 
		{ 
			mCanvas.fSetSelectedNodes( mNewSelected, mAddTo );
			mCanvas.fSetSelectedConnections( mNewSelectedConn, mAddTo ); 
			mCanvas.fSetSelectedOutput( mNewSelectedOuts, mAddTo ); 
		}
		virtual b32 fDirtyingAction( ) const { return false; }
	};

	class tools_export tMoveSelectedNodesAction : public tEditorAction
	{
		tDAGNodeCanvas& mCanvas;
		tDAGNodeCanvas::tDAGNodeList mSelected;
		tGrowableArray<wxPoint> mStartPositions, mEndPositions;
	public:
		tMoveSelectedNodesAction( tDAGNodeCanvas& canvas )
			: mCanvas( canvas )
			, mSelected( canvas.fSelectedNodes( ) )
		{
			fSetIsLive( true );
			fSavePositions( mStartPositions );
		}
		virtual void fUndo( )
		{
			mCanvas.fSetSelectedNodes( mSelected );
			for( u32 i = 0; i < mSelected.fCount( ); ++i )
				mSelected[ i ]->fSetPosition( mStartPositions[ i ] );
			mCanvas.Refresh( );
		}
		virtual void fRedo( )
		{
			mCanvas.fSetSelectedNodes( mSelected );
			for( u32 i = 0; i < mSelected.fCount( ); ++i )
				mSelected[ i ]->fSetPosition( mEndPositions[ i ] );
			mCanvas.Refresh( );
		}
		virtual void fEnd( )
		{
			fSavePositions( mEndPositions );
			tEditorAction::fEnd( );
		}
		virtual b32 fDirtyingAction( ) const { return true; }
	private:
		void fSavePositions( tGrowableArray<wxPoint>& positions )
		{
			positions.fSetCount( mSelected.fCount( ) );
			for( u32 i = 0; i < mSelected.fCount( ); ++i )
				positions[ i ] = mSelected[ i ]->fPosition( );
		}
	};

	tAddDeleteConnectionAction::tAddDeleteConnectionAction( tDAGNodeCanvas& canvas, const tDAGNodeConnectionPtr& connection, b32 add )
		: mCanvas( canvas )
		, mAdd( add )
	{
		mConnections.fPushBack( connection );
		fRedo( );
	}
	tAddDeleteConnectionAction::tAddDeleteConnectionAction( tDAGNodeCanvas& canvas, const tDAGNodeCanvas::tDAGNodeConnectionList& connections, b32 add )
		: mCanvas( canvas )
		, mConnections( connections )
		, mAdd( add )
	{
		fRedo( );
	}
	void tAddDeleteConnectionAction::fUndo( ) { mAdd ? fDeleteConnections( ) : fAddConnections( ); }
	void tAddDeleteConnectionAction::fRedo( ) { mAdd ? fAddConnections( ) : fDeleteConnections( ); }
	void tAddDeleteConnectionAction::fAddConnections( )
	{
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			mCanvas.fAddConnection( mConnections[ i ] );
		mCanvas.Refresh( );
	}
	void tAddDeleteConnectionAction::fDeleteConnections( )
	{
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			mCanvas.fDeleteConnection( mConnections[ i ] );
		mCanvas.Refresh( );
	}
}

namespace Sig
{

	tDAGNodeCanvas::tDAGNodeCanvas( wxWindow* parent )
		: wxPanel( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize )
		, mOnSelChanged( false )
		, mTopLeft( 0, 0 )
		, mMarqueeSelectAddOrRemove( 0 )
		, mMarqueeSelecting( false )
		, mRenderOutputStyle( tDAGNode::cRenderInputsBeforeOutputs )
		, mPreventCycles( false )
		, mCurvedConnections( false )
		, mAllowOutputSelect( false )
		, mLightConnectedConnections( true )
		, mZoom( 1.f )
		, mBgColor( 0x99, 0x99, 0x99 )
		, mBgBrush( mBgColor )
	{
		SetBackgroundStyle( wxBG_STYLE_CUSTOM );

		Connect( wxEVT_PAINT, wxPaintEventHandler( tDAGNodeCanvas::fOnPaint ) );
		Connect( wxEVT_ERASE_BACKGROUND, wxEraseEventHandler( tDAGNodeCanvas::fOnEraseBackground ) );
		Connect( wxEVT_SIZE, wxSizeEventHandler( tDAGNodeCanvas::fOnSize ) );
		Connect( wxEVT_MOTION, wxMouseEventHandler( tDAGNodeCanvas::fOnMouseMove ) );
		Connect( wxEVT_LEFT_DOWN, wxMouseEventHandler( tDAGNodeCanvas::fOnMouseLeftButtonDown ) );
		Connect( wxEVT_LEFT_UP, wxMouseEventHandler( tDAGNodeCanvas::fOnMouseLeftButtonUp ) );
		Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( tDAGNodeCanvas::fOnMouseRightButtonDown ) );
		Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tDAGNodeCanvas::fOnMouseRightButtonUp ) );
		Connect( wxEVT_MIDDLE_DOWN, wxMouseEventHandler( tDAGNodeCanvas::fOnMouseMiddleButtonDown ) );
		Connect( wxEVT_MOUSEWHEEL, wxMouseEventHandler( tDAGNodeCanvas::fOnMouseWheel ) );
		Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( tDAGNodeCanvas::fOnKeyDown ) );
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tDAGNodeCanvas::fOnAction ) );
	}
	tDAGNodeCanvas::~tDAGNodeCanvas( )
	{
	}
	void tDAGNodeCanvas::fAddNode( const tDAGNodePtr& shadeNode )
	{
		mDAGNodes.fPushFront( shadeNode );
		mDAGNodes.fFront( )->fSetOwner( this );

		if( shadeNode->fSelected( ) )
		{
			mSelectedNodes.fPushBack( shadeNode );
			mOnSelChanged.fFire( );
		}
	}
	void tDAGNodeCanvas::fDeleteNode( const tDAGNodePtr& shadeNode )
	{
		const tDAGNodePtr* find = mDAGNodes.fFind( shadeNode );
		if( find )
			fDeleteNode( fPtrDiff( find, mDAGNodes.fBegin( ) ) );
	}
	void tDAGNodeCanvas::fReset( const tDAGNodeList& nodes, const tDAGNodeConnectionList& connections, b32 addTo )
	{
		if( addTo )
			mDAGNodes.fJoin( nodes );
		else
			mDAGNodes = nodes;

		u32 startAt;
		if( addTo )
		{
			startAt = mConnections.fCount( );
			mConnections.fJoin( connections );
		}
		else
		{
			for( u32 i = 0; i < mConnections.fCount( ); ++i )
				mConnections[ i ]->fUnhook( );
			startAt = 0;
			mConnections = connections;
		}

		for( u32 i = startAt; i < mConnections.fCount( ); ++i )
			mConnections[ i ]->fRehook( );
	}
	void tDAGNodeCanvas::fClearCanvas( )
	{
		fEditorActions( ).fClearDirty( );
		fEditorActions( ).fReset( );
		mMoveAction.fRelease( );
		mDragging.fRelease( );
		mConnection.fRelease( );
		mDAGNodes.fSetCount( 0 );
		mSelectedNodes.fSetCount( 0 );
		mSelectedConnections.fSetCount( 0 );
		mOnSelChanged.fFire( );
		mConnections.fSetCount( 0 );
		mTopLeft = wxPoint( 0, 0 );
		Refresh( );
	}
	void tDAGNodeCanvas::fFrame( b32 selectedOnly )
	{
		if( mSelectedNodes.fCount( ) == 0 )
			selectedOnly = false; // nothing selected, just frame everything

		if( mDAGNodes.fCount( ) == 0 )
			mTopLeft = wxPoint( 0, 0 );
		else
		{
			const int bigNum = 9999999;
			wxPoint topLeft = wxPoint( +bigNum, +bigNum );
			wxPoint botRight = wxPoint( -bigNum, -bigNum );
			for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
			{
				if( selectedOnly && !mDAGNodes[ i ]->fSelected( ) )
					continue;
				const wxPoint tl = mDAGNodes[ i ]->fTopLeft( );
				const wxPoint br = tl + mDAGNodes[ i ]->fDimensions( );
				topLeft.x = fMin( topLeft.x, tl.x );
				topLeft.y = fMin( topLeft.y, tl.y );
				botRight.x = fMax( botRight.x, br.x );
				botRight.y = fMax( botRight.y, br.y );
			}

			const wxPoint dims = botRight - topLeft;
			const wxPoint center = topLeft + wxPoint( dims.x / 2, dims.y / 2 );
			const wxPoint windowCenter = wxPoint( fGetSize( ).x / 2, fGetSize( ).y / 2 );
			mTopLeft = center - windowCenter;
		}

		Refresh( );
	}
	void tDAGNodeCanvas::fUndo( )
	{
		mConnection.fRelease( );
		mDragging.fRelease( );
		mEditorActions.fUndo( );
		mOnSelChanged.fFire( );
	}
	void tDAGNodeCanvas::fRedo( )
	{
		mConnection.fRelease( );
		mDragging.fRelease( );
		mEditorActions.fRedo( );
		mOnSelChanged.fFire( );
	}
	void tDAGNodeCanvas::fDeleteSelected( )
	{
		fEditorActions( ).fBeginCompoundAction( );
		tEditorActionPtr action( new tAddDeleteDAGNodeAction( *this, mSelectedNodes, false ) );
		fEditorActions( ).fAddAction( tEditorActionPtr( new tAddDeleteConnectionAction( *this, mSelectedConnections, false ) ) );
		fEditorActions( ).fEndCompoundAction( action );
	}
	void tDAGNodeCanvas::fMoveSelectedNodes( const wxPoint& delta )
	{
		for( u32 i = 0; i < mSelectedNodes.fCount( ); ++i )
			mSelectedNodes[ i ]->fMove( delta );
		Refresh( );
	}
	void tDAGNodeCanvas::fSelectSingleNode( const tDAGNodePtr& node, b32 addTo )
	{
		tDAGNodeList newSel;
		newSel.fPushBack( node );
		fEditorActions( ).fAddAction( tEditorActionPtr( new tSelectNodesAction( *this, newSel, tDAGNodeConnectionList( ), tDAGNodeOutputList( ), addTo ) ) );
	}
	void tDAGNodeCanvas::fDeselectSingleNode( const tDAGNodePtr& node )
	{
		tDAGNodeList newSel = mSelectedNodes;
		newSel.fFindAndErase( node );
		fEditorActions( ).fAddAction( tEditorActionPtr( new tSelectNodesAction( *this, newSel, tDAGNodeConnectionList( ) ) ) );
	}
	void tDAGNodeCanvas::fSetSelectedNodes( const tDAGNodeList& nodes, b32 addTo )
	{
		u32 start;
		if( addTo )
		{
			start = mSelectedNodes.fCount( );
			for( u32 i = 0; i < nodes.fCount( ); ++i )
				mSelectedNodes.fFindOrAdd( nodes[ i ] );
		}
		else
		{
			fClearSelection( true, false, false );
			start = 0;
			mSelectedNodes.fSetCount( 0 );
			for( u32 i = 0; i < nodes.fCount( ); ++i )
				mSelectedNodes.fFindOrAdd( nodes[ i ] );
		}

		for( u32 i = start; i < mSelectedNodes.fCount( ); ++i )
		{
			if( mDAGNodes.fFind( mSelectedNodes[ i ] ) )
			{
				mSelectedNodes[ i ]->fSetSelected( true );
				fMoveToFront( mSelectedNodes[ i ] );
			}
			else
			{
				mSelectedNodes.fErase( i );
				--i;
			}
		}

		mOnSelChanged.fFire( );
		Refresh( );
	}
	void tDAGNodeCanvas::fSetSelectedConnections( const tDAGNodeConnectionList& conns, b32 addTo )
	{
		u32 start;
		if( addTo )
		{
			start = mSelectedConnections.fCount( );
			for( u32 i = 0; i < conns.fCount( ); ++i )
				mSelectedConnections.fFindOrAdd( conns[ i ] );
		}
		else
		{
			fClearSelection( false, true, false );
			start = 0;
			mSelectedConnections.fSetCount( 0 );
			for( u32 i = 0; i < conns.fCount( ); ++i )
				mSelectedConnections.fFindOrAdd( conns[ i ] );
		}

		for( u32 i = start; i < mSelectedConnections.fCount( ); ++i )
		{
			if( mConnections.fFind( mSelectedConnections[ i ] ) )
			{
				mSelectedConnections[ i ]->fSetSelected( true );
				//fMoveToFront( mSelectedConnections[ i ] );
			}
			else
			{
				mSelectedConnections.fErase( i );
				--i;
			}
		}

		mOnSelChanged.fFire( );
		Refresh( );
	}
	void tDAGNodeCanvas::fSetSelectedOutput( const tDAGNodeOutputList& outs, b32 addTo )
	{
		if( outs.fCount( ) == 0 )
			mSelectedOutput.fRelease( );
		else
			mSelectedOutput = outs[ 0 ];

		mOnSelChanged.fFire( );
		Refresh( );
	}
	void tDAGNodeCanvas::fClearSelection( b32 nodes, b32 connections, b32 undoable )
	{
		mSelectedOutput.fRelease( );

		if( undoable )
		{
			tDAGNodeConnectionList clist = connections ? tDAGNodeConnectionList( ) : mSelectedConnections;
			tDAGNodeList nlist = nodes ? tDAGNodeList( ) : mSelectedNodes;

			fEditorActions( ).fAddAction( tEditorActionPtr( new tSelectNodesAction( *this, nlist, clist ) ) );
		}
		else
		{
			if( nodes )
			{
				for( u32 i = 0; i < mSelectedNodes.fCount( ); ++i )
					mSelectedNodes[ i ]->fSetSelected( false );
				mSelectedNodes.fSetCount( 0 );
			}
			if( connections )
			{
				for( u32 i = 0; i < mSelectedConnections.fCount( ); ++i )
					mSelectedConnections[ i ]->fSetSelected( false );
				mSelectedConnections.fSetCount( 0 );
			}
		}
	}
	void tDAGNodeCanvas::fMoveToFront( const tDAGNodePtr& node )
	{
		if( mDAGNodes.fFindAndEraseOrdered( node ) )
			mDAGNodes.fPushFront( node );
	}
	void tDAGNodeCanvas::fAddConnection( const tDAGNodeConnectionPtr& connection )
	{
		tDAGNodeConnectionPtr* inList = mConnections.fFind( connection );
		if( !inList )
		{
			connection->fRehook( );
			mConnections.fPushBack( connection );
			if( connection->fSelected( ) )
				mSelectedConnections.fFindOrAdd( connection );
		}
	}
	void tDAGNodeCanvas::fDeleteConnection( const tDAGNodeConnectionPtr& connection )
	{
		connection->fUnhook( );
		mConnections.fFindAndErase( connection );
		mSelectedConnections.fFindAndErase( connection );
	}
	void tDAGNodeCanvas::fSetToolTipText( const wxPoint& absolutePos )
	{
		wxMouseEvent fakeEvent;

		for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
		{
			tDAGNode::tActionContext context;
			const tDAGNode::tAction result = mDAGNodes[ i ]->fOnLeftButtonDown( absolutePos, fakeEvent, context );
			if( result == tDAGNode::cActionDelete )
			{
				SetToolTip( "Delete this node." );
				return;
			}
			else if( result == tDAGNode::cActionBeginDrag )
			{
				SetToolTip( mDAGNodes[ i ]->fToolTip( ) );
				return;
			}
			else if( context.mOutput )
			{
				SetToolTip( context.mOutput->fToolTip( ) );
				return;
			}
			else if( context.mInput )
			{
				SetToolTip( context.mInput->fToolTip( ) );
				return;
			}
		}

		const u32 pad = 1;
		const wxRect selectRect( absolutePos.x - pad, absolutePos.y - pad, 2*pad, 2*pad );
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			if( mConnections[ i ]->fIntersects( selectRect ) )
			{
				SetToolTip( mConnections[ i ]->fToolTip( ) );
				return;
			}
		}

		SetToolTip( "" );
	}
	void tDAGNodeCanvas::fOnPaint( wxPaintEvent& event )
	{
		wxAutoBufferedPaintDC  dc( this );
		dc.SetUserScale( mZoom, mZoom );

		fClearBackground( dc );
		fRenderNodeWindows( dc );
		fRenderConnections( dc );
		if( mMarqueeSelecting )
			fRenderMarqueeSelect( dc );

		// This little trick only works on MSW
		dc.SetUserScale( 1.f, 1.f );
	}
	void tDAGNodeCanvas::fOnEraseBackground( wxEraseEvent& event )
	{
	}
	void tDAGNodeCanvas::fOnSize( wxSizeEvent& event )
	{
		Refresh( );
	}
	void tDAGNodeCanvas::fOnMouseMove( wxMouseEvent& event )
	{
		const wxPoint absolutePos = fScreenToAbsolute( event.GetPosition() );
		const wxPoint delta( f32(event.GetPosition( ).x - mLastMouseMove.x) / mZoom, f32(event.GetPosition( ).y - mLastMouseMove.y) / mZoom );

		fSetToolTipText( absolutePos );

		if( event.LeftIsDown( ) )
		{
			if( mConnection )
			{
				mConnection->fUpdate( absolutePos );
				Refresh( );
			}
			else if( mDragging )
			{
				if( !mMoveAction )
				{
					mMoveAction.fReset( new tMoveSelectedNodesAction( *this ) );
					fEditorActions( ).fAddAction( mMoveAction );
				}
				fMoveSelectedNodes( delta );
			}
			else if( mMarqueeSelecting )
			{
				mMarqueeSelectEnd = absolutePos;
				Refresh( );
			}
		}
		else if(   
			  event.MiddleIsDown( ) ||
			( event.RightIsDown( ) && event.AltDown( ) ) )
		{
			mTopLeft -= delta;
			Refresh( );
		}

		mLastMouseMove = event.GetPosition( );
	}

	void tDAGNodeCanvas::fOnMouseWheel( wxMouseEvent& event )
	{
		// Don't try to zoom while moving the view port
		if( event.MiddleIsDown( ) ||
			( event.RightIsDown( ) && event.AltDown( ) ) )
			return;

		const f32 oldZoom = mZoom;
		const wxPoint oldPos = fScreenToAbsolute( event.GetPosition() );

		// Scale the mouse rotation down and clamp the zoom.
		const f32 wheelDelta = (f32)event.GetWheelRotation( );
		mZoom += wheelDelta / 1200.f;
		mZoom = fClamp( mZoom, 0.25f, 1.f );

		// Don't adjust anything if our zoom level didn't change.
		if( oldZoom == mZoom )
			return;

		// Calculate the size different and adjust the centering to match (to cursor).
		const wxPoint newPos = fScreenToAbsolute( event.GetPosition() );
		mTopLeft += oldPos - newPos;

		Refresh( ); // Triggers the paint event.
	}

	void tDAGNodeCanvas::fSelectOutput( const tDAGNodeOutputPtr& outp )
	{
		tGrowableArray< tDAGNodeOutputPtr > oList;
		oList.fPushBack( outp );
		fEditorActions( ).fAddAction( tEditorActionPtr( new tSelectNodesAction( *this, tDAGNodeList( ), tDAGNodeConnectionList( ), oList, mMarqueeSelectAddOrRemove > 0 ) ) );
	}

	wxPoint tDAGNodeCanvas::fScreenToAbsolute( const wxPoint& screen )
	{
		wxPoint ret = screen;
		ret.x /= mZoom;
		ret.y /= mZoom;
		ret += mTopLeft;

		return ret;
	}

	wxSize tDAGNodeCanvas::fGetSize( ) const
	{
		wxSize size = GetSize( );
		size.x /= mZoom;
		size.y /= mZoom;

		return size;
	}

	void tDAGNodeCanvas::fOnMouseLeftButtonDown( wxMouseEvent& event )
	{
		SetFocus( );
		mRedoingPrevInput.fRelease( );

		const wxPoint absolutePos = fScreenToAbsolute( event.GetPosition() );

		for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
		{
			tDAGNode::tActionContext context;
			const tDAGNode::tAction result = mDAGNodes[ i ]->fOnLeftButtonDown( absolutePos, event, context );
			switch( result )
			{
			case tDAGNode::cActionDelete:
				{
					fEditorActions( ).fBeginCompoundAction( );
					tEditorActionPtr action( new tAddDeleteDAGNodeAction( *this, mDAGNodes[ i ], false ) );
					fEditorActions( ).fEndCompoundAction( action );
				}
				return;
			case tDAGNode::cActionBeginDrag:
				fBeginDrag( i, event );
				return;
			case tDAGNode::cActionBeginConnection:
				fBeginConnection( i, context, absolutePos );
				if( context.mOutput && mAllowOutputSelect ) fSelectOutput( context.mOutput );
				return;
			case tDAGNode::cActionRedoConnection:
				fRedoConnection( i, context, absolutePos );
				return;
			case tDAGNode::cActionOutputSelect:
				{
					if( context.mOutput && mAllowOutputSelect )
					{
						fSelectOutput( context.mOutput );
						return;
					}
				}
				break;
			}
		}

		// if we made it here, it means no node intercepted this event, meaning it's our option now
		fBeginMarqueeSelect( absolutePos, event );
	}
	void tDAGNodeCanvas::fOnMouseLeftButtonUp( wxMouseEvent& event )
	{
		if( mMarqueeSelecting )
		{
			mMarqueeSelecting = false;

			const wxRect selectRect( 
				fMin( mMarqueeSelectStart.x, mMarqueeSelectEnd.x ), 
				fMin( mMarqueeSelectStart.y, mMarqueeSelectEnd.y ),
				fAbs( mMarqueeSelectStart.x - mMarqueeSelectEnd.x ),
				fAbs( mMarqueeSelectStart.y - mMarqueeSelectEnd.y ) );

			tDAGNodeList newSel;
			tDAGNodeConnectionList newSelConns;

			for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
			{
				if( mDAGNodes[ i ]->fIntersects( selectRect ) )
					newSel.fPushBack( mDAGNodes[ i ] );
			}

			for( u32 i = 0; i < mConnections.fCount( ); ++i )
			{
				if( mConnections[ i ]->fIntersects( selectRect ) )
					newSelConns.fPushBack( mConnections[ i ] );
			}

			if( mMarqueeSelectAddOrRemove < 0 )
			{
				tDAGNodeList intersection = mSelectedNodes;
				for( u32 i = 0; i < newSel.fCount( ); ++i )
					intersection.fFindAndErase( newSel[ i ] );
				newSel.fSwap( intersection );
			}

			fEditorActions( ).fAddAction( tEditorActionPtr( new tSelectNodesAction( *this, newSel, newSelConns, tDAGNodeOutputList( ), mMarqueeSelectAddOrRemove > 0 ) ) );
		}

		mDragging.fRelease( );

		if( mMoveAction )
		{
			mMoveAction->fEnd( );
			mMoveAction.fRelease( );
		}

		const wxPoint absolutePos = fScreenToAbsolute( event.GetPosition() );
		if( mConnection )
		{
			b32 madeConnect = false;
			for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
			{
				if( mDAGNodes[ i ]->fTryToCompleteConnection( absolutePos, *mConnection, mPreventCycles ) )
				{
					madeConnect = true;
					if( mRedoingPrevInput == mConnection->fInput( ) )
					{
						// we reconnected to the same input we were connected to, pop the delete connection action and take no further action
						tAddDeleteConnectionAction* action = dynamic_cast<tAddDeleteConnectionAction*>( fEditorActions( ).fPopAction( ).fGetRawPtr( ) );
						sigassert( action && !action->fAdd( ) );
						fAddConnection( mConnection );
						Refresh( );
					}
					else
					{
						// new connection
						fConnectionCreated( mConnection );

						fEditorActions( ).fBeginCompoundAction( );
						tEditorActionPtr action( new tAddDeleteConnectionAction( *this, mConnection, true ) );

						tDAGNodeConnectionList newSelConns;
						newSelConns.fPushBack( mConnection );
						fEditorActions( ).fAddAction( tEditorActionPtr( new tSelectNodesAction( *this, tDAGNodeList( ), newSelConns, tDAGNodeOutputList( ), false ) ) );
						fEditorActions( ).fEndCompoundAction( action );
					}

					mRedoingPrevInput.fRelease( );
					break;
				}
			}

			if( madeConnect )
				mConnection.fRelease( );
			else
			{
				mConnection->fUnhook( );
				mConnection.fRelease( );
				Refresh( );
			}

			mOnSelChanged.fFire( );
			Refresh( );
		}
	}
	void tDAGNodeCanvas::fOnMouseRightButtonDown( wxMouseEvent& event )
	{
	}
	void tDAGNodeCanvas::fOnMouseRightButtonUp( wxMouseEvent& event )
	{
		tEditorContextAction::fDisplayContextMenuOnRightClick( this, event, mContextActions );
	}
	void tDAGNodeCanvas::fOnMouseMiddleButtonDown( wxMouseEvent& event )
	{
	}
	void tDAGNodeCanvas::fOnKeyDown( wxKeyEvent& event )
	{
		const int keyCode = event.GetKeyCode( );
		if( keyCode == WXK_DELETE )
			fDeleteSelected( );
		else if( keyCode == 'A' )
			fFrame( false );
		else if( keyCode == 'F' )
			fFrame( true );
	}
	void tDAGNodeCanvas::fOnAction( wxCommandEvent& event )
	{
		tEditorContextAction::fHandleContextActionFromRightClick( this, event, tEditorContextAction::fLastActionList( ) );
	}
	void tDAGNodeCanvas::fClearBackground( wxDC& dc )
	{
		const wxSize size = fGetSize( );
		dc.SetBackground( mBgBrush );
		dc.Clear( );
	}
	void tDAGNodeCanvas::fRenderConnections( wxDC& dc )
	{
		const wxSize size = fGetSize( );
		if( mConnection )
			mConnection->fOnPaint( mTopLeft, size, mCurvedConnections, dc );

		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			mConnections[ i ]->fOnPaint( mTopLeft, size, mCurvedConnections, dc );
	}
	void tDAGNodeCanvas::fRenderNodeWindows( wxDC& dc )
	{
		const wxSize size = fGetSize( );
		for( u32 i = mDAGNodes.fCount( ); i > 0; --i )
			mDAGNodes[ i - 1 ]->fOnPaint( mTopLeft, size, dc, (tDAGNode::tRenderStyle)mRenderOutputStyle );
	}
	void tDAGNodeCanvas::fRenderMarqueeSelect( wxDC& dc )
	{
		dc.SetPen( wxPen( wxColour( 0x00, 0xff, 0x00 ), 1, wxSOLID ) );
		dc.SetBrush( wxBrush( wxColour( 0x00, 0x00, 0x00 ), wxTRANSPARENT ) );
		dc.DrawRectangle( mMarqueeSelectStart - mTopLeft, wxSize( mMarqueeSelectEnd.x - mMarqueeSelectStart.x, mMarqueeSelectEnd.y - mMarqueeSelectStart.y ) );
	}
	void tDAGNodeCanvas::fDeleteNode( u32 ithNode )
	{
		tDAGNodeOutput::tConnectionList connections;
		mDAGNodes[ ithNode ]->fCollectAllConnections( connections );

		if( connections.fCount( ) > 0 )
		{
			tDAGNodeConnectionList connectionsPtrs( connections.fCount( ) );
			for( u32 i = 0; i < connectionsPtrs.fCount( ); ++i )
				connectionsPtrs[ i ].fReset( connections[ i ] );
			fEditorActions( ).fAddAction( tEditorActionPtr( new tAddDeleteConnectionAction( *this, connectionsPtrs, false ) ) );
		}

		mSelectedNodes.fFindAndErase( mDAGNodes[ ithNode ] );
		mDAGNodes.fEraseOrdered( ithNode );
		mOnSelChanged.fFire( );
	}
	void tDAGNodeCanvas::fBeginDrag( u32 ithNode, wxMouseEvent& event )
	{
		mDragging = mDAGNodes[ ithNode ];

		if( mDragging->fSelected( ) )
		{
			if( event.CmdDown( ) )
			{
				fDeselectSingleNode( mDragging );
				mDragging.fRelease( );
			}
		}
		else
			fSelectSingleNode( mDragging, event.ShiftDown( ) );

		fMoveToFront( mDragging );
		Refresh( );
	}
	void tDAGNodeCanvas::fBeginConnection( u32 ithNode, const tDAGNode::tActionContext& context, const wxPoint& cursorPos )
	{
		if( context.mInput )
			mConnection.fReset( new tDAGNodeConnection( context.mInput, cursorPos ) );
		else
		{
			sigassert( context.mOutput );
			mConnection.fReset( new tDAGNodeConnection( context.mOutput, cursorPos ) );
		}

		Refresh( );
	}
	void tDAGNodeCanvas::fRedoConnection( u32 ithNode, const tDAGNode::tActionContext& context, const wxPoint& cursorPos )
	{
		sigassert( context.mInput && context.mConnection );
		mConnection.fReset( new tDAGNodeConnection( *context.mConnection ) );
		mRedoingPrevInput = mConnection->fInput( );
		mConnection->fRemoveInput( );
		fEditorActions( ).fAddAction( tEditorActionPtr( new tAddDeleteConnectionAction( *this, context.mConnection, false ) ) );
	}
	void tDAGNodeCanvas::fBeginMarqueeSelect( const wxPoint& absolutePos, wxMouseEvent& event )
	{
		if( event.ShiftDown( ) )
			mMarqueeSelectAddOrRemove = +1;
		else if( event.CmdDown( ) )
			mMarqueeSelectAddOrRemove = -1;
		else
		{
			mMarqueeSelectAddOrRemove = 0;
			fClearSelection( true, true );
		}

		mMarqueeSelecting = true;
		mMarqueeSelectStart = absolutePos;
		mMarqueeSelectEnd = absolutePos;
	}

}
