#ifndef __tWxNumericText__
#define __tWxNumericText__

namespace Sig
{

	class tools_export tWxNumericText : public wxTextCtrl
	{
	protected:
		f32 mMin;
		f32 mMax;
		f32 mPrecision;
	public:
		tWxNumericText( wxWindow* parent, f32 min, f32 max, u32 precision, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize );
		void fSetValue( f32 value, b32 notify = true );
		f32 fGetValue( ) const;

		void fSetIndeterminate( );
		b32 fIsIndeterminate( ) const;

	protected:

		///
		/// \brief This method will be called when 'enter' is pressed.
		virtual void fOnValueUpdated( ) { }

		///
		/// \brief You can optionally override this method and
		/// call fOnTextModified( ) if you want to update values on focus-lost events
		virtual void fOnLostFocus( ) { }

		void fOnTextBoxEnter( wxCommandEvent& );
		void fOnTextBoxLostFocus( wxFocusEvent& );
		void fOnTextModified( );
	};

}

#endif//__tWxNumericText__

