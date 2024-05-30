#ifndef __tAnifigPanel__
#define __tAnifigPanel__
#include "tWxToolsPanel.hpp"
#include "wx/treectrl.h"

namespace Sig
{
	class tSigAnimMainWindow;
	class tConfigurableBrowserTree;

	/// 
	/// \brief 
	class tAnifigPanel : public tWxToolsPanelTool
	{
		tConfigurableBrowserTree*	mBrowser;
		b32							mRefreshed;

	public:
		tAnifigPanel( tWxToolsPanel * parent, tSigAnimMainWindow & mainWindow );
		virtual void fOnTick( );
		void fMarkForRefresh( ) { mRefreshed = false; }
	};
}

#endif//__tAnifigPanel__
