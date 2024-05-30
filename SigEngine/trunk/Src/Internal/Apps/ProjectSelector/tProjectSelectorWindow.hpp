#ifndef __tProjectSelectorWindow__
#define __tProjectSelectorWindow__
#include "tProjectSettingsDialog.hpp"

namespace Sig
{

	class tProjectSelectorWindow : public wxFrame
	{
	public:

		tProjectSelectorWindow(const wxString& title);
		~tProjectSelectorWindow( );
		void fOnClose(wxCloseEvent& event);
		void fOnAbout(wxCommandEvent& event);
		void fOnQuit(wxCommandEvent& event);
		void fSetActiveProject(wxCommandEvent& event);
		void fSetupDefaultsForActiveProject(wxCommandEvent& event);
		void fCompileGameSettings(wxCommandEvent& event);
		void fModifyGameSettings(wxCommandEvent& event);

		void fRollBackBuild(wxCommandEvent& event);

		void fOnNew(wxCommandEvent& event);
		void fOnEdit(wxCommandEvent& event);
		void fOnDelete(wxCommandEvent& event);

		void fOnListBoxDClick(wxMouseEvent& event);
		void fOnListBoxRUp(wxMouseEvent& event);

	private:

		void fCreateSettingsDialog( );
		ToolsPaths::tProjectProfile* fGetProfileAt( int index );
		void fAddProfile( const ToolsPaths::tProjectProfile& profile );
		void fEditCurrentProfile( );
		void fActivateProfile( int index );
		void fLoadProfiles( );
		void fSaveProfiles( );

		wxListBox*							mListBox;
		tStrongPtr<tProjectSettingsDialog>	mSettingsDialog;

	private:
		DECLARE_EVENT_TABLE()
	};

}

#endif//__tProjectSelectorWindow__

