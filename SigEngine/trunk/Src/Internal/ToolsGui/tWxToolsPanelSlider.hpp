#ifndef __tWxToolsPanelSlider__
#define __tWxToolsPanelSlider__
#include "tWxSlapOnControl.hpp"

namespace Sig
{
	class toolsgui_export tWxToolsPanelSlider : public wxSlider
	{
		wxWindow* mEditorWindow;
		wxStaticText* mDisplayValueText;
	public:
		tWxToolsPanelSlider( wxWindow* parent, const char* labelName, wxWindow* editorWindow, f32 initialValue = 0.5f, b32 showValueText = false );
		inline void fSetValue( f32 zeroToOne ) { SetValue( fRound<u32>( zeroToOne * 100 ) ); }
		inline f32	fGetValue( ) const { return GetValue( ) / 100.f; }
		void fSetDisplayValueText( f32 value );

	protected:
		virtual void fOnValueChanged( ) { }
		virtual void fOnScrollingChanged( ) { }

	private:
		void fScrollChanged( wxScrollEvent& event );
		void fOnScrolling( wxScrollEvent& event );
	};
}

#endif//__tWxToolsPanelSlider__
