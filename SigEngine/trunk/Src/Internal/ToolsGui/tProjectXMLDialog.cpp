#include "ToolsGuiPch.hpp"
#include "tProjectXMLDialog.hpp"


namespace Sig
{
	namespace
	{
		static const u32 cDialogFlags = wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxRESIZE_BORDER | wxTAB_TRAVERSAL | wxFRAME_NO_TASKBAR | wxSTAY_ON_TOP;
	}

	tProjectXMLDialog::tProjectXMLDialog( wxWindow* parent )
		: wxDialog( parent, wxID_ANY, "Project XML", wxDefaultPosition, wxSize( 575, 400 ), cDialogFlags )
	{
		fSetChanged( false );
		SetIcon( wxIcon( "appicon" ) );
		SetBackgroundColour( GetBackgroundColour( ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		wxPanel* enumsPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE, "Enums" );
		GetSizer( )->Add( enumsPanel, 1, wxEXPAND | wxALL, 2 );
		enumsPanel->SetSizer( new wxBoxSizer( wxVERTICAL ) );

		// flags
		{
			wxBoxSizer* nameSizer = new wxBoxSizer( wxHORIZONTAL );
			enumsPanel->GetSizer( )->Add( nameSizer, wxSizerFlags( 0 ).Right( ) );

			nameSizer->Add( new wxStaticText( enumsPanel, wxID_ANY, "Flags: " ), 0 );
			mFlags = new wxComboBox( enumsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY );
			//mFlags->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(tProjectXMLDialog::fOnFlagBoxChange), NULL, this );
			nameSizer->Add( mFlags, 3 );

			wxButton* addB = new wxButton( enumsPanel, wxID_ANY, "Add" );
			nameSizer->Add( addB, 1 );
			addB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnAddFlag), NULL, this );

			wxButton* removeB = new wxButton( enumsPanel, wxID_ANY, "Remove" );
			nameSizer->Add( removeB, 1 );
			removeB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRemoveFlag), NULL, this );
			
			wxButton* renameB = new wxButton( enumsPanel, wxID_ANY, "Rename" );
			nameSizer->Add( renameB, 1 );
			renameB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRenameFlag), NULL, this );
		}
		// game events
		{
			wxBoxSizer* nameSizer = new wxBoxSizer( wxHORIZONTAL );
			enumsPanel->GetSizer( )->Add( nameSizer, wxSizerFlags( 0 ).Right( ) );

			nameSizer->Add( new wxStaticText( enumsPanel, wxID_ANY, "Game Events: " ), 0 );
			mGameEvents = new wxComboBox( enumsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY );
			//mFlags->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(tProjectXMLDialog::fOnFlagBoxChange), NULL, this );
			nameSizer->Add( mGameEvents, 3 );

			wxButton* addB = new wxButton( enumsPanel, wxID_ANY, "Add" );
			nameSizer->Add( addB, 1 );
			addB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnAddGameEvent), NULL, this );

			wxButton* removeB = new wxButton( enumsPanel, wxID_ANY, "Remove" );
			nameSizer->Add( removeB, 1 );
			removeB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRemoveGameEvent), NULL, this );

			wxButton* renameB = new wxButton( enumsPanel, wxID_ANY, "Rename" );
			nameSizer->Add( renameB, 1 );
			renameB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRenameGameEvent), NULL, this );
		}
		// keyframe events
		{
			wxBoxSizer* nameSizer = new wxBoxSizer( wxHORIZONTAL );
			enumsPanel->GetSizer( )->Add( nameSizer, wxSizerFlags( 0 ).Right( ) );

			nameSizer->Add( new wxStaticText( enumsPanel, wxID_ANY, "Keyframe Events: " ), 0 );
			mKeyframeEvents = new wxComboBox( enumsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY );
			//mFlags->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(tProjectXMLDialog::fOnFlagBoxChange), NULL, this );
			nameSizer->Add( mKeyframeEvents, 3 );

			wxButton* addB = new wxButton( enumsPanel, wxID_ANY, "Add" );
			nameSizer->Add( addB, 1 );
			addB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnAddKeyFrameEvent), NULL, this );

			wxButton* removeB = new wxButton( enumsPanel, wxID_ANY, "Remove" );
			nameSizer->Add( removeB, 1 );
			removeB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRemoveKeyFrameEvent), NULL, this );

			wxButton* renameB = new wxButton( enumsPanel, wxID_ANY, "Rename" );
			nameSizer->Add( renameB, 1 );
			renameB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRenameKeyFrame), NULL, this );
		}
		// ai flags
		{
			wxBoxSizer* nameSizer = new wxBoxSizer( wxHORIZONTAL );
			enumsPanel->GetSizer( )->Add( nameSizer, wxSizerFlags( 0 ).Right( ) );

			nameSizer->Add( new wxStaticText( enumsPanel, wxID_ANY, "AI Flags: " ), 0 );
			mAIFlags = new wxComboBox( enumsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY );
			//mFlags->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(tProjectXMLDialog::fOnFlagBoxChange), NULL, this );
			nameSizer->Add( mAIFlags, 3 );

			wxButton* addB = new wxButton( enumsPanel, wxID_ANY, "Add" );
			nameSizer->Add( addB, 1 );
			addB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnAddAIFlag), NULL, this );

			wxButton* removeB = new wxButton( enumsPanel, wxID_ANY, "Remove" );
			nameSizer->Add( removeB, 1 );
			removeB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRemoveAIFlag), NULL, this );

			wxButton* renameB = new wxButton( enumsPanel, wxID_ANY, "Rename" );
			nameSizer->Add( renameB, 1 );
			renameB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRenameAIFlag), NULL, this );
		}
		// enum names
		{
			wxBoxSizer* enumNameSizer = new wxBoxSizer( wxHORIZONTAL );
			enumsPanel->GetSizer( )->Add( enumNameSizer, wxSizerFlags( 0 ).Right( ) );

			enumNameSizer->Add( new wxStaticText( enumsPanel, wxID_ANY, "Enums: " ), 0 );
			mEnums = new wxComboBox( enumsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY );
			mEnums->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(tProjectXMLDialog::fOnEnumBoxChange), NULL, this );
			enumNameSizer->Add( mEnums, 3 );

			wxButton* addB = new wxButton( enumsPanel, wxID_ANY, "Add" );
			enumNameSizer->Add( addB, 1 );
			addB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnAddEnum), NULL, this );

			wxButton* removeB = new wxButton( enumsPanel, wxID_ANY, "Remove" );
			enumNameSizer->Add( removeB, 1 );
			removeB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRemoveEnum), NULL, this );

			wxButton* renameB = new wxButton( enumsPanel, wxID_ANY, "Rename" );
			enumNameSizer->Add( renameB, 1 );
			renameB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRenameEnum), NULL, this );
		}

		wxBoxSizer* enumValueTopSizer = new wxBoxSizer( wxHORIZONTAL );
		enumsPanel->GetSizer( )->Add( enumValueTopSizer, 3, wxEXPAND | wxALL );

		mEnumValues = new wxListBox( enumsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_SINGLE | wxLB_NEEDED_SB );
		mEnumValues->Connect( wxEVT_COMMAND_LISTBOX_SELECTED , wxCommandEventHandler(tProjectXMLDialog::fOnEnumValueBoxChange), NULL, this );
		enumValueTopSizer->Add( mEnumValues, 3, wxEXPAND | wxALL );

		wxBoxSizer* enumValueButtonSizer = new wxBoxSizer( wxVERTICAL );
		enumValueTopSizer->Add( enumValueButtonSizer, 0 );

		wxButton* addB = new wxButton( enumsPanel, wxID_ANY, "<--Add--<" );
		enumValueButtonSizer->Add( addB, 1 );
		addB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnAddEnumValue), NULL, this );

		wxButton* insertB = new wxButton( enumsPanel, wxID_ANY, "<-Insert-<" );
		insertB->SetToolTip( "Insert the enum values before the currently selected one." );
		enumValueButtonSizer->Add( insertB, 1 );
		insertB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnInsertEnumValue), NULL, this );

		wxButton* removeB = new wxButton( enumsPanel, wxID_ANY, "Remove" );
		enumValueButtonSizer->Add( removeB, 1 );
		removeB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fOnRemoveEnumValue), NULL, this );

		wxButton* moveUpB = new wxButton( enumsPanel, wxID_ANY, "Move Up" );
		enumValueButtonSizer->Add( moveUpB, 1 );
		moveUpB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fMoveEnumValueUp), NULL, this );
		wxButton* moveDownB = new wxButton( enumsPanel, wxID_ANY, "Move Down" );
		enumValueButtonSizer->Add( moveDownB, 1 );
		moveDownB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fMoveEnumValueDown), NULL, this );
		wxButton* renameB = new wxButton( enumsPanel, wxID_ANY, "Rename" );
		enumValueButtonSizer->Add( renameB, 1 );
		renameB->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fRenameEnumValue), NULL, this );


		mNewValues = new wxTextCtrl( enumsPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_DONTWRAP );
		enumValueTopSizer->Add( mNewValues, 3, wxEXPAND | wxALL );

		// Add button that will handle Esc and also closing the dialog too.
		wxBoxSizer* bottomSizer = new wxBoxSizer( wxHORIZONTAL );
		wxButton* cancelButton = new wxButton( this, wxID_ANY, "Cancel" );
		wxButton* saveButton = new wxButton( this, wxID_ANY, "Save" );
		cancelButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fCancel), NULL, this );
		saveButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tProjectXMLDialog::fSave), NULL, this );

		GetSizer( )->AddSpacer( 2 );
		GetSizer( )->Add( bottomSizer, 0, wxEXPAND | wxALL );
		bottomSizer->Add( cancelButton, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, 2 );
		bottomSizer->Add( saveButton, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT, 2 );

		Connect( wxEVT_CLOSE_WINDOW, wxCloseEventHandler( tProjectXMLDialog::fOnClose ), NULL, this );
		
		fLoad( );
		fPopulateFlags( );
		fPopulateGameEvents( );
		fPopulateKeyframeEvents( );
		fPopulateAIFlags( );
		fPopulateEnumTypes( );
		fPopulateEnumValues( );

		Layout( );
		Refresh( );
	}

	void tProjectXMLDialog::fSetChanged( b32 changed )
	{
		if( changed )
		{
			mChanged = true;
		}
		else
		{
			mChanged = false;
			mRequiresRebuildOfSigmls = false;
			mRequiresStringReferenceUpdates = false;
			mRequiresValidationOfEnumValueOrderDependentStuff = false;
			mRequiresRebuildingGoamls = false;
		}
	}

	void tProjectXMLDialog::fOnEnumBoxChange( wxCommandEvent& event )
	{
		fPopulateEnumValues( );
	}

	void tProjectXMLDialog::fOnEnumValueBoxChange( wxCommandEvent& event )
	{
		const s32 newIdx = mEnumValues->GetSelection( );
	}

	void tProjectXMLDialog::fLoad( )
	{
		mFile.fLoadXml( ToolsPaths::fGetCurrentProjectFilePath( ) );
		fSetChanged( false );
	}

	void tProjectXMLDialog::fSave( wxCommandEvent& )
	{
		mFile.fSaveXml( ToolsPaths::fGetCurrentProjectFilePath( ), true );

		std::string changesNeeded;
		if( mRequiresRebuildOfSigmls ) changesNeeded += "You must now rebuild all sigmls. (cmd: rebuildsigmls)\n";
		if( mRequiresStringReferenceUpdates ) changesNeeded += "You must now update all scripts and datatables that reference the enum values by name.\n";
		if( mRequiresValidationOfEnumValueOrderDependentStuff ) changesNeeded += "You must now update all datatables that rely on the order of the enum values.\n";
		if( mRequiresRebuildingGoamls ) changesNeeded += "You must now run the command: rebuildgoamls\n";

		fSetChanged( false );
		if( wxMessageBox( changesNeeded + "\nCompile game settings?", "Build?", wxYES_NO, this ) == wxYES )
			mFile.fCompileGameSettings( true );
	}

	tProjectFile::tGameEnumeratedType* tProjectXMLDialog::fCurrentEnum( )
	{
		s32 id = mEnums->GetSelection( );
		return ( id > -1 ) ? &mFile.mGameEnumeratedTypes[ id ] : NULL;
	}

	void tProjectXMLDialog::fPopulateFlags( )
	{
		mFlags->Clear( );

		for( u32 i = 0; i < mFile.mGameTags.fCount( ); ++i )
		{
			mFlags->Append( mFile.mGameTags[ i ].mName );
		}
	}

	void tProjectXMLDialog::fPopulateAIFlags( )
	{
		mAIFlags->Clear( );

		for( u32 i = 0; i < mFile.mAIFlags.fCount( ); ++i )
		{
			mAIFlags->Append( mFile.mAIFlags[ i ].mName );
		}
	}

	void tProjectXMLDialog::fPopulateGameEvents( )
	{
		mGameEvents->Clear( );

		for( u32 i = 0; i < mFile.mGameEvents.fCount( ); ++i )
		{
			mGameEvents->Append( mFile.mGameEvents[ i ].mName );
		}
	}

	void tProjectXMLDialog::fPopulateKeyframeEvents( )
	{
		mKeyframeEvents->Clear( );

		for( u32 i = 0; i < mFile.mKeyFrameEvents.fCount( ); ++i )
		{
			mKeyframeEvents->Append( mFile.mKeyFrameEvents[ i ].mName );
		}
	}

	void tProjectXMLDialog::fPopulateEnumTypes( )
	{
		mEnums->Clear( );
		mEnumValues->Clear( );

		for( u32 i = 0; i < mFile.mGameEnumeratedTypes.fCount( ); ++i )
		{
			mEnums->Append( mFile.mGameEnumeratedTypes[ i ].mName );
		}
	}

	void tProjectXMLDialog::fPopulateEnumValues( )
	{
		mEnumValues->Clear( );

		tProjectFile::tGameEnumeratedType* type = fCurrentEnum( );
		if( type )
		{
			for( u32 i = 0; i < type->mValues.fCount( ); ++i )
			{
				mEnumValues->Append( type->mValues[ i ].mName );
			}
		}
	}

	void tProjectXMLDialog::fOnAddFlag( wxCommandEvent& event )
	{
		wxString str = wxGetTextFromUser( "Enter Flag Name: ", "Add flag value.", "", this );
		std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
		if( result.length( ) > 0 )
		{
			mFile.fInsertTag( result, -1 );
			fPopulateFlags( );
			mFlags->Select( mFlags->GetCount( ) - 1 );
			fSetChanged( true );
		}
	}

	void tProjectXMLDialog::fOnRemoveFlag( wxCommandEvent& event )
	{
		s32 id = mFlags->GetSelection( );
		if( id > -1 )
		{
			if ( wxMessageBox("Remove flag: " + mFile.mGameTags[ id ].mName + "?",
				"Please confirm",
				wxICON_QUESTION | wxYES_NO) == wxYES )
			{
				mFile.mGameTags.fEraseOrdered( id );
				fPopulateFlags( );
				mFlags->Select( id - 1);
				mRequiresRebuildingGoamls = true;
			}
		}
	}

	void tProjectXMLDialog::fOnRenameFlag( wxCommandEvent& event )
	{
		s32 id = mFlags->GetSelection( );
		if( id > -1 )
		{
			tProjectFile::tGameTag &value = mFile.mGameTags[ id ];

			wxString str = wxGetTextFromUser( "Enter New Name: OldName = " + value.mName, "Rename flag.", value.mName, this );
			std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
			if( result.length( ) > 0 && result != value.mName )
			{
				value.mName = result.c_str( );
				fSetChanged( true );
				fPopulateFlags( );
				mFlags->Select( id );
			}
		}
		else
			wxMessageBox( "No flag selected." );
	}

	void tProjectXMLDialog::fOnAddEnum( wxCommandEvent& event )
	{
		wxString str = wxGetTextFromUser( "Enter Enum Name: ", "Add enum value.", "", this );
		std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
		if( result.length( ) > 0 )
		{
			mFile.fInsertEnum( result, -1 );
			fPopulateEnumTypes( );
			mEnums->Select( mEnums->GetCount( ) - 1 );
			fPopulateEnumValues( );
			fSetChanged( true );
		}
	}

	void tProjectXMLDialog::fOnRemoveEnum( wxCommandEvent& event )
	{
		s32 id = mEnums->GetSelection( );
		if( id > -1 )
		{
			if ( wxMessageBox("Remove enum: " + mFile.mGameEnumeratedTypes[ id ].mName + "?",
				"Please confirm",
				wxICON_QUESTION | wxYES_NO) == wxYES )
			{
				mFile.mGameEnumeratedTypes.fEraseOrdered( id );
				fPopulateEnumTypes( );
				mEnums->Select( id - 1);
				fPopulateEnumValues( );
				fSetChanged( true );
			}
		}
	}

	void tProjectXMLDialog::fOnRenameEnum( wxCommandEvent& event )
	{
		s32 id = mEnums->GetSelection( );
		if( id > -1 )
		{
			tProjectFile::tGameEnumeratedType &value = mFile.mGameEnumeratedTypes[ id ];

			wxString str = wxGetTextFromUser( "Enter New Name: OldName = " + value.mName, "Rename enum.", value.mName, this );
			std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
			if( result.length( ) > 0 && result != value.mName )
			{
				value.mName = result.c_str( );
				fSetChanged( true );
				fPopulateEnumTypes( );
				mEnums->Select( id );
				fPopulateEnumValues( );
			}
		}
		else
			wxMessageBox( "No enum selected." );
	}

	void tProjectXMLDialog::fOnAddGameEvent( wxCommandEvent& event )
	{
		wxString str = wxGetTextFromUser( "Enter Game Event Name: ", "Add game event value.", "", this );
		std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
		if( result.length( ) > 0 )
		{
			mFile.fInsertGameEvent( result, -1 );
			fPopulateGameEvents( );
			mGameEvents->Select( mGameEvents->GetCount( ) - 1 );
			fSetChanged( true );
		}
	}

	void tProjectXMLDialog::fOnRemoveGameEvent( wxCommandEvent& event )
	{
		s32 id = mGameEvents->GetSelection( );
		if( id > -1 )
		{
			if ( wxMessageBox("Remove game event: " + mFile.mGameEvents[ id ].mName + "?",
				"Please confirm",
				wxICON_QUESTION | wxYES_NO) == wxYES )
			{
				mFile.mGameEvents.fEraseOrdered( id );
				fPopulateGameEvents( );
				mGameEvents->Select( id - 1);
			}
		}
	}

	void tProjectXMLDialog::fOnRenameGameEvent( wxCommandEvent& event )
	{
		s32 id = mGameEvents->GetSelection( );
		if( id > -1 )
		{
			tProjectFile::tEvent &value = mFile.mGameEvents[ id ];

			wxString str = wxGetTextFromUser( "Enter New Name: OldName = " + value.mName, "Rename game event.", value.mName, this );
			std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
			if( result.length( ) > 0 && result != value.mName )
			{
				value.mName = result.c_str( );
				fSetChanged( true );
				fPopulateGameEvents( );
				mGameEvents->Select( id );
			}
		}
		else
			wxMessageBox( "No game event selected." );
	}

	void tProjectXMLDialog::fOnAddKeyFrameEvent( wxCommandEvent& event )
	{
		wxString str = wxGetTextFromUser( "Enter Keyframe Event Name: ", "Add keyframe event value.", "", this );
		std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
		if( result.length( ) > 0 )
		{
			mFile.fInsertKeyframeEvent( result, -1 );
			fPopulateKeyframeEvents( );
			mKeyframeEvents->Select( mKeyframeEvents->GetCount( ) - 1 );
			fSetChanged( true );
		}
	}

	void tProjectXMLDialog::fOnRemoveKeyFrameEvent( wxCommandEvent& event )
	{
		s32 id = mKeyframeEvents->GetSelection( );
		if( id > -1 )
		{
			if ( wxMessageBox("Remove keyframe event: " + mFile.mKeyFrameEvents[ id ].mName + "?",
				"Please confirm",
				wxICON_QUESTION | wxYES_NO) == wxYES )
			{
				mFile.mKeyFrameEvents.fEraseOrdered( id );
				fPopulateKeyframeEvents( );
				mKeyframeEvents->Select( id - 1);
			}
		}
	}

	void tProjectXMLDialog::fOnRenameKeyFrame( wxCommandEvent& event )
	{
		s32 id = mKeyframeEvents->GetSelection( );
		if( id > -1 )
		{
			tProjectFile::tEvent &value = mFile.mKeyFrameEvents[ id ];

			wxString str = wxGetTextFromUser( "Enter New Name: OldName = " + value.mName, "Rename keyframe event.", value.mName, this );
			std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
			if( result.length( ) > 0 && result != value.mName )
			{
				value.mName = result.c_str( );
				fSetChanged( true );
				fPopulateKeyframeEvents( );
				mKeyframeEvents->Select( id );
			}
		}
		else
			wxMessageBox( "No keyframe event selected." );
	}

	void tProjectXMLDialog::fOnAddAIFlag( wxCommandEvent& event )
	{
		wxString str = wxGetTextFromUser( "Enter AI Flag Name: ", "Add AI Flag value.", "", this );
		std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
		if( result.length( ) > 0 )
		{
			mFile.fInsertAIFlags( result, -1 );
			fPopulateAIFlags( );
			mAIFlags->Select( mKeyframeEvents->GetCount( ) - 1 );
			fSetChanged( true );
		}
	}

	void tProjectXMLDialog::fOnRemoveAIFlag( wxCommandEvent& event )
	{
		s32 id = mAIFlags->GetSelection( );
		if( id > -1 )
		{
			if ( wxMessageBox("Remove AI flag event: " + mFile.mAIFlags[ id ].mName + "?",
				"Please confirm",
				wxICON_QUESTION | wxYES_NO) == wxYES )
			{
				mFile.mAIFlags.fEraseOrdered( id );
				fPopulateAIFlags( );
				mAIFlags->Select( id - 1);
			}
		}
	}

	void tProjectXMLDialog::fOnRenameAIFlag( wxCommandEvent& event )
	{
		s32 id = mAIFlags->GetSelection( );
		if( id > -1 )
		{
			tProjectFile::tEvent &value = mFile.mAIFlags[ id ];

			wxString str = wxGetTextFromUser( "Enter New Name: OldName = " + value.mName, "Rename AI flag.", value.mName, this );
			std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
			if( result.length( ) > 0 && result != value.mName )
			{
				value.mName = result.c_str( );
				fSetChanged( true );
				fPopulateAIFlags( );
				mAIFlags->Select( id );
			}
		}
		else
			wxMessageBox( "No AI flag selected." );
	}


	void tProjectXMLDialog::fOnAddEnumValue( wxCommandEvent& event )
	{
		tProjectFile::tGameEnumeratedType* type = fCurrentEnum( );
		if( type )
		{
			mNewValues->SetValue( fInsertEnumValues( mNewValues->GetValue( ), -1 ) );
		}
		else
			wxMessageBox( "No enum selected." );
	}

	void tProjectXMLDialog::fOnInsertEnumValue( wxCommandEvent& event )
	{
		tProjectFile::tGameEnumeratedType* type = fCurrentEnum( );
		if( type )
		{
			s32 id = mEnumValues->GetSelection( );
			if( id > -1 )
			{
				mNewValues->SetValue( fInsertEnumValues( mNewValues->GetValue( ), id ) );
			}
			else
				wxMessageBox( "No insertion point selected." );
		}
		else
			wxMessageBox( "No enum selected." );
	}

	wxString tProjectXMLDialog::fInsertEnumValues( const wxString& newLineDelVals, u32 insertIndex )
	{
		wxString notInserted;

		tProjectFile::tGameEnumeratedType* type = fCurrentEnum( );
		sigassert( type );

		tGrowableArray< std::string > strs;
		StringUtil::fSplit( strs, newLineDelVals.c_str( ), "\n" );

		insertIndex = fMin( insertIndex, type->mValues.fCount( ) );
		if( insertIndex < type->mValues.fCount( ) )
		{
			mRequiresRebuildOfSigmls = true;
			mRequiresStringReferenceUpdates = true;
		}

		u32 lastIndex = insertIndex;

		for( u32 i = 0; i < strs.fCount( ); ++i )
		{
			std::string trimed = StringUtil::fEatWhiteSpace( strs[ i ] );
			trimed = StringUtil::fReplaceAllOf( trimed, "_", " " );
			if( type->fFindValueIndexByName( trimed ) == ~0 )
			{
				type->fInsertValue( trimed, insertIndex + i );
				lastIndex = insertIndex + i + 1;
			}
			else
				notInserted += trimed + "\n";
		}

		if( notInserted.length( ) > 0 )
			wxMessageBox( "Already exist:\n" + notInserted );

		fSetChanged( true );
		fPopulateEnumValues( );
		mEnumValues->SetSelection( lastIndex );

		return notInserted;
	}

	void tProjectXMLDialog::fOnRemoveEnumValue( wxCommandEvent& event )
	{
		tProjectFile::tGameEnumeratedType* type = fCurrentEnum( );
		if( type )
		{
			s32 id = mEnumValues->GetSelection( );
			if( id > -1 )
			{
				mRequiresStringReferenceUpdates = true;
				mRequiresRebuildOfSigmls = true;
				if( id < (s32)mEnumValues->GetCount( ) - 1 ) mRequiresValidationOfEnumValueOrderDependentStuff = true;
				fSetChanged( true );
				type->mValues.fEraseOrdered( id );
				fPopulateEnumValues( );
				mEnumValues->SetSelection( id );
			}
			else
				wxMessageBox( "No enum value selected." );
		}
		else
			wxMessageBox( "No enum selected." );
	}

	void tProjectXMLDialog::fRenameEnumValue( wxCommandEvent& event )
	{
		tProjectFile::tGameEnumeratedType* type = fCurrentEnum( );
		if( type )
		{
			s32 id = mEnumValues->GetSelection( );
			if( id > -1 )
			{
				tProjectFile::tGameEnumeratedValue &value = type->mValues[ id ];
				
				wxString str = wxGetTextFromUser( "Enter New Name: OldName = " + value.mName, "Rename enum value.", value.mName, this );
				std::string result = StringUtil::fEatWhiteSpace( std::string( str ) );
				if( result.length( ) > 0 && result != value.mName )
				{
					value.mName = result;
					mRequiresStringReferenceUpdates = true;
					fSetChanged( true );
					fPopulateEnumValues( );
					mEnumValues->SetSelection( id );
				}
			}
			else
				wxMessageBox( "No enum value selected." );
		}
		else
			wxMessageBox( "No enum selected." );
	}

	void tProjectXMLDialog::fMoveEnumValueUp( wxCommandEvent& event )
	{
		fMoveEnumValue( true );
	}

	void tProjectXMLDialog::fMoveEnumValueDown( wxCommandEvent& event )
	{
		fMoveEnumValue( false );
	}

	void tProjectXMLDialog::fMoveEnumValue( b32 up )
	{
		tProjectFile::tGameEnumeratedType* type = fCurrentEnum( );
		if( type )
		{
			s32 id = mEnumValues->GetSelection( );
			if( id > -1 )
			{
				tProjectFile::tGameEnumeratedValue value = type->mValues[ id ];
				u32 newID = fClamp<s32>( id + (up ? -1 : 1), 0, type->mValues.fCount( ) - 1 );

				mRequiresValidationOfEnumValueOrderDependentStuff = true;
				mRequiresRebuildOfSigmls = true;
				fSetChanged( true );
				type->mValues.fEraseOrdered( id );
				type->mValues.fInsertSafe( newID, value );
				fPopulateEnumValues( );
				mEnumValues->SetSelection( newID );
			}
			else
				wxMessageBox( "No enum value selected." );
		}
		else
			wxMessageBox( "No enum selected." );
	}

	void tProjectXMLDialog::fOnClose(wxCloseEvent& event)
	{
		if ( mChanged && wxMessageBox("The file has not been saved... continue closing?",
			"Please confirm",
			wxICON_QUESTION | wxYES_NO) != wxYES )
		{
			event.Veto();
			return;
		}

		Destroy();
		event.Skip( );
	}

	void tProjectXMLDialog::fCancel( wxCommandEvent& event )
	{
		Close( );
	}
}
