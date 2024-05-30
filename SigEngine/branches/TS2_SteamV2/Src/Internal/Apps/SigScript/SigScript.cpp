#include "SigScriptPch.hpp"
#include "tSigScriptWindow.hpp"
#include <wx/cmdline.h>

namespace Sig
{

	static const wxCmdLineEntryDesc cCmdLineOptions[ ] =
	{
		{ wxCMD_LINE_SWITCH,	wxT("h"), wxT("help"), wxT("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
		{ wxCMD_LINE_SWITCH,	wxT("c"), wxT("cleanopen"), wxT("opens sigscript with no files pre-loaded") },
		{ wxCMD_LINE_OPTION,	wxT("f"), wxT("focusline"), wxT("focuses all pre-loaded scripts to the specified line number. the line number is zero-based."), wxCMD_LINE_VAL_NUMBER },
		{ wxCMD_LINE_PARAM,		0, 0, wxT("input file list"), wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE | wxCMD_LINE_PARAM_OPTIONAL },

		{ wxCMD_LINE_NONE }
	};

	///
	/// \brief Derived wxApp, basically an entry point into the wx widgets application.
	class tSigScriptApp : public wxApp
	{
		tSigScriptWindow* mMainFrame;
	public:

		///
		/// this one is called on application startup and is a good place for the app
		/// initialization (doing it here and not in the ctor allows to have an error
		/// return: if OnInit() returns false, the application terminates)
		virtual bool OnInit()
		{
			mMainFrame = new tSigScriptWindow(wxT("SigScript"));
			mMainFrame->Show( );
			SetTopWindow( mMainFrame );

			// This triggers the OnCmd stuff below.
			if( !wxApp::OnInit( ) )
				return false;
			
			return true;
		}

	private:
		void OnInitCmdLine(wxCmdLineParser& parser)
		{
			parser.SetDesc( cCmdLineOptions );
			parser.SetSwitchChars( "-" );
		}

		bool OnCmdLineParsed(wxCmdLineParser& parser)
		{
			// Skip loading things if this is supposed to be a clean open.
			const b32 doCleanOpen = parser.Found( "cleanopen" );
			if( doCleanOpen || parser.GetParamCount( ) > 0 )
				mMainFrame->fClearAll( );

			// Parse all unlabeled parameters. These are potential input files.
			for (u32 i = 0; i < parser.GetParamCount( ); i++)
			{
				tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( tFilePathPtr( parser.GetParam(i).c_str( ) ) );
				mMainFrame->fOpenDoc( absolutePath );
			}

			long lineNum;
			if( parser.Found( "focusline", &lineNum ) )
			{
				mMainFrame->fFocusAllOnLine( lineNum );
			}

			return true;
		}
	};

}

using namespace Sig;
IMPLEMENT_APP(tSigScriptApp)

