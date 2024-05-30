#include "ToolsGuiPch.hpp"
#include "tSigAnimNodeCanvas.hpp"
#include "tSigAnimDialog.hpp"
#include "DerivedAnimNodes.hpp"

namespace Sig
{

	class tCreateNodeContextAction : public tEditorContextAction
	{
	public:
		struct tNodeDesc
		{
			std::string mName;
			u32 mActionId;
			Rtti::tClassId mClassId;
			
			tNodeDesc( ) { }

			tNodeDesc( const std::string& name, u32 aid, Rtti::tClassId cid )
				: mName( name ), mActionId( aid ), mClassId( cid )
			{
			}
		};

		typedef tGrowableArray<tNodeDesc> tShadeNodeDescList;

		struct tNodeCategory
		{
			wxString mName;
			tShadeNodeDescList mNodeDescs;
		};

		typedef tGrowableArray<tNodeCategory> tNodeCategoryList;
	private:
		tSigAnimNodeCanvas& mCanvas;
		u32 mBaseId;
		u32 mMaxId;
		u32 mDefaultId;
		tNodeCategoryList mCategories;
	public:
		tCreateNodeContextAction( tSigAnimNodeCanvas& canvas )
			: mCanvas( canvas )
			, mBaseId( fNextUniqueActionId( ) )
			, mMaxId( mBaseId )
			, mDefaultId( ~0 )
		{
			mCategories.fSetCount( 1 );
			fCreateGoals( mCategories[ 0 ] );

			//reserve the rest of the unique ids used
			for( s32 id = 0; id < s32(mMaxId - mBaseId); ++id )
				fNextUniqueActionId( );
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			//category zero means, just in the menu
			const tNodeCategory& category0 = mCategories[ 0 ];
			for( u32 j = 0; j < category0.mNodeDescs.fCount( ); ++j )
			{
				const u32 actionId = category0.mNodeDescs[ j ].mActionId;
				wxMenuItem* menuItem = menu.Append( actionId, category0.mNodeDescs[ j ].mName );
			}

			// now the categorized items
			for( u32 i = 1; i < mCategories.fCount( ); ++i )
			{
				const tNodeCategory& category = mCategories[ i ];
				wxMenu* subMenu = new wxMenu;
				menu.AppendSubMenu( subMenu, category.mName );
				for( u32 j = 0; j < category.mNodeDescs.fCount( ); ++j )
				{
					const u32 actionId = category.mNodeDescs[ j ].mActionId;
					wxMenuItem* menuItem = subMenu->Append( actionId, category.mNodeDescs[ j ].mName );
				}
			}
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId < mBaseId || actionId >= mMaxId )
				return false;
			const tNodeDesc* desc = fFindNodeDesc( actionId );
			if( !desc )
				return false;

			const wxPoint p = mCanvas.fScreenToAbsolute( fRightClickPos() );
			fAddNode( *desc, p );
			return true;
		}
		void fAddDefaultNode( const wxPoint& p = wxPoint( 120, 120 ) )
		{
			const tNodeDesc* desc = fFindNodeDesc( mDefaultId );
			if( !desc )
				return;
			fAddNode( *desc, p ); // add default node by default
		}
	private:
		void fAddNode( const tNodeDesc& desc, const wxPoint& p )
		{
			tAnimBaseNode* node = ( tAnimBaseNode* )Rtti::fNewClass( desc.mClassId );
			tDAGNodePtr dagNode = tDAGNodePtr( node );
			dagNode->fSetPosition( p );
			dagNode->fSetOwner( &mCanvas );
			mCanvas.fEditorActions( ).fAddAction( tEditorActionPtr( new tAddDeleteDAGNodeAction( mCanvas, dagNode, true ) ) );
		}
		const tNodeDesc* fFindNodeDesc( u32 actionId ) const
		{
			for( u32 i = 0; i < mCategories.fCount( ); ++i )
			{
				const tNodeCategory& category = mCategories[ i ];
				for( u32 j = 0; j < category.mNodeDescs.fCount( ); ++j )
				{
					if( category.mNodeDescs[ j ].mActionId == actionId )
						return &category.mNodeDescs[ j ];
				}
			}
			return 0;
		}
		void fCreateGoals( tNodeCategory& category )
		{
			category.mName = "Anims";

			category.mNodeDescs.fPushBack( tNodeDesc( "Input", mMaxId++, Rtti::fGetClassId<tAnimInputNode>( ) ) );
			category.mNodeDescs.fPushBack( tNodeDesc( "Blend", mMaxId++, Rtti::fGetClassId<tAnimBlendNode>( ) ) );
			category.mNodeDescs.fPushBack( tNodeDesc( "Stack", mMaxId++, Rtti::fGetClassId<tAnimStackNode>( ) ) );

			mDefaultId = mMaxId;
			category.mNodeDescs.fPushBack( tNodeDesc( "Output", mMaxId++, Rtti::fGetClassId<tAnimOutputNode>( ) ) );
		}
	};

}

namespace Sig
{

	tSigAnimNodeCanvas::tSigAnimNodeCanvas( wxWindow* parent, tSigAnimDialog* mainWindow )
		: tDAGNodeCanvas( parent )
		, mMainWindow( mainWindow )
		, mNodeCreator( new tCreateNodeContextAction( *this ) )
	{
		//mCurvedConnections = true;
		mAllowOutputSelect = true;
		mLightConnectedConnections = false;
		//mRenderOutputStyle = tDAGNode::cRenderOutputsNextToInputs;
		mContextActions.fPushBack( tEditorContextActionPtr( mNodeCreator ) );
	}
	void tSigAnimNodeCanvas::fAddDefaultNode( const wxPoint& p )
	{
		wxPoint realP;
		if( p.x < 0 || p.y < 0 )
		{
			const wxSize s = GetSize( );
			realP.x = s.x / 2 - 60;
			realP.y = s.y / 2 - 60;
		}
		else
			realP = p;

		mNodeCreator->fAddDefaultNode( realP );

		fFrame( );
	}

	namespace
	{
		static const tFilePathPtr cScratchFilePath = ToolsPaths::fCreateTempEngineFilePath( ".momap", tFilePathPtr("SigAnim"), "scratch" );

		class tPasteNodesAction : public tEditorAction
		{
			tSigAnimNodeCanvas& mCanvas;
			tDAGNodeCanvas::tDAGNodeList mPrevSelected;
			tDAGNodeCanvas::tDAGNodeList mPrevNodes, mNewNodes;
			tDAGNodeCanvas::tDAGNodeConnectionList mPrevConnections, mNewConnections, mPrevSelectedConn;
		public:
			tPasteNodesAction( tSigAnimNodeCanvas& canvas, Momap::tFile& clipboard )
				: mCanvas( canvas )
				, mPrevSelected( canvas.fSelectedNodes( ) )
				, mPrevSelectedConn( canvas.fSelectedConnections( ) )
				, mPrevNodes( canvas.fAllNodes( ) )
				, mPrevConnections( canvas.fAllConnections( ) )
			{
				sigassert( clipboard.mMoState.mNodes.fCount( ) > 0 );
				clipboard.mMoState.fCollect( &canvas, mNewNodes, mNewConnections, false );
				fRedo( );
			}
			virtual void fUndo( )
			{
				mCanvas.fReset( mPrevNodes, mPrevConnections );
				mCanvas.fSetSelectedNodes( mPrevSelected );
				mCanvas.fSetSelectedConnections( mPrevSelectedConn );
			}
			virtual void fRedo( )
			{
				mCanvas.fClearSelection( true, true, false );
				mCanvas.fReset( mNewNodes, mNewConnections, true );
				mCanvas.fSetSelectedNodes( mNewNodes );
				mCanvas.fSetSelectedConnections( mNewConnections );
			}
		};
	}

	void tSigAnimNodeCanvas::fCopy( )
	{
		mClipboard = Momap::tFile( );
		fToFile( mClipboard.mMoState, true );
		mClipboard.fSaveXml( cScratchFilePath, false );
	}

	void tSigAnimNodeCanvas::fPaste( )
	{
		if( !mClipboard.fLoadXml( cScratchFilePath ) )
			return;

		for( u32 i = 0; i < mClipboard.mMoState.mNodes.fCount( ); ++i )
		{
			mClipboard.mMoState.mNodes[ i ]->fMove( wxPoint( 8, 8 ) );

			// Set the new node ids so they don't conflict with copied from node ids
			mClipboard.mMoState.mNodes[ i ]->fSetUniqueNodeId( mDAGNodes.fCount( ) + i );
		}

		mClipboard.fSaveXml( cScratchFilePath, false );

		if( mClipboard.mMoState.mNodes.fCount( ) > 0 )
		{
			fEditorActions( ).fBeginCompoundAction( );
			tEditorActionPtr action( new tPasteNodesAction( *this, mClipboard ) );
			fEditorActions( ).fEndCompoundAction( action );
		}
	}

	void tSigAnimNodeCanvas::fOnMouseRightButtonUp( wxMouseEvent& event )
	{
		//const wxPoint absolutePos = fScreenToAbsolute( event.GetPosition( ) );
		//for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
		//{
		//	tDAGNode::tActionContext context;
		//	const tDAGNode::tAction result = mDAGNodes[ i ]->fOnRightButtonUp( absolutePos, event, context );
		//	if( result == tDAGNode::cActionUserOutput )
		//	{
		//		// Edit handler
		//		tGoalAINode* goalNode = dynamic_cast<tGoalAINode*>( mDAGNodes[ i ].fGetRawPtr( ) );
		//		if( goalNode && context.mOutput )
		//		{
		//			tGoalEventHandler *handler = dynamic_cast< tGoalEventHandler* >( context.mOutput->fData( ).fGetRawPtr( ) );
		//			sigassert( handler );

		//			mEventHandlerAction->mContext.mAINode.fReset( goalNode );
		//			mEventHandlerAction->mContext.mName = context.mOutput->fName( );
		//			mEventHandlerAction->mContext.mDAGObject.fReset( context.mOutput.fGetRawPtr( ) );
		//			mEventHandlerAction->mEventHandlerSelected.fReset( handler );
		//			mEventHandlerAction->mNodeSelected.fRelease( );
		//			tEditorContextAction::fDisplayContextMenuOnRightClick( this, event, mActionList );
		//			return;
		//		}
		//	}
		//	else if( result == tDAGNode::cActionUserNode )
		//	{
		//		// Edit node
		//		tGoalAINode* goalNode = dynamic_cast<tGoalAINode*>( mDAGNodes[ i ].fGetRawPtr( ) );
		//		if( goalNode )
		//		{
		//			mEventHandlerAction->mContext.mAINode.fReset( goalNode );
		//			mEventHandlerAction->mEventHandlerSelected.fRelease( );
		//			mEventHandlerAction->mNodeSelected.fReset( goalNode );
		//			tEditorContextAction::fDisplayContextMenuOnRightClick( this, event, mActionList );
		//			return;
		//		}
		//	}
		//}

		tDAGNodeCanvas::fOnMouseRightButtonUp( event );
	}

	void tSigAnimNodeCanvas::fConnectionCreated( const tDAGNodeConnectionPtr& connection )
	{
		tAnimStackNode* node = dynamic_cast<tAnimStackNode*>( &connection->fInput( )->fOwner( ) );
		if( node )
			node->fCheckConnections( );
	}

	namespace
	{
		struct tSortNodesById
		{
			inline b32 operator( )( const tAnimBaseNodePtr& a, const tAnimBaseNodePtr& b ) const
			{
				return a->fUniqueNodeId( ) < b->fUniqueNodeId( );
			}
		};
	}

	void tSigAnimNodeCanvas::fToFile( Momap::tMoState& file, b32 selectedOnly )
	{
		for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
		{
			tAnimBaseNode* node = dynamic_cast< tAnimBaseNode* >( mDAGNodes[ i ].fGetRawPtr( ) );
			if( !node )
				continue;
			if( selectedOnly && !mSelectedNodes.fFind( mDAGNodes[ i ] ) )
				continue;

			file.mNodes.fPushBack( tAnimBaseNodePtr( node ) );
		}

		// Sort the nodes by id so the files are more consistent
		std::sort( file.mNodes.fBegin( ), file.mNodes.fEnd( ), tSortNodesById( ) );

		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			tDAGNodeInputPtr input = mConnections[ i ]->fInput( );
			tDAGNodeOutputPtr output = mConnections[ i ]->fOutput( );

			if( !input || !output )
				continue;

			tAnimBaseNodePtr* findInput = file.mNodes.fFind( dynamic_cast<const tAnimBaseNode*>( &input->fOwner( ) ) );
			tAnimBaseNodePtr* findOutput = file.mNodes.fFind( dynamic_cast<const tAnimBaseNode*>( &output->fOwner( ) ) );
			if( !findInput || !findOutput )
				continue;
			if( selectedOnly && ( !mSelectedNodes.fFind( &input->fOwner( ) ) || !mSelectedNodes.fFind( &output->fOwner( ) ) ) )
				continue;

			file.mConnections.fPushBack( Momap::tConnection( 
				fPtrDiff( findInput, file.mNodes.fBegin( ) ), 
				input->fIndex( ), 
				fPtrDiff( findOutput, file.mNodes.fBegin( ) ), 
				output->fIndex( )
				) );
		}

		file.mNextUniqueNodeId = tAnimBaseNode::fNextUniqueNodeId( );
		sigassert( !file.mNodes.fCount( ) || file.mNodes.fBack( )->fUniqueNodeId( ) < file.mNextUniqueNodeId );
	}

	void tSigAnimNodeCanvas::fFromFile( const Momap::tMoState& file, b32 addToScene )
	{
		tDAGNodeList savedNodes;
		tDAGNodeConnectionList savedConnections;
		if( addToScene )
		{
			savedNodes = mDAGNodes;
			savedConnections = mConnections;
		}

		file.fCollect( this, mDAGNodes, mConnections, false );

		fClearSelection( true, true, false );

		if( addToScene )
		{
			fSetSelectedNodes( mDAGNodes );
			mDAGNodes.fJoin( savedNodes );
			mConnections.fJoin( savedConnections );
		}
		else
		{
			if( file.mNextUniqueNodeId > 0 )
				tAnimBaseNode::fSetNextUniqueNodeId( file.mNextUniqueNodeId );
		}

		Refresh( );
	}

}
