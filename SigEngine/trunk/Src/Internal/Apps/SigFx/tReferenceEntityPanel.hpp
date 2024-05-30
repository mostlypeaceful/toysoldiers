#ifndef __tReferenceEntityPanel__
#define __tReferenceEntityPanel__
#include "tWxToolsPanel.hpp"
#include "wx/treectrl.h"

namespace Sig
{
	class tSigFxMainWindow;
	class tResourceBrowserTree;

	class tReferenceEntityPanel : public tWxToolsPanelTool
	{
		Time::tStopWatch mRefreshTimer;
		tResourceBrowserTree* mBrowser;
		wxTreeItemId mCurSel;
		b32 mRefreshed;
	public:
		tReferenceEntityPanel( tSigFxMainWindow* appWindow, tWxToolsPanel* parent );
		virtual void fOnTick( );
	};
}

#endif//__tReferenceEntityPanel__
