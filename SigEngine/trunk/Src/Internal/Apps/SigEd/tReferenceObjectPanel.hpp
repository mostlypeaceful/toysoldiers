#ifndef __tReferenceObjectPanel__
#define __tReferenceObjectPanel__
#include "tWxToolsPanel.hpp"
#include "wx/treectrl.h"

namespace Sig
{
	class tEditorAppWindow;
	class tResourceBrowserTree;
	class tWxDirectoryBrowser;

	class tReferenceObjectPanel : public tWxToolsPanelTool
	{
		Time::tStopWatch mRefreshTimer;
		tResourceBrowserTree* mBrowser;
		wxTreeItemId mCurSel;
		b32 mRefreshed;
	public:
		tReferenceObjectPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent );
		virtual void fOnTick( );

		tWxDirectoryBrowser* fGetBrowser( );
	};
}

#endif//__tReferenceObjectPanel__
