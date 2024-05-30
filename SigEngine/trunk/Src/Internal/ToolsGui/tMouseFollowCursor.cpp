#include "ToolsGuiPch.hpp"
#include "tMouseFollowCursor.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "Editor/tEditorSelectionList.hpp"

namespace Sig
{
	tMouseFollowCursor::tMouseFollowCursor( tEditorCursorControllerButton* button, const char* statusText, const tManipulationGizmoPtr& gizmo )
		: tSelectObjectsCursor( button, statusText, gizmo )
	{
		
	}

	void tMouseFollowCursor::fOnTick( )
	{
		fHandleGizmo( );
		fHandleHover( );
		//fHandleSelection( );
		//fHandleDelete( );
	}

}

