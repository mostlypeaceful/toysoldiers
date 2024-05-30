//------------------------------------------------------------------------------
// \file tTilePaintPanel.cpp - 07 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tTilePaintPanel.hpp"
#include "tEditorCursorControllerButton.hpp"
#include "tTilePaintBrushButton.hpp"
#include "tTileDbPanel.hpp"
#include "tSigTileMainWindow.hpp"

namespace Sig
{

	///
	/// \class tTileSetPaletteButton
	/// \brief Holds a brush definition that is referred to when selecting primitives buttons.
	class tTileSetPaletteButton : public tWxSlapOnRadioBitmapButton
	{
		tEditableTileSetPigment* mPalette;

	public:
		tTileSetPaletteButton( tWxSlapOnRadioBitmapButtonGroup* parent, tEditableTileSetPigment* brush, const char* tooltip )
			: tWxSlapOnRadioBitmapButton( parent, wxBitmap( "TilePainting" ), wxBitmap( "TilePainting" ), tooltip )
			, mPalette( brush )
		{
			fSetColor(  brush->fColor( ) );
		}

		tEditableTileSetPigment* fPalette( ) const { return mPalette; }
	};

	///
	/// \class tTileSetPaletteButtonGroup
	/// \brief Exists to grab the right palette from whatever button is selected.
	class tTileSetPaletteButtonGroup : public tWxSlapOnRadioBitmapButtonGroup
	{
		tTilePaintPanel* mPaintPanel;

	public:
		tTileSetPaletteButtonGroup( tTilePaintPanel* parent, const char* label )
			: tWxSlapOnRadioBitmapButtonGroup( parent->fGetMainPanel( ), label, false, 6 )
			, mPaintPanel( parent )
		{ }

		tEditableTileSetPigment* fSelectedPalette( ) const
		{
			if( mButtons.fCount( ) == 0 )
				return NULL;

			s32 selectedButton = fGetSelected( );
			if( selectedButton == -1 )
				return NULL;

			const tTileSetPaletteButton* brushButton = dynamic_cast< tTileSetPaletteButton* >( mButtons[ selectedButton ] );
			if( !brushButton )
				return NULL;

			return brushButton->fPalette( );
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

			if( selectedButton != cEraseButton && !mPaintPanel->fTileSetBrushIsSelected( ) )
			{
				tStrongPtr<wxMessageDialog> warningDialog( new wxMessageDialog(
					fGetMainPanel( ),
					"No tile set palette selected. Please select one above.",
					"No Palette Selected" ) );

				warningDialog->ShowModal( );
				fClearSelection( );
				return;
			}

			if( selectedButton != cEraseButton && !mPaintPanel->fTileSetBrushHasTileSets( ) )
			{
				tStrongPtr<wxMessageDialog> warningDialog( new wxMessageDialog(
					fGetMainPanel( ),
					"The selected palette has no tile sets specified. Please add at least one tile set to your palette.",
					"Palette Lacks Tile Sets" ) );

				warningDialog->ShowModal( );
				fClearSelection( );
				return;
			}

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
	tTilePaintPanel::tTilePaintPanel( tWxToolsPanel* parent, tSigTileMainWindow* mainWindow )
		: tWxToolsPanelTool( parent, "Tile Painting", "Tile Painting", "TilePainting" )
		, mMainWindow( mainWindow )
	{
		// Brushes
		mPalettesGroup = new tTileSetPaletteButtonGroup( this, "Tile Set Palettes" );
		mOpenPalettesDialogButton = new wxButton( mPalettesGroup->fGetMainPanel( ), wxID_ANY, "Create Tile Set Palette" );
		mOpenPalettesDialogButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tTilePaintPanel::fOpenPalettesDialog ), NULL, this );
		mPalettesGroup->fGetMainPanel( )->GetSizer( )->Add( mOpenPalettesDialogButton, 0, wxALL | wxCENTER, 4 );

		mButtonGroup = new tTilePaintButtonGroup( this, "Tile Painting" );

		tTileCanvas* canvas = mMainWindow->fCanvas( ).fGetRawPtr( );

		// These must be added in the order that matches the tTilePaintButtonGroup enum.
		mAutoFloorButton = new tAutoFloorBrushButton( mButtonGroup, canvas );
		new tTilePaintBrushButton( mButtonGroup, this, canvas, cFloor, "TileFloorSel", "TileFloorDeSel", "Paint floor" );
		new tTilePaintBrushButton( mButtonGroup, this, canvas, cCorner, "TileCornerSel", "TileCornerDeSel", "Paint corner" );
		new tTilePaintBrushButton( mButtonGroup, this, canvas, cNiche, "TileNicheSel", "TileNicheDeSel", "Paint niche" );
		new tTilePaintBrushButton( mButtonGroup, this, canvas, cWall, "TileWallSel", "TileWallDeSel", "Paint wall" );
		new tTilePaintBrushButton( mButtonGroup, this, canvas, cUniques, "TileUniqueSel", "TileUniqueDeSel", "Paint a unique" );
		new tTileEraseBrushButton( mButtonGroup, canvas );

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

		horiz->Add( clockWise, 0, wxBOTTOM, 8 );
		horiz->Add( counterClockWise, 0, wxLEFT, 3 );

		fGetMainPanel( )->GetSizer( )->Add( horiz, 0, wxCENTER );
	}

	void tTilePaintPanel::fRefreshBrushes( )
	{
		// Track the last selected palette.
		const tEditableTileSetPigment* previouslySelected = mPalettesGroup->fSelectedPalette( );

		mPalettesGroup->fDeleteButtons( );

		tEditableTileDb* tileDb = mMainWindow->fDataBase( );
		mPaletteButtons.fNewArray( tileDb->fNumPigments( ) );
		for( u32 i = 0; i < tileDb->fNumPigments( ); ++i )
		{
			tEditableTileSetPigment* brush = tileDb->fPigmentByIdx( i );
			mPaletteButtons[i] = new tTileSetPaletteButton( mPalettesGroup, brush, brush->fName( ).c_str( ) );
		}

		// Do some special set up based on number of buttons.
		if( mPaletteButtons.fCount( ) == 0 )
		{
			// If there's no palettes, show a link to the palette dialog.
			mOpenPalettesDialogButton->Show( );
		}
		else
		{
			// If palettes exist, hide the dialog button.
			mOpenPalettesDialogButton->Hide( );

			// If something was previously selected, keep it selected.
			if( previouslySelected )
			{
				for( u32 i = 0; i < mPaletteButtons.fCount( ); ++i )
				{
					if( previouslySelected == mPaletteButtons[i]->fPalette( ) )
					{
						mPalettesGroup->fSetSelected( i );
						break;
					}
				}
			}
			else if( mPaletteButtons.fCount( ) >= 1 )
			{
				// If there's only one button that exists, select that one.
				mPalettesGroup->fSetSelected( 0u );
			}
		}

		mPalettesGroup->fGetMainPanel( )->Layout( );
		mPalettesGroup->fGetMainPanel( )->Refresh( );
		fGetMainPanel( )->Layout( );
		fGetMainPanel( )->Refresh( );
		fGetMainPanel( )->GetParent( )->Layout( );
		fGetMainPanel( )->GetParent( )->Refresh( );
	}

	void tTilePaintPanel::fClearSelection( )
	{
		mButtonGroup->fClearSelection( );
	}

	b32 tTilePaintPanel::fTileSetBrushIsSelected( ) const
	{
		return mPalettesGroup->fSelectedPalette( ) != NULL;
	}

	b32 tTilePaintPanel::fTileSetBrushHasTileSets( ) const
	{
		return mPalettesGroup->fSelectedPalette( )->fNumTileSetGuids( ) > 0;
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

		tEditableTileSetPigment* brush = mPalettesGroup->fSelectedPalette( );
		if( !brush )
			return;

		if( brush->fHasRandomTiles( type ) )
			newStrings.push_back( "RANDOM" );

		const tModelList& specModels = brush->fSpecificModelList( type );
		for( u32 i = 0; i < specModels.fCount( ); ++i )
			newStrings.push_back( specModels[i].fCStr( ) );

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

	tEditableTileEntityPtr tTilePaintPanel::fGetTileWithPanelParams( u32 tileType, u32 numRotations )
	{
		s32 variationId = mIndividualsDisplay->GetSelection( );
		if( variationId == -1 )
			variationId = 0;

		wxString label = mIndividualsDisplay->GetString( variationId );

		if( label == "RANDOM" )
			return fGetRandomizedTile( (tTileTypes)tileType, 0.f, 0.f, numRotations );

		tFilePathPtr modelPath( label );

		return fGetSpecificTile( (tTileTypes)tileType, modelPath, 0.f, 0.f, numRotations );
	}

	tEditableTileEntityPtr tTilePaintPanel::fGetSpecificTile( tTileTypes requestedType, const tFilePathPtr& modelPath, f32 x, f32 y, u32 numRotationsCW ) const
	{
		tEditableTileSetPigment* brush = mPalettesGroup->fSelectedPalette( );
		sigassert( brush );

		tEditableTileSet* tileSet = mMainWindow->fDataBase( )->fTileSetByGuid( brush->fGetTileSetGuidAuto( ) );
		const u32 variationId = tileSet->fGetVariationId( requestedType, modelPath );

		Math::tMat3f tileXform( Math::MatrixUtil::cIdentityTag );
		tileXform.fSetTranslation( Math::tVec3f( x, brush->fTileHeight( ), y ) );
		tEditableTileEntity* newTile = new tEditableTileEntity( 
			requestedType,
			brush->fGuid( ), 
			tileSet->fGetTileTexture( requestedType, variationId ), 
			tileSet->fGetTileModel( requestedType, variationId ), 
			tileSet->fGetTileModelPath( requestedType, variationId ), 
			tileXform,
			brush->fColorU32( ),
			brush->fTileSize( ),
			tileSet->fGetTileDims( requestedType, variationId ),
			cNotRandom,
			brush->fIsSpecificTileSet( ),
			true );

		for( u32 i = 0; i < numRotationsCW; ++i )
			newTile->fRotate( false );

		return tEditableTileEntityPtr( newTile );
	}

	tEditableTileEntityPtr tTilePaintPanel::fGetRandomizedTile( tTileTypes requestedType, f32 x, f32 y, u32 numRotationsCW ) const
	{
		tEditableTileSetPigment* brush = mPalettesGroup->fSelectedPalette( );
		sigassert( brush );

		Math::tMat3f tileXform( Math::MatrixUtil::cIdentityTag );
		tileXform.fSetTranslation( Math::tVec3f( x, brush->fTileHeight( ), y ) );

		tEditableTileEntity* newTile = new tEditableTileEntity( 
			requestedType,
			brush->fGuid( ), 
			mMainWindow->fDataBase( )->fRandomTileMarker( requestedType ), 
			tSgFileRefEntityPtr( ), 
			tFilePathPtr( ), 
			tileXform, 
			brush->fColorU32( ), 
			brush->fTileSize( ),
			Math::tVec2u( 1, 1 ),
			cRandomizeAtAssetGen, // TODO: specify asset gen or level load
			false,
			false );

		for( u32 i = 0; i < numRotationsCW; ++i )
			newTile->fRotate( false );

		tEditableTileEntityPtr returnTile( newTile );

		mMainWindow->fDataBase( )->fBakeRandomTileForEditor( returnTile );

		return returnTile;
	}

	void tTilePaintPanel::fOnRotateCW( wxCommandEvent& )
	{
		fRotateSelectedTile( false );
	}

	void tTilePaintPanel::fOnRotateCCW( wxCommandEvent& )
	{
		fRotateSelectedTile( true );
	}

	void tTilePaintPanel::fOnListBoxSelect( wxCommandEvent& )
	{
		tTileBrushBase* cursor = dynamic_cast<tTileBrushBase*>( fGuiApp( ).fCurrentCursor( ).fGetRawPtr( ) );

		if( !cursor )
			return;

		cursor->fSyncCursor( );
	}

	void tTilePaintPanel::fOpenPalettesDialog( wxCommandEvent& )
	{
		mMainWindow->fOpenPalettesDialog( );
	}
}
