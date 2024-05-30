#include "ToolsGuiPch.hpp"
#include "tConfigurableBrowserTree.hpp"
#include "Anifig.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Editor\tRenameFilepathDialog.hpp"
#include "Threads\tProcess.hpp"

namespace Sig
{
	enum tAction
	{
		cActionRefreshDirectory = 1,
		cActionOpenFile,
		cActionRenameFile, 
		cActionRenameFileSameType, 

		cLastCodeAction,
	};

	tConfigurableBrowserTree::tConfigurableBrowserTree( wxWindow* parent, tFilterFunction filter, u32 minHeight, b32 processActivation, b32 showRenameInRightClick )
		: tWxDirectoryBrowser( parent, minHeight )
		, mFilterFn( filter )
		, mProcessActivation( processActivation )
		, mShowRenameInRightClick( showRenameInRightClick )
	{
		Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler(tConfigurableBrowserTree::fOnItemRightClick), 0, this );
		Disconnect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(tWxDirectoryBrowser::fOnSelChanged), 0, this );
		Connect( wxEVT_COMMAND_TREE_ITEM_ACTIVATED, wxTreeEventHandler(tConfigurableBrowserTree::fOnDoubleClick), 0, this );

		mLastActionId = cLastCodeAction;
	}

	void tConfigurableBrowserTree::fAddMenuOption( tMenuOptionPtr& newOption )
	{
		s32 thisActionId = mLastActionId++;
		Connect( thisActionId, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(tConfigurableBrowserTree::fOnAction), 0, this );

		newOption->fSetActionId( thisActionId );
		mMenuOptions.fPushBack( newOption );
	}

	b32 tConfigurableBrowserTree::fFilterPath( const tFilePathPtr& path )
	{
		sigassert( mFilterFn );
		return !mFilterFn( path );
	}

	void tConfigurableBrowserTree::fOnItemRightClick( wxTreeEvent& event )
	{
		// Save the item for use in response functions.
		mRightClickItem = event.GetItem( );

		// Always allow refreshing the browser.
		wxMenu menu;
		menu.Append( cActionRefreshDirectory, _T("Refresh browser") );

		// If an actual item is selected, add open file option.
		tFileEntryData* fileItem = fIsFileItem( event.GetItem( ) );
		if( fileItem )
		{
			sigassert( !fFilterPath( fileItem->fXmlPath( ) ) );
			menu.Append( cActionOpenFile, _T("Open file") );
		}

		// Arranged so that user options appear underneath Open file
		for( u32 i = 0; i < mMenuOptions.fCount( ); ++i )
			mMenuOptions[i]->tAddOption( fileItem, event, menu );

		if( fileItem && mShowRenameInRightClick )
		{
			menu.AppendSeparator( );
			menu.Append( cActionRenameFile, _T("Rename file") );
			//menu.Append( cActionRenameFileSameType, _T("Rename file (only files of same type)") );
		}

		// Present menu.
		PopupMenu(&menu, event.GetPoint( ).x, event.GetPoint( ).y);

		event.Skip( );
	}

	void tConfigurableBrowserTree::fOnAction( wxCommandEvent& event )
	{
		if( !mRightClickItem.IsOk( ) )
			return;

		s32 actionId = event.GetId( );
		const tFileEntryData* fileEntryData = fIsFileItem( mRightClickItem );

		if( actionId >= cLastCodeAction )
		{
			for( u32 i = 0; i < mMenuOptions.fCount( ); ++i )	
			{
				if( mMenuOptions[i]->fActionId( ) == actionId )
				{
					mMenuOptions[i]->tExecuteOption( fileEntryData, event );
					mRightClickItem = wxTreeItemId( );
					return;
				}
			}
		}

		switch( actionId )
		{
		case cActionRefreshDirectory:
			{
				fRefresh( );

				// Clear right click item.
				mRightClickItem = wxTreeItemId( );
			}
			break;

		case cActionOpenFile:
			{
				if( !mRightClickItem.IsOk( ) )
					break;

				if( fileEntryData )
				{
					sigassert( !fFilterPath( fileEntryData->fXmlPath( ) ) );

					// Open this file through whoever inherits.
					fOpenDoc( ToolsPaths::fMakeResAbsolute( fileEntryData->fXmlPath( ) ) );
				}

				// Clear right click item.
				mRightClickItem = wxTreeItemId( );
			}
			break;
		case cActionRenameFile:
		case cActionRenameFileSameType:
			{
				if( !mRightClickItem.IsOk( ) )
					break;

				const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( mRightClickItem ) );
				if( fileEntryData )
				{
					new tRenameFilepathDialog( this, fileEntryData->fXmlPath( ), (actionId == cActionRenameFileSameType) );
					//fRefresh( );
				}
			}
			break;
		default: { log_warning( 0, "Unrecognized action!" ); }
				 event.Skip( );
				 break;
		}
	}

	void tConfigurableBrowserTree::fOnDoubleClick( wxTreeEvent& event )
	{
		if( !mProcessActivation )
			return;

		wxTreeItemId id = event.GetItem( );
		if( !id.IsOk( ) )
			return;

		const tFileEntryData* fileEntryData = dynamic_cast< tFileEntryData* >( GetItemData( id ) );
		if( fileEntryData && !fFilterPath( fileEntryData->fXmlPath( ) ) )
		{
			fOpenDoc( ToolsPaths::fMakeResAbsolute( fileEntryData->fXmlPath( ) ) );
		}
	}

	BEGIN_EVENT_TABLE(tConfigurableBrowserTree, wxTreeCtrl)
		EVT_MENU(					cActionRefreshDirectory,	tConfigurableBrowserTree::fOnAction)
		EVT_MENU(					cActionOpenFile,			tConfigurableBrowserTree::fOnAction)
		EVT_MENU(					cActionRenameFile,			tConfigurableBrowserTree::fOnAction)
		EVT_MENU(					cActionRenameFileSameType,	tConfigurableBrowserTree::fOnAction)
	END_EVENT_TABLE()
}
