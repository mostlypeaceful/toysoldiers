#ifndef __tDAGNodeCanvas__
#define __tDAGNodeCanvas__
#include "tDAGNode.hpp"
#include "Editor/tEditorAction.hpp"
#include "Editor/tEditorContextAction.hpp"

namespace Sig
{
	class tools_export tDAGNodeCanvas : public wxPanel
	{
	public:
		typedef tGrowableArray<tDAGNodePtr> tDAGNodeList;
		typedef tGrowableArray<tDAGNodeConnectionPtr> tDAGNodeConnectionList;
		typedef tGrowableArray<tDAGNodeOutputPtr> tDAGNodeOutputList;
		typedef tEvent<void ( )> tOnSelectionChanged;
	public:
		tOnSelectionChanged mOnSelChanged;
	protected:
		wxPoint mTopLeft;
		wxPoint mLastMouseMove;
		wxPoint mMarqueeSelectStart, mMarqueeSelectEnd;
		s32 mMarqueeSelectAddOrRemove;
		b8 mMarqueeSelecting;
		b8 mOutputsBeforeInputs;
		b8 mPreventCycles; //No cyclic connections
		b8 mCurvedConnections;
		b16 mAllowOutputSelect;
		b16 mLightConnectedConnections;
		wxColour mBgColor;
		wxBrush mBgBrush;
		tEditorActionPtr mMoveAction;
		tDAGNodePtr mDragging;
		tDAGNodeConnectionPtr mConnection;
		tDAGNodeInputPtr mRedoingPrevInput;
		tDAGNodeList mDAGNodes;
		tDAGNodeList mSelectedNodes;
		tDAGNodeConnectionList mConnections;
		tDAGNodeConnectionList mSelectedConnections;
		tDAGNodeOutputPtr mSelectedOutput;
		tEditorActionStack mEditorActions;
		tEditorContextActionList mContextActions;
	public:
		explicit tDAGNodeCanvas( wxWindow* parent );
		~tDAGNodeCanvas( );
		virtual void fAddNode( const tDAGNodePtr& shadeNode );
		virtual void fDeleteNode( const tDAGNodePtr& shadeNode );
		virtual void fReset( const tDAGNodeList& nodes, const tDAGNodeConnectionList& connections, b32 addTo = false );
		virtual void fClearCanvas( );
		void fFrame( b32 selectedOnly = false );
		void fUndo( );
		void fRedo( );
		void fDeleteSelected( );
		void fMoveSelectedNodes( const wxPoint& delta );
		const tDAGNodeList& fSelectedNodes( ) const { return mSelectedNodes; }
		const tDAGNodeConnectionList& fSelectedConnections( ) const { return mSelectedConnections; }
		const tDAGNodeOutputList fSelectedOutputs( ) const { tDAGNodeOutputList list; if( mSelectedOutput ) list.fPushBack( mSelectedOutput ); return list; }
		const tDAGNodeList& fAllNodes( ) const { return mDAGNodes; }
		const tDAGNodeConnectionList& fAllConnections( ) const { return mConnections; }
		void fSelectSingleNode( const tDAGNodePtr& node, b32 addTo = false );
		void fDeselectSingleNode( const tDAGNodePtr& node );
		void fSetSelectedNodes( const tDAGNodeList& nodes, b32 addTo = false );
		void fSetSelectedConnections( const tDAGNodeConnectionList& conns, b32 addTo = false );
		void fSetSelectedOutput( const tDAGNodeOutputList& outs, b32 addTo = false );
		void fClearSelection( b32 nodes, b32 connections, b32 undoable = true );
		void fFireSelection( ) { mOnSelChanged.fFire( ); }
		void fMoveToFront( const tDAGNodePtr& node );
		void fAddConnection( const tDAGNodeConnectionPtr& connection );
		void fDeleteConnection( const tDAGNodeConnectionPtr& connection );
		tEditorActionStack& fEditorActions( ) { return mEditorActions; }
		const wxPoint& fTopLeft( ) const { return mTopLeft; }
		const wxColour& fBgColor( ) const { return mBgColor; }
		b32 fLightConnectedConnections( ) const { return mLightConnectedConnections; }
	protected:
		void fOnPaint( wxPaintEvent& event );
		void fOnEraseBackground( wxEraseEvent& event );
		void fOnSize( wxSizeEvent& event );
		void fOnMouseMove( wxMouseEvent& event );
		virtual void fOnMouseLeftButtonDown( wxMouseEvent& event );
		virtual void fOnMouseLeftButtonUp( wxMouseEvent& event );
		virtual void fOnMouseRightButtonDown( wxMouseEvent& event );
		virtual void fOnMouseRightButtonUp( wxMouseEvent& event );
		virtual void fOnMouseMiddleButtonDown( wxMouseEvent& event );
		virtual void fSetToolTipText( const wxPoint& absolutePos );
		void fOnKeyDown( wxKeyEvent& event );
		void fOnAction( wxCommandEvent& event );

		virtual void fConnectionCreated( const tDAGNodeConnectionPtr& connection ) { } //Give the user a chance to do something with the new conneciton, like give it custom data and set its label.
	private:
		void fClearBackground( wxDC& dc );
		void fRenderConnections( wxDC& dc );
		void fRenderNodeWindows( wxDC& dc );
		void fRenderMarqueeSelect( wxDC& dc );
		void fDeleteNode( u32 ithNode );
		void fBeginDrag( u32 ithNode, wxMouseEvent& event );
		void fBeginConnection( u32 ithNode, const tDAGNode::tActionContext& context, const wxPoint& cursorPos );
		void fRedoConnection( u32 ithNode, const tDAGNode::tActionContext& context, const wxPoint& cursorPos );
		void fBeginMarqueeSelect( const wxPoint& absolutePos, wxMouseEvent& event );
		void fSelectOutput( const tDAGNodeOutputPtr& outp );
	};


	class tools_export tAddDeleteDAGNodeAction : public tEditorAction
	{
		tDAGNodeCanvas& mCanvas;
		tDAGNodeCanvas::tDAGNodeList mSelected;
		tDAGNodeCanvas::tDAGNodeList mNodes;
		b32 mAdd;
	public:
		tAddDeleteDAGNodeAction( tDAGNodeCanvas& canvas, const tDAGNodePtr& node, b32 add );
		tAddDeleteDAGNodeAction( tDAGNodeCanvas& canvas, const tDAGNodeCanvas::tDAGNodeList& nodes, b32 add );
		virtual void fUndo( );
		virtual void fRedo( );
	private:
		void fAddNodes( );
		void fDeleteNodes( );
	};

	class tools_export tAddDeleteConnectionAction : public tEditorAction
	{
		tDAGNodeCanvas& mCanvas;
		tDAGNodeCanvas::tDAGNodeConnectionList mConnections;
		b32 mAdd;
	public:
		tAddDeleteConnectionAction( tDAGNodeCanvas& canvas, const tDAGNodeConnectionPtr& connection, b32 add );
		tAddDeleteConnectionAction( tDAGNodeCanvas& canvas, const tDAGNodeCanvas::tDAGNodeConnectionList& connections, b32 add );
		virtual void fUndo( );
		virtual void fRedo( );
		b32 fAdd( ) const { return mAdd; }
	private:
		void fAddConnections( );
		void fDeleteConnections( );
	};

}

#endif//__tDAGNodeCanvas__
