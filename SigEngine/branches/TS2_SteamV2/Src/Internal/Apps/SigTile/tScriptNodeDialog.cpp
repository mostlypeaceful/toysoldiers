//------------------------------------------------------------------------------
// \file tScriptPaletteDialog.cpp - 15 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tScriptNodeDialog.hpp"
#include "tSigTileMainWindow.hpp"
#include "tDesignMarkupPanel.hpp"
#include "tWxSlapOnColorPicker.hpp"
#include "tWxSlapOnTextBox.hpp"
#include "tEditableObjectContainer.hpp"


namespace Sig
{
	namespace
	{
		static const u32 cBrushMinHeight = 26;
		static const u32 cBrushMaxHeight = 50;
	}

	//------------------------------------------------------------------------------
	// tScriptNodeGui
	//------------------------------------------------------------------------------
	tScriptNodeGui::tScriptNodeGui( wxWindow* parent, tScriptNodesDialog* container, tEditableScriptNodeDef* node, u32 layerIndex )
		: wxPanel( parent )
		, mParentDialog( container )
		, mParentSizer( NULL )
		, mTextName( NULL )
		, mButtonColor( NULL )
		, mButtonDelete( NULL )
		, mNode( node )
	{
		SetMaxSize( wxSize( wxDefaultSize.x, cBrushMaxHeight ) );

		mParentSizer = parent->GetSizer( );

		mParentSizer->Insert( layerIndex, this, 0, wxEXPAND | wxTOP, 4 );
		SetBackgroundColour( wxColour( 0xdd, 0xdd, 0xdd ) );

		wxSizer* overallSizer = new wxBoxSizer( wxVERTICAL );
		SetSizer( overallSizer );

		// Add buttons/text
		wxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
		overallSizer->Add( buttonSizer );

		mButtonColor = new wxButton( this, wxID_ANY, "", wxDefaultPosition, wxSize( 20, 20 ) );
		mButtonColor->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tScriptNodeGui::fOnColorButtonPressed ), NULL, this );

		mTextName = new wxTextCtrl( this, wxID_ANY, fName( ), wxDefaultPosition, wxSize( 230, wxDefaultSize.y ), wxBORDER_NONE | wxTE_PROCESS_ENTER );
		mTextName->SetBackgroundColour( GetBackgroundColour( ) );
		mTextName->Connect( wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler( tScriptNodeGui::fOnEnterPressed ), NULL, this );
		mTextName->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( tScriptNodeGui::fOnTextLostFocus ), NULL, this );

		mButtonDelete = new wxButton( this, wxID_ANY, "X", wxDefaultPosition, wxSize( 20, 20 ) );
		mButtonDelete->SetForegroundColour( wxColour( 0x99, 0x00, 0x00 ) );
		mButtonDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tScriptNodeGui::fOnDeleteButtonPressed ), NULL, this );

		buttonSizer->Add( mButtonColor, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4 );
		buttonSizer->Add( mTextName, 0, wxALIGN_LEFT | wxALIGN_TOP | wxLEFT | wxTOP, 6 );

		buttonSizer->AddStretchSpacer( );
		buttonSizer->Add( mButtonDelete, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4 );

		fUpdateColorButton( );

		// Script path input
		mScriptPathBox =  new tWxSlapOnTextBox( this, "Script Path", 150 );
		mScriptPathBox->fSetValue( fScriptPath( ).fCStr( ) );
		wxButton* browseForTex = new wxButton( this, wxID_ANY, "...", wxDefaultPosition, wxSize( 22, 20 ) );
		browseForTex->SetForegroundColour( wxColour( 0x22, 0x22, 0xff ) );
		browseForTex->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tScriptNodeGui::fOnBrowse ), NULL, this );
		mScriptPathBox->fAddWindowToSizer( browseForTex, true );
		overallSizer->AddSpacer( 4 );

		Layout( );
		parent->Layout( );
	}

	std::string tScriptNodeGui::fName( ) const
	{
		return mNode->fName( );
	}

	Math::tVec4f tScriptNodeGui::fColor( ) const
	{
		return mNode->fColor( );
	}

	tFilePathPtr tScriptNodeGui::fScriptPath( ) const
	{
		return mNode->fScriptPath( );
	}

	void tScriptNodeGui::fSetColor( const Math::tVec4f& rgba )
	{
		mNode->fSetColor( rgba );
	}

	void tScriptNodeGui::fUpdateColorButton( )
	{
		mButtonColor->SetBackgroundColour( wxColour( fRound<u8>( fColor( ).x * 255.f ), fRound<u8>( fColor( ).y * 255.f ), fRound<u8>( fColor( ).z * 255.f ) ) );
	}

	void tScriptNodeGui::fOnDeleteButtonPressed( wxCommandEvent& )
	{
		tStrongPtr<wxMessageDialog> warningDialog( new wxMessageDialog(
			mParentDialog,
			"Deleting a script node type cannot currently be undone. All script nodes placed with this entry will be deleted.",
			"Confirm Delete",
			wxOK | wxCANCEL | wxICON_EXCLAMATION ) );

		if( warningDialog->ShowModal( ) != wxID_OK )
			return;

		mParentDialog->fDeleteNode( this );
	}

	void tScriptNodeGui::fOnColorButtonPressed( wxCommandEvent& )
	{
		tColorPickerData cpd( fColor( ).fXYZ( ) );
		if( tWxSlapOnColorPicker::fPopUpDialog( mButtonColor, cpd ) )
		{
			fSetColor( cpd.fExpandRgba( ) );
			fOnNodeChanged( );
		}
	}

	void tScriptNodeGui::fOnEnterPressed( wxCommandEvent& )
	{
		mNode->fSetName( mTextName->GetValue( ).c_str( ) );
		fOnNodeChanged( );
	}

	void tScriptNodeGui::fOnTextFocus( wxFocusEvent& )
	{
		//mOwner->fSetDialogInputActive( true );
	}

	void tScriptNodeGui::fOnTextLostFocus( wxFocusEvent& )
	{
		//mOwner->fSetDialogInputActive( false );
		mNode->fSetName( mTextName->GetValue( ).c_str( ) );
		fOnNodeChanged( );
	}

	void tScriptNodeGui::fOnBrowse( wxCommandEvent& evt )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			this,
			"Select Script",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxEmptyString,
			"*.nut",
			wxFD_OPEN ) );

		if( openFileDialog->ShowModal( ) == wxID_OK )	
		{
			mScriptPath = ToolsPaths::fMakeResRelative( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ) );
			mScriptPathBox->fSetValue( mScriptPath.fCStr( ) );
			mNode->fSetScriptPath( mScriptPath );
		}
	}

	void tScriptNodeGui::fOnNodeChanged( )
	{
		mParentDialog->fOnNodeChanged( true );
	}

	//------------------------------------------------------------------------------
	// tScriptPalettesDialog
	//------------------------------------------------------------------------------
	tScriptNodesDialog::tScriptNodesDialog( tSigTileMainWindow* parent )
		: tSigTileDialog( parent, "ScriptNodessDialog" )
		, mParent( parent )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetTitle( "Script Nodes" );

		const int width = 325;
		SetMinSize( wxSize( width, -1 ) );
		SetMaxSize( wxSize( width, -1 ) );
		SetSize( wxSize( width, -1 ) );
		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		mMainPanel = new wxScrolledWindow( this, wxID_ANY, wxDefaultPosition, wxDefaultSize );
		mMainPanel->SetSizer( new wxBoxSizer( wxVERTICAL ) );
		mMainPanel->SetBackgroundColour( GetBackgroundColour( ) );
		mMainPanel->SetScrollbars( 0, 20, 1, 50 );

		GetSizer( )->Add( mMainPanel, 1, wxEXPAND | wxALL, 4 );

		fBuildGui( );
	}

	void tScriptNodesDialog::fClear( )
	{
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
			delete mNodes[i];
		mNodes.fDeleteArray( );

		fOnNodeChanged( false );
	}

	void tScriptNodesDialog::fRebuildNodes( )
	{
		tEditableTileDb* tileDb = mParent->fDataBase( );
		for( u32 i = 0; i < tileDb->fNumNodes( ); ++i )
		{
			tEditableScriptNodeDef* node = tileDb->fNodeByIdx( i );

			mNodes.fPushBack( new tScriptNodeGui( mMainPanel, this, node, mNodes.fCount( )+1 ) );
		}

		fOnNodeChanged( false );
		mMainPanel->FitInside( );
	}

	bool tScriptNodesDialog::Show( bool show )
	{
		return tWxSlapOnDialog::Show( show );
	}

	void tScriptNodesDialog::fAddEmptyNode( )
	{
		// Add the brush to the real brush container.
		tEditableScriptNodeDef* node = mMainWindow->fDataBase( )->fAddEmptyScriptNode( mMainWindow->fGuiApp( ).fEditableObjects( ).fGetResourceDepot( ) );

		// +1 for the button that exists at the top of the panel
		mNodes.fPushBack( new tScriptNodeGui( mMainPanel, this, node, mNodes.fCount( )+1 ) );
		mNodes.fBack( )->Layout( );

		fOnNodeChanged( true );

		mMainPanel->FitInside( );

		mMainPanel->Layout( );
		mMainPanel->Refresh( );
		Layout( );
		Refresh( );
	}

	void tScriptNodesDialog::fDeleteNode( tScriptNodeGui* node )
	{
		const u32 nodeGuidToDelete = node->fNode( )->fGuid ( );
		mMainWindow->fDataBase( )->fDeleteNode( node->fNode( ) );

		mNodes.fFindAndEraseOrdered( node );
		node->Destroy( );

		mMainWindow->fMarkupPanel( )->fRefreshNodes( );
		mMainWindow->fCanvas( )->fDeleteScriptNodesWithGuid( nodeGuidToDelete );

		mMainPanel->FitInside( );

		mMainPanel->Layout( );
		mMainPanel->Refresh( );
		Layout( );
		Refresh( );
	}

	void tScriptNodesDialog::fOnNodeChanged( b32 markDirty )
	{
		mMainWindow->fMarkupPanel( )->fRefreshNodes( );
		mMainWindow->fCanvas( )->fRefreshTiles( );

		if( markDirty )
			mMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
	}

	void tScriptNodesDialog::fBuildGui( )
	{
		wxSizer* vSizer = mMainPanel->GetSizer( );

		wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
		vSizer->Add( buttonSizer, 0, wxALIGN_RIGHT, 2 );

		wxStaticText* newBrushText = new wxStaticText( mMainPanel, wxID_ANY, "New Node" );
		wxButton* newBrush = new wxButton( mMainPanel, wxID_ANY, "+", wxDefaultPosition, wxSize( 20, 20 ) );
		newBrush->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tScriptNodesDialog::fOnNewNodePressed ), NULL, this );

		buttonSizer->Add( newBrushText, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2 );
		buttonSizer->AddSpacer( 2 );
		buttonSizer->Add( newBrush, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2 );
		buttonSizer->AddSpacer( 4 );

		Layout( );
		Refresh( );
	}

	void tScriptNodesDialog::fOnNewNodePressed( wxCommandEvent& event )
	{
		fAddEmptyNode( );
		mMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( );
	}
}
