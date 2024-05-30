#ifndef __tMeshSelectorDialog__
#define __tMeshSelectorDialog__
#include "tWxSlapOnDialog.hpp"

namespace Sig
{
	class tSigFxParticleSystem;
	class tSigFxMainWindow;
	class tMeshSystemResourceBrowserTree;
	class tEditorSelectionList;

	class tMeshSelectorDialog : public tWxSlapOnDialog
	{
		tSigFxMainWindow* mSigFx;
		tMeshSystemResourceBrowserTree* mMeshSystemResourceBrowser;
		wxStaticText* mSelectedMeshText;
		wxButton* mQuadStyleButton;
		tGrowableArray< tSigFxParticleSystem* > mParticleSystemsList;
		b32 mRefreshed;
	public:
		tMeshSelectorDialog( tSigFxMainWindow* parent, tEditorActionStack& actionStack, const std::string& regKeyName );
		void fOnTick( );
		void fUpdateSelectedList( tEditorSelectionList& list );
		void fUpdateText( );
	private:
		void fOnClearMesh( wxCommandEvent& event );
	};
}

#endif//__tMeshSelectorDialog__
