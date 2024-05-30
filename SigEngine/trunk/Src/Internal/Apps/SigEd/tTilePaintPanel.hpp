//------------------------------------------------------------------------------
// \file tTilePaintPanel.hpp - 07 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTilePaintPanel__
#define __tTilePaintPanel__
#include "tWxToolsPanel.hpp"
#include "tEditableTileEntity.hpp"
#include "tEditableTileCanvas.hpp"
#include "tEditableTileDb.hpp"
#include "tWxSlapOnComboBox.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tTilePaintButtonGroup;
	class tAutoFloorBrushButton;
	class tEditableTiledml;
	class tTileBrushBase;
	class tTiledmlPanel;
	enum tTileTypes;


	///
	/// \class tTilePaintPanel
	/// \brief Panel container for buttons. Must communicate down to the tile set to
	/// configure the buttons.
	/// TODO: This needs to become the kernel of a plugin that houses the db.
	class tTilePaintPanel : public tWxToolsPanelTool
	{
		class tWxTileSetComboBox : public tWxSlapOnComboBox
		{
			tTilePaintPanel* mParentPanel;
		public:
			tWxTileSetComboBox( wxWindow* parent, const char* label, tTilePaintPanel* panel, wxArrayString* choices, s64 styleFlags = 0 )
				: tWxSlapOnComboBox( parent, label, choices, styleFlags )
				, mParentPanel( panel )
			{ }
			virtual void fOnControlUpdated()
			{
				mParentPanel->fOnComboBoxChanged();
			}
		};

		tEditorAppWindow*		mMainWindow;
		wxButton*				mOpenPalettesDialogButton;
		tTilePaintButtonGroup*	mButtonGroup;
		tAutoFloorBrushButton*	mAutoFloorButton;
		tWxTileSetComboBox*		mSelectedTileSet;
		tEditableTiledml		mDatabase;
		tTiledmlPanel*			mTiledmlPanel;
		wxListBox*				mIndividualsDisplay;
		// The first element of this array is garbage to line up with the individuals display.
		// Stored in <tile set idx, tile def idx>
		tGrowableArray< tPair<u32,u32> > mTileKeys; 

	public:
		tTilePaintPanel( tEditorAppWindow* mainWindow, tWxToolsPanel* parent );
		~tTilePaintPanel() { }

		void fClearSelection();

		void fLoadDefaultResources( b32 loadModels );

		// TODO: refactor
		tEditableTiledml* fDatabase( );

		void fAddCursorHotKeys( tTileBrushBase* cursor, b32 previewOnly );
		void fRefreshSelectionDrop( );
		void fRefreshIndividualsDisplay( );
		void fFillIndividualsDisplay( tTileTypes type );
		void fClearIndividualsDisplay( );
		b32 fIndividualsDisplayHasTiles( ) const;

		void fRotateSelectedTile( b32 ccw );
		void fPreviewTiles( );
		void fHideProps( );

		b32 fLoaded( ) const;
		b32 fAnyLoaded( ) const;

		tEditableTileEntityPtr fGetTileWithPanelParams( tEditableObjectContainer& container, u32 tileType );

		tEditableTileEntityPtr fGetSpecificTile( tTileTypes requestedType, const tFilePathPtr& modelPath );

	private:
		void fOnRotateCW( wxCommandEvent& );
		void fOnRotateCCW( wxCommandEvent& );
		void fOnPreview( wxCommandEvent& );
		void fOnHideProps( wxCommandEvent& );
		void fOnListBoxSelect( wxCommandEvent& );
		void fOnComboBoxChanged( );
	};

	///
	/// \class tTiledmlPanel
	/// \brief Panel that mainly acts as a viewer for the tile database. Has
	/// some light editing capabilities.
	class tTiledmlPanel : public wxPanel
	{
		tTilePaintPanel*	mPaintPanel; // TODO: Wring out all the juices and put them in a plugin.
		tTiledmlTree*		mDbTree;
		tTiledmlPropPanel*	mDbProperties;
		wxButton*			mLoadButton;

	public:
		tTiledmlPanel( tTilePaintPanel* parentWindow, wxPanel* parent );

		void fSetPaintPanel( tTilePaintPanel* paint ) { mPaintPanel = paint; }

		void fOnSave( );
		void fOnSetUpRendering( );

		void fHideAll( );

		void fConfigureForSet( tEditableTileSet* set );
		void fConfigureForTile( tEditableTileDef* tile );

		void fRefreshBrowser( );

		void fAddTileSet( const char* familyName = NULL );

		tTilePaintPanel* fTilePaintPanel() { return mPaintPanel; }

	private:
		void fOnSaveButton( wxCommandEvent& );
		void fOnLoadButton( wxCommandEvent& );
	};
}

#endif//__tTilePaintPanel__
