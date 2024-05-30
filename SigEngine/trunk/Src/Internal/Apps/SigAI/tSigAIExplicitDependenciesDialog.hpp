#ifndef __tSigAIExplicitDependenciesDialog__
#define __tSigAIExplicitDependenciesDialog__
#include "tExplicitDependenciesDialog.hpp"
#include "tWxSlapOnDialog.hpp"

namespace Sig
{

	class tSigAIMainWindow;

	class tSigAIExplicitDependenciesDialog : public tWxSlapOnDialog, public tExplicitDependenciesDialog
	{
		tSigAIMainWindow* mEditorWindow;

	protected:
		virtual void fOnChanged( );

	public:
		tSigAIExplicitDependenciesDialog( tSigAIMainWindow* editorWindow );

	};

}

#endif//__tSigAIExplicitDependenciesDialog__
