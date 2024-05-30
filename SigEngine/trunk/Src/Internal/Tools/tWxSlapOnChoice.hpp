#ifndef __tWxSlapOnChoice__
#define __tWxSlapOnChoice__
#include "tWxSlapOnControl.hpp"

class wxChoice;
class wxCommandEvent;

namespace Sig
{

	class tools_export tWxSlapOnChoice : public tWxSlapOnControl
	{
		wxChoice*		mChoice;

	public:

		tWxSlapOnChoice( wxWindow* parent, const char* label, const wxString enumNames[], u32 numEnumNames, u32 defChoice = ~0 );

		virtual void fEnableControl( );
		virtual void fDisableControl( );

		void		fSetValue( u32 index );
		u32			fGetValue( );
		wxString	fGetValueString( );
		void		fSetToolTip( const wxString& toolTip );

	private:

		void fOnControlUpdatedInternal( wxCommandEvent& );
	};

}

#endif//__tWxSlapOnChoice__
