#include "SigAIPch.hpp"
#include "tSigAINodeCanvas.hpp"
#include "tSigAIMainWindow.hpp"
#include "DerivedAINodes.hpp"
#include "../Tools/Editor/tEditScriptSnippetDialog.hpp"

namespace Sig
{

	class tMoveEventHandlerAction : public tEditorAction
	{
	protected:
		tGoalAINodePtr			mNode;
		tGoalEventHandlerPtr	mHandlerPtr;
		tDAGNodeCanvas&			mCanvas;
		u32						mPrevIndex;
		u32						mNewIndex;
	public:
		tMoveEventHandlerAction( tDAGNodeCanvas& canvas, tGoalAINodePtr& node, tGoalEventHandlerPtr& handler, u32 newIndex )
			: mCanvas( canvas ), mNode( node ), mHandlerPtr( handler ), mNewIndex( newIndex )
		{
			mPrevIndex = mNode->fIndexOfEventHandler( handler );
			fRedo( );
		}

		virtual void fUndo( ) 
		{ 
			mNode->fMoveEventHandler( mHandlerPtr, mPrevIndex );
			mCanvas.Refresh( );
		}
		virtual void fRedo( ) 
		{ 
			mNode->fMoveEventHandler( mHandlerPtr, mNewIndex );
			mCanvas.Refresh( );
		}
	};

	class tCreateNodeContextAction : public tEditorContextAction
	{
	public:
		struct tNodeDesc
		{
			std::string mName;
			u32 mActionId;
			Rtti::tClassId mClassId;
			u32 mType;
			
			tNodeDesc( ) { }

			tNodeDesc( const std::string& name, u32 aid, Rtti::tClassId cid, u32 type )
				: mName( name ), mActionId( aid ), mClassId( cid ), mType( type )
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
		tSigAINodeCanvas& mCanvas;
		u32 mBaseId;
		u32 mMaxId;
		u32 mDefaultGoalId;
		tNodeCategoryList mCategories;
	public:
		tCreateNodeContextAction( tSigAINodeCanvas& canvas )
			: mCanvas( canvas )
			, mBaseId( fNextUniqueActionId( ) )
			, mMaxId( mBaseId )
			, mDefaultGoalId( ~0 )
		{
			mCategories.fSetCount( 1 );
			u32 i = 0;

			fCreateGoals( mCategories[ i++ ] );

			sigassert( i == mCategories.fCount( ) );

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

			const wxPoint p = mCanvas.fTopLeft( ) + fRightClickPos( );
			fAddNode( *desc, p );
			return true;
		}
		void fAddDefaultNode( const wxPoint& p = wxPoint( 120, 120 ) )
		{
			const tNodeDesc* desc = fFindNodeDesc( mDefaultGoalId );
			if( !desc )
				return;
			fAddNode( *desc, p ); // add default node by default
		}
	private:
		void fAddNode( const tNodeDesc& desc, const wxPoint& p )
		{
			tGoalAINode* goalNode = ( tGoalAINode* )Rtti::fNewClass( desc.mClassId );
			goalNode->fSetType( desc.mType );

			tDAGNodePtr dagNode = tDAGNodePtr( goalNode );
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
			category.mName = "Goals";

			mDefaultGoalId = mMaxId;
			category.mNodeDescs.fPushBack( tNodeDesc( "Add Goal", mMaxId++, Rtti::fGetClassId<tGoalAINode>( ), tGoalAINode::cGoalType ) );
			category.mNodeDescs.fPushBack( tNodeDesc( "Add Switch", mMaxId++, Rtti::fGetClassId<tGoalAINode>( ), tGoalAINode::cSwitchType ) );
			category.mNodeDescs.fPushBack( tNodeDesc( "Add Sequence", mMaxId++, Rtti::fGetClassId<tGoalAINode>( ), tGoalAINode::cSequenceType ) );
		}
	};

	class tEditEventHandlerContextAction : public tEditorContextAction
	{
	public:
		enum tActions 
		{ 
			cActionRemoveHandler,  
			cActionEditScript, 
			cActionViewScript, 
			cActionMoveUp, 
			cActionMoveDown, 
			cActionMoveTop, 
			cActionMoveBottom, 
			cActionCount 
		};

		struct tContext
		{
			tGoalAINodePtr	mAINode;
			tDAGObjectPtr	mDAGObject;
			std::string		mName;
			s32 mID;

			tContext( ) : mName( "" ), mID( -1 ) 
			{ }
		};

		s32 mBaseID;
		tSigAINodeCanvas *mOwner;
		tSigAIMainWindow* mMainWindow;
		tGoalEventHandlerPtr mEventHandlerSelected;
		tGoalAINodePtr mNodeSelected;

		tEditEventHandlerContextAction( tSigAINodeCanvas *owner, tSigAIMainWindow* mainWindow )
			: mOwner( owner ), mMainWindow( mainWindow )
		{ 
			mBaseID = fNextUniqueActionId( );

			for( u32 i = 1; i < cActionCount; ++i )
				fNextUniqueActionId( );
		}

		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mNodeSelected && mNodeSelected->fType( ) == tGoalAINode::cGoalType )
			{
				menu.Append( mBaseID + cActionViewScript, "View Generated Script" );
			}
			else if( mEventHandlerSelected )
			{
				menu.Append( mBaseID + cActionEditScript, "Edit Script" );
				menu.AppendSeparator( );

				if( !mEventHandlerSelected->mLocked )
				{
					menu.Append( mBaseID + cActionMoveTop, "Move Top" );
					menu.Append( mBaseID + cActionMoveUp, "Move Up" );
					menu.Append( mBaseID + cActionMoveDown, "Move Down" );
					menu.Append( mBaseID + cActionMoveBottom, "Move Bottom" );

					menu.AppendSeparator( );
					menu.Append( mBaseID + cActionRemoveHandler, "Remove Handler" );
				}
			}

			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			switch( actionId - mBaseID )
			{
			case cActionViewScript:
				sigassert( mContext.mAINode );
				{
					mMainWindow->fCanvas( )->fSelectSingleNode( tDAGNodePtr( mContext.mAINode.fGetRawPtr( ) ) );
					mMainWindow->fViewScript( );
				}
				return true;
			case cActionEditScript:
				sigassert( mContext.mAINode && mEventHandlerSelected );
				{
					mOwner->fEditHandler( mEventHandlerSelected );
				}
				return true;
			case cActionRemoveHandler:
				sigassert( mContext.mAINode && mEventHandlerSelected );
				{
					if( mContext.mAINode->fType( ) != tGoalAINode::cGoalType )
					{
						if( mContext.mAINode->fOutputCount( ) == 1 )
							return true; //keep at least one handler.
					}

					mOwner->fEditorActions( ).fBeginCompoundAction( );
					tEditorActionPtr action( new tAddRemoveEventHandlerAction( *mOwner, mContext.mAINode, mEventHandlerSelected, false ) );
					mOwner->fEditorActions( ).fEndCompoundAction( action );
				}
				return true;
			case cActionMoveTop:
				sigassert( mContext.mAINode && mEventHandlerSelected );
				mOwner->fEditorActions( ).fAddAction( tEditorActionPtr( new tMoveEventHandlerAction( *mOwner, mContext.mAINode, mEventHandlerSelected, 0 ) ) );
				return true;
			case cActionMoveBottom:
				sigassert( mContext.mAINode && mEventHandlerSelected );
				mOwner->fEditorActions( ).fAddAction( tEditorActionPtr( new tMoveEventHandlerAction( *mOwner, mContext.mAINode, mEventHandlerSelected, mContext.mAINode->fUserHandlerCount( ) ) ) );
				return true;
			case cActionMoveUp:
				{
					sigassert( mContext.mAINode && mEventHandlerSelected );
					u32 newIndex = fMax<u32>( 0, mContext.mAINode->fIndexOfEventHandler( mEventHandlerSelected ) - 1 );
					mOwner->fEditorActions( ).fAddAction( tEditorActionPtr( new tMoveEventHandlerAction( *mOwner, mContext.mAINode, mEventHandlerSelected, newIndex ) ) );
					return true;
				}
			case cActionMoveDown:
				{
					sigassert( mContext.mAINode && mEventHandlerSelected );
					u32 newIndex = mContext.mAINode->fIndexOfEventHandler( mEventHandlerSelected ) + 1;
					mOwner->fEditorActions( ).fAddAction( tEditorActionPtr( new tMoveEventHandlerAction( *mOwner, mContext.mAINode, mEventHandlerSelected, newIndex ) ) );
					return true;
				}
			}

			return false;
		}

		tContext mContext;
	};
}

namespace Sig
{

	tSigAINodeCanvas::tSigAINodeCanvas( wxWindow* parent, tSigAIMainWindow* mainWindow )
		: tDAGNodeCanvas( parent )
		, mMainWindow( mainWindow )
		, mNodeCreator( new tCreateNodeContextAction( *this ) )
	{
		//mCurvedConnections = true;
		mAllowOutputSelect = true;
		mLightConnectedConnections = false;
		mContextActions.fPushBack( tEditorContextActionPtr( mNodeCreator ) );

		mEventHandlerAction.fReset( new tEditEventHandlerContextAction( this, mMainWindow ) );
		mActionList.fPushBack( tEditorContextActionPtr( mEventHandlerAction.fGetRawPtr( ) ) );
	}
	void tSigAINodeCanvas::fAddNode( const tDAGNodePtr& shadeNode )
	{
		tDAGNodeCanvas::fAddNode( shadeNode );
	}
	void tSigAINodeCanvas::fDeleteNode( const tDAGNodePtr& shadeNode )
	{
		tDAGNodeCanvas::fDeleteNode( shadeNode );
	}
	void tSigAINodeCanvas::fReset( const tDAGNodeList& nodes, const tDAGNodeConnectionList& connections, b32 addTo )
	{
		tDAGNodeCanvas::fReset( nodes, connections, addTo );
	}
	void tSigAINodeCanvas::fClearCanvas( )
	{
		tDAGNodeCanvas::fClearCanvas( );
	}
	void tSigAINodeCanvas::fAddDefaultNode( const wxPoint& p )
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
		static const tFilePathPtr cScratchFilePath = ToolsPaths::fCreateTempEngineFilePath( ".goaml", tFilePathPtr("SigAI"), "scratch" );

		class tPasteNodesAction : public tEditorAction
		{
			tSigAINodeCanvas& mCanvas;
			tDAGNodeCanvas::tDAGNodeList mPrevSelected;
			tDAGNodeCanvas::tDAGNodeList mPrevNodes, mNewNodes;
			tDAGNodeCanvas::tDAGNodeConnectionList mPrevConnections, mNewConnections, mPrevSelectedConn;
		public:
			tPasteNodesAction( tSigAINodeCanvas& canvas, Goaml::tFile& clipboard )
				: mCanvas( canvas )
				, mPrevSelected( canvas.fSelectedNodes( ) )
				, mPrevSelectedConn( canvas.fSelectedConnections( ) )
				, mPrevNodes( canvas.fAllNodes( ) )
				, mPrevConnections( canvas.fAllConnections( ) )
			{
				sigassert( clipboard.mNodes.fCount( ) > 0 );
				clipboard.fCollect( &canvas, mNewNodes, mNewConnections, false );
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

	void tSigAINodeCanvas::fEditHandler( tGoalEventHandlerPtr& handler, u32 row, u32 col )
	{
		tEditScriptSnippetDialog *edit = new tEditScriptSnippetDialog( this, "Edit Snippet", handler->fStaticScriptText( ), handler->fScript( ), 0 );

		if( edit->fShowDialog( row, col ) == tEditScriptSnippetDialog::cResultChanged )
			fEditorActions( ).fAddAction( tEditorActionPtr( new tScriptChangeAction( *this, handler, std::string( edit->fGetScript( ) ) ) ) );
	}

	void tSigAINodeCanvas::fCopy( )
	{
		mClipboard = Goaml::tFile( );
		fToGoamlFile( mClipboard, true );
		mClipboard.fSaveXml( cScratchFilePath, false );
	}

	void tSigAINodeCanvas::fPaste( )
	{
		if( !mClipboard.fLoadXml( cScratchFilePath ) )
			return;

		for( u32 i = 0; i < mClipboard.mNodes.fCount( ); ++i )
		{
			mClipboard.mNodes[ i ]->fMove( wxPoint( 8, 8 ) );

			// Set the new node ids so they don't conflict with copied from node ids
			mClipboard.mNodes[ i ]->fSetUniqueNodeId( mDAGNodes.fCount() + i );
		}

		mClipboard.fSaveXml( cScratchFilePath, false );

		if( mClipboard.mNodes.fCount( ) > 0 )
		{
			fEditorActions( ).fBeginCompoundAction( );
			tEditorActionPtr action( new tPasteNodesAction( *this, mClipboard ) );
			fEditorActions( ).fEndCompoundAction( action );
		}
	}

	void tSigAINodeCanvas::fOnMouseRightButtonUp( wxMouseEvent& event )
	{
		const wxPoint absolutePos = mTopLeft + event.GetPosition( );
		for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
		{
			tDAGNode::tActionContext context;
			const tDAGNode::tAction result = mDAGNodes[ i ]->fOnRightButtonUp( absolutePos, event, context );
			if( result == tDAGNode::cActionUserOutput )
			{
				// Edit handler
				tGoalAINode* goalNode = dynamic_cast<tGoalAINode*>( mDAGNodes[ i ].fGetRawPtr( ) );
				if( goalNode && context.mOutput )
				{
					tGoalEventHandler *handler = dynamic_cast< tGoalEventHandler* >( context.mOutput->fData( ).fGetRawPtr( ) );
					sigassert( handler );

					mEventHandlerAction->mContext.mAINode.fReset( goalNode );
					mEventHandlerAction->mContext.mName = context.mOutput->fName( );
					mEventHandlerAction->mContext.mDAGObject.fReset( context.mOutput.fGetRawPtr( ) );
					mEventHandlerAction->mEventHandlerSelected.fReset( handler );
					mEventHandlerAction->mNodeSelected.fRelease( );
					tEditorContextAction::fDisplayContextMenuOnRightClick( this, event, mActionList );
					return;
				}
			}
			else if( result == tDAGNode::cActionUserNode )
			{
				// Edit node
				tGoalAINode* goalNode = dynamic_cast<tGoalAINode*>( mDAGNodes[ i ].fGetRawPtr( ) );
				if( goalNode )
				{
					mEventHandlerAction->mContext.mAINode.fReset( goalNode );
					mEventHandlerAction->mEventHandlerSelected.fRelease( );
					mEventHandlerAction->mNodeSelected.fReset( goalNode );
					tEditorContextAction::fDisplayContextMenuOnRightClick( this, event, mActionList );
					return;
				}
			}
		}

		tDAGNodeCanvas::fOnMouseRightButtonUp( event );
	}

	void tSigAINodeCanvas::fConnectionCreated( const tDAGNodeConnectionPtr& connection )
	{
		tAIConnectionData* data = dynamic_cast<tAIConnectionData*>( connection->fData( ).fGetRawPtr( ) );
		if( !data )
		{
			connection->fSetText( "" );
			data = new tAIConnectionData( );
			connection->fSetData( tDAGDataPtr( data ) );
			data->fSetConnection( connection );
		}

		data->fReadProps( );
	}

	void tSigAINodeCanvas::fToGoamlFile( Goaml::tFile& file, b32 selectedOnly )
	{
		for( u32 i = 0; i < mDAGNodes.fCount( ); ++i )
		{
			tAINode* aiNode = dynamic_cast< tAINode* >( mDAGNodes[ i ].fGetRawPtr( ) );
			if( !aiNode )
				continue;
			if( selectedOnly && !mSelectedNodes.fFind( mDAGNodes[ i ] ) )
				continue;

			file.mNodes.fPushBack( tAINodePtr( aiNode ) );
		}

		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			tDAGNodeInputPtr input = mConnections[ i ]->fInput( );
			tDAGNodeOutputPtr output = mConnections[ i ]->fOutput( );
			if( !input || !output )
				continue;
			tAINodePtr* findInput = file.mNodes.fFind( dynamic_cast<const tAINode*>( &input->fOwner( ) ) );
			tAINodePtr* findOutput = file.mNodes.fFind( dynamic_cast<const tAINode*>( &output->fOwner( ) ) );
			if( !findInput || !findOutput )
				continue;
			if( selectedOnly && ( !mSelectedNodes.fFind( &input->fOwner( ) ) || !mSelectedNodes.fFind( &output->fOwner( ) ) ) )
				continue;

			tAIConnectionData* data = dynamic_cast< tAIConnectionData* >( mConnections[ i ]->fData( ).fGetRawPtr( ) );
			sigassert( data );

			file.mConnections.fPushBack( Goaml::tConnection( 
				fPtrDiff( findInput, file.mNodes.fBegin( ) ), 
				input->fIndex( ), 
				fPtrDiff( findOutput, file.mNodes.fBegin( ) ), 
				output->fIndex( ), 
				data
				) );
		}

		file.mNextUniqueNodeId = 0;
		for( u32 i = 0; i < file.mNodes.fCount( ); ++i )
			file.mNextUniqueNodeId = fMax( file.mNextUniqueNodeId, file.mNodes[ i ]->fUniqueNodeId( ) );
		file.mNextUniqueNodeId += 1;
	}

	void tSigAINodeCanvas::fFromGoamlFile( const Goaml::tFile& file, b32 addToScene )
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
				tAINode::fSetNextUniqueNodeId( file.mNextUniqueNodeId );
		}

		Refresh( );
	}

}
