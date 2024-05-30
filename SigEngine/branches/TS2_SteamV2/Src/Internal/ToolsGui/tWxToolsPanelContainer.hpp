#ifndef __tWxToolsPanelContainer__
#define __tWxToolsPanelContainer__
#include "tWxAutoDelete.hpp"

namespace Sig
{
	class tToolsGuiApp;
	class tToolsGuiMainWindow;
	class tWxToolsPanel;
	class tWxUnifiedToolsPanel;

	class toolsgui_export tWxToolsPanelContainer : public wxPanel
	{
		friend class tWxToolsPanel;
	protected:

		tToolsGuiApp&					mGuiApp;
		wxBoxSizer*						mMainSizer;
		wxToolBar*						mToolBar;
		tGrowableArray<tWxToolsPanel*>	mToolsPanels;

	public:
		tWxToolsPanelContainer( tToolsGuiMainWindow& mainWindow );
		void fAfterAllToolsPanelsAdded( );
		void fOnTick( );

		tToolsGuiApp& fGuiApp( ) const { return mGuiApp; }
		wxPanel* fGetContainerPanel( ) { return this; }

	private:
		void fEnableTool( wxCommandEvent& event );
	};
}

#endif//__tWxToolsPanelContainer__

