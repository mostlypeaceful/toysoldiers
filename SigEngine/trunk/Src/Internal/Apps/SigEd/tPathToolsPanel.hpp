#ifndef __tPathToolsPanel__
#define __tPathToolsPanel__
#include "tWxToolsPanel.hpp"
#include "tEditorCursorControllerButton.hpp"

namespace Sig
{
	class tEditorAppWindow;

	class tPathToolsPanel : public tWxToolsPanelTool
	{
		tEditorAppWindow* mAppWindow;
		tEditorHotKeyPtr mConnectHotKey;
	public:
		tPathToolsPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent );
		void fAddCursorHotKeys( tEditorButtonManagedCursorController* cursor );
		void fUpdateParametersOnCursor( tEditorButtonManagedCursorController* cursor );
	};
}

#endif//__tPathToolsPanel__
