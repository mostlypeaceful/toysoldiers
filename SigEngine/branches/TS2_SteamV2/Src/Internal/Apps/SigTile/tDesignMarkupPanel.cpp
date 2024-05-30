//------------------------------------------------------------------------------
// \file tDesignMarkupPanel.cpp - 11 Nov 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "SigTilePch.hpp"
#include "tDesignMarkupPanel.hpp"
#include "tWxSlapOnRadioBitmapButton.hpp"
#include "tSigTileMainWindow.hpp"
#include "tScriptNodePaintButton.hpp"
#include "tEditableTileDb.hpp"

namespace Sig
{
	tDesignMarkupPanel::tDesignMarkupPanel( tWxToolsPanel* parent, tSigTileMainWindow* mainWindow )
		: tWxToolsPanelTool( parent, "Design Mark Up", "Design Mark Up", "DesignMarkup" )
		, mMainWindow( mainWindow )
	{
		mNodesGroup = new tEditorCursorControllerButtonGroup( this, "Script Node Painting", false, 6 );
		mManipGroup = new tEditorCursorControllerButtonGroup( this, "Script Node Manipulation", false, 6 );
		new tScriptEraseBrushButton( mManipGroup, mMainWindow->fCanvas( ).fGetRawPtr( ) );
		mManipGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 3 );

		mOpenScriptNodeDialogButton = new wxButton( mNodesGroup->fGetMainPanel( ), wxID_ANY, "Create Script Node" );
		mOpenScriptNodeDialogButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tDesignMarkupPanel::fOpenScriptsDialog ), NULL, this );
		mNodesGroup->fGetMainPanel( )->GetSizer( )->Add( mOpenScriptNodeDialogButton, 0, wxALL | wxCENTER, 4 );
	}

	void tDesignMarkupPanel::fRefreshNodes( )
	{
		// Track the last selected palette.
		//const tTileSetPalette* previouslySelected = mNodesGroup->fSelectedPalette( );
		const tEditableScriptNodeDef* previouslySelected = NULL;
		// TODO

		mNodesGroup->fDeleteButtons( );

		tEditableTileDb* tileDb = mMainWindow->fDataBase( );
		mBrushButtons.fNewArray( tileDb->fNumNodes( ) );
		for( u32 i = 0; i < tileDb->fNumNodes( ); ++i )
		{
			tEditableScriptNodeDef* brush = tileDb->fNodeByIdx( i );
			mBrushButtons[i] = new tScriptPaintBrushButton( 
				mNodesGroup, 
				mMainWindow->fCanvas( ).fGetRawPtr( ), 
				mMainWindow->fDataBase( ), 
				brush->fGuid( ), 
				"TilePainting", 
				brush->fName( ).c_str( ) );
		}

		// Do some special set up based on number of buttons.
		if( mBrushButtons.fCount( ) == 0 )
		{
			// If there's no palettes, show a link to the palette dialog.
			mOpenScriptNodeDialogButton->Show( );
		}
		else
		{
			// If palettes exist, hide the dialog button.
			mOpenScriptNodeDialogButton->Hide( );

			// If something was previously selected, keep it selected.
			if( previouslySelected )
			{
				for( u32 i = 0; i < mBrushButtons.fCount( ); ++i )
				{
					// TODO
					//if( previouslySelected == mBrushButtons[i]->fPalette( ) )
					//{
					//	mNodesGroup->fSetSelected( i );
					//	break;
					//}
				}
			}
		}

		mNodesGroup->fGetMainPanel( )->Layout( );
		mNodesGroup->fGetMainPanel( )->Refresh( );
		fGetMainPanel( )->Layout( );
		fGetMainPanel( )->Refresh( );
		fGetMainPanel( )->GetParent( )->Layout( );
		fGetMainPanel( )->GetParent( )->Refresh( );
	}

	const tEditableTileDb* tDesignMarkupPanel::fDatabase( )
	{
		return mMainWindow->fDataBase( );
	}

	void tDesignMarkupPanel::fOpenScriptsDialog( wxCommandEvent& )
	{
		mMainWindow->fOpenScriptNodesDialog( );
	}
}
