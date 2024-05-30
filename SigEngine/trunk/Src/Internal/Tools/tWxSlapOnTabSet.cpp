#include "ToolsPch.hpp"
#include "tWxSlapOnTabSet.hpp"

namespace Sig
{
	tWxSlapOnTabSet::tWxSlapOnTabSet( wxWindow* parent )
		: wxNotebook( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE )
	{
		SetBackgroundColour( parent->GetBackgroundColour( ) );
		SetForegroundColour( parent->GetForegroundColour( ) );
	}
}
