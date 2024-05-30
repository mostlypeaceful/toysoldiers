#include "MayaPluginPch.hpp"
#include "tMayaDialogGroupBox.hpp"

namespace Sig
{

	tMayaDialogGroupBox::tMayaDialogGroupBox( wxWindow* parent, const char* label, b32 collapsible )
		: tWxSlapOnGroup( parent, label, collapsible )
	{
	}

}
