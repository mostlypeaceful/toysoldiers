#include "tSigAnimMainWindow.hpp"
#include "Editor\tEditableSgFileRefEntity.hpp"

namespace Sig
{
	class tOpenSelectedObjectContextMenu : public tEditorContextAction
	{
		tSigAnimMainWindow* mAnimWindow;
		u32 mId;
	public:
		tOpenSelectedObjectContextMenu( tSigAnimMainWindow* animWindow )
			: mAnimWindow( animWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			const b32 someObjectsSelected = mAnimWindow->fGuiApp( ).fSelectionList( ).fCount( ) > 0;
			if( someObjectsSelected )
			{
				menu.Append( mId, _T("&Open .sigml in file editor"));
				menu.AppendSeparator( );
				return true;
			}
			return false;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mId )
			{
				tGrowableArray< tEditableSgFileRefEntity* > sigmls;
				mAnimWindow->fGuiApp( ).fSelectionList( ).fCullByType< tEditableSgFileRefEntity >( sigmls );
				if( sigmls.fCount( ) == 1 && sigmls.fFront( )->fIsEditable( ) )
				{
					const tFilePathPtr path = Sigml::fSigbPathToSigml( sigmls.fFront( )->fResourcePath( ) );
					mAnimWindow->fOpenDoc( ToolsPaths::fMakeResAbsolute( path ) );
				}
			}
			else
				return false;
			return true;
		}
	};

	class tAnimOptionsContextMenu : public tEditorContextAction
	{
		tSigAnimMainWindow* mAnimWindow;
		wxMenu * mMenu;
		static b32 tAnimOptionsContextMenu::mChecked;
		u32 mId;
	public:
		tAnimOptionsContextMenu( tSigAnimMainWindow* animWindow )
			: mAnimWindow( animWindow )
			, mId( fNextUniqueActionId( ) )
		{
		}
		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			mMenu = &menu;
			menu.AppendCheckItem( mId, _T("&Lock to 30 FPS"));
			menu.AppendSeparator( );
			menu.Check( mId, ( mChecked == true ) );
			return true;
		}
		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mId )
			{
				mChecked = !mChecked;
				mAnimWindow->fLockFPS( mChecked );
			}
			else
				return false;
			return true;
		}
	};

	b32 tAnimOptionsContextMenu::mChecked = false;
}
