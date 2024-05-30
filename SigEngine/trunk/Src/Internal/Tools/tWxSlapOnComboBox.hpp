#ifndef __tWxSlapOnComboBox__
#define __tWxSlapOnComboBox__
#include "tWxSlapOnControl.hpp"

class wxComboBox;
class wxCommandEvent;
class wxArrayString;

namespace Sig
{

	class tools_export tWxSlapOnComboBox : public tWxSlapOnControl
	{
		wxComboBox*		mComboBox;

	public:
		tWxSlapOnComboBox( wxWindow* parent, const char* label, wxArrayString* choices, s64 styleFlags = 0 );

		virtual void fEnableControl( );
		virtual void fDisableControl( );

		void			fSetSelection( u32 val );
		u32				fGetSelection( );
		std::string		fGetString( );
		
		void fClearChoices( );
		void fAddString( const wxString& addedString );

	private:

		void fOnControlUpdatedInternal( wxCommandEvent& );
	};

}

#endif//__tWxSlapOnComboBox__