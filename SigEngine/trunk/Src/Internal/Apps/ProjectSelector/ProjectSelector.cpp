#include "ProjectSelectorPch.hpp"
#include "tProjectSelectorWindow.hpp"
#include "tCmdLineOption.hpp"

namespace Sig
{
	namespace
	{
		static void fHandleExternalInstall( )
		{
			tGrowableArray<ToolsPaths::tProjectProfile> profiles;
			ToolsPaths::tProjectProfile::fLoadProfilesFromRegistry( profiles );

			// automatically determine the current profile
			const std::string exeName = Win32Util::fGetCurrentApplicationFileName( );
			const std::string engineDir = StringUtil::fUpNDirectories( exeName.c_str( ), 2 );
			const std::string projectDir = StringUtil::fUpNDirectories( engineDir.c_str( ), 1 );
			const std::string projectName = StringUtil::fNameFromPath( StringUtil::fStripExtension( projectDir.c_str( ) ).c_str( ) );
			ToolsPaths::tProjectProfile activeProfile;
			activeProfile.mEnginePath = tFilePathPtr( engineDir );
			activeProfile.mProjectPath = tFilePathPtr( projectDir );
			activeProfile.mProfileName = projectName;

			// set existing profiles to inactive
			for( u32 i = 0; i < profiles.fCount( ); ++i )
				profiles[ i ].mActiveProfile = false;

			// add new profile and set to active
			profiles.fFindOrAdd( activeProfile ).mActiveProfile = true;

			// save profiles to registry
			ToolsPaths::tProjectProfile::fSaveProfilesToRegistry( profiles );

			// configure environment for new profile
			activeProfile.fConfigureEnvironment( );
		}

		static void fHandleSetCurrentProject( const std::string& projectName )
		{
			log_line( 0, "Setting current project to [" << projectName << "]" );

			tGrowableArray<ToolsPaths::tProjectProfile> profiles;
			ToolsPaths::tProjectProfile::fLoadProfilesFromRegistry( profiles );

			// set existing profiles to inactive
			ToolsPaths::tProjectProfile* activeProfile = 0;
			for( u32 i = 0; i < profiles.fCount( ); ++i )
			{
				profiles[ i ].mActiveProfile = ( profiles[ i ].mProfileName == projectName );
				if( profiles[ i ].mActiveProfile )
					activeProfile = &profiles[ i ];
			}

			// save profiles to registry
			ToolsPaths::tProjectProfile::fSaveProfilesToRegistry( profiles );

			// configure environment for active profile
			if( activeProfile )
				activeProfile->fConfigureEnvironment( );
		}
	}


	///
	/// \brief Derived wxApp, basically an entry point into the wx widgets application.
	class tProjectSelectorApp : public wxApp
	{
	public:

		///
		/// this one is called on application startup and is a good place for the app
		/// initialization (doing it here and not in the ctor allows to have an error
		/// return: if OnInit() returns false, the application terminates)
		virtual bool OnInit()
		{
			const char* cmdLine = GetCommandLine( );
			const std::string cmdLineBuffer = cmdLine;

			const tCmdLineOption externalInstall( "externalInstall", cmdLineBuffer );
			if( externalInstall.fFound( ) )
			{
				fHandleExternalInstall( );
				return false;
			}

			const tCmdLineOption setCurrentProject( "setCurrentProject", cmdLineBuffer );
			if( setCurrentProject.fFound( ) )
			{
				fHandleSetCurrentProject( setCurrentProject.fGetOption( ) );
				return false;
			}

			wxFrame *frame = new tProjectSelectorWindow(wxT("SigEngine ProjectSelector"));
			frame->Show( );
			SetTopWindow( frame );
			return true;
		}
	};

}

using namespace Sig;
IMPLEMENT_APP(tProjectSelectorApp)

