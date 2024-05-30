//------------------------------------------------------------------------------
// \file tTileDbPanel.hpp - 29 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTileDbPanel__
#define __tTileDbPanel__
#include "tWxToolsPanel.hpp"
#include "tSigTileMainWindow.hpp"

namespace Sig
{
	///
	/// \class tTileDbPanel
	/// \brief Panel that mainly acts as a viewer for the tile database. Has
	/// some light editing capabilities.
	class tTileDbPanel : public tWxToolsPanelTool
	{
		tSigTileMainWindow* mMainWindow;
		tTilePaintPanel*	mPaintPanel; // TODO: reconsider this relationship
		tTileDbTree*		mDbTree;
		tTileDbPropPanel*	mDbProperties;

	public:
		tTileDbPanel( tSigTileMainWindow* appWindow, tWxToolsPanel* parent );

		void fSetPaintPanel( tTilePaintPanel* paint ) { mPaintPanel = paint; }

		void fOnSave( );
		void fOnSetUpRendering( );

		void fHideAll( );

		void fConfigureForSet( tEditableTileSet* set );
		void fConfigureForTile( tEditableTileDef* tile );

		void fRefreshBrowser( );

		void fAddTileSet( const char* familyName = NULL );


		tSigTileMainWindow* fMainWindow( ) { return mMainWindow; }
	};
}

#endif//__tTileDbPanel__
