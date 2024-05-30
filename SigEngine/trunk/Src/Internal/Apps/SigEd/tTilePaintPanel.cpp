//------------------------------------------------------------------------------
// \file tTilePaintPanel.cpp - 07 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigEdPch.hpp"
#include "tTilePaintPanel.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "tTilePaintBrushButton.hpp"
#include "tEditorAppWindow.hpp"
#include "tEditableObjectContainer.hpp"
#include "tWxNotificationMessage.hpp"
#include "tTileDbTree.hpp"

namespace Sig
{
	///
	/// \class tSigTileToggleViewMode
	/// \brief Previews tiles.
	class tSigTileToggleViewMode : public tEditorHotKey
	{
		tTilePaintPanel* mOwner;
	public:
		tSigTileToggleViewMode( tTilePaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp().fHotKeys(), Input::tKeyboard::cButtonF5, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const 
		{
			mOwner->fPreviewTiles( ); 
		}
	};

	///
	/// \class tSigTileRotateHotKey
	/// \brief Hot key that rotates a tile.
	class tSigTileRotateHotKey : public tEditorHotKey
	{
		tTilePaintPanel* mOwner;
		b32 mCcw;
	public:
		tSigTileRotateHotKey( tTilePaintPanel* owner, u32 button, b32 ccw ) 
			: tEditorHotKey( owner->fGuiApp().fHotKeys(), button, 0 ), mOwner( owner ) 
			, mCcw( ccw )
		{ }
		virtual void fFire( ) const 
		{
			mOwner->fRotateSelectedTile( mCcw );
		}
	};

	///
	/// \class tTilePaintButtonGroup
	/// \brief A group for holding the tile primitives/erase/autopaint buttons.
	class tTilePaintButtonGroup : public tEditorCursorControllerButtonGroup
	{
		// This must match the order they're added to the group.
		enum tButtonOrder
		{
			cAutoFloorButton,
			cFloorButton,
			cCornerButton,
			cNicheButton,
			cWallButton,
			cUniquesButton,
			cEraseButton,
			
			cNumButtons,
		};

		tTilePaintPanel* mPaintPanel;

	public:
		tTilePaintButtonGroup( tTilePaintPanel* parent, const char* label )
			: tEditorCursorControllerButtonGroup( parent, label, false, 6 )
			, mPaintPanel( parent )
		{
		}

		virtual void fOnSelChanged( )
		{
			const s32 selectedButton = fGetSelected( );
			if( selectedButton == -1 )
				return;

			// Ensure that the tile databases are loaded.
			if( !mPaintPanel->fAnyLoaded() )
				mPaintPanel->fLoadDefaultResources( true );

			// Fill the individuals display based on what's available in the 
			// databases for this type.
			tTileTypes type = fGetSelectedButtonType( );
			if( type != cNumTileTypes )
			{
				mPaintPanel->fFillIndividualsDisplay( type );
				
				if( !mPaintPanel->fIndividualsDisplayHasTiles( ) )
				{
					tStrongPtr<wxMessageDialog> warningDialog( new wxMessageDialog(
						fGetMainPanel( ),
						"The selected palette has no tiles in that category.",
						"No Tiles In Category" ) );

					warningDialog->ShowModal( );
					fClearSelection( );
					return;
				}
			}
			else
				mPaintPanel->fClearIndividualsDisplay( );

			// Must occur last so the individuals display is refreshed so the
			// newly created button doesn't try to index anything crazy.
			tEditorCursorControllerButtonGroup::fOnSelChanged( );
		}

		tTileTypes fGetSelectedButtonType( )
		{
			s32 selectedButton = fGetSelected( );
			if( selectedButton == -1 )
				return cNumTileTypes;

			const tTilePaintBrushButton* tilePainter = dynamic_cast< tTilePaintBrushButton* >( mButtons[ selectedButton ] );
			if( !tilePainter )
				return cNumTileTypes;

			return tilePainter->fTileType( );
		}
	};

	//------------------------------------------------------------------------------
	// tTilePaintPanel
	//------------------------------------------------------------------------------
	tTilePaintPanel::tTilePaintPanel( tEditorAppWindow* mainWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Tile Painting", "Tile Painting", "TilePainting" )
		, mMainWindow( mainWindow )
	{
		mButtonGroup = new tTilePaintButtonGroup( this, "Tile Painting" );

		// These must be added in the order that matches the tTilePaintButtonGroup enum.
		mAutoFloorButton = new tAutoFloorBrushButton( mButtonGroup, this );
		new tTilePaintBrushButton( mButtonGroup, this, cFloor, "TileFloorSel", "TileFloorDeSel", "Paint floor" );
		new tTilePaintBrushButton( mButtonGroup, this, cCorner, "TileCornerSel", "TileCornerDeSel", "Paint corner" );
		new tTilePaintBrushButton( mButtonGroup, this, cNiche, "TileNicheSel", "TileNicheDeSel", "Paint niche" );
		new tTilePaintBrushButton( mButtonGroup, this, cWall, "TileWallSel", "TileWallDeSel", "Paint wall" );
		new tTilePaintBrushButton( mButtonGroup, this, cUniques, "TileUniqueSel", "TileUniqueDeSel", "Paint a unique" );
		new tTilePaintBrushButton( mButtonGroup, this, cWallDoor, "TileDoorSel", "TileDoorDeSel", "Paint doors" );
		new tTilePaintBrushButton( mButtonGroup, this, cWallExit, "TileExitSel", "TileExitDeSel", "Paint exits" );
		new tTilePaintBrushButton( mButtonGroup, this, cFloorDoor, "TileFloorDoorSel", "TileFloorDoorDeSel", "Paint floor doors" );
		new tTilePaintBrushButton( mButtonGroup, this, cPropCeiling, "TileCPropSel", "TileCPropDeSel", "Paint a ceiling prop" );
		new tTilePaintBrushButton( mButtonGroup, this, cPropFloor, "TileFPropSel", "TileFPropDeSel", "Paint a floor prop" );
		new tTileEraseBrushButton( mButtonGroup, this );

		mSelectedTileSet = new tWxTileSetComboBox( mButtonGroup->fGetMainPanel(), "Tileset", this, NULL, wxCB_READONLY );

		mButtonGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 3 );

		mIndividualsDisplay = new wxListBox( fGetMainPanel( ), wxID_ANY, wxDefaultPosition, wxSize( -1, 200 ), wxArrayString( ), wxLB_SINGLE | wxLB_NEEDED_SB | wxLB_HSCROLL );
		fGetMainPanel( )->GetSizer( )->Add( mIndividualsDisplay, 0, wxALL | wxEXPAND, 3 );
		fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		wxSizer* horiz = new wxBoxSizer( wxHORIZONTAL );
		wxButton* clockWise = new wxButton( fGetMainPanel( ), wxID_ANY, "Rotate CW (D)" );
		wxButton* counterClockWise = new wxButton( fGetMainPanel( ), wxID_ANY, "Rotate CCW (S)" );

		counterClockWise->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTilePaintPanel::fOnRotateCCW ), NULL, this );
		clockWise->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTilePaintPanel::fOnRotateCW ), NULL, this );
		mIndividualsDisplay->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( tTilePaintPanel::fOnListBoxSelect ), NULL, this );

		horiz->Add( clockWise, 0, wxBOTTOM, 3 );
		horiz->Add( counterClockWise, 0, wxLEFT, 3 );

		fGetMainPanel( )->GetSizer( )->Add( horiz, 0, wxCENTER );

		wxSizer* doubleHoriz = new wxBoxSizer( wxHORIZONTAL );
		wxButton* previewToggle = new wxButton( fGetMainPanel( ), wxID_ANY, "Preview Toggle (F5)" );
		wxButton* hideProps = new wxButton( fGetMainPanel( ), wxID_ANY, "Hide Props (F6)" );

		previewToggle->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTilePaintPanel::fOnPreview ), NULL, this );
		hideProps->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTilePaintPanel::fOnHideProps ), NULL, this );

		doubleHoriz->Add( previewToggle, 0, wxBOTTOM, 8 );
		doubleHoriz->Add( hideProps, 0, wxLEFT, 3 );

		fGetMainPanel( )->GetSizer( )->Add( doubleHoriz, 0, wxCENTER );

		mTiledmlPanel = new tTiledmlPanel( this, fGetMainPanel() );
	}

	void tTilePaintPanel::fClearSelection( )
	{
		mButtonGroup->fClearSelection( );
	}

	void tTilePaintPanel::fLoadDefaultResources( b32 loadModels )
	{
		const tResourceDepotPtr& resourceDepot = fGuiApp().fEditableObjects().fGetResourceDepot();

		if( loadModels )
			mDatabase.fLoadTileAssets( resourceDepot );

		tGrowableArray< Tiledml::tFile > loadedFiles;
		if( Tiledml::tFile::fLoadTiledmls( loadedFiles ) )
		{
			for( u32 i = 0; i < loadedFiles.fCount( ); ++i )
			{
				if( !mDatabase.fDeserializeTileSet( loadedFiles[i], true ) )
				{
					// Failure loading all tile sets. Let em know.
					std::stringstream errorMessage( "The tile set failed to load: " );
					errorMessage << loadedFiles[i].mResDir.fCStr( );
					tWxNotificationMessage notify( mMainWindow, std::string("Tile Set Load Failed"), errorMessage.str() );
					notify.Show( );
				}
			}
		}

		if( loadModels )
		{
			mDatabase.fLoadAllTileSets( resourceDepot );

			mTiledmlPanel->fOnSetUpRendering( );

			// Put all those tile sets as options for the drop down.
			fRefreshSelectionDrop();
		}

		mSelectedTileSet->fSetSelection( 0 );
	}

	tEditableTiledml* tTilePaintPanel::fDatabase( )
	{
		return &mDatabase;
	}

	void tTilePaintPanel::fAddCursorHotKeys( tTileBrushBase* cursor, b32 previewOnly )
	{
		if( !cursor )
			return;

		if( !previewOnly )
		{
			cursor->fAddHotKey( tEditorHotKeyPtr( new tSigTileRotateHotKey( this, Input::tKeyboard::cButtonS, true ) ) );
			cursor->fAddHotKey( tEditorHotKeyPtr( new tSigTileRotateHotKey( this, Input::tKeyboard::cButtonD, false ) ) );
		}
		cursor->fAddHotKey( tEditorHotKeyPtr( new tSigTileToggleViewMode( this ) ) );
	}

	void tTilePaintPanel::fRefreshSelectionDrop( )
	{
		mSelectedTileSet->fAddString( "All" );
		for( u32 i = 0; i < mDatabase.fNumTileSets(); ++i )
		{
			if( mDatabase.fTileSet(i)->fLoaded() )
				mSelectedTileSet->fAddString( mDatabase.fTileSet(i)->fFileName() );
		}
		mSelectedTileSet->fSetSelection( 0 );
	}

	void tTilePaintPanel::fRefreshIndividualsDisplay( )
	{
		const tTileTypes selectedType = mButtonGroup->fGetSelectedButtonType( );
		if( selectedType == cNumTileTypes )
			return;

		fFillIndividualsDisplay( selectedType );
	}

	void tTilePaintPanel::fFillIndividualsDisplay( tTileTypes type )
	{
		wxArrayString newStrings;

		// Add basic random type.
		newStrings.push_back( "NON SPECIFIC" );

		// Add specific tile options.
		mTileKeys.fSetCount( 1 );
		if( mSelectedTileSet->fGetSelection() != 0 )
		{
			const std::string filename = mSelectedTileSet->fGetString();
			tEditableTileSet* tileSet = NULL;
			u32 tileSetIdx = 0;
			for( ; tileSetIdx < mDatabase.fNumTileSets(); ++tileSetIdx )
			{
				tEditableTileSet* thisSet = mDatabase.fTileSet( tileSetIdx );
				if( thisSet->fFileName() == filename )
				{
					tileSet = thisSet;
					break;
				}
			}

			if( tileSet != NULL )
			{
				// Get just one tile set's pieces.
				tEditableTileTypeList* typeList = tileSet->fTileTypeList( type );
				for( u32 i = 0; i < typeList->fNumTileDefs(); ++i )
				{
					newStrings.push_back( typeList->fTileDef(i)->mShortName );
					mTileKeys.fPushBack( tPair<u32,u32>(tileSetIdx, i) );
				}

			}
		}
		else
		{
			// Get em all
			for( u32 tileSetIdx = 0; tileSetIdx < mDatabase.fNumTileSets(); ++tileSetIdx )
			{
				tEditableTileSet* tileSet = mDatabase.fTileSet( tileSetIdx );
				tEditableTileTypeList* typeList = tileSet->fTileTypeList( type );
				for( u32 tileDefIdx = 0; tileDefIdx < typeList->fNumTileDefs(); ++tileDefIdx )
				{
					newStrings.push_back( typeList->fTileDef( tileDefIdx )->mShortName );
					mTileKeys.fPushBack( tPair<u32,u32>(tileSetIdx, tileDefIdx) );
				}
			}
		}

		mIndividualsDisplay->Set( newStrings );

		if( newStrings.Count( ) > 0 )
			mIndividualsDisplay->SetSelection( 0 );
	}

	void tTilePaintPanel::fClearIndividualsDisplay( )
	{
		mIndividualsDisplay->Set( wxArrayString( ) );
	}

	b32 tTilePaintPanel::fIndividualsDisplayHasTiles( ) const
	{
		return mIndividualsDisplay->GetCount( );
	}

	void tTilePaintPanel::fRotateSelectedTile( b32 ccw )
	{
		tTileBrushBase* cursor = dynamic_cast<tTileBrushBase*>( fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );

		if( !cursor )
			return;

		cursor->fRotateTile( ccw );
	}

	void tTilePaintPanel::fPreviewTiles( )
	{
		tGrowableArray< tEditableTileCanvas* > canvasObjs;
		fGuiApp( ).fEditableObjects( ).fCollectByType< tEditableTileCanvas >( canvasObjs );

		b32 flipModels = true;
		if( canvasObjs.fCount() > 0 )
			flipModels = canvasObjs[0]->fGetViewingModels();

		for( u32 i = 0; i < canvasObjs.fCount(); ++i )
		{
			if( flipModels )
				canvasObjs[i]->fSetViewModel( false );
			else
				canvasObjs[i]->fPreviewRandomized( &mDatabase );
		}
	}

	void tTilePaintPanel::fHideProps( )
	{
		tGrowableArray< tEditableTileCanvas* > canvasObjs;
		fGuiApp( ).fEditableObjects( ).fCollectByType< tEditableTileCanvas >( canvasObjs );

		b32 hideProps = true;
		if( canvasObjs.fCount() == 0 )
			return;

		hideProps = !canvasObjs[0]->fGetHidingProps();

		for( u32 i = 0; i < canvasObjs.fCount(); ++i )
				canvasObjs[i]->fSetHideProps( hideProps );
	}

	b32 tTilePaintPanel::fLoaded( ) const
	{
		return mDatabase.fAllTileSetsLoaded();
	}

	b32 tTilePaintPanel::fAnyLoaded( ) const
	{
		return mDatabase.fAnyTileSetLoaded();
	}

	tEditableTileEntityPtr tTilePaintPanel::fGetTileWithPanelParams( tEditableObjectContainer& container, u32 tileType )
	{
		wxString label = mIndividualsDisplay->GetStringSelection();

		if( label == "NON SPECIFIC" )
			return fDatabase()->fGetRandomizedTile( container, (tTileTypes)tileType );

		return fDatabase()->fGetSpecificTile( container, (tTileTypes)tileType, mTileKeys[ mIndividualsDisplay->GetSelection() ] );
	}

	

	void tTilePaintPanel::fOnRotateCW( wxCommandEvent& )
	{
		fRotateSelectedTile( false );
	}

	void tTilePaintPanel::fOnRotateCCW( wxCommandEvent& )
	{
		fRotateSelectedTile( true );
	}

	void tTilePaintPanel::fOnPreview( wxCommandEvent& )
	{
		fPreviewTiles();
	}

	void tTilePaintPanel::fOnHideProps( wxCommandEvent& )
	{
		fHideProps();
	}

	void tTilePaintPanel::fOnListBoxSelect( wxCommandEvent& )
	{
		tTileBrushBase* cursor = dynamic_cast<tTileBrushBase*>( fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );

		if( !cursor )
			return;

		cursor->fSyncCursor( );
	}

	void tTilePaintPanel::fOnComboBoxChanged()
	{
		mButtonGroup->fOnSelChanged();
	}

	//------------------------------------------------------------------------------
	// tTileDbPanel
	//------------------------------------------------------------------------------
	tTiledmlPanel::tTiledmlPanel( tTilePaintPanel* parentWindow, wxPanel* parent )
		: wxPanel( parent, wxID_ANY )
		, mPaintPanel( parentWindow )
	{
		tWxSlapOnGroup* tileDBGroup = new tWxSlapOnGroup( parent, "Database Editor", false );

		tileDBGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 3 );

		mDbTree = new tTiledmlTree( tileDBGroup->fGetMainPanel( ), this, mPaintPanel->fDatabase( ) );
		tileDBGroup->fGetMainPanel( )->GetSizer( )->Add( mDbTree, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 3 );

		wxSizer* horiz = new wxBoxSizer( wxHORIZONTAL );

		wxButton* saveButton = new wxButton( parent, wxID_ANY, "Save" );
		saveButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTiledmlPanel::fOnSaveButton ), NULL, this );
		horiz->Add( saveButton, 0, wxBOTTOM, 3 );

		mLoadButton = new wxButton( parent, wxID_ANY, "Load All" );
		mLoadButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTiledmlPanel::fOnLoadButton ), NULL, this );
		horiz->Add( mLoadButton, 0, wxLEFT, 3 );

		parent->GetSizer()->Add( horiz, 0, wxCENTER );

		mDbProperties = new tTiledmlPropPanel( parent, "Item Properties", mDbTree, parent->GetParent(), mPaintPanel->fGuiApp().fEditableObjects().fGetResourceDepot() );

		mPaintPanel->fLoadDefaultResources( false );
		fRefreshBrowser( );
	}

	void tTiledmlPanel::fOnSave( )
	{
		mPaintPanel->fDatabase( )->fSerializeTileSets( );
	}

	void tTiledmlPanel::fOnSetUpRendering( )
	{
		fRefreshBrowser( );
	}

	void tTiledmlPanel::fHideAll( )
	{
		mDbProperties->fHideAll( );
	}

	void tTiledmlPanel::fConfigureForSet( tEditableTileSet* set )
	{
		mDbProperties->fConfigureForSet( set );
	}

	void tTiledmlPanel::fConfigureForTile( tEditableTileDef* tile )
	{
		mDbProperties->fConfigureForTile( tile );
	}

	void tTiledmlPanel::fRefreshBrowser( )
	{
		if( mPaintPanel->fDatabase() && mPaintPanel->fDatabase()->fAllTileSetsLoaded() )
		{
			mLoadButton->Hide( );
			mPaintPanel->fGetMainPanel( )->GetSizer( )->Layout( );
		}

		mDbTree->fRefresh( );
	}

	void tTiledmlPanel::fAddTileSet( const char* familyName )
	{
		// Browse for the root directory to build from.
		tStrongPtr<wxDirDialog> openDirDialog( new wxDirDialog( 
			mPaintPanel->fGetMainPanel(), 
			"Select Tile Set Directory",
			wxString( ToolsPaths::fGetCurrentProjectResFolder().fCStr() ) ) );

		if( openDirDialog->ShowModal() != wxID_OK )
			return;

		tFilePathPtr rootDir( openDirDialog->GetPath().c_str() );

		// Browse for a tile set to extract textures from.
		tStrongPtr<wxSingleChoiceDialog> textureDialog( new wxSingleChoiceDialog( 
			mPaintPanel->fGetMainPanel(), 
			"Minimap Texture Template",
			"Pick an existing loaded tile set from which to draw minimap textures. This is optional but you will need to set up minimap textures outside SigEd.",
			mPaintPanel->fDatabase()->fGetLoadedTileSetsForDisplay() ) );

		tEditableTileSet* texturesSet = NULL;

		if( textureDialog->ShowModal() == wxID_OK )
		{
			wxString selectedSet = textureDialog->GetStringSelection();
			texturesSet = mPaintPanel->fDatabase()->fTileSetByFileName( selectedSet );
		}

		mPaintPanel->fDatabase( )->fAddTileSet( mPaintPanel->fGuiApp().fEditableObjects().fGetResourceDepot(), rootDir, texturesSet );
		fRefreshBrowser( );
		mPaintPanel->fRefreshSelectionDrop();
	}

	void tTiledmlPanel::fOnSaveButton( wxCommandEvent& )
	{
		fOnSave( );
	}

	void tTiledmlPanel::fOnLoadButton( wxCommandEvent& )
	{
		// Reach back into the parent and initiate loading all tile sets.
		mPaintPanel->fLoadDefaultResources( true );
	}
}
