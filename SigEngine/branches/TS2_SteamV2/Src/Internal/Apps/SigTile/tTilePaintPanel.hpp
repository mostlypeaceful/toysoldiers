//------------------------------------------------------------------------------
// \file tTilePaintPanel.hpp - 07 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTilePaintPanel__
#define __tTilePaintPanel__
#include "tWxToolsPanel.hpp"
#include "Tieml.hpp"
#include "tEditableTileEntity.hpp"

namespace Sig
{
	class tSigTileMainWindow;
	class tTilePaintBrushButton;
	class tAutoFloorBrushButton;
	class tTilePaintPanel;
	class tEditableTileSet;
	class tTileDbPanel;
	class tTileCanvas;
	class tTileSetPaletteButtonGroup;
	class tTileSetPaletteButton;
	class tTilePaintButtonGroup;
	enum tTileTypes;


	///
	/// \class tTilePaintPanel
	/// \brief Panel container for buttons. Must communicate down to the tile set to
	/// configure the buttons.
	class tTilePaintPanel : public tWxToolsPanelTool
	{
		tSigTileMainWindow*					mMainWindow;
		tTileSetPaletteButtonGroup*			mPalettesGroup;
		tDynamicArray<tTileSetPaletteButton*> mPaletteButtons;
		wxButton*							mOpenPalettesDialogButton;
		tTilePaintButtonGroup*				mButtonGroup;
		tAutoFloorBrushButton*				mAutoFloorButton;
		wxListBox*							mIndividualsDisplay;

	public:
		tTilePaintPanel( tWxToolsPanel* parent, tSigTileMainWindow* mainWindow );
		~tTilePaintPanel( ) { }

		void fRefreshBrushes( );

		void fClearSelection( );

		b32 fTileSetBrushIsSelected( ) const;
		b32 fTileSetBrushHasTileSets( ) const;
		void fRefreshIndividualsDisplay( );
		void fFillIndividualsDisplay( tTileTypes type );
		void fClearIndividualsDisplay( );
		b32 fIndividualsDisplayHasTiles( ) const;

		void fRotateSelectedTile( b32 ccw );

		tEditableTileEntityPtr fGetTileWithPanelParams( u32 tileType, u32 numRotations = 0 );

		tEditableTileEntityPtr fGetSpecificTile( tTileTypes requestedType, const tFilePathPtr& modelPath, f32 x = 0.f, f32 y = 0.f, u32 numRotationsCW = 0 ) const;
		tEditableTileEntityPtr fGetRandomizedTile( tTileTypes requestedType, f32 x = 0.f, f32 y = 0.f, u32 numRotationsCW = 0 ) const;

	private:
		void fOnRotateCW( wxCommandEvent& );
		void fOnRotateCCW( wxCommandEvent& );
		void fOnListBoxSelect( wxCommandEvent& );
		void fOpenPalettesDialog( wxCommandEvent& );
	};
}

#endif//__tTilePaintPanel__
