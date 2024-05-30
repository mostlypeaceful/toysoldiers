#include "SigEdPch.hpp"
#include "tEditorDialog.hpp"
#include "tEditorAppWindow.hpp"

namespace Sig
{
	tEditorDialog::tEditorDialog( tEditorAppWindow* editorWindow, const char* regKeyName )
		: tWxSlapOnDialog( "", editorWindow, editorWindow->fGuiApp( ).fRegKeyName( ) + "\\" + regKeyName )
		, mEditorWindow( editorWindow )
	{
		fLoad( );
	}

	void tEditorDialog::fOnTick( )
	{
		if( fIsActive( ) )
			mEditorWindow->fSetDialogInputActive( );
	}

}
