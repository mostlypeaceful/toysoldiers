#ifndef __tMayaDialogGroupBox__
#define __tMayaDialogGroupBox__
#include "tWxSlapOnGroup.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{
	class tMayaDialogGroupBox : public tWxSlapOnGroup, public tMayaContainerBase
	{
	public:
		tMayaDialogGroupBox( wxWindow* parent, const char* label, b32 collapsible );
	};
}

#endif//__tMayaDialogGroupBox__
