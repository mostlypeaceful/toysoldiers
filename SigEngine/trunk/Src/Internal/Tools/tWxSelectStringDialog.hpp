#ifndef __tWxSelectStringDialog__
#define __tWxSelectStringDialog__

namespace Sig
{
	class tools_export tWxSelectStringDialog : public wxDialog
	{
		wxListBox* mStringListBox;
	public:
		tWxSelectStringDialog( wxWindow* parent, const char* title, const tGrowableArray<std::string>& names );
		std::string fGetResultString( ) const;
	private:
		void fOnDoubleClick( wxCommandEvent& );
	};
}

#endif//__tWxSelectStringDialog__
