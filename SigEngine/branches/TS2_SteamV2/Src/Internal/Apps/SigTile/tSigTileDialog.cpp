#include "SigTilePch.hpp"
#include "tSigTileDialog.hpp"
#include "tSigTileMainWindow.hpp"

namespace Sig
{
	tSigTileDialog::tSigTileDialog( tSigTileMainWindow* mainWindow, const char* regKeyName )
		: tWxSlapOnDialog( "", mainWindow, mainWindow->fGuiApp( ).fRegKeyName( ) + "\\" + regKeyName )
		, mMainWindow( mainWindow )
	{
		fSetTopMost( true );
		fLoad( );
	}

	void tSigTileDialog::fOnTick( )
	{
		if( fIsActive( ) )
			mMainWindow->fSetDialogInputActive( );
	}

}
