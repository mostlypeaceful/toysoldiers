#include "MayaPluginPch.hpp"
#include "Sigml.hpp"
#include "Sklml.hpp"
#include "Animl.hpp"
#include "tMayaSigmlQuickExport.hpp"
#include "MayaUtil.hpp"
#include "tPlatform.hpp"

namespace Sig
{
	tMayaSigmlQuickExport::tMayaSigmlQuickExport( wxWindow* parent, const char* label )
		: tWxSlapOnQuickExport( parent, label )
	{
		mSceneOpened.fReset( new tMayaEvent(
			tMayaEvent::fEventNameSceneOpened( ), make_delegate_memfn( tMayaEvent::tCallback, tMayaSigmlQuickExport, fOnSceneOpened ) ) );
		mSceneImported.fReset( new tMayaEvent(
			tMayaEvent::fEventNameSceneImported( ), make_delegate_memfn( tMayaEvent::tCallback, tMayaSigmlQuickExport, fOnSceneOpened ) ) );

		fOnSceneOpened( );
	}

	void tMayaSigmlQuickExport::fOnSceneOpened( )
	{
		std::string savedExportPath;
		b32 result = MayaUtil::fGetSceneAttribute( "SigmlQuickExport", savedExportPath );

		if( result && savedExportPath.length( ) > 0 )
		{
			fSetValue( wxString( savedExportPath ) );
			mDoQuick->SetValue( true );
		}
		else
		{
			fSetValue( wxString( cPathNotSet ) );
			mDoQuick->SetValue( false );
		}
	}

	wxString tMayaSigmlQuickExport::fGetFileWildcard( ) const
	{
		return wxString( "*.mshml;*.skin;*.sklml;*.animl" );
	}

	void tMayaSigmlQuickExport::fOnControlUpdated( )
	{
		// get whether to do a quick export or not
		const b32 quickExport = mDoQuick->GetValue( )!=0;

		// get the path from the control
		std::string path = fGetValue( ).c_str( );

		// save path on maya file
		MayaUtil::fSetSceneAttribute( "SigmlQuickExport", path.c_str( ) );
	}

	void tMayaSigmlQuickExport::fOnButtonPressed( )
	{
		// get whether to do a quick export or not
		const b32 quickExport = mDoQuick->GetValue( ) != 0;
		const b32 exportSelected = mDoSelected->GetValue( ) != 0;

		// get the path from the control
		std::string path = fGetValue( ).c_str( );
		const tFilePathPtr pathPath = tFilePathPtr( path );

		if( Win32Util::fIsFileReadOnly( pathPath ) )
			ToolsPaths::fCheckout( pathPath );

		std::string ext = StringUtil::fGetExtension( path.c_str( ), false );

		MString exporterType = ext.c_str( );

		// sanitize path for use as an argument in mel script
		const char slash[]={ fPlatformFilePathSlash( cCurrentPlatform ), '\0' };
		StringUtil::fReplaceAllOf( path, "/", slash );
		StringUtil::fReplaceAllOf( path, slash, "\\\\" );

		// execute mel script command to export the file
		MString exportCommand = 
			MString( "file -op \"\" -typ \"" + exporterType + "\" -pr" ) + MString( exportSelected ? " -es" : " -ea" ) +
			MString( quickExport ? " -f -pmt false \"" : " -pmt true \"" ) + 
			MString( path.c_str( ) ) + "\";";
		MGlobal::executeCommand( exportCommand );
	}

}
