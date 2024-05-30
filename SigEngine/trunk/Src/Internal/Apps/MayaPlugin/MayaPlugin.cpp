#include "MayaPluginPch.hpp"
#include "tMayaUIApp.hpp"
#include "tMayaAnimlExporter.hpp"
#include "tMayaDermlMaterial.hpp"
#include "tMayaSigmlExporter.hpp"
#include "tMayaSklmlExporter.hpp"
#include "Threads/tThread.hpp"
#include <maya/MFnPlugin.h>
#include "../Tools/WxUtil.hpp"
using namespace Sig;

IMPLEMENT_APP_NO_MAIN(tMayaUIApp)

namespace Sig { namespace WxUtil
{
	// 02/08/12 KJC: Anything beyond Maya 2010 has an issue with properly refreshing the wxWidgets file dialogs - the current solution is to use Maya's built-in dialog
	b32 fMayaBrowseForFile( std::string& pathOut, wxWindow* parent, const char* caption, const char* defDir, const char* defFile, const char* filter, u32 style )
	{
		b32 retVal = false;

		// Don't let them open more than one instance of the dialog
		// for this control
		parent->Freeze( );

		// The path needs to be properly escaped in the command string
		wxString defDirStr( defDir );
		defDirStr.Replace("\\", "\\\\");

		// Format the file filter the way Maya expects it
		wxString fileFilter(filter);
		fileFilter.Replace( ";", " " );
		wxString formattedFilter(filter);
		formattedFilter += " (";
		formattedFilter += fileFilter;
		formattedFilter += ")";

		// Command reference: http://download.autodesk.com/us/maya/2011help/Commands/fileDialog2.html
		wxString command = "fileDialog2";
		if (style == wxFD_OPEN)
		{
			command += " -caption \"" + wxString( caption ) + "\"";
			command += " -fileMode 1";
			command += " -startingDirectory \"" + wxString( defDirStr ) + "\"";
			command += " -returnFilter 0";
			command += " -fileFilter \"" + wxString( formattedFilter ) + "\"";
		}

		MCommandResult result;
		MStringArray selectedFiles;
		MGlobal::executeCommand( MString( command ), result );
		result.getResult( selectedFiles );

		if ( selectedFiles.length( ) > 0 )
		{
			log_line( 0, "Selected: " + selectedFiles[ 0 ] );
			pathOut = selectedFiles[ 0 ].asChar( );
			retVal = true;
		}

		parent->Thaw( );
		return retVal;
	}
}}

namespace
{
	void fLogFunction( const char* text, u32 flag )
	{
		// maya's cout seems to require that
		// we explicitly flush the stream...
		cout << text;
		cout.flush( );
	}

	struct tMayaPlugins
	{
		declare_singleton( tMayaPlugins );
		void fInitialize( MObject& mayaObject, MFnPlugin& mayaPlugin )
		{
			mAniml.fOnMayaInitialize( mayaObject, mayaPlugin );
			mDerml.fOnMayaInitialize( mayaObject, mayaPlugin );
			mSigml.fOnMayaInitialize( mayaObject, mayaPlugin );
			mSklml.fOnMayaInitialize( mayaObject, mayaPlugin );
		}
		void fShutdown( MObject& mayaObject, MFnPlugin& mayaPlugin )
		{
			mAniml.fOnMayaShutdown( mayaObject, mayaPlugin );
			mDerml.fOnMayaShutdown( mayaObject, mayaPlugin );
			mSigml.fOnMayaShutdown( mayaObject, mayaPlugin );
			mSklml.fOnMayaShutdown( mayaObject, mayaPlugin );
		}

		tAnimlMayaPlugin mAniml;
		tDermlMayaPlugin mDerml;
		tSigmlMayaPlugin mSigml;
		tSklmlMayaPlugin mSklml;
	};
}

///
/// \brief Simple maya command for toggling the exporter panel on/off.
class tToggleExporterPanel : public MPxCommand
{
public:
	virtual MStatus	doIt ( const MArgList& )
	{
		if( wxTheApp ) wxGetApp( ).fToggleShowExporterDialog( );
		return MS::kSuccess; 
	}
	static void* fCreate( ) { return new tToggleExporterPanel; }
	static const char* fGetCommandText( ) { return "SigToggleExporterPanel"; }
};

///
/// \brief Simple maya command for toggling the material editor on/off.
class tToggleMatEd : public MPxCommand
{
public:
	virtual MStatus	doIt ( const MArgList& args )
	{
		if( wxTheApp )
		{
			MString mtlNodeName;
			for( u32 i = 0; i < args.length(); ++i )
				if( MString( "-mtl" ) == args.asString( i ) )
					mtlNodeName = args.asString( ++i );

			if( mtlNodeName.length( ) > 0 )
				wxGetApp( ).fShowMatEd( mtlNodeName );
			else
				wxGetApp( ).fToggleShowMatEd( );
		}
		return MS::kSuccess; 
	}
	static void* fCreate( ) { return new tToggleMatEd; }
	static const char* fGetCommandText( ) { return "SigToggleMatEd"; }
};

///
/// \brief Required maya exported dll function for initializing the plugin.
dll_export MStatus initializePlugin( MObject mayaObj )
{
	sigassert( Sig::Threads::tThread::fMainThreadId( ) == Sig::Threads::tThread::fCurrentThreadId( ) );

	Sig::WxUtil::gBrowseForFile = &Sig::WxUtil::fMayaBrowseForFile;

	// create maya plugin object in order to register all maya plugins
	MFnPlugin mayaPlugin( mayaObj, "SignalStudios", "0.0", "Any" );

	Log::fSetLogFilterMask( Log::cFlagNetwork );
	Log::fAddOutputFunction( fLogFunction );

	// init plugins
	tMayaPlugins::fInstance( ).fInitialize( mayaObj, mayaPlugin );

	// register all simple maya commands
	mayaPlugin.registerCommand( tToggleExporterPanel::fGetCommandText( ), tToggleExporterPanel::fCreate );
	mayaPlugin.registerCommand( tToggleMatEd::fGetCommandText( ), tToggleMatEd::fCreate );

	// intialize wxWidget-based UI
	int argc=0; char* argv[] = {""};
	if( wxEntryStart( argc, argv ) )
		wxGetApp( ).fOnMayaStartup( );

	tMayaEvent::fLogEventNames( );

	return MStatus::kSuccess;
}


///
/// \brief Required maya exported dll function for shutting down the plugin.
dll_export MStatus uninitializePlugin( MObject mayaObj ) 
{
	// create maya plugin object in order to shut everything down
	MFnPlugin mayaPlugin( mayaObj );

	// unregister all simple maya commands
	mayaPlugin.deregisterCommand( tToggleExporterPanel::fGetCommandText( ) );

	// shutdown plugins
	tMayaPlugins::fInstance( ).fShutdown( mayaObj, mayaPlugin );

	// shutdown wxWidget-based UI
	if( wxTheApp )
	{
		wxTheApp->OnExit( );
		wxEntryCleanup( );
	}

	return MStatus::kSuccess;
}
