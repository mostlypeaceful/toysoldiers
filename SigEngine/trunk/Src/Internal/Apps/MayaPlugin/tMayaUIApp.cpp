#include "MayaPluginPch.hpp"
#include "tMayaUIApp.hpp"

namespace Sig
{

	namespace
	{

	const Sig::byte gSigEngineShelfCreation[]=
	{
#include "SigEngineShelfCreation.h"
	};

	}


	tMayaUIApp::tMayaUIApp( )
	{
	}

	tMayaUIApp::~tMayaUIApp( )
	{
	}

	void tMayaUIApp::fToggleShowExporterDialog( )
	{
		if( mExporterDialog ) mExporterDialog->fToggle( );
	}

	void tMayaUIApp::fToggleShowMatEd( )
	{
		if( mMatEd ) mMatEd->fToggle( );
	}

	void tMayaUIApp::fShowMatEd( const MString& mtlNodeName )
	{
		if( mMatEd )
		{
			if( mMatEd->fIsMinimized( ) )
				mMatEd->fRestoreFromMinimized( );
			else if( !mMatEd->IsShown( ) )
				mMatEd->Show( true );
		}
	}

	void tMayaUIApp::fOnMayaStartup( )
	{
		// create exporter dialog
		const tFilePathPtr sigEngineIconPathSmall = tFilePathPtr::fConstructPath( 
			ToolsPaths::fGetEngineBinFolder( ), 
			tFilePathPtr( "SigEngineIconSmall.bmp" ) );
		mExporterDialog.fReset( new tMayaExporterToolbox( sigEngineIconPathSmall ) );

		// create material editor
		mMatEd.fReset( new tMayaMatEdWindow( 0 ) );

		// register on-maya-quit callback
		mOnMayaQuit.fReset( new tMayaEvent( 
			tMayaEvent::fEventNameQuit( ), 
			make_delegate_memfn( tMayaEvent::tCallback, tMayaUIApp, fOnMayaQuit ) ) );

		// setup maya shelf
		std::stringstream melScriptStream;
		melScriptStream << gSigEngineShelfCreation;

		// execute shelf creation script
		MGlobal::executeCommandOnIdle( MString( melScriptStream.str( ).c_str( ) ), true );
	}

	void tMayaUIApp::fOnMayaQuit( )
	{
		mMatEd.fRelease( );

		OnExit( );
		wxEntryCleanup( );
	}

	bool tMayaUIApp::OnInit( )
	{
		return true;
	}

	int tMayaUIApp::OnExit( )
	{
		mExporterDialog.fRelease( );
		mMatEd.fRelease( );
		mOnMayaQuit.fRelease( );
		return wxApp::OnExit( );
	}

}
