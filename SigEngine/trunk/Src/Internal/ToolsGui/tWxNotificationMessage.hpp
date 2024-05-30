#ifndef __tWxNotificationMessage__
#define __tWxNotificationMessage__

namespace Sig
{
	// To be replaced by the real wxNotificationMesasge in 2.9 if it ever gets
	// labeled as a stable release.

	class toolsgui_export tWxNotificationMessage : public wxDialog
	{
	public:
		tWxNotificationMessage( wxWindow* parent, const std::string& title, const std::string& text );
	};
}

#endif//__tWxNotificationMessage__
