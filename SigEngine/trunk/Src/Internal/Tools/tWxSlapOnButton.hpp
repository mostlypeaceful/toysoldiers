#ifndef __tWxSlapOnButton__
#define __tWxSlapOnButton__
#include "tWxSlapOnControl.hpp"

class wxButton;
class wxCommandEvent;

namespace Sig
{

	class tools_export tWxSlapOnButton : public tWxSlapOnControl
	{
	protected:
		wxButton*		mButton;

	public:

		tWxSlapOnButton( wxWindow* parent, const char* label, const char* spacerLabel = NULL );

		virtual void	fEnableControl( );
		virtual void	fDisableControl( );

		wxButton* fButton( ) const { return mButton; }

	private:

		void			fOnButtonPressedInternal( wxCommandEvent& );
	};

}

#endif//__tWxSlapOnButton__
