#ifndef __tWxSlapOnPanel__
#define __tWxSlapOnPanel__

namespace Sig
{

	class tools_export tWxSlapOnPanel : public wxScrolledWindow
	{
	protected:
		wxString	mLabel;
		wxNotebook* mTabOwner;
	public:
		tWxSlapOnPanel( wxWindow* parent, const char* label );
		tWxSlapOnPanel( wxNotebook* parent, const char* label );
	protected:
		void fShow( );
		void fHide( );
		s32	 fGetTabPageIndex( );
	private:
		void fCommonCtor( );
	};

}

#endif//__tWxSlapOnPanel__
