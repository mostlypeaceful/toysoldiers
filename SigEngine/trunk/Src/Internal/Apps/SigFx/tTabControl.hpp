#ifndef __tTabControl__
#define __tTabControl__

#include "wx/notebook.h"
#include "tTabPanel.hpp"
#include "tToolsGuiMainWindow.hpp"

namespace Sig
{

	class tTabControl : public wxNotebook
	{
	public:
		tTabControl( wxWindow* parent, tToolsGuiMainWindow* toolswindow );
		virtual ~tTabControl( );

		void fOnTickSelectedTab( f32 dt );

		void fBuildPageFromEntities( tEditorSelectionList& selectedObjects );

		void fOnPageChanged( wxCommandEvent& event );

	private:

		tToolsGuiMainWindow* mToolsWindow;
		
	};




}

#endif // __tTabControl__

