//------------------------------------------------------------------------------
// \file tTileDbPanel.cpp - 29 Sep 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tTileDbPanel.hpp"
#include "tTilePaintPanel.hpp"
#include "tEditableObjectContainer.hpp"
#include "tTileDbTree.hpp"
#include "tSigTileMainWindow.hpp"

namespace Sig
{


	//------------------------------------------------------------------------------
	// tTileDbPanel
	//------------------------------------------------------------------------------
	tTileDbPanel::tTileDbPanel( tSigTileMainWindow* appWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Tile Database", "Tile Database", "TileDB" )
		, mMainWindow( appWindow )
		, mPaintPanel( NULL )
	{
		tWxSlapOnGroup* tileDBGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Database Editor", false );

		tileDBGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 3 );

		mDbTree = new tTileDbTree( tileDBGroup->fGetMainPanel( ), this, mMainWindow->fDataBase( ) );
		tileDBGroup->fGetMainPanel( )->GetSizer( )->Add( mDbTree, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 3 );

		mDbProperties = new tTileDbPropPanel( fGetMainPanel( ), "Item Properties", mDbTree, parent );
	}

	void tTileDbPanel::fOnSave( )
	{
		mMainWindow->fDataBase( )->fSerializeTileSets( );
	}

	void tTileDbPanel::fOnSetUpRendering( )
	{
		fRefreshBrowser( );
	}

	void tTileDbPanel::fHideAll( )
	{
		mDbProperties->fHideAll( );
	}

	void tTileDbPanel::fConfigureForSet( tEditableTileSet* set )
	{
		mDbProperties->fConfigureForSet( set );
	}

	void tTileDbPanel::fConfigureForTile( tEditableTileDef* tile )
	{
		mDbProperties->fConfigureForTile( tile );
	}

	void tTileDbPanel::fRefreshBrowser( )
	{
		mDbTree->fRefresh( );
	}

	void tTileDbPanel::fAddTileSet( const char* familyName )
	{
		// browse for a new path
		tStrongPtr<wxDirDialog> openDirDialog( new wxDirDialog( 
			fGetMainPanel( ), 
			"Select Tile Set Directory",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ) ) );

		if( openDirDialog->ShowModal( ) == wxID_OK )
		{
			mMainWindow->fDataBase( )->fAddTileSet( mMainWindow->fGuiApp( ).fEditableObjects( ).fGetResourceDepot( ), tFilePathPtr( openDirDialog->GetPath( ).c_str( ) ), familyName );
			fRefreshBrowser( );
		}
	}
}
