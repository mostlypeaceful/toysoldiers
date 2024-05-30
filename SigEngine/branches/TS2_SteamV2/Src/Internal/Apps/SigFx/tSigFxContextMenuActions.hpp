#ifndef __tSigFxContextMenuActions__
#define __tSigFxContextMenuActions__

#include "Editor/tEditorContextAction.hpp"
#include "tSigFxSystem.hpp"

namespace Sig
{
	class tCloneSelectedFxObjectsContextAction : public tEditorContextAction
	{
		tSigFxMainWindow* mFxWindow;
		u32 mId;
	public:
		tCloneSelectedFxObjectsContextAction( tSigFxMainWindow* window )
			: mFxWindow( window ), mId( fNextUniqueActionId( ) )
		{

		}

		virtual b32 fAddToContextMenu( wxMenu& menu )
		{
			if( mFxWindow->fGuiApp( ).fSelectionList( ).fCount( ) )
			{
				menu.Append( mId, _T( "Clone Object(s)" ) );
				return true;
			}
			return false;
		}

		virtual b32 fHandleAction( u32 actionId )
		{
			if( actionId == mId )
			{
				mFxWindow->fCopySelectedObjects( );
				return true;
			}
			return false;
		}

	};


	class tAddNewParticleSystemContextAction : public tEditorContextAction
	{
		tSigFxMainWindow*	mFxWindow;
		FX::tSigFxSystem* mFxScene;
		u32 mId;
		
	public:
		tAddNewParticleSystemContextAction( tSigFxMainWindow* window, FX::tSigFxSystem* scene )
			: mFxWindow( window ), mFxScene( scene ), mId( fNextUniqueActionId( ) )
		{

		}

		virtual b32	fAddToContextMenu( wxMenu& menu )
		{
			menu.Append( mId, _T("Add New Particle System") );
			menu.AppendSeparator( );
			return true;
		}

		virtual b32	fHandleAction( u32 actionId )
		{
			if( actionId == mId )
			{
				//mFxScene->fAddNewParticleSystem( );
			}
			else
				return false;
			return true;
		}
	};
}

#endif //__tSigFxContextMenuActions__

