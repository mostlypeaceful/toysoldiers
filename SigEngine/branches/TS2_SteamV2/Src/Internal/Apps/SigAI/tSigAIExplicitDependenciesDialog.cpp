#include "SigAIPch.hpp"
#include "tSigAIMainWindow.hpp"
#include "tSigAIExplicitDependenciesDialog.hpp"

namespace Sig
{

	tSigAIExplicitDependenciesDialog::tSigAIExplicitDependenciesDialog( tSigAIMainWindow* editorWindow )
		: tWxSlapOnDialog( "ExplicitDependencies", editorWindow )
		, tExplicitDependenciesDialog( this )
		, mEditorWindow( editorWindow )
	{
		fSetTopMost( true ); 
	}

	void tSigAIExplicitDependenciesDialog::fOnChanged( ) 
	{ 
		mEditorWindow->fEditorAction( ).fForceSetDirty( true );
		mEditorWindow->SetFocus( );
	}

}

