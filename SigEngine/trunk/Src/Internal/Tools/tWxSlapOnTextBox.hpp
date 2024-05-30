#ifndef __tWxSlapOnTextBox__
#define __tWxSlapOnTextBox__
#include "tWxSlapOnControl.hpp"

namespace Sig
{

	class tools_export tWxSlapOnTextBox : public tWxSlapOnControl
	{
	protected:
		wxTextCtrl*		mTextCtrl;
		std::string		mLastValue;

	public:

		tWxSlapOnTextBox( wxWindow* parent, const char* label, s32 widthOverride = -1, b32 labelIsButton = false );

		virtual void	fEnableControl( );
		virtual void	fDisableControl( );

		b32				fIsEmpty( ) const;
		std::string		fGetValue( ) const;
		void			fSetValue( const std::string& s );
		void			fSetToolTip( const wxString& toolTip );

	private:

		void			fOnTextBoxEnter( wxCommandEvent& );
		void			fOnTextBoxLostFocus( wxFocusEvent& );
		void			fOnTextModified( );
	};

}


#endif//__tWxSlapOnTextBox__
