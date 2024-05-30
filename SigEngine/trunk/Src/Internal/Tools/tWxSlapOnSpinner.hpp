#ifndef __tWxSlapOnSpinner__
#define __tWxSlapOnSpinner__
#include "tWxSlapOnControl.hpp"

class wxTextCtrl;
class wxSpinButton;
class wxSpinEvent;
class wxCommandEvent;
class wxFocusEvent;

namespace Sig
{

	class tools_export tWxSlapOnSpinner : public tWxSlapOnControl
	{
		wxTextCtrl*		mTextCtrl;
		wxSpinButton*	mSpinner;

	protected:
		f32 mLastValue;
		f32 mMin, mMax, mIncrement;
		u32 mPrecision;

		virtual void fOnControlUpdated( );

	public:

		tWxSlapOnSpinner( wxWindow* parent, const char* label, f32 min, f32 max, f32 increment, u32 precision, s32 widthOverride = -1 );

		virtual void fEnableControl( );
		virtual void fDisableControl( );
		virtual b32  fHasFocus( );

		f32		fGetValue( );
		void	fSetValue( f32 value );
		void	fSetToolTip( const wxString& toolTip );

	public:

		void fSetValueNoEvent( f32 value );
		void fSetIndeterminateNoEvent( );

	private:

		void fOnSpin( wxSpinEvent& );
		void fOnTextBoxEnter( wxCommandEvent& );
		void fOnTextBoxLostFocus( wxFocusEvent& );
		void fOnTextModified( );
	};

}

#endif//__tWxSlapOnSpinner__
