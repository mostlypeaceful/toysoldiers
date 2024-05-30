#ifndef __tWxSlapOnControl__
#define __tWxSlapOnControl__
#include "tWxAutoDelete.hpp"
#include "wx/gbsizer.h"

class wxBoxSizer;
class wxStaticText;

namespace Sig
{

	class tools_export tWxSlapOnGridSizer : public tWxAutoDelete
	{
	public:
		tWxSlapOnGridSizer( wxWindow* parent, u32 columns, u32 leftPadding );
		virtual ~tWxSlapOnGridSizer( );

		wxGridSizer*	fGrid( ) const { return mGrid; }
		wxSizer*		fGetSizer( ) const { return mGrid; }

	private:
		wxGridSizer*	mGrid;
	};

	class tools_export tWxSlapOnControl : public wxEvtHandler, public tWxAutoDelete
	{
	private:
		static u32 gLabelWidth;
		static u32 gControlWidth;
		static u32 gSizerPadding;
	public:
		static void fSetLabelWidth( u32 width ) { gLabelWidth = width; }
		static void fSetControlWidth( u32 width ) { gControlWidth = width; }
		static u32 fLabelWidth( ) { return gLabelWidth; }
		static u32 fControlWidth( ) { return gControlWidth; }
		static u32 fSizerPadding( ) { return gSizerPadding; }

	private:
		wxControl*	mLabel;

	protected:
		wxBoxSizer*		mSizer;

	public:
		tWxSlapOnControl( wxWindow* parent, const char* label, b32 labelIsButton = false, tWxSlapOnGridSizer* parentSizer = NULL );
		virtual ~tWxSlapOnControl( );

		wxSizer* fGetSizer( ) const { return mSizer; }

		wxString fGetLabelText( );
		void SetToolTip( const wxString& toolTip );

		virtual void fEnableControl( )		= 0;
		virtual void fDisableControl( )		= 0;

		virtual void fOnLabelButtonClicked( wxCommandEvent& event ) { }

		void fAddWindowToSizer( wxWindow* window, b32 rightMost );

		static u32 fGlobalLabelWidth( ) { return gLabelWidth; }
		static u32 fGlobalSizerPadding( ) { return gSizerPadding; }

	protected:
		virtual void fOnControlUpdated( ) { }
	};

}


#endif//__tWxSlapOnControl__
