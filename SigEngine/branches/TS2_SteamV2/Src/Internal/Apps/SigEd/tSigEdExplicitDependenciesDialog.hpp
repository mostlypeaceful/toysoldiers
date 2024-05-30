#ifndef __tSigEdExplicitDependenciesDialog__
#define __tSigEdExplicitDependenciesDialog__
#include "tExplicitDependenciesDialog.hpp"
#include "tEditorDialog.hpp"

namespace Sig
{
	class tEditorAppWindow;

	class tSigEdExplicitDependenciesDialog : public tEditorDialog, public tExplicitDependenciesDialog
	{
		tEditorAppWindow* mEditorWindow;

	protected:
		virtual void fOnChanged( );

	public:
		tSigEdExplicitDependenciesDialog( tEditorAppWindow* editorWindow );

	};

}

#endif//__tSigEdExplicitDependenciesDialog__
