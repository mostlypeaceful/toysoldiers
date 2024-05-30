#ifndef __tRollBackBuildDialog__
#define __tRollBackBuildDialog__
#include "tStrongPtr.hpp"
#include "tBrowseControl.hpp"

namespace Sig
{
	class tRollBackBuildDialog : public wxDialog
	{
	public:

		tRollBackBuildDialog( wxWindow* parent, tFilePathPtr buildsDirectory );
		~tRollBackBuildDialog( );

		void fOnClose(wxCloseEvent& event);
		void fOnCloseButtonClick(wxCommandEvent& event);
		void fOnListBoxDoubleClick(wxMouseEvent& event);

	private:
		tFilePathPtr mBuildsDirectory;
		tFilePathPtr mCurrentBuildFile;
		tFilePathPtr mCurrentChangelistFile;
		
		std::string mCurrentBuild;
		std::string mCurrentChangelist;

		wxListBox* mBuildList;

		std::string fReadLineFromFile( std::string& output, tFilePathPtr& path );

		void fWriteLineToFile( std::string& line, tFilePathPtr& path );

		void fRefreshBuildsList( );

		DECLARE_EVENT_TABLE()
	};

}

#endif//__tRollBackBuildDialog__
