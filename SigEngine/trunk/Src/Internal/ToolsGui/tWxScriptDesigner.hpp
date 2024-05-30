#ifndef __tWxScriptDesigner__
#define __tWxScriptDesigner__
#include "tStrongPtr.hpp"
#include <wx/stc/stc.h>
#include "tScriptFileConverter.hpp"

namespace Sig
{

	class tWxTextEditor;

	class toolsgui_export tWxScriptDesigner : public wxScrolledWindow
	{
		wxWindow*		mParentPanel;
		tWxTextEditor*  mAssociatedText;

	public:
		tWxScriptDesigner( 
			wxWindow* parent,
			tWxTextEditor* associatedText,
			wxWindowID id = wxID_ANY,
			const wxPoint& pos = wxDefaultPosition,
			const wxSize& size = wxDefaultSize,
			long style = 0,
			const wxString& name = wxString( "Default Text Editor Control" ) );
		
		void fParseScript( );

	private:
		void fPushPad( s32 columnsUsed );
		void fPushGroup( const wxString& groupName );
		wxStaticText* fMakeLabel( const std::string& description );
		void fPushTextField( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex );
		void fPushFloatSpin( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex );
		void fPushIntSpin( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex );
		void fPushComboBox( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex );
		void fPushFilename( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex );
		void fPushVector( tScriptFileConverter::tExportedVariable& variable, u32 varIndex );

		void fOnFileBrowse( wxCommandEvent& event );
		void fTextChanged( wxCommandEvent& event );
		void fComboSelected( wxCommandEvent& event );
		void fIntChanged( wxSpinEvent& event );
		void fIntChanged( wxCommandEvent& event );
		void fFloatChanged( wxSpinEvent& event );
		void fVectorChanged( wxSpinEvent& event );

		void fPostVariable( const tScriptFileConverter::tExportedVariable& variable );

		tGrowableArray< tScriptFileConverter::tExportedVariable > mVariables;
		tGrowableArray< wxTextCtrl* > mTextControls;

		DECLARE_EVENT_TABLE()
	};
}

#endif//__tWxScriptDesigner__
