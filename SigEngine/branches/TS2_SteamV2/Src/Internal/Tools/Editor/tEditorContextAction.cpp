#include "ToolsPch.hpp"
#include "tEditorContextAction.hpp"
#include "wx/menu.h"

namespace Sig
{
	u32 tEditorContextAction::gUniqueActionId = 0;
	tEditorContextActionList tEditorContextAction::gLastActionList;

	void tEditorContextAction::fDisplayContextMenuOnRightClick( wxWindow* window, wxMouseEvent& event, const tEditorContextActionList& contextActions )
	{
		gLastActionList = contextActions;

		if( !event.AltDown( ) && contextActions.fCount( ) > 0 )
		{
			wxMenu menu;
			const wxPoint pos = event.GetPosition();

			for( u32 i = 0; i < contextActions.fCount( ); ++i )
			{
				contextActions[ i ]->fAddToContextMenu( menu );
				contextActions[ i ]->mRightClickPos = pos;
			}

			if( menu.GetMenuItems( ).GetLast( ) )
			{
				wxMenuItem* lastItem = menu.GetMenuItems( ).GetLast( )->GetData( );
				if( lastItem->IsSeparator( ) )
					delete menu.Remove( lastItem );
			}

			window->PopupMenu( &menu, pos.x, pos.y );
		}

		event.Skip( );
	}

	void tEditorContextAction::fDisplayContextMenuOnRightClick( wxWindow* window, wxMouseState& mouse, const tEditorContextActionList& contextActions )
	{
		gLastActionList = contextActions;

		if( !mouse.AltDown( ) && contextActions.fCount( ) > 0 )
		{
			wxMenu menu;
			
			const wxPoint pos = window->ScreenToClient( wxPoint( mouse.GetX(), mouse.GetY() ) ); // This version of wxWidgets doesn't have GetPosition on wxMouseState.

			for( u32 i = 0; i < contextActions.fCount( ); ++i )
			{
				contextActions[ i ]->fAddToContextMenu( menu );
				contextActions[ i ]->mRightClickPos = pos;
			}

			if( menu.GetMenuItems( ).GetLast( ) )
			{
				wxMenuItem* lastItem = menu.GetMenuItems( ).GetLast( )->GetData( );
				if( lastItem->IsSeparator( ) )
					delete menu.Remove( lastItem );
			}

			window->PopupMenu( &menu, pos.x, pos.y );
		}
	}

	void tEditorContextAction::fHandleContextActionFromRightClick( wxWindow* window, wxCommandEvent& event, const tEditorContextActionList& contextActions )
	{
		const u32 actionId = event.GetId( );

		b32 handled = false;
		for( u32 i = 0; i < contextActions.fCount( ); ++i )
		{
			if( contextActions[ i ]->fHandleAction( actionId ) )
			{
				handled = true;
				break;
			}
		}

		if( !handled )
		{
			log_warning( 0, "Unrecognized context action!" );
			event.Skip( );
		}
	}

}
