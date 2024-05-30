#ifndef __tWxSlapOnQuickExport__
#define __tWxSlapOnQuickExport__
#include "tWxSlapOnControl.hpp"

class wxStaticText;
class wxButton;
class wxCheckBox;
class wxCommandEvent;

namespace Sig
{

	class tools_export tWxSlapOnQuickExport : public tWxSlapOnControl
	{
	protected:
		static const char cPathNotSet[];
		wxStaticText*	mPath;
		wxButton*		mExport;
		wxCheckBox*		mDoQuick;

	public:

		tWxSlapOnQuickExport( wxWindow* parent, const char* label );

		virtual void	fEnableControl( );
		virtual void	fDisableControl( );

		wxString		fGetValue( );
		void			fSetValue( const char* path );

	protected:

		virtual wxString fGetFileWildcard( ) const;

	private:

		void			fOnButtonPressedInternal( wxCommandEvent& );
		virtual void	fOnButtonPressed( ) { }
	};

}

#endif//__tWxSlapOnQuickExport__
