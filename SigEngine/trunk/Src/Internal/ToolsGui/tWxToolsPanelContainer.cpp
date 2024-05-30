#include "ToolsGuiPch.hpp"
#include "tWxToolsPanelContainer.hpp"
#include "tWxToolsPanel.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"

namespace Sig
{

	tWxToolsPanelContainer::tWxToolsPanelContainer( tToolsGuiMainWindow& mainWindow )
		: wxPanel( &mainWindow, wxID_ANY, wxDefaultPosition, wxDefaultSize )
		, mGuiApp( mainWindow.fGuiApp( ) )
		, mMainSizer( 0 )
		, mToolBar( 0 )
	{
		mMainSizer = new wxBoxSizer( wxHORIZONTAL );
		SetSizer( mMainSizer );
		Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler( tWxToolsPanelContainer::fEnableTool ), NULL, this );
	}

	void tWxToolsPanelContainer::fAfterAllToolsPanelsAdded( )
	{
		sigassert( !mToolBar );

		if( mToolsPanels.fCount( ) == 0 )
			return; // don't need a toolbar, there are no tools

		// now that the tools panels are created, we can setup the toolbar
		mToolBar = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxTB_RIGHT );
		mToolBar->SetToolBitmapSize( wxSize( 32, 32 ) );
		u32 runningActionId = 1;
		for( u32 i = 0; i < mToolsPanels.fCount( ); ++i )
		{
			mToolsPanels[ i ]->fAddToToolBar( mToolBar, runningActionId );
			if( i < mToolsPanels.fCount( ) - 1 )
				mToolBar->AddSeparator( );
		}
		mToolBar->Realize( );
		mMainSizer->Add( mToolBar, 0, wxEXPAND | wxTOP | wxBOTTOM | wxALIGN_RIGHT, 0 );
	}

	void tWxToolsPanelContainer::fOnTick( )
	{
		for( u32 i = 0; i < mToolsPanels.fCount( ); ++i )
			mToolsPanels[ i ]->fOnTick( );
	}

//tEditorAppWindow* tWxToolsPanelContainer::fGetEditorWindow( )
//{
//	return mAppWindow;
//}

	void tWxToolsPanelContainer::fEnableTool( wxCommandEvent& event )
	{
		const u32 actionId = event.GetId( );
		tWxToolsPanel* handled = 0;

		Freeze( );

		for( u32 i = 0; !handled && i < mToolsPanels.fCount( ); ++i )
		{
			if( mToolsPanels[ i ]->fHandleAction( actionId ) )
				handled = mToolsPanels[ i ];
		}

		if( handled )
		{
			handled->Layout( );
			Layout( );
			GetParent( )->Layout( );
			Refresh( );
			Update( );
		}

		Thaw( );
	}

}


