#include "SigEdPch.hpp"
#include "FileSystem.hpp"
#include "tEditorAppWindow.hpp"
#include "tRemapReferencesDialog.hpp"
#include "tWxSlapOnGroup.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"

namespace Sig
{
	tRemapReferencesDialog::tRemapReferencesDialog( tEditorAppWindow* editorWindow )
		: tEditorDialog( editorWindow, "RemapReferencesDialog" )
		, mMainPanel( 0 )
		, mListBox( 0 )
		, mIncludeHiddens( 0 )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetTitle( "Remap Sigml/Siged References" );

		mMainPanel = new wxScrolledWindow( this );
		mMainPanel->SetBackgroundColour( GetBackgroundColour( ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		GetSizer( )->Add( mMainPanel, 1, wxEXPAND | wxALL, 0 );

		fBuildGui( );
	}

	bool tRemapReferencesDialog::Show( bool show )
	{
		const bool o = tWxSlapOnDialog::Show( show );

		if( IsShown( ) )
		{
			fRefreshList( );
		}

		return o;
	}

	void tRemapReferencesDialog::fBuildGui( )
	{
		wxBoxSizer *vbox = new wxBoxSizer( wxVERTICAL );
		mMainPanel->SetSizer( vbox );

		//
		// add Existing Resources static text...
		//

		wxBoxSizer* hbox0 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox0, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 0 );

		wxStaticText* staticText = new wxStaticText( mMainPanel, wxID_ANY, wxT("Existing References"), wxDefaultPosition, wxDefaultSize );
		hbox0->Add( staticText, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxLEFT, 5 );

		//
		// add List Box of resources...
		//

		wxBoxSizer* hbox1 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox1, 1, wxGROW | wxALL, 0 );

		mListBox = new wxListBox( mMainPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString(), wxLB_SINGLE );
		hbox1->Add( mListBox, 1, wxGROW | wxALL, 5 );

		//
		// add Check Boxes...
		//

		wxBoxSizer* hbox3 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox3, 0, wxALIGN_LEFT  | wxALL, 0 );

		mIncludeHiddens = new wxCheckBox( mMainPanel, wxID_ANY, wxT("Include Hidden Objects") );
		mIncludeHiddens->SetValue( false );
		hbox3->Add( mIncludeHiddens, 0, wxALIGN_BOTTOM | wxALL, 5 );

		mIncludeUnselected = new wxCheckBox( mMainPanel, wxID_ANY, wxT("Include Unselected Objects") );
		mIncludeUnselected->SetValue( false );
		hbox3->Add( mIncludeUnselected, 0, wxALIGN_BOTTOM | wxALL, 5 );

		//
		// add Buttons...
		//

		wxBoxSizer* hbox2 = new wxBoxSizer( wxHORIZONTAL );
		vbox->Add( hbox2, 0, wxALIGN_RIGHT | wxALL, 0 );

		wxButton* selectItems = new wxButton( mMainPanel, wxID_ANY, wxT("Select Items"), wxDefaultPosition, wxDefaultSize );
		hbox2->Add( selectItems, 0, wxALIGN_BOTTOM | wxALL, 5 );

		wxButton* refreshList = new wxButton( mMainPanel, wxID_ANY, wxT("Refresh"), wxDefaultPosition, wxDefaultSize );
		hbox2->Add( refreshList, 0, wxALIGN_BOTTOM | wxALL, 5 );


		// connect events

		selectItems->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tRemapReferencesDialog::fOnSelectPressed), NULL, this );
		refreshList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tRemapReferencesDialog::fOnRefreshPressed), NULL, this );
		mListBox->Connect( wxEVT_LEFT_DCLICK, wxMouseEventHandler(tRemapReferencesDialog::fOnListBoxDClick), NULL, this);

		Layout( );
		Refresh( );
	}

	tFilePathPtr tRemapReferencesDialog::fGetSelectedPath( ) const
	{
		wxString listBoxSelectedString = mListBox->GetString( mListBox->GetSelection( ) );
		if( listBoxSelectedString[ 0 ] == '*' )
			listBoxSelectedString = &listBoxSelectedString[ 1 ];
		return Sigml::fSigmlPathToSigb( tFilePathPtr( listBoxSelectedString.c_str( ) ) );
	}

	struct tSortPathAlphabetically
	{
		inline b32 operator()( const wxString& _a, const wxString& _b ) const
		{
			const char* a = _a.c_str( )[0] == '*' ? _a.c_str( ) + 1 : _a.c_str( );
			const char* b = _b.c_str( )[0] == '*' ? _b.c_str( ) + 1 : _b.c_str( );
			return _stricmp( a, b ) < 0;
		}
	};

	void tRemapReferencesDialog::fOnSelectPressed( wxCommandEvent& event )
	{
		if( mListBox->GetSelection( ) < 0 || mListBox->GetSelection( ) >= ( s32 )mListBox->GetCount( ) )
			return;

		tGrowableArray< tEditableSgFileRefEntity* > referenceEntities;
		fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fCollectByType( referenceEntities );

		tEditorSelectionList newSelection;

		const tFilePathPtr curRefPath = fGetSelectedPath( );
		for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
		{
			if( referenceEntities[ i ]->fResourcePath( ) == curRefPath )
				newSelection.fAdd( tEntityPtr( referenceEntities[ i ] ), false );
		}

		fEditorWindow( )->fGuiApp( ).fSelectionList( ).fReset( newSelection );
	}

	void tRemapReferencesDialog::fOnRefreshPressed( wxCommandEvent& event )
	{
		fRefreshList( );
	}

	void tRemapReferencesDialog::fOnListBoxDClick( wxMouseEvent& event )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			this, 
			"Select New Reference",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "" ),
			wxString( "*.mshml;*.sigml" ),
			wxFD_OPEN ) );

		fEditorWindow( )->SetFocus( );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			const tFilePathPtr newResPath = ToolsPaths::fMakeResRelative( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
			const tFilePathPtr newRefPath = Sigml::fSigmlPathToSigb( newResPath );
			const tFilePathPtr oldRefPath = fGetSelectedPath( );
			fReplaceReference( oldRefPath, newRefPath );
			fEditorWindow( )->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	}

	void tRemapReferencesDialog::fRefreshList( )
	{
		mListBox->Clear( );

		tGrowableArray< tEditableSgFileRefEntity* > referenceEntities;
		fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fCollectByType( referenceEntities );

		if( mIncludeHiddens->GetValue( ) )
			fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fCollectHiddenByType( referenceEntities );

		tGrowableArray< wxString > uniquePaths;
		for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
		{
			wxString path;

			if( referenceEntities[ i ]->fIsRedBox( ) )
				path = "*";

			path += Sigml::fSigbPathToSigml( referenceEntities[ i ]->fResourcePath( ) ).fCStr( );

			uniquePaths.fFindOrAdd( path );
		}

		std::sort( uniquePaths.fBegin( ), uniquePaths.fEnd( ), tSortPathAlphabetically( ) );

		for( u32 i = 0; i < uniquePaths.fCount( ); ++i )
			mListBox->Append( uniquePaths[ i ] );
	}

	void tRemapReferencesDialog::fReplaceReference( const tFilePathPtr& oldRef, const tFilePathPtr& newRef )
	{
		log_line( 0, "replacing [" << oldRef << "] with [" << newRef << "]" );
		if( oldRef == newRef )
			return;

		tGrowableArray< tEditableSgFileRefEntity* > referenceEntities;
		if( mIncludeUnselected->GetValue( ) )
			fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fCollectByType( referenceEntities );
		else
			fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fCollectSelectedOrOnly( referenceEntities );

		if( mIncludeHiddens->GetValue( ) )
			fEditorWindow( )->fGuiApp( ).fEditableObjects( ).fCollectHiddenByType( referenceEntities );


		for( u32 i = 0; i < referenceEntities.fCount( ); ++i )
		{
			if( referenceEntities[ i ]->fResourcePath( ) == oldRef )
				if( mIncludeUnselected )
					referenceEntities[ i ]->fResetReference( newRef );
				else if( fEditorWindow( )->fGuiApp( ).fSelectionList( ).fContains( Sig::tEntityPtr( referenceEntities[ i ] ) ) )
					referenceEntities[ i ]->fResetReference( newRef );
		}

		fRefreshList( );
	}

}

