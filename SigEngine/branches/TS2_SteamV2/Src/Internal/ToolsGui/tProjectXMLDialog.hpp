#ifndef __tProjectXMLDialog__
#define __tProjectXMLDialog__
//#include <wx/dialog.h>
#include "tProjectFile.hpp"

class wxScrolledWindow;
class wxTextCtrl;
class wxCheckBox;

namespace Sig
{

	/// 
	/// \brief
	/// A dialog for modifying the project XML data.
	class toolsgui_export tProjectXMLDialog : public wxDialog
	{
	protected:
		tProjectFile mFile;
		wxComboBox *mFlags;
		wxComboBox *mGameEvents;
		wxComboBox *mKeyframeEvents;
		wxComboBox *mEnums;
		wxListBox  *mEnumValues;
		wxTextCtrl	*mNewValues;
		
		b8 mRequiresRebuildOfSigmls;
		b8 mRequiresStringReferenceUpdates;
		b8 mRequiresValidationOfEnumValueOrderDependentStuff;
		b8 mChanged;

		void fLoad( );
		void fSave( wxCommandEvent& );

		void fPopulateFlags( );
		void fPopulateGameEvents( );
		void fPopulateKeyframeEvents( );
		void fPopulateEnumTypes( );
		void fPopulateEnumValues( );

		void fOnEnumBoxChange( wxCommandEvent& event );
		void fOnEnumValueBoxChange( wxCommandEvent& event );
		tProjectFile::tGameEnumeratedType* fCurrentEnum( );
		void fOnAddFlag( wxCommandEvent& event );
		void fOnRemoveFlag( wxCommandEvent& event );
		void fOnRenameFlag( wxCommandEvent& event );
		void fOnAddEnum( wxCommandEvent& event );
		void fOnRemoveEnum( wxCommandEvent& event );
		void fOnRenameEnum( wxCommandEvent& event );
		void fOnAddGameEvent( wxCommandEvent& event );
		void fOnRemoveGameEvent( wxCommandEvent& event );
		void fOnRenameGameEvent( wxCommandEvent& event );
		void fOnAddKeyFrameEvent( wxCommandEvent& event );
		void fOnRemoveKeyFrameEvent( wxCommandEvent& event );
		void fOnRenameKeyFrame( wxCommandEvent& event );
		void fOnAddEnumValue( wxCommandEvent& event );
		void fOnInsertEnumValue( wxCommandEvent& event );
		wxString fInsertEnumValues( const wxString& newLineDelVals, u32 insertIndex );
		void fOnRemoveEnumValue( wxCommandEvent& event );
		void fRenameEnumValue( wxCommandEvent& event );
		void fMoveEnumValueUp( wxCommandEvent& event );
		void fMoveEnumValueDown( wxCommandEvent& event );
		void fMoveEnumValue( b32 up );
		void fOnClose(wxCloseEvent& event);
		void fCancel( wxCommandEvent& event );
		void fSetChanged( b32 changed );

	public:
		tProjectXMLDialog( wxWindow* parent );
	};
}

#endif//__tProjectXMLDialoge__
