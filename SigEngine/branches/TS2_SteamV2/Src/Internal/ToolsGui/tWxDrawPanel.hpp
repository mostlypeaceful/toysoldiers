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

		void fEventPaint( wxPaintEvent& event );

		DECLARE_EVENT_TABLE()
	protected:
		virtual void fRender( wxDC& dc );
	};
}

#endif//#ifndef __tWxDrawPanel__