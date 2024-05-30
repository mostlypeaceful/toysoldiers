#include "SigEdPch.hpp"
#include "tObjectBrowserDialog.hpp"
#include "FileSystem.hpp"
#include "tEditorAppWindow.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "Editor/tFxmlReferenceEntity.hpp"
#include "Editor/tEditableTerrainEntity.hpp"
#include "Editor/tEditableLightEntity.hpp"
#include "Editor/tEditableAttachmentEntity.hpp"
#include "Editor/tEditableShapeEntity.hpp"
#include "Editor/tEditableWaypointEntity.hpp"
#include "Editor/tEditablePathDecalWaypointEntity.hpp"
#include "Editor/tEditableNavGraphNodeEntity.hpp"
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
		, mMainPanel( 0 )
		, mSearchBox( 0 )
		, mMshmlBox( 0 )
		, mSigmlBox( 0 )
		, mFxmlBox( 0 )
		, mTerrainBox( 0 )
		, mLightBox( 0 )
		, mAttachBox( 0 )
		, mShapeBox( 0 )
		, mWaypointBox( 0 )
		, mNameBox( 0 )
		, mFilePathBox( 0 )
		, mScriptBox( 0 )
		, mGamePropsBox( 0 )
		, mResultsBox( 0 )
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
		wxBoxSizer* topHalf = new wxBoxSizer( wxHORIZONTAL );
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
		searchSizer->Add( mSearchBox, 0, wxLEFT | wxRIGHT | wxEXPAND, cItemSpacer );

		// Search button.
		//wxButton* searchButton = new wxButton( searchPanel, wxID_ANY, wxT("Search"), wxDefaultPosition, wxSize( 100, 20 ) );
		//searchSizer->Add( searchButton, 0, wxALIGN_RIGHT | wxLEFT | wxRIGHT, cItemSpacer );



		// Panel for filters.
		wxPanel* typeFilterPanel = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
		wxBoxSizer* typeFilterSizer = new wxBoxSizer( wxVERTICAL );
		typeFilterPanel->SetSizer( typeFilterSizer );
		topHalf->Add( typeFilterPanel, 0, wxALL, cOutsideBorderSpacer );
		{
			// Title.
			wxStaticText* staticText = new wxStaticText( typeFilterPanel, wxID_ANY, wxT("Type Filters"), wxDefaultPosition, wxDefaultSize );
			typeFilterSizer->Add( staticText, 0, wxALL, cItemSpacer );
		}

		// Mshml box.
		mMshmlBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Mshml") );
		mMshmlBox->SetValue( true );
		typeFilterSizer->Add( mMshmlBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer ); // Space out the bottom independently.

		// Sigml box.
		mSigmlBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Sigml") );
		mSigmlBox->SetValue( true );
		typeFilterSizer->Add( mSigmlBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );

		// Fxml box.
		mFxmlBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Fx") );
		mFxmlBox->SetValue( true );
		typeFilterSizer->Add( mFxmlBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );

		// Terrain box.
		mTerrainBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Terrain") );
		mTerrainBox->SetValue( true );
		typeFilterSizer->Add( mTerrainBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );

		// Light box.
		mLightBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Lights") );
		mLightBox->SetValue( true );
		typeFilterSizer->Add( mLightBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );

		// Attachment box.
		mAttachBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Attachments") );
		mAttachBox->SetValue( true );
		typeFilterSizer->Add( mAttachBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );

		// Shape box.
		mShapeBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Shapes") );
		mShapeBox->SetValue( true );
		typeFilterSizer->Add( mShapeBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );

		// Waypoint box.
		mWaypointBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Waypoints") );
		mWaypointBox->SetValue( true );
		typeFilterSizer->Add( mWaypointBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );

		// Decal box.
		mDecalsBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Decals") );
		mDecalsBox->SetValue( true );
		typeFilterSizer->Add( mDecalsBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );

		// Nav box.
		mNavBox = new wxCheckBox( typeFilterPanel, wxID_ANY, wxT("Nav Nodes") );
		mNavBox->SetValue( true );
		typeFilterSizer->Add( mNavBox, 0, wxLEFT, 15 );
		typeFilterSizer->AddSpacer( cItemSpacer );



		// Panel for filters.
		wxPanel* fieldFilterPanel = new wxPanel( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE );
		wxBoxSizer* fieldFilterSizer = new wxBoxSizer( wxVERTICAL );
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
		fieldFilterSizer->Add( mFilePathBox, 0, wxLEFT, 15 );
		fieldFilterSizer->AddSpacer( cItemSpacer );

		// Reference name box.
		mNameBox = new wxCheckBox( fieldFilterPanel, wxID_ANY, wxT("Name") );
		mNameBox->SetValue( true );
		fieldFilterSizer->Add( mNameBox, 0, wxLEFT, 15 );
		fieldFilterSizer->AddSpacer( cItemSpacer );

		// Script name box.
		mScriptBox = new wxCheckBox( fieldFilterPanel, wxID_ANY, wxT("Script") );
		mScriptBox->SetValue( true );
		fieldFilterSizer->Add( mScriptBox, 0, wxLEFT, 15 );
		fieldFilterSizer->AddSpacer( cItemSpacer );

		// Game props box.
		mGamePropsBox = new wxCheckBox( fieldFilterPanel, wxID_ANY, wxT("Game Properties") );
		mGamePropsBox->SetValue( true );
		fieldFilterSizer->Add( mGamePropsBox, 0, wxLEFT, 15 );
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

		// Decal box.
		mSearchHiddensBox = new wxCheckBox( searchResultsPanel, wxID_ANY, wxT("Include Hidden Objects") );
		mSearchHiddensBox->SetValue( false );
		bottomHorizSizer->Add( mSearchHiddensBox, 0, wxALIGN_LEFT | wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );

		// Select item button.
		wxButton* selectButton = new wxButton( searchResultsPanel, wxID_ANY, wxT("Select"), wxDefaultPosition, wxSize( 100, 20 ) );
		bottomHorizSizer->Add( selectButton, 0, wxALIGN_RIGHT | wxALL | wxALIGN_CENTER_VERTICAL, cItemSpacer );

		searchResultsSizer->Add( bottomHorizSizer, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL );

		// Connect all events.
		//searchButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mSearchBox->Connect( wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler( tObjectBrowserDialog::fOnSearchPressed ), NULL, this );
		mSearchBox->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		selectButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSelectPressed), NULL, this );
		mResultsBox->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler(tObjectBrowserDialog::fOnListBoxDClick), NULL, this);

		// All the filter check boxes need to update the search when they're toggled.
		mMshmlBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mSigmlBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mFxmlBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mTerrainBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mLightBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mAttachBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mShapeBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mWaypointBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mDecalsBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mNavBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );

		mNameBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mFilePathBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mScriptBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );
		mGamePropsBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );

		mSearchHiddensBox->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(tObjectBrowserDialog::fOnSearchPressed), NULL, this );

		Layout( );
		Refresh( );
	}

	void tObjectBrowserDialog::fOnSelectPressed( wxCommandEvent& event )
	{
		fSelectItem( );

		// Directly return focus to the main window.
		fEditorWindow( )->SetFocus( );
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
	b32 tObjectBrowserDialog::ProcessType( tGrowableArray< tListRowData >& rowPackages, tEditableObject* editObjPtr, b32 skip, b32 fptOn, b32 namOn, b32 scpOn, b32 prpOn, const wxString filePathOrType )
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
		return testMsh && StringUtil::fCheckExtension( resPath.fCStr( ), tSceneGraphFile::fGetSigmlFileExtension( 0 ) ) 
			|| testSig && StringUtil::fCheckExtension( resPath.fCStr( ), tSceneGraphFile::fGetSigmlFileExtension( 1 ) );
	}

	/// 
	/// \brief Returns true if the entity passes any of the filters.
	b32 tObjectBrowserDialog::fFilterFields( const tListRowData& rowData, b32 testFilePathName, b32 testRefName, b32 testScriptName, b32 testGameProps )
	{
		if( mSearchBox->IsEmpty( ) )
			return true;

		// Be case insensitive.
		wxString searchText = mSearchBox->GetLineText( 0 ).Lower( );

		if( testFilePathName )
		{
			if( strstr( rowData.mText[cFileOrName].Lower( ).c_str( ), searchText ) )
				return true;
		}

		if( testRefName )
		{
			if( strstr( rowData.mText[cReferenceName].Lower( ).c_str( ), searchText ) )
				return true;
		}

		if( testScriptName )
		{
			if( strstr( rowData.mText[cScript].Lower( ).c_str( ), searchText ) )
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
				wxString name( tags[i]->fGetName( ) );
				// TODO
				//StringUtil::fStrStrI
				if( strstr( name.Lower( ).c_str( ), searchText ) )
					foundPartial = true;
			}

			// Check props.
			for( u32 i = 0; !foundPartial && i < props.fCount( ); ++i )
			{
				wxString name( props[i]->fGetName( ) );
				if( strstr( name.Lower( ).c_str( ), searchText ) )
					foundPartial = true;
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
		const b32 attOn = mAttachBox->GetValue( );
		const b32 shpOn = mShapeBox->GetValue( );
		const b32 wptOn = mWaypointBox->GetValue( );
		const b32 dclOn = mDecalsBox->GetValue( );
		const b32 navOn = mNavBox->GetValue( );

		const b32 doTypeFilter = mshOn || sigOn || fxOn || terOn || ligOn || attOn || shpOn || wptOn || dclOn || navOn;

		// Field filters.
		const b32 fptOn = mFilePathBox->GetValue( );
		const b32 namOn = mNameBox->GetValue( );
		const b32 scpOn = mScriptBox->GetValue( );
		const b32 prpOn = mGamePropsBox->GetValue( );

		const b32 doFieldFilter = fptOn || namOn || scpOn || prpOn;

		tEditableObjectContainer::tEntityMasterList shown;
		fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fGetShown( shown );

		if( mSearchHiddensBox->GetValue( ) )
		{
			tEditableObjectContainer::tEntityMasterList hidden;
			fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fGetHidden( hidden );
			shown.fJoin( hidden );
		}

		for( u32 i = 0; i < shown.fCount( ); ++i )
		{
			tEditableObject* editObjPtr = shown[ i ]->fDynamicCast< tEditableObject >( );
			sigassert( editObjPtr );

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
			if( ProcessType< tEditableTerrainGeometry >( rowPackages, editObjPtr, !terOn, fptOn, namOn, scpOn, prpOn, wxT("Terrain") ) )
				continue;

			// Lights gather/filter.
			if( ProcessType< tEditableLightEntity >( rowPackages, editObjPtr, !ligOn, fptOn, namOn, scpOn, prpOn, wxT("Light") ) )
				continue;

			// Attachments gather/filter.
			if( ProcessType< tEditableAttachmentEntity >( rowPackages, editObjPtr, !attOn, fptOn, namOn, scpOn, prpOn, wxT("Attachment") ) )
				continue;

			// Shapes gather/filter.
			if( ProcessType< tEditableShapeEntity >( rowPackages, editObjPtr, !shpOn, fptOn, namOn, scpOn, prpOn, wxT("Shape") ) )
				continue;

			// Waypoints gather/filter.
			if( ProcessType< tEditableWaypointEntity >( rowPackages, editObjPtr, !wptOn, fptOn, namOn, scpOn, prpOn, wxT("Waypoint") ) )
				continue;

			// Decals gather/filter.
			if( ProcessType< tEditablePathDecalWaypoint >( rowPackages, editObjPtr, !dclOn, fptOn, namOn, scpOn, prpOn, wxT("Decal Waypoint") ) )
				continue;

			// Nav nodes gather/filter.
			if( ProcessType< tEditableNavGraphNodeEntity >( rowPackages, editObjPtr, !navOn, fptOn, namOn, scpOn, prpOn, wxT("Nav Graph Node") ) )
				continue;
		}

		// Sort things first.
		std::sort( rowPackages.fBegin( ), rowPackages.fEnd( ), tSortRowPackageAlphabeticallyByFirstColumn( ) );

		// Add sorted packages to the view box.
		for( u32 i = 0; i < rowPackages.fCount( ); ++i )
		{
			const tListRowData& thisPackage = rowPackages[ i ];

			// Note: the objects may only add a ref here.
			const u64 addedItem = mResultsBox->fCreateRow( thisPackage.mText );
			fRefCounterPtrAddRef( thisPackage.mObject );
			mResultsBox->SetItemPtrData( addedItem, reinterpret_cast< wxUIntPtr >( thisPackage.mObject ) );
		}

		mResultsBox->SetColumnWidth( 0, wxLIST_AUTOSIZE_USEHEADER );
		mResultsBox->SetColumnWidth( 1, wxLIST_AUTOSIZE_USEHEADER );
		mResultsBox->SetColumnWidth( 2, wxLIST_AUTOSIZE_USEHEADER );
	}

	void tObjectBrowserDialog::fReplaceReference( const tFilePathPtr& oldRef, const tFilePathPtr& newRef )
	{
		log_line( 0, "replacing [" << oldRef << "] with [" << newRef << "]" );
		if( oldRef == newRef )
			return;

		tGrowableArray< tEditableSgFileRefEntity* > referenceEntities;
		fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fCollectByType( referenceEntities );

		for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
		{
			if( referenceEntities[ i ]->fResourcePath( ) == oldRef )
				referenceEntities[ i ]->fResetReference( newRef );
		}

		fRefreshList( );
	}

}

