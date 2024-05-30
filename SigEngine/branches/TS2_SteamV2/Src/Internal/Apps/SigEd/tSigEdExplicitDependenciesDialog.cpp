#include "SigEdPch.hpp"
#include "tEditorAppWindow.hpp"
#include "tSigEdExplicitDependenciesDialog.hpp"

namespace Sig
{

	tSigEdExplicitDependenciesDialog::tSigEdExplicitDependenciesDialog( tEditorAppWindow* editorWindow )
		: tEditorDialog( editorWindow, "ExplicitDependenciesDialog" )
		, tExplicitDependenciesDialog( this )
		, mEditorWindow( editorWindow )
	{ }

	void tSigEdExplicitDependenciesDialog::fOnChanged( ) 
	{ 
		mEditorWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		mEditorWindow->SetFocus( );
	}

}

