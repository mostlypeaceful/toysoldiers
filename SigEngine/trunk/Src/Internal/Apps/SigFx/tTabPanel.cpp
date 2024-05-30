#include "SigFxPch.hpp"
#include "tTabPanel.hpp"


namespace Sig
{

	tTabPanel::tTabPanel( wxNotebook* parent  )
		: wxNotebookPage( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxBG_STYLE_CUSTOM | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN )
	{

	}

	tTabPanel::~tTabPanel( )
	{

	}


}

