#include "SigAIPch.hpp"
#include "tSigAIControlPanel.hpp"
#include "tAINode.hpp"
#include "DerivedAINodes.hpp"
#include "tWxSlapOnControl.hpp"
#include "tWxSelectStringDialog.hpp"
#include "Editor/tEditablePropertyTypes.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	namespace 
	{
		const u32 cPanelWidth = 280;
		const u32 cPanelBorder = 8;

		enum tSystemEventHandler
		{ 
			cSystemAddEventHandler,
			cSystemSpacer0,
			//cSystemAddPolledEvent,
			cSystemAddPotentialEvent,
			cSystemAddFirstChanceEvent,
			cSystemSpacer1,
			cSystemEventHandlerCount
		};

		const std::string cSystemEventHandlerNames[ cSystemEventHandlerCount ] = 
		{ 
			"Add Event Handler...", 
			"----------------", 
			//"Polled Event",  
			"Potential Event", 
			"First Chance Event", 
			cSystemEventHandlerNames[ cSystemSpacer0 ],

		};


		enum tSequenceSwitchEventHandler
		{ 
			cSequenceAddEventHandler,
			cSequenceSpacer0,
			cSequenceEvent,
			cSequenceEventHandlerCount
		};

		const std::string cSequenceSwitchEventHandlerNames[ cSequenceEventHandlerCount ] = 
		{ 
			"Add Event Handler...", 
			"----------------", 
			"Handler"
		};

		const char cEventHandlerQueuedFailed[] = "Handler Setup.Queue Failed Events";



		class tChangePriorityAction : public tEditorAction
		{
		protected:
			tGoalAINodePtr			mNode;
			tDAGNodeCanvas&			mCanvas;
			u32						mPrevPriority;
			u32						mNewPriority;
		public:
			tChangePriorityAction( tDAGNodeCanvas& canvas, tGoalAINodePtr& node, u32 newPriority )
				: mCanvas( canvas ), mNode( node ), mPrevPriority( mNode->fPriority( ) ), mNewPriority( newPriority )
			{
				mNode->fSetPriority( mNewPriority );
			}

			virtual void fUndo( ) 
			{ 
				mNode->fSetPriority( mPrevPriority );
				mCanvas.Refresh( );
				mCanvas.fFireSelection( );
			}
			virtual void fRedo( ) 
			{ 
				mCanvas.Refresh( );
				mCanvas.fFireSelection( );
			}
		};
		class tChangeMaxClearAction : public tEditorAction
		{
		protected:
			tAIConnectionData*		mData;
			tDAGNodeCanvas&			mCanvas;
			u32						mPrevPriority;
			u32						mNewPriority;
		public:
			tChangeMaxClearAction( tDAGNodeCanvas& canvas, tAIConnectionData* data, u32 newPriority )
				: mCanvas( canvas ), mData( data ), mPrevPriority( data->fMaxClearablePriority( ) ), mNewPriority( newPriority )
			{
				mData->fSetMaxClearablePriority( mNewPriority );
			}

			virtual void fUndo( ) 
			{ 
				mData->fSetMaxClearablePriority( mPrevPriority );
				mCanvas.Refresh( );
				mCanvas.fFireSelection( );
			}
			virtual void fRedo( ) 
			{ 
				mCanvas.Refresh( );
				mCanvas.fFireSelection( );
			}
		};
	}


	const std::string tSigAIControlPanel::cNoMoMapText = "No MotionMap Chosen";

	tSigAIControlPanel::tSigAIControlPanel( wxWindow* parent, tSigAINodeCanvas* canvas )
		: wxScrolledWindow( parent, wxID_ANY, wxDefaultPosition, wxSize( cPanelWidth, wxDefaultSize.y ), wxTAB_TRAVERSAL | wxBORDER_SIMPLE | wxVSCROLL )
		, mCanvas( canvas )
		, mHeaderText( 0 )
		, mEventList( 0 )
		, mGoalList( 0 )
		, mPropertyPanel( 0 )
		, mMoMapFileName( cNoMoMapText )
	{
		SetMinSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetMaxSize( wxSize( GetSize( ).x, wxDefaultSize.y ) );
		SetBackgroundColour( wxColour( 0x33, 0x33, 0x33 ) );
		SetForegroundColour( wxColour( 0x11, 0xff, 0x11 ) );

		mOnPropertyChanged.fFromMethod< tSigAIControlPanel, &tSigAIControlPanel::fOnPropertyChanged >( this );
		mCommonProps.mOnPropertyChanged.fAddObserver( &mOnPropertyChanged );

		tWxSlapOnControl::fSetLabelWidth( 80 );
		tWxSlapOnControl::fSetControlWidth( 150 );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mHeaderText = new wxStaticText( this, wxID_ANY, "Selection: None" );
		GetSizer( )->AddSpacer( 8 );
		GetSizer( )->Add( mHeaderText, 0, wxLEFT, 8 );
		GetSizer( )->AddSpacer( 8 );

		// populate event list
		tFilePathPtr currentProjectFile = ToolsPaths::fGetCurrentProjectFilePath( );
		if( FileSystem::fFileExists( currentProjectFile ) )
		{
			tProjectFile projFile;
			projFile.fLoadXml( currentProjectFile );
			mEventList = projFile.mGameEvents;
		}

		const s32 idealWidth = cPanelWidth - 2*cPanelBorder;
		mEventCombo = new wxComboBox( this, wxID_ANY, "Add Event Handler...", wxDefaultPosition, wxSize( idealWidth, wxDefaultSize.y ), 0, NULL, wxCB_READONLY );
		mEventCombo->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( tSigAIControlPanel::fOnEventHandlerSelected ), NULL, this );
		GetSizer( )->AddSpacer( 8 );
		GetSizer( )->Add( mEventCombo, 0, wxLEFT, 8 );
		GetSizer( )->AddSpacer( 8 );

		// Property panel
		mPropertyPanel = new wxScrolledWindow( this );
		mPropertyPanel->SetBackgroundColour( wxColour( 0x55, 0x55, 0x55 ) );
		mPropertyPanel->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
		GetSizer( )->Add( mPropertyPanel, 1, wxEXPAND | wxALL, 0 );

		// Priority controls
		wxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
		GetSizer( )->Add( hSizer, 0, wxSHRINK | wxALL | wxALIGN_CENTER, 0 );
		hSizer->Add( new wxStaticText( this, wxID_ANY, "Priority Viewer:" ), 0, wxSHRINK | wxALL, 8 );

		wxButton* insertBtn = new wxButton( this, wxID_ANY, "Insert" );
		insertBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigAIControlPanel::fOnInsertPriority ), NULL, this );
		insertBtn->SetToolTip( "Make selected priority available my incrementing priorities >= to the selected priority." );
		wxButton* removeBtn = new wxButton( this, wxID_ANY, "Remove" );
		removeBtn->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tSigAIControlPanel::fOnRemovePriority ), NULL, this );
		removeBtn->SetToolTip( "Remove excess priorities my decrementing priorities >= to the selected priority." );
		hSizer->Add( insertBtn, 0, wxEXPAND | wxALL, 4 );
		hSizer->Add( removeBtn, 0, wxEXPAND | wxALL, 4 );

		// Goal list
		mGoalList = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( idealWidth, 100 ) );
		GetSizer( )->Add( mGoalList, 0, wxEXPAND | wxALL, 4 );


		// Setup properties to use for event handlers
		mEventHandlerProps.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cEventHandlerQueuedFailed, false ) ) );

		tEditablePropertyCustomString::tCallbackData data;
		data.mFunc.fFromMethod< tSigAIControlPanel, &tSigAIControlPanel::fChooseMoState >( this );

		tEditablePropertyCustomString::fAddCallback( tGoalAINode::cMoStatePropertiesName, data );
	}

	b32 tSigAIControlPanel::fChooseMoState( tEditablePropertyCustomString& prop, std::string& newValue )
	{
		wxArrayString choices;
		for( u32 i = 0; i < mMoMapFunctions.fCount( ); ++i )
			choices.push_back( mMoMapFunctions[ i ] );

		s32 choice = wxGetSingleChoiceIndex( wxString( mMoMapFileName.fCStr( ), mMoMapFileName.fLength( ) ), "Choose MotionState:", choices, this->GetParent( ) );
		if( choice > -1 )
		{
			newValue = "\"" + mMoMapFunctions[ choice ] + "\"";
			return true;
		}
		else
			return false;
	}

	void tSigAIControlPanel::fFillEvents( )
	{
		mEventCombo->Clear( );

		if( mSelectedNode )
		{
			if( mSelectedNode->fType( ) == tGoalAINode::cGoalType )
			{
				for( u32 i = 0; i < cSystemEventHandlerCount; ++i )
					mEventCombo->Append( cSystemEventHandlerNames[ i ] );

				for( u32 i = 0; i < mEventList.fCount( ); ++i )
					mEventCombo->Append( mEventList[ i ].mName );
			}
			else
			{
				for( u32 i = 0; i < cSequenceEventHandlerCount; ++i )
					mEventCombo->Append( cSequenceSwitchEventHandlerNames[ i ] );
			}
		}

		fResetEventCombo( );
	}

	void tSigAIControlPanel::fOnInsertPriority( wxCommandEvent& )
	{
		if( mGoalList->GetSelection( ) >= 0 )
			fShiftPriorities( 1, dynamic_cast< tGoalAINode* >( mCanvas->fSelectedNodes( )[ 0 ].fGetRawPtr( ) ), static_cast< tGoalAINode* >( mGoalList->GetClientData( mGoalList->GetSelection( ) ) )->fPriority( ) );
	}

	void tSigAIControlPanel::fOnRemovePriority( wxCommandEvent& )
	{
		if( mGoalList->GetSelection( ) >= 0 )
			fShiftPriorities( -1, dynamic_cast< tGoalAINode* >( mCanvas->fSelectedNodes( )[ 0 ].fGetRawPtr( ) ), static_cast< tGoalAINode* >( mGoalList->GetClientData( mGoalList->GetSelection( ) ) )->fPriority( ) );
	}

	void tSigAIControlPanel::fShiftPriorities( s32 value, tGoalAINode* goal, u32 selectedPriority )
	{
		if( selectedPriority == 0 && value < 0 ) return;

		tEditorActionPtr mainAction;

		mCanvas->fEditorActions( ).fBeginCompoundAction( );

		for( u32 i = 0; i < mGoalList->GetCount( ); ++i )
		{
			tGoalAINode *goal = static_cast< tGoalAINode* >( mGoalList->GetClientData( i ) );
			if( goal->fPriority( ) >= selectedPriority )
			{
				u32 newPriority = goal->fPriority( ) + value;
				if( mainAction ) mCanvas->fEditorActions( ).fAddAction( tEditorActionPtr( new tChangePriorityAction( *mCanvas, tGoalAINodePtr( goal ), newPriority ) ) );
				else mainAction.fReset( new tChangePriorityAction( *mCanvas, tGoalAINodePtr( goal ), newPriority ) );
			}
		}
		tDAGNodeOutput::tConnectionList conns;
		goal->fCollectAllConnections( conns );
		for( u32 i = 0; i < conns.fCount( ); ++i )
		{
			tAIConnectionData *data = static_cast< tAIConnectionData* >( conns[ i ]->fData( ).fGetRawPtr( ) );
			if( data->fMaxClearablePriority( ) >= selectedPriority )
			{
				u32 newPriority = data->fMaxClearablePriority( ) + value;
				if( mainAction ) mCanvas->fEditorActions( ).fAddAction( tEditorActionPtr( new tChangeMaxClearAction( *mCanvas, data, newPriority ) ) );
				else mainAction.fReset( new tChangeMaxClearAction( *mCanvas, data, newPriority ) );
			}
		}

		mCanvas->fEditorActions( ).fEndCompoundAction( mainAction );
		mCanvas->Refresh( );
		fPopulateGoals( );
	}

	b32 fSortGoals( const tGoalAINode* a, const tGoalAINode* b )
	{
		return a->fPriority( ) > b->fPriority( );
	}


	void fCollectGoalsRecursive( tGrowableArray< const tGoalAINode* >& uniqueNodes, const tGoalAINode* node, tGrowableArray< const tGoalAINode* >& branches )
	{
		tDAGNodeOutput::tConnectionList conns;
		node->fCollectAllConnections( conns );

		for( u32 i = 0; i < conns.fCount( ); ++i )
		{
			const tGoalAINode *goal = dynamic_cast< const tGoalAINode* >( &conns[ i ]->fInput( )->fOwner( ) );
			if( goal && goal != node )
			{
				if( goal->fType( ) == tGoalAINode::cGoalType )
					uniqueNodes.fFindOrAdd( goal );
				else
				{
					if( !branches.fFind( goal ) )
					{
						// prevent infinite loops, dont revisit same branch twice
						branches.fPushBack( goal );
						fCollectGoalsRecursive( uniqueNodes, goal, branches );
					}
				}
			}
		}
	}

	void tSigAIControlPanel::fPopulateGoals( )
	{
		tGoalAINode* prevSelected = mGoalList->GetSelection( ) > -1 ? (tGoalAINode*)mGoalList->GetClientData( mGoalList->GetSelection( ) ) : NULL;
		mGoalList->Clear( );

		if( mCanvas->fSelectedNodes( ).fCount( ) == 1 )
		{
			tGoalAINode* node = dynamic_cast< tGoalAINode* >( mCanvas->fSelectedNodes( )[ 0 ].fGetRawPtr( ) );
			
			if( node->fType( ) == tGoalAINode::cGoalType )
			{
				tGrowableArray< const tGoalAINode* > uniqueNodes;
				tGrowableArray< const tGoalAINode* > branches; //ignore these
				fCollectGoalsRecursive( uniqueNodes, node, branches );
				std::sort( uniqueNodes.fBegin( ), uniqueNodes.fEnd( ), fSortGoals );

				for( u32 i = 0; i < uniqueNodes.fCount( ); ++i )
				{
					mGoalList->Append( uniqueNodes[ i ]->fVerboseName( ), (void*)uniqueNodes[ i ] );
					if( uniqueNodes[ i ] == prevSelected )
						mGoalList->SetSelection( i );
				}
			}
		}
	}

	void tSigAIControlPanel::fResetEventCombo( )
	{
		mEventCombo->SetSelection( 0 );
	}

	void tSigAIControlPanel::fOnEventSelectionChanged( const tDAGNodeList& nodes, const tDAGNodeConnectionList& conns, const tDAGNodeOutputList& outputs )
	{
		Freeze( );
		mPropertyPanel->DestroyChildren( );
		mCommonProps.fClearGui( );
		mEventCombo->Show( false );
		mSelectedNode.fRelease( );
		mSelectedConn.fRelease( );
		mSelectedHandler.fRelease( );

		if( outputs.fCount( ) == 1 )
		{
			mHeaderText->SetLabel( "Selection: Event Handler" );

			tGoalEventHandler* handler = dynamic_cast< tGoalEventHandler* >( outputs.fFront( )->fData( ).fGetRawPtr( ) );
			mSelectedHandler.fReset( handler );

			handler->fFillProps( );

			mCommonProps = handler->mProps;
			mCommonProps.fCollectCommonPropertiesForGui( handler->mProps );
			tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps, false );
		}
		else if( conns.fCount( ) == 1 )
		{
			tDAGNodeConnectionPtr connection = conns.fFront( );
			if( connection->fInput( )->fIndex( ) != tGoalAINode::cInputActivate )
			{
				mHeaderText->SetLabel( "Selection: Termination Connection" );
			}
			else
			{
				mHeaderText->SetLabel( "Selection: Connection" );

				tAIConnectionData* data = dynamic_cast< tAIConnectionData* >( connection->fData( ).fGetRawPtr( ) );
				sigassert( data );

				data->fFillProps( );

				mCommonProps = data->mProps;
				mCommonProps.fCollectCommonPropertiesForGui( data->mProps );
				tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps, false );
				mSelectedConn = conns.fFront( );
			}
		}
		else if( nodes.fCount( ) == 0 )
			mHeaderText->SetLabel( "Selection: None" );
		else if( nodes.fCount( ) > 1 )
			mHeaderText->SetLabel( "Selection: Multiple" );
		else
		{
			tGoalAINode* aiNode = dynamic_cast<tGoalAINode*>( nodes.fFront( ).fGetRawPtr( ) );
			if( aiNode )
			{
				mSelectedNode.fReset( aiNode );
				mSelectedNode->fSetControlPanel( this );
				fFillEvents( );

				mHeaderText->SetLabel( "Selection: " + aiNode->fName( ) );
				mEventCombo->Show( true );
				mPropertyPanel->Layout( );

				mCommonProps = aiNode->fAIProps( );
				mCommonProps.fCollectCommonPropertiesForGui( aiNode->fAIProps( ) );
				tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps, false );
			}
			else
			{
				mHeaderText->SetLabel( "Selection: Invalid" );
			}
		}

		fPopulateGoals( );
		mPropertyPanel->Layout( );
		Thaw( );
	}

	void tSigAIControlPanel::fRefreshProperties( )
	{
		if( mSelectedNode )
		{
			Freeze( );
			mPropertyPanel->DestroyChildren( );
			mCommonProps.fClearGui( );

			mCommonProps = mSelectedNode->fAIProps( );
			mCommonProps.fCollectCommonPropertiesForGui( mSelectedNode->fAIProps( ) );
			tEditablePropertyTable::fAddPropsToWindow( mPropertyPanel, mCommonProps, false );

			mPropertyPanel->Layout( );
			Thaw( );
		}
	}

	void tSigAIControlPanel::fLoadMoMap( const tFilePathPtr& path )
	{
		mMoMapFileName = path;
		mMoMapFunctions.fSetCount( 0 );

		const std::string cFunction = "function";

		std::string moMap;
		if( FileSystem::fReadFileToString( moMap, ToolsPaths::fMakeResAbsolute( path ) ) )
		{
			const char* start = moMap.c_str( );
			do 
			{
				const char* next = StringUtil::fReadLine( start );
				if( next )
				{
					std::string line( start, next );
					u32 funcStart = line.find( cFunction );

					if( funcStart != std::string::npos )
					{	
						const char *s = start + funcStart + cFunction.length( );
						const char *e = StringUtil::fReadUntilNewLineOrCharacter( s, '(' );

						std::string funcName( s, e );
						mMoMapFunctions.fPushBack( StringUtil::fEatWhiteSpace( funcName ) );
					}
				}
				start = next;
			} while( start );
		}
	}

	void tSigAIControlPanel::fOnPropertyChanged( tEditableProperty& property )
	{
		mCanvas->fEditorActions( ).fForceSetDirty( true );

		if( mSelectedNode )
		{
			mSelectedNode->fApplyPropertyValues( );
			mCanvas->Refresh( );
		}

		if( mSelectedConn )
		{
			tAIConnectionData* data = dynamic_cast< tAIConnectionData* >( mSelectedConn->fData( ).fGetRawPtr( ) );
			sigassert( data );

			data->fReadProps( );
			mCanvas->Refresh( );
		}

		if( mSelectedHandler )
		{
			mSelectedHandler->fReadProps( );
			mCanvas->Refresh( );
		}
	}

	void tSigAIControlPanel::fOnEventHandlerSelected( wxCommandEvent& event )
	{
		if( !mSelectedNode ) return;
		tGoalAINode* goal = dynamic_cast< tGoalAINode* >( mSelectedNode.fGetRawPtr( ) );
		if( !goal ) return;

		s32 selectedIndex = mEventCombo->GetSelection( );

		if( goal->fType( ) == tGoalAINode::cGoalType )
		{
			//if( selectedIndex == cSystemAddPolledEvent )
			//{
			//	mCanvas->fEditorActions( ).fAddAction( tEditorActionPtr( 
			//		new tAddRemoveEventHandlerAction( *mCanvas, tGoalAINodePtr( goal ), 
			//		tGoalEventHandlerPtr( new tGoalEventHandler( goal, tGoalEventHandler::cPolledType, "Event", false ) ), true ) ) );
			//}
			//else 
			if( selectedIndex == cSystemAddPotentialEvent )
			{
				if( !goal->fHasEventHandler( tGoalEventHandler::cPotentialType ) )
					mCanvas->fEditorActions( ).fAddAction( tEditorActionPtr( 
						new tAddRemoveEventHandlerAction( *mCanvas, tGoalAINodePtr( goal ), 
						tGoalEventHandlerPtr( new tGoalEventHandler( goal, tGoalEventHandler::cPotentialType, "Event", false  ) ), true ) ) );
			}
			else if ( selectedIndex == cSystemAddFirstChanceEvent )
			{
				if( !goal->fHasEventHandler( tGoalEventHandler::cFirstChanceType ) )
					mCanvas->fEditorActions( ).fAddAction( tEditorActionPtr( 
					new tAddRemoveEventHandlerAction( *mCanvas, tGoalAINodePtr( goal ), 
					tGoalEventHandlerPtr( new tGoalEventHandler( goal, tGoalEventHandler::cFirstChanceType, "Event", false  ) ), true ) ) );
			}
			else
			{
				// add event handler
				s32 index = selectedIndex - cSystemEventHandlerCount;
				if( index > -1 )
				{
					const std::string& name = StringUtil::fToString( mEventList[ index ].mKey );
					if( !goal->fHasEventHandler( name ) )
						mCanvas->fEditorActions( ).fAddAction( tEditorActionPtr( 
							new tAddRemoveEventHandlerAction( *mCanvas, tGoalAINodePtr( goal ), 
							tGoalEventHandlerPtr( new tGoalEventHandler( goal, tGoalEventHandler::cRegularType, name, false ) ), true ) ) );
				}
			}
		}
		else
		{
			// sequence or switch
			if( selectedIndex == cSequenceEvent )
			{
				tGoalEventHandler::tType handlerType = tGoalEventHandler::cSequenceType;
				if( goal->fType( ) == tGoalAINode::cSwitchType ) handlerType = tGoalEventHandler::cSwitchType;

				mCanvas->fEditorActions( ).fAddAction( tEditorActionPtr( 
					new tAddRemoveEventHandlerAction( *mCanvas, tGoalAINodePtr( goal ), 
					tGoalEventHandlerPtr( new tGoalEventHandler( goal, handlerType, "", false ) ), true ) ) );
			}
		}

		fResetEventCombo( );
	}

}
