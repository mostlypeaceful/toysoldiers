//------------------------------------------------------------------------------
// \file tNavGraphToolsPanel.hpp - 03 Dec 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tNavGraphToolsPanel__
#define __tNavGraphToolsPanel__
#include "tWxToolsPanel.hpp"
#include "tEditorCursorControllerButton.hpp"

namespace Sig
{
	class tEditorAppWindow;

	class tNavGraphToolsPanel : public tWxToolsPanelTool
	{
		tEditorAppWindow* mAppWindow;
		//tEditorHotKeyPtr mConnectHotKey;
	public:
		tNavGraphToolsPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent );
		void fAddCursorHotKeys( tEditorButtonManagedCursorController* cursor );
		void fUpdateParametersOnCursor( tEditorButtonManagedCursorController* cursor );
	};
}

#endif //__tNavGraphToolsPanel__
