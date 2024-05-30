#include "SigTilePch.hpp"
#include "tTileSetPigmentsDialog.hpp"
#include "tSigTileMainWindow.hpp"
#include "tEditorAction.hpp"
#include "tWxSlapOnColorPicker.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tEditableTileDb.hpp"
#include "tTileDbPanel.hpp"
#include "tTilePaintPanel.hpp"

namespace Sig
{
	namespace
	{
		static const u32 cBrushMinHeight = 26;
		static const u32 cBrushMaxHeight = 50;
	}

	//------------------------------------------------------------------------------
	// tNotifySpinner
	//------------------------------------------------------------------------------
	class tNotifySpinner : public tWxSlapOnSpinner
	{
		tPigmentGui* mParent;

	public:
		tNotifySpinner( tPigmentGui* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision, u32 widthOverride )
			: tWxSlapOnSpinner( parent, label, min, max, increment, precision, widthOverride )
			, mParent( parent )
		{ }

	protected:
		virtual void fOnControlUpdated( )
		{
			tWxSlapOnSpinner::fOnControlUpdated( );

			mParent->fOnSpinnerChanged( );
		}
	};

	//------------------------------------------------------------------------------
	// tBrushGui
	//------------------------------------------------------------------------------
	tPigmentGui::tPigmentGui(  wxWindow* parent, tTileSetPigmentsDialog* container, tEditableTileSetPigment* palette, u32 layerIndex )
		: wxPanel( parent )
		, mParentDialog( container )
		, mParentSizer( NULL )
		, mTextName( NULL )
		, mButtonColor( NULL )
		, mButtonAddTileSet( NULL )
		, mButtonRemoveTileSet( NULL )
		, mButtonDelete( NULL )
		, mHeight( NULL )
		, mSize( NULL )
		, mPalette( palette )
	{
		//SetMinSize( wxSize( wxDefaultSize.x, cBrushMinHeight ) );
		SetMaxSize( wxSize( wxDefaultSize.x, cBrushMaxHeight ) );

		//Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tObjectLayerGui::fOnMouseRightButtonUp ) );
		//Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tObjectLayerGui::fOnAction ) );

		mParentSizer = parent->GetSizer( );

		mParentSizer->Insert( layerIndex, this, 0, wxEXPAND | wxTOP, 4 );
		SetBackgroundColour( wxColour( 0xdd, 0xdd, 0xdd ) );

		wxSizer* overallSizer = new wxBoxSizer( wxVERTICAL );
		SetSizer( overallSizer );

		// Add buttons/text
		wxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
		overallSizer->Add( buttonSizer );

		mButtonColor = new wxButton( this, wxID_ANY, "", wxDefaultPosition, wxSize( 20, 20 ) );
		mButtonColor->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tPigmentGui::fOnColorButtonPressed ), NULL, this );

		mTextName = new wxTextCtrl( this, wxID_ANY, fName( ), wxDefaultPosition, wxSize( 180, wxDefaultSize.y ), wxBORDER_NONE | wxTE_PROCESS_ENTER );
		mTextName->SetBackgroundColour( GetBackgroundColour( ) );
		mTextName->Connect( wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler( tPigmentGui::fOnEnterPressed ), NULL, this );
		//mTextName->Connect( wxEVT_SET_FOCUS, wxFocusEventHandler( tBrushGui::fOnTextFocus ), NULL, this );
		mTextName->Connect( wxEVT_KILL_FOCUS, wxFocusEventHandler( tPigmentGui::fOnTextLostFocus ), NULL, this );
		//mTextName->Connect( wxEVT_RIGHT_UP, wxMouseEventHandler( tBrushGui::fOnMouseRightButtonUp ), NULL, this  );

		mButtonAddTileSet = new wxButton( this, wxID_ANY, "+", wxDefaultPosition, wxSize( 20, 20 ) );
		mButtonAddTileSet->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tPigmentGui::fOnAddTileSetPressed ), NULL, this );

		mButtonRemoveTileSet = new wxButton( this, wxID_ANY, "-", wxDefaultPosition, wxSize( 20, 20 ) );
		mButtonRemoveTileSet->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tPigmentGui::fOnRemoveTileSetPressed ), NULL, this );

		mButtonDelete = new wxButton( this, wxID_ANY, "X", wxDefaultPosition, wxSize( 20, 20 ) );
		mButtonDelete->SetForegroundColour( wxColour( 0x99, 0x00, 0x00 ) );
		mButtonDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tPigmentGui::fOnDeleteButtonPressed ), NULL, this );

		buttonSizer->Add( mButtonColor, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT, 4 );
		buttonSizer->Add( mTextName, 0, wxALIGN_LEFT | wxALIGN_TOP | wxLEFT | wxTOP, 6 );

		buttonSizer->AddStretchSpacer( );
		buttonSizer->Add( mButtonAddTileSet, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4 );
		buttonSizer->Add( mButtonRemoveTileSet, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4 );
		buttonSizer->Add( mButtonDelete, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxRIGHT, 4 );

		fUpdateColorButton( );

		// Add spinners for tile size and height.
		const f32 max = 10000.f; // Are there even numbers above ten thousand?
		mHeight = new tNotifySpinner( this, "Height", -10000.f, max, 1.f, 2, 150 );
		mHeight->fSetValueNoEvent( fHeight( ) );
		mSize = new tNotifySpinner( this, "Tile Size", 0.1f, max, 1.f, 2, 150 );
		mSize->fSetValueNoEvent( fSize( ) );
		overallSizer->AddSpacer( 4 );

		// Add list box.
		wxArrayString empty;
		mListedTileSets = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, empty, wxLB_NEEDED_SB );
		mListedTileSets->SetMinSize( wxSize( -1, 50 ) );
		overallSizer->Add( mListedTileSets, 0, wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT, 4 );

		fRefreshListedTileSets( );

		Layout( );
		parent->Layout( );
	}

	std::string tPigmentGui::fName( ) const
	{
		return mPalette->fName( );
	}

	Math::tVec4f tPigmentGui::fColor( ) const
	{
		return mPalette->fColor( );
	}

	f32 tPigmentGui::fHeight( ) const
	{
		return mPalette->fTileHeight( ) * fSize( );
	}

	f32 tPigmentGui::fSize( ) const
	{
		return mPalette->fTileSize( );
	}

	void tPigmentGui::fSetColor( const Math::tVec4f& rgba )
	{
		mPalette->fSetColor( rgba );
	}

	void tPigmentGui::fRefreshListedTileSets( )
	{
		wxArrayString strings;
		const tEditableTileDb* database = mParentDialog->fParent( )->fDataBase( );
		for( u32 i = 0; i < mPalette->fNumTileSetGuids( ); ++i )
		{
			const tEditableTileSet* set = database->fTileSetByGuid( mPalette->fTileSetGuid( i ) );
			if( !set )
			{
				mPalette->fDeleteTileSetGuid( i-- );
				continue;
			}

			strings.push_back( set->fName( ) );
		}

		mListedTileSets->Set( strings );
	}

	void tPigmentGui::fOnSpinnerChanged( )
	{
		const f32 tileSize = mSize->fGetValue( );
		mPalette->fSetSize( tileSize );
		mPalette->fSetHeight( mHeight->fGetValue( ) / tileSize );

		fOnPigmentChanged( );
	}

	void tPigmentGui::fUpdateColorButton( )
	{
		mButtonColor->SetBackgroundColour( wxColour( fRound<u8>( fColor( ).x * 255.f ), fRound<u8>( fColor( ).y * 255.f ), fRound<u8>( fColor( ).z * 255.f ) ) );
	}

	void tPigmentGui::fOnDeleteButtonPressed( wxCommandEvent& )
	{
		tStrongPtr<wxMessageDialog> warningDialog( new wxMessageDialog(
			mParentDialog,
			"Deleting a brush cannot currently be undone. All tiles placed with this brush will be deleted and lost.",
			"Confirm Delete",
			wxOK | wxCANCEL | wxICON_EXCLAMATION ) );

		if( warningDialog->ShowModal( ) != wxID_OK )
			return;

		mParentDialog->fDeletePigment( this );
	}

	void tPigmentGui::fOnColorButtonPressed( wxCommandEvent& )
	{
		tColorPickerData cpd( fColor( ).fXYZ( ) );
		if( tWxSlapOnColorPicker::fPopUpDialog( mButtonColor, cpd ) )
		{
			fSetColor( cpd.fExpandRgba( ) );
			fOnPigmentChanged( );
		}
	}

	void tPigmentGui::fOnAddTileSetPressed( wxCommandEvent& )
	{
		const tEditableTileDb* tileDb = mParentDialog->fParent( )->fDataBase( );
		wxArrayString choices;
		tGrowableArray<u32> tileSets;

		for( u32 i = 0; i < tileDb->fNumTileSets( ); ++i )
		{
			const tEditableTileSet* thisSet = tileDb->fTileSet( i );
			choices.push_back( thisSet->fName( ) );
			tileSets.fPushBack( thisSet->fGuid( ) );
		}

		if( tileSets.fCount( ) == 0 )
		{
			tStrongPtr<wxMessageDialog> warningDialog( new wxMessageDialog(
				mParentDialog,
				"No tile sets were found in the database.",
				"No Tile Sets Found" ) );

			warningDialog->ShowModal( );
			return;
		}

		tStrongPtr<wxMultiChoiceDialog> newTileSetDialog( new wxMultiChoiceDialog( 
			mParentDialog,
			"Pick tile set to add.",
			"Pick Tile Set",
			choices ) );

		if( newTileSetDialog->ShowModal( ) == wxID_OK )
		{
			const wxArrayInt selections = newTileSetDialog->GetSelections( );
			for( u32 i = 0; i < selections.size( ); ++i )
				mPalette->fAddTileSetGuid( tileSets[ selections[i] ] );

			fRefreshListedTileSets( );
		}

		fOnPigmentChanged( );
	}

	void tPigmentGui::fOnRemoveTileSetPressed( wxCommandEvent& )
	{
		const s32 selectedTileSet = mListedTileSets->GetSelection( );
		if( selectedTileSet == -1 )
			return;

		mPalette->fDeleteTileSetGuid( selectedTileSet );
		fRefreshListedTileSets( );

		fOnPigmentChanged( );
	}

	void tPigmentGui::fOnEnterPressed( wxCommandEvent& )
	{
		mPalette->fSetName( mTextName->GetValue( ).c_str( ) );
		fOnPigmentChanged( );
	}

	void tPigmentGui::fOnTextFocus( wxFocusEvent& )
	{
		//mOwner->fSetDialogInputActive( true );
	}

	void tPigmentGui::fOnTextLostFocus( wxFocusEvent& )
	{
		//mOwner->fSetDialogInputActive( false );
		mPalette->fSetName( mTextName->GetValue( ).c_str( ) );
		fOnPigmentChanged( );
	}

	void tPigmentGui::fOnPigmentChanged( )
	{
		mParentDialog->fOnPigmentChanged( true );
	}

	//------------------------------------------------------------------------------
	// tTileSetBrushesDialog
	//------------------------------------------------------------------------------
	tTileSetPigmentsDialog::tTileSetPigmentsDialog( tSigTileMainWindow* parent )
		: tSigTileDialog( parent, "TileSetBrushesDialog" )
		, mParent( parent )
	{
		SetIcon( wxIcon( "appicon" ) );
		SetTitle( "Tile Set Palettes" );

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

	void tTileSetPigmentsDialog::fClear( )
	{
		for( u32 i = 0; i < mPalettes.fCount( ); ++i )
			delete mPalettes[i];
		mPalettes.fDeleteArray( );

		fOnPigmentChanged( false );
	}

	void tTileSetPigmentsDialog::fRebuildPigments( )
	{
		tEditableTileDb* tileDb = mParent->fDataBase( );
		for( u32 i = 0; i < tileDb->fNumPigments( ); ++i )
		{
			tEditableTileSetPigment* brush = tileDb->fPigmentByIdx( i );

			mPalettes.fPushBack( new tPigmentGui( mMainPanel, this, brush, mPalettes.fCount( )+1 ) );
		}

		fOnPigmentChanged( false );
		mMainPanel->FitInside( );
	}

	bool tTileSetPigmentsDialog::Show( bool show )
	{
		const bool o = tWxSlapOnDialog::Show( show );

		return o;
	}

	void tTileSetPigmentsDialog::fAddEmptyPigment( )
	{
		// Add the pigment to the real pigment container.
		tEditableTileSetPigment* pigment = mMainWindow->fDataBase( )->fAddEmptyPigment( );

		// +1 for the button that exists at the top of the panel
		mPalettes.fPushBack( new tPigmentGui( mMainPanel, this, pigment, mPalettes.fCount( )+1 ) );
		mPalettes.fBack( )->Layout( );

		fOnPigmentChanged( true );

		mMainPanel->FitInside( );

		mMainPanel->Layout( );
		mMainPanel->Refresh( );
		Layout( );
		Refresh( );
	}

	void tTileSetPigmentsDialog::fDeletePigment( tPigmentGui* pigment )
	{
		const u32 pigmentToDelete = pigment->fPalette( )->fGuid( );
		mMainWindow->fDataBase( )->fDeletePigment( pigment->fPalette( ) );

		mPalettes.fFindAndEraseOrdered( pigment );
		pigment->Destroy( );

		mMainWindow->fTilePaintPanel( )->fRefreshBrushes( );
		mMainWindow->fCanvas( )->fDeleteTilesWithGuid( pigmentToDelete );

		mMainPanel->FitInside( );

		mMainPanel->Layout( );
		mMainPanel->Refresh( );
		Layout( );
		Refresh( );
	}

	void tTileSetPigmentsDialog::fOnPigmentChanged( b32 markDirty )
	{
		mMainWindow->fTilePaintPanel( )->fRefreshBrushes( );
		mMainWindow->fCanvas( )->fRefreshTiles( );

		if( markDirty )
			mMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
	}

	void tTileSetPigmentsDialog::fBuildGui( )
	{
		// Add "New Palette" button
		wxSizer* vSizer = mMainPanel->GetSizer( );

		wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
		vSizer->Add( buttonSizer, 0, wxALIGN_RIGHT, 2 );

		wxStaticText* newBrushText = new wxStaticText( mMainPanel, wxID_ANY, "New Palette Entry" );
		wxButton* newBrush = new wxButton( mMainPanel, wxID_ANY, "+", wxDefaultPosition, wxSize( 20, 20 ) );
		newBrush->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTileSetPigmentsDialog::fOnNewPigmentPressed ), NULL, this );

		buttonSizer->Add( newBrushText, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2 );
		buttonSizer->AddSpacer( 2 );
		buttonSizer->Add( newBrush, 0, wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL, 2 );
		buttonSizer->AddSpacer( 4 );

		Layout( );
		Refresh( );
	}

	void tTileSetPigmentsDialog::fOnNewPigmentPressed( wxCommandEvent& event )
	{
		fAddEmptyPigment( );
		mMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( );
	}
}
