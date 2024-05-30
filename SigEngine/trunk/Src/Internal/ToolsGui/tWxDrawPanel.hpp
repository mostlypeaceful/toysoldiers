#ifndef __tWxDrawPanel__
#define __tWxDrawPanel__

#include "wx/wx.h"

namespace Sig
{
	///
	/// \brief Encapsulates a single panel that can be drawn on using GDI
	class toolsgui_export tWxDrawPanel : public wxScrolledWindow
	{
	public:
		tWxDrawPanel( wxWindow* parent );
		~tWxDrawPanel( );
	};
}

#endif//#ifndef __tWxDrawPanel__