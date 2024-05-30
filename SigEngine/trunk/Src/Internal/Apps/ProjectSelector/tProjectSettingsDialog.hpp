#ifndef __tProjectSettingsDialog__
#define __tProjectSettingsDialog__
#include "ToolsPaths.hpp"
#include "tStrongPtr.hpp"
#include "tBrowseControl.hpp"

namespace Sig
{
	class tProjectSettingsDialog : public wxDialog
	{
		tStrongPtr<tFolderBrowseControl> mEnginePath;
		tStrongPtr<tFolderBrowseControl> mProjectPath;
		wxTextCtrl* mProfileName;
		wxTextCtrl* mWorkSpaceOverride;
        wxTextCtrl* mDeployToDir;
		wxTextCtrl* mPerforcePort;
		wxCheckBox* mActiveProject;
		wxCheckBox* mSyncChangelist;

	public:

		tProjectSettingsDialog( wxWindow* parent );
		~tProjectSettingsDialog( );

		b32 fDoDialog( ToolsPaths::tProjectProfile& projectProfile, b32 newProfile );

		void fOnClose(wxCloseEvent& event);
		void fOnOk(wxCommandEvent& event);
		void fOnCancel(wxCommandEvent& event);

	private:



		DECLARE_EVENT_TABLE()
	};

}

#endif//__tProjectSettingsDialog__
