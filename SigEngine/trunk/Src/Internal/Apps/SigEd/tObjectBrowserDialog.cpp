#include "SigEdPch.hpp"
#include "tObjectBrowserDialog.hpp"
#include "FileSystem.hpp"
#include "tEditorAppWindow.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "Editor/tFxmlReferenceEntity.hpp"
#include "Editor/tEditableTerrainEntity.hpp"
#include "Editor/tEditableLightEntity.hpp"
#include "Editor/tEditableLightProbeEntity.hpp"
#include "Editor/tEditableAttachmentEntity.hpp"
#include "Editor/tEditableShapeEntity.hpp"
#include "Editor/tEditableWaypointEntity.hpp"
#include "Editor/tEditablePathDecalWaypointEntity.hpp"
#include "Editor/tEditableNavGraphNodeEntity.hpp"
#include "Editor/tEditableCameraEntity.hpp"
#include "Editor/tEditableTileCanvas.hpp"
#include "Editor/tEditableTileEntity.hpp"
#include "Editor/tEditableGroupEntity.hpp"
#include "Fxml.hpp"
#include "tWxColumnListBox.hpp"

namespace Sig
{
	static const u32 cOutsideBorderSpacer	= 2;
	static const u32 cItemSpacer			= 6;

	enum tColumnTypes
	{
		cFileOrName = 0,
		cReferenceName,
		cScript,
	};


	tObjectBrowserDialog::tObjectBrowserDialog( tEditorAppWindow* editorWindow )
		: tEditorDialog( editorWindow, "ObjectBrowserDialog" )
		, mMainPanel( NULL )
		, mSearchBox( NULL )
		, mMshmlBox( NULL )
		, mSigmlBox( NULL )
		, mFxmlBox( NULL )
		, mTerrainBox( NULL )
		, mLightBox( NULL )
		, mLightProbeBox( NULL )
		, mAttachBox( NULL )
		, mShapeBox( NULL )
		, mWaypointBox( NULL )
		, mNavBox( NULL )
		, mCameraBox( NULL )
		, mTileCanvasBox( NULL )
		, mTilePropsBox( NULL )
		, mGroupsBox( NULL )
		, mNameBox( NULL )
		, mFilePathBox( NULL )
		, mScriptBox( NULL )
		, mGamePropsBox( NULL )
		, mResultsBox( NULL )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetTitle( "Object Browser" );

		mMainPanel = new wxScrolledWindow( this );
		mMainPanel->SetBackgroundColour( GetBackgroundColour( ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		GetSizer( )->Add( mMainPanel, 1, wxEXPAND, 0 );

		fBuildGui( );
	}

	bool tObjectBrowserDialog::Show( bool show )
	{
		const bool o = tWxSlapOnDialog::Show( show );

		if( IsShown( ) )
		{
			fRefreshList( );
		}

		return o;
	}

	void tObjectBrowserDialog::fClear( )
	{
		// Release all refs.
		const u32 numItems = mResultsBox->GetItemCount( );
		for( u32 i = 0; i < numItems; ++i )
		{
			tEditableObject* objPtr = reinterpret_cast< tEditableObject* >( mResultsBox->GetItemData( i ) );
			fRefCounterPtrDecRef( objPtr );
		}

		// Clear everything that exists.
		mResultsBox->DeleteAllItems( );
	}

	void tObjectBrowserDialog::fBuildGui( )
	{
		// Create main window panel's sizer.
		mMainPanel->SetSizer( new wxBoxSizer( wxVERTICAL ) );

		// Top half sizer to hold search box and filters.
		wxBoxSizer* topHalf = new wxBoxSizer( wxVERTICAL );
		mMainPanel->GetSizer( )->Add( topHalf, 0, wxALIGN_CENTER_HORIZONTAL | wxEXPAND, 0 );



		// Make panel for search box to put in top half sizer.
		wxPanel* searchPanel = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
		wxBoxSizer* searchSizer = new wxBoxSizer( wxVERTICAL );
		searchPanel->SetSizer( searchSizer );
		topHalf->Add( searchPanel, 1, wxALL | wxEXPAND, cOutsideBorderSpacer );
		{
			// Title.
			wxStaticText* staticText = new wxStaticText( searchPanel, wxID_ANY, wxT("Text Filter"), wxDefaultPosition, wxDefaultSize );
			searchSizer->Add( staticText, 0, wxALL, cItemSpacer );
		}

		// Text box.
		mSearchBox = new wxTextCtrl( searchPanel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER );
		searchSizer->Add( mSearchBox, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, cItemSpacer );

		// Search button.
		//wxButton* searchButton = new wxButton( searchPanel, wxID_ANY, wxT("Search"), wxDefaultPosition, wxSize( 100, 20 ) );
		//searchSizer->Add( searchButton, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT, cItemSpacer );



		// Panel for filters.
		wxPanel* typeFilterPanel = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
		wxBoxSizer* typePanelSizer = new wxBoxSizer( wxHORIZONTAL );
		
		wxBoxSizer* typeFilterCol = new wxBoxSizer( wxVERTICAL );

		wxBoxSizer* typeFilterRow1 = new wxBoxSizer( wxHORIZONTAL );
		wxBoxSizer* typeFilterRow2 = new wxBoxSizer( wxHORIZONTAL );

		// Overall horizontal sizer.
		typeFilterPanel->SetSizer( typePanelSizer );

		// Add static text label for first element of overall horizontal direction.
		topHalf->Add( typeFilterPanel, 0, wxALL | wxEXPAND, cOutsideBorderSpacer );
		{
			// Title.
			wxStaticText* staticText = new wxStaticText( typeFilterPanel, wxID_ANY, wxT("Type Filters"), wxDefaultPosition, wxDefaultSize );
			typePanelSizer->Add( staticText, 0, wxALL, cItemSpacer );
		}

		// Add vertical column and then two rows of horizontal spaces for filters.
		typePanelSizer->Add( typeFilterCol );
		typeFilterCol->Add( typeFilterRow1 );
		typeFilterCol->Add( typeFilterRow2 );

		// Mshml box.
		mMshmlBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Mshml") );
		mMshmlBox->SetValue( true );
		typeFilterRow1->Add( mMshmlBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer ); // Space out the bottom independently.

		// Sigml box.
		mSigmlBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Sigml") );
		mSigmlBox->SetValue( true );
		typeFilterRow1->Add( mSigmlBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Fxml box.
		mFxmlBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Fx") );
		mFxmlBox->SetValue( true );
		typeFilterRow1->Add( mFxmlBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Terrain box.
		mTerrainBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Terrain") );
		mTerrainBox->SetValue( true );
		typeFilterRow1->Add( mTerrainBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Light box.
		mLightBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Lights") );
		mLightBox->SetValue( true );
		typeFilterRow1->Add( mLightBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// LightProbe box.
		mLightProbeBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Light Probes") );
		mLightProbeBox->SetValue( true );
		typeFilterRow1->Add( mLightProbeBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Attachment box.
		mAttachBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Attachments") );
		mAttachBox->SetValue( true );
		typeFilterRow1->Add( mAttachBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Shape box.
		mShapeBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Shapes") );
		mShapeBox->SetValue( true );
		typeFilterRow1->Add( mShapeBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Waypoint box.
		mWaypointBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Waypoints") );
		mWaypointBox->SetValue( true );
		typeFilterRow1->Add( mWaypointBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Decal box.
		mDecalsBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Decals") );
		mDecalsBox->SetValue( true );
		typeFilterRow1->Add( mDecalsBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Nav box.
		mNavBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Nav Nodes") );
		mNavBox->SetValue( true );
		typeFilterRow1->Add( mNavBox, 0, wxALL, cItemSpacer );
		typeFilterRow1->AddSpacer( cItemSpacer );

		// Camera box.
		mCameraBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Cameras") );
		mCameraBox->SetValue( true );
		typeFilterRow2->Add( mCameraBox, 0, wxALL, cItemSpacer );
		typeFilterRow2->AddSpacer( cItemSpacer );

		// Tile canvases box.
		mTileCanvasBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Tile Canvases") );
		mTileCanvasBox->SetValue( true );
		typeFilterRow2->Add( mTileCanvasBox, 0, wxALL, cItemSpacer );
		typeFilterRow2->AddSpacer( cItemSpacer );

		// Tile props box.
		mTilePropsBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Tile Props") );
		mTilePropsBox->SetValue( true );
		typeFilterRow2->Add( mTilePropsBox, 0, wxALL, cItemSpacer );
		typeFilterRow2->AddSpacer( cItemSpacer );

		// Groups box.
		mGroupsBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Groups") );
		mGroupsBox->SetValue( true );
		typeFilterRow2->Add( mGroupsBox, 0, wxALL, cItemSpacer );
		typeFilterRow2->AddSpacer( cItemSpacer );



		// Panel for filters.
		wxPanel* fieldFilterPanel = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
		wxBoxSizer* fieldFilterSizer = new wxBoxSizer( wxHORIZONTAL );
		fieldFilterPanel->SetSizer( fieldFilterSizer );
		topHalf->Add( fieldFilterPanel, 0, wxALL | wxEXPAND, cOutsideBorderSpacer );
		{
			// Title.
			wxStaticText* staticText = new wxStaticText( fieldFilterPanel, wxID_ANY, wxT("Field Filters"), wxDefaultPosition, wxDefaultSize );
			fieldFilterSizer->Add( staticText, 0, wxALL, cItemSpacer );
		}

		// File path box.
		mFilePathBox = new wxCheckBox( fieldFilterPanel, wxID_ANY, "File/Type" );
		mFilePathBox->SetValue( true );
		fieldFilterSizer->Add( mFilePathBox, 0, wxALL, cItemSpacer );
		fieldFilterSizer->AddSpacer( cItemSpacer );

		// Reference name box.
		mNameBox = new wxCheckBox( fieldFilterPanel, wxID_ANY, wxT("Name") );
		mNameBox->SetValue( true );
		fieldFilterSizer->Add( mNameBox, 0, wxALL, cItemSpacer );
		fieldFilterSizer->AddSpacer( cItemSpacer );

		// Script name box.
		mScriptBox = new wxCheckBox( fieldFilterPanel, wxID_ANY, wxT("Script") );
		mScriptBox->SetValue( true );
		fieldFilterSizer->Add( mScriptBox, 0, wxALL, cItemSpacer );
		fieldFilterSizer->AddSpacer( cItemSpacer );

		// Game props box.
		mGamePropsBox = new wxCheckBox( fieldFilterPanel, wxID_ANY, wxT("Game Properties") );
		mGamePropsBox->SetValue( true );
		fieldFilterSizer->Add( mGamePropsBox, 0, wxALL, cItemSpacer );
		fieldFilterSizer->AddSpacer( cItemSpacer );



		// Lower panel for search results.
		wxPanel* searchResultsPanel = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
		wxBoxSizer* searchResultsSizer = new wxBoxSizer( wxVERTICAL );
		searchResultsPanel->SetSizer( searchResultsSizer );
		mMainPanel->GetSizer( )->Add( searchResultsPanel, 1, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, cOutsideBorderSpacer );
		{
			// Title.
			wxStaticText* staticText = new wxStaticText( searchResultsPanel, wxID_ANY, wxT("Search Results"), wxDefaultPosition, wxDefaultSize );
			searchResultsSizer->Add( staticText, 0, wxALIGN_LEFT | wxALL, cItemSpacer );
		}

		// Results list box.
		mResultsBox = new tWxColumnListBox( searchResultsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT );
		searchResultsSizer->Add( mResultsBox, 1, wxGROW | wxLEFT | wxRIGHT, cItemSpacer );

		// Create these in the same order as the tColumnTypes enum.
		mResultsBox->fCreateColumn( wxT("File/Type") );
		mResultsBox->fCreateColumn( wxT("Name") );
		mResultsBox->fCreateColumn( wxT("Script") );

		mResultsBox->fAutosizeAllColumns( );

		wxBoxSizer* bottomHorizSizer = new wxBoxSizer( wxHORIZONTAL );

		// Selection options.
		mFindDuplicatesBox = new wxCheckBox( searchResultsPanel, wxID_ANY, wxT("Find Duplicates") );
		mFindDuplicatesBox->SetValue( false );
		bottomHorizSizer->Add( mFindDuplicatesBox, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );

		mSearchHiddensBox = new wxCheckBox( searchResultsPanel, wxID_ANY, wxT("Include Hidden Objects") );
		mSearchHiddensBox->SetValue( false );
		bottomHorizSizer->Add( mSearchHiddensBox, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );

		mSearchSelection = new wxCheckBox( searchResultsPanel, wxID_ANY, wxT("Search Selection Only") );
		mSearchSelection->SetValue( false );
		bottomHorizSizer->Add( mSearchSelection, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );

		mFrameSelectedBox = new wxCheckBox( searchResultsPanel, wxID_ANY, wxT("Frame Selection") );
		mFrameSelectedBox->SetValue( true );
		bottomHorizSizer->Add( mFrameSelectedBox, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );

		// Select item button.
		wxButton* selectButton = new wxButton( searchResultsPanel, wxID_ANY, wxT("Select"), wxDefaultPosition, wxSize( 100, 20 ) );
		bottomHorizSizer->Add( selectButton, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );

		searchResultsSizer->Add( bottomHorizSizer, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL );

		// Connect all events.
		mSearchBox->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( tObjectBrowserDialog::fOnSearchPressed ), NULL, this );
		selectButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSelectPressed), NULL, this );
		mResultsBox->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler(tObjectBrowserDialog::fOnListBoxDClick), NULL, this);

		// All the filter check boxes need to update the search when they're toggled.
		mMshmlBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mSigmlBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mFxmlBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mTerrainBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mLightBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mLightProbeBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mAttachBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mShapeBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mWaypointBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mDecalsBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mNavBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mCameraBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mTileCanvasBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mTilePropsBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );

		mNameBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mFilePathBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mScriptBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mGamePropsBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );

		mFindDuplicatesBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mSearchSelection->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mSearchHiddensBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );

		Layout( );
		Refresh( );
	}

	void tObjectBrowserDialog::fOnSelectPressed( wxCommandEvent& event )
	{
		fSelectItem( );
	}

	void tObjectBrowserDialog::fSelectItem( )
	{
		// Test for if anything is selected.
		u64 selected = mResultsBox->GetFirstSelected( );
		if( selected == -1 )
			return;

		// Retrieval above has primed the selected idx. Loop through selected
		// entries until we run out of items selected.
		tEditorSelectionList newSelection;
		for( ; selected != -1; selected = mResultsBox->GetNextSelected( selected ) )
		{
			tEditableObject* objPtr = reinterpret_cast< tEditableObject* >( mResultsBox->GetItemData( selected ) );
			newSelection.fAdd( tEntityPtr( objPtr ), false );
		}

		fEditorWindow( )->fGuiApp( ).fSelectionList( ).fReset( newSelection );
		if( mFrameSelectedBox->GetValue() )
			fEditorWindow( )->fFrameSelection( true );

		// Directly return focus to the main window.
		fEditorWindow( )->SetFocus( );
	}

	void tObjectBrowserDialog::fOnSearchPressed( wxCommandEvent& event )
	{
		fRefreshList( );
	}

	void tObjectBrowserDialog::fOnListBoxDClick( wxMouseEvent& event )
	{
		fSelectItem( );
	}

	struct tListRowData
	{
		// Don't use this one.
		tListRowData( ) { }

		// Specific ctor for the data format in this file.
		tListRowData( tEditableObject* obj, wxString fileName, wxString refName, wxString scriptName )
			: mObject( obj )
		{
			mText.fPushBack( fileName );
			mText.fPushBack( refName );
			mText.fPushBack( scriptName );
		}

		tEditableObject*			mObject;
		tGrowableArray< wxString >	mText;
	};

	struct tSortRowPackageAlphabeticallyByFirstColumn
	{
		inline b32 operator()( const tListRowData& a, const tListRowData& b ) const
		{
			sigassert( a.mText.fCount( ) > 0 && b.mText.fCount( ) > 0 );
			return _stricmp( a.mText[0].Lower( ).c_str( ), b.mText[0].Lower( ).c_str( ) ) < 0;
		}
	};

	/// 
	/// Returns true if the loop should go to the next iteration.
	template< class T >
	b32 tObjectBrowserDialog::fProcessType( tGrowableArray< tListRowData >& rowPackages, tEditableObject* editObjPtr, b32 skip, b32 fptOn, b32 namOn, b32 scpOn, b32 prpOn, const wxString filePathOrType )
	{
		// Terrain gather/filter.
		const T* c = editObjPtr->fDynamicCast< T >( );
		if( c )
		{
			if( skip )
				return true;

			// Record the data for this row!
			tListRowData rowData( editObjPtr, filePathOrType, c->fGetName( ), c->fGetScriptName( ) );

			// Filter by fields.
			if( !fFilterFields( rowData, fptOn, namOn, scpOn, prpOn ) )
				return false;

			rowPackages.fPushBack( rowData );
			return true;
		}

		return false;
	}

	/// 
	/// \brief Returns true if the entity passes the specified filters.
	b32 fFilterReferenceEntity( const tEditableSgFileRefEntity* c, b32 testMsh, b32 testSig  )
	{
		const tStringPtr resPath( Sigml::fSigbPathToSigml( c->fResourcePath( ) ).fCStr( ) );

		// Returns true if the entity satisfies the filters.
		return (testMsh && StringUtil::fCheckExtension( resPath.fCStr( ), tSceneGraphFile::fGetSigmlFileExtension( 0 ) )) 
			|| (testSig && StringUtil::fCheckExtension( resPath.fCStr( ), tSceneGraphFile::fGetSigmlFileExtension( 1 ) ));
	}

	/// 
	/// \brief Returns true if the entity passes any of the filters.
	b32 tObjectBrowserDialog::fFilterFields( const tListRowData& rowData, b32 testFilePathName, b32 testRefName, b32 testScriptName, b32 testGameProps )
	{
		if( mSearchBox->IsEmpty( ) )
			return true;

		// Be case insensitive.
		wxString searchText = mSearchBox->GetLineText( 0 ).Lower( );

		tGrowableArray< std::string > includes;
		tGrowableArray< std::string > excludes;
		StringUtil::fSplitSearchParams( includes, excludes, mSearchBox->GetLineText( 0 ).c_str() );

		if( testFilePathName )
		{
			const char* fileStr = rowData.mText[cFileOrName].c_str( );
			if( StringUtil::fAndSearch( fileStr, includes ) && StringUtil::fNotSearch( fileStr, excludes ) )
				return true;
		}

		if( testRefName )
		{
			const char* refStr = rowData.mText[cReferenceName].c_str( );
			if( StringUtil::fAndSearch( refStr, includes ) && StringUtil::fNotSearch( refStr, excludes ) )
				return true;
		}

		if( testScriptName )
		{
			const char* scriptStr = rowData.mText[cScript].c_str( );
			if( StringUtil::fAndSearch( scriptStr, includes ) && StringUtil::fNotSearch( scriptStr, excludes ) )
				return true;
		}

		if( testGameProps )
		{
			b32 foundPartial = false;
			tGrowableArray< tEditablePropertyPtr > tags;
			tGrowableArray< tEditablePropertyPtr > props;
			rowData.mObject->fGetEditableProperties( ).fGetGroup( Sigml::tObject::fEditablePropGameTagName( ), tags );
			rowData.mObject->fGetEditableProperties( ).fGetGroup( Sigml::tObject::fEditablePropGameEnumName( ), props );

			// If there's none of either, then we are lacking the proper tag/enum.
			if( tags.fCount( ) == 0 && props.fCount( ) == 0 )
				return false;

			// Check tags.
			for( u32 i = 0; !foundPartial && i < tags.fCount( ); ++i )
			{
				if( tags[i]->fClassId( ) == Rtti::fGetClassId< tEditablePropertyProjectFileTag >( ) )
				{
					log_warning( "Non-tag props have infiltrated the find window." );
					continue;
				}

				u32 data = -1;
				tags[i]->fGetData( data );
				const tProjectFile::tGameTag* tag = tProjectFile::fInstance( ).fFindTagByKey( data );
				if( !tag )
					continue;

				wxString name( tag->mName );
				if( strstr( name.Lower( ).c_str( ), searchText ) )
				{
					foundPartial = true;
					break;
				}
			}

			// Check props.
			for( u32 i = 0; !foundPartial && i < props.fCount( ); ++i )
			{
				if( props[i]->fClassId( ) != Rtti::fGetClassId< tEditablePropertyProjectFileEnum >( ) )
				{
					log_warning( "Non-enum props have infiltrated the find window." );
					continue;
				}

				const tEditablePropertyProjectFileEnum* enumProp = dynamic_cast<const tEditablePropertyProjectFileEnum*>( props[i].fGetRawPtr( ) );
				const tProjectFile::tGameEnumeratedType* enumer = enumProp->fEnumType();

				wxString name( enumer->mName );
				if( strstr( name.Lower( ).c_str( ), searchText ) )
				{
					foundPartial = true;
					break;
				}

				u32 enumIdx = -1;
				props[i]->fGetData( enumIdx );
				if( enumIdx >= enumer->mValues.fCount() )
				{
					log_warning( "A property's enum value is outside its range. Enum: " << enumer->mName.c_str() );
					if( rowData.mText.fCount() == 3 )
						log_warning( ">>> Bad Enum Object text: " << rowData.mText[0] << " " << rowData.mText[1] << " " << rowData.mText[2] );
					continue;
				}

				wxString valName( enumer->mValues[ enumIdx ].mName );
				if( strstr( valName.Lower( ).c_str( ), searchText ) )
				{
					foundPartial = true;
					break;
				}
			}

			if( foundPartial )
				return true;
		}

		// All filter attempts failed.
		return false;
	}

	void tObjectBrowserDialog::fRefreshList( )
	{
		// Clear it out.
		fClear( );
		
		tGrowableArray< tListRowData > rowPackages;

		// Type filters.
		const b32 mshOn = mMshmlBox->GetValue( );
		const b32 sigOn = mSigmlBox->GetValue( );
		const b32 fxOn = mFxmlBox->GetValue( );
		const b32 terOn = mTerrainBox->GetValue( );
		const b32 ligOn = mLightBox->GetValue( );
		const b32 ligPrOn = mLightProbeBox->GetValue( );
		const b32 attOn = mAttachBox->GetValue( );
		const b32 shpOn = mShapeBox->GetValue( );
		const b32 wptOn = mWaypointBox->GetValue( );
		const b32 dclOn = mDecalsBox->GetValue( );
		const b32 navOn = mNavBox->GetValue( );
		const b32 camOn = mCameraBox->GetValue( );
		const b32 tcOn = mTileCanvasBox->GetValue( );
		const b32 tpOn = mTilePropsBox->GetValue( );
		const b32 gOn = mGroupsBox->GetValue( );

		const b32 doTypeFilter = mshOn || sigOn || fxOn || terOn || ligOn || ligPrOn || attOn || shpOn || wptOn || dclOn || navOn || camOn || tcOn || tpOn || gOn;

		// Field filters.
		const b32 fptOn = mFilePathBox->GetValue( );
		const b32 namOn = mNameBox->GetValue( );
		const b32 scpOn = mScriptBox->GetValue( );
		const b32 prpOn = mGamePropsBox->GetValue( );

		const b32 doFieldFilter = fptOn || namOn || scpOn || prpOn;

		tEditableObjectContainer::tEntityMasterList shown;
		if( mSearchSelection->GetValue() )
		{
			// Do a search only through the selected items.
			const tEditorSelectionList& list = fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fGetSelectionList();

			// I don't understand why there isn't a way to just get a basic array of the
			// selected entities from the selection list.
			shown.fReserve( list.fCount() );
			for( u32 i = 0; i < list.fCount(); ++i )
				shown.fPushBack( list[i] );
		}
		else
		{
			// Do standard search of all shown and optionally hidden.
			fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fGetShown( shown );

			if( mSearchHiddensBox->GetValue( ) )
			{
				tEditableObjectContainer::tEntityMasterList hidden;
				fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fGetHidden( hidden );
				shown.fJoin( hidden );
			}
		}

		// Have to do extra work to get all the children of all the canvases.
		if( tpOn )
		{
			tEditableObjectContainer::tEntityMasterList lilBabies;
			for( u32 i = 0; i < shown.fCount(); ++i )
			{
				tEditableTileCanvas* entity = shown[ i ]->fDynamicCast< tEditableTileCanvas >( );
				if( !entity )
					continue;

				lilBabies.fJoin( entity->fGetContents( true ) );
			}

			for( u32 i = 0; i < lilBabies.fCount(); ++i )
				shown.fFindOrAdd( lilBabies[i] );
		}

		for( u32 i = 0; i < shown.fCount( ); ++i )
		{
			tEditableObject* editObjPtr = shown[ i ]->fDynamicCast< tEditableObject >( );
			sigassert( editObjPtr );

			// Don't list anything inside of a group.
			if( editObjPtr->fHasGroupParent() )
				continue;

			// Reference (sigml, mshml) entities gather/filter.
			const tEditableSgFileRefEntity* sre = editObjPtr->fDynamicCast< tEditableSgFileRefEntity >( );
			if( sre )
			{
				// Test that we're searching this object type.
				if( !fFilterReferenceEntity( sre, mshOn, sigOn ) )
					continue;

				// Record the data for this row.
				wxString fileName( Sigml::fSigbPathToSigml( sre->fResourcePath( ) ).fCStr( ) );
				tListRowData rowData( editObjPtr, fileName, sre->fGetName( ), sre->fGetScriptName( ) );

				// Filter fields.
				if( !fFilterFields( rowData, fptOn, namOn, scpOn, prpOn ) )
					continue;

				rowPackages.fPushBack( rowData );
				continue;
			}

			// FX gather/filter.
			const tFxmlReferenceEntity* fre = editObjPtr->fDynamicCast< tFxmlReferenceEntity >( );
			if( fre )
			{
				if( !fxOn )
					continue;

				wxString fileName( Fxml::fFxbPathToFxml( fre->fResourcePath( ) ).fCStr( ) );
				tListRowData rowData( editObjPtr, fileName, fre->fGetName( ), fre->fGetScriptName( ) );

				// Filter fields.
				if( !fFilterFields( rowData, fptOn, namOn, scpOn, prpOn ) )
					continue;

				rowPackages.fPushBack( rowData );
				continue;
			}

			// Terrain gather/filter.
			if( fProcessType< tEditableTerrainGeometry >( rowPackages, editObjPtr, !terOn, fptOn, namOn, scpOn, prpOn, wxT("Terrain") ) )
				continue;

			// Lights gather/filter.
			if( fProcessType< tEditableLightEntity >( rowPackages, editObjPtr, !ligOn, fptOn, namOn, scpOn, prpOn, wxT("Light") ) )
				continue;

			// Light probes gather/filter.
			if( fProcessType< tEditableLightProbeEntity >( rowPackages, editObjPtr, !ligPrOn, fptOn, namOn, scpOn, prpOn, wxT("LightProbe") ) )
				continue;

			// Attachments gather/filter.
			if( fProcessType< tEditableAttachmentEntity >( rowPackages, editObjPtr, !attOn, fptOn, namOn, scpOn, prpOn, wxT("Attachment") ) )
				continue;

			// Shapes gather/filter.
			if( fProcessType< tEditableShapeEntity >( rowPackages, editObjPtr, !shpOn, fptOn, namOn, scpOn, prpOn, wxT("Shape") ) )
				continue;			

			// Waypoints gather/filter.
			if( fProcessType< tEditableWaypointEntity >( rowPackages, editObjPtr, !wptOn, fptOn, namOn, scpOn, prpOn, wxT("Waypoint") ) )
				continue;

			// Decals gather/filter.
			if( fProcessType< tEditablePathDecalWaypoint >( rowPackages, editObjPtr, !dclOn, fptOn, namOn, scpOn, prpOn, wxT("Decal Waypoint") ) )
				continue;

			// Nav nodes gather/filter.
			if( fProcessType< tEditableNavGraphNodeEntity >( rowPackages, editObjPtr, !navOn, fptOn, namOn, scpOn, prpOn, wxT("Nav Graph Node") ) )
				continue;

			// Camera gather/filter.
			if( fProcessType< tEditableCameraEntity >( rowPackages, editObjPtr, !camOn, fptOn, namOn, scpOn, prpOn, wxT("Camera") ) )
				continue;

			// Tile canvas gather/filter.
			if( fProcessType< tEditableTileCanvas >( rowPackages, editObjPtr, !tcOn, fptOn, namOn, scpOn, prpOn, wxT("Tile Canvas") ) )
				continue;

			// Tile prop gather/filter.
			if( fProcessType< tEditableTileEntity >( rowPackages, editObjPtr, !tpOn, fptOn, namOn, scpOn, prpOn, wxT("Tile Entity") ) )
				continue;

			// Tile prop gather/filter.
			if( fProcessType< tEditableGroupEntity >( rowPackages, editObjPtr, !gOn, fptOn, namOn, scpOn, prpOn, wxT("Group") ) )
				continue;
		}

		const b32 searchingForDuplicates = mFindDuplicatesBox->GetValue();

		if( searchingForDuplicates )
		{
			tGrowableArray< tListRowData > copyguy = rowPackages;
			rowPackages.fDeleteArray();

			// Find all duplicates and copy them back to the row packages.
			for( u32 i = 0; i < copyguy.fCount(); ++i )
			{
				tEditableObject* master = copyguy[ i ].mObject->fDynamicCast< tEditableObject >( );

				for( u32 j = i+1; j < copyguy.fCount(); ++j )
				{
					tEditableObject* cmp = copyguy[ j ].mObject->fDynamicCast< tEditableObject >( );
					if( master == cmp )
					{
						log_warning( "Somehow we've ended up with comparing the same entity to itself that's weird");
						continue;
					}

					// Exact position = duplicate as far as we're concerned.
					if( master->fWorldToObject().fGetTranslation() != cmp->fWorldToObject().fGetTranslation() )
						continue;

					const tEditableTileEntity* masterAsProp = master->fDynamicCast< tEditableTileEntity >( );
					const tEditableTileEntity* cmpAsProp = master->fDynamicCast< tEditableTileEntity >( );
					
					// Except in the case where it's two tile props and they are obviously different IDs.
					if( masterAsProp && cmpAsProp && masterAsProp->mIdString == cmpAsProp->mIdString )
						continue;

					rowPackages.fPushBack( copyguy[i] );
					rowPackages.fBack().mText.fFront() << " - p1";
					rowPackages.fPushBack( copyguy[j] );
					rowPackages.fBack().mText.fFront() << " - p2";
				}
			}
		}

		if( !searchingForDuplicates )
		{
			// Sort things first.
			std::sort( rowPackages.fBegin( ), rowPackages.fEnd( ), tSortRowPackageAlphabeticallyByFirstColumn( ) );
		}

		// Add sorted packages to the view box.
		for( u32 i = 0; i < rowPackages.fCount( ); ++i )
		{
			const tListRowData& thisPackage = rowPackages[ i ];

			// Note: the objects may only add a ref here.
			const u64 addedItem = mResultsBox->fCreateRow( thisPackage.mText );
			fRefCounterPtrAddRef( thisPackage.mObject );
			mResultsBox->SetItemPtrData( addedItem, reinterpret_cast< wxUIntPtr >( thisPackage.mObject ) );

			// Color the fields to indicate the pairs.
			if( searchingForDuplicates )
			{
				if( i % 2 )
				{
					mResultsBox->SetItemBackgroundColour( addedItem, wxColor( 230, 230, 255, 255 ) );
				}
				else
				{
					mResultsBox->SetItemBackgroundColour( addedItem, wxColor( 230, 255, 230, 255 ) );
				}
			}
		}

		mResultsBox->SetColumnWidth( 0, wxLIST_AUTOSIZE_USEHEADER );
		mResultsBox->SetColumnWidth( 1, wxLIST_AUTOSIZE_USEHEADER );
		mResultsBox->SetColumnWidth( 2, wxLIST_AUTOSIZE_USEHEADER );
	}

}

