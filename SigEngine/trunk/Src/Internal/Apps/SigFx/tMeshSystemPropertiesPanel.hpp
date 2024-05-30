#ifndef __tMeshSystemPropertiesPanel__
#define __tMeshSystemPropertiesPanel__
#include "tWxToolsPanel.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "tWxSlapOnRadioBitmapButton.hpp"
#include "FxEditor/tSigFxMeshSystem.hpp"
#include "tParticleSystemPropertiesPanel.hpp"

namespace Sig
{
	class tSigFxMainWindow;
	class tMeshSystemEmitterType;
	class tMeshSystemFlagsCheckListBox;
	class tMeshSystemSyncWithParticleSystemChoice;
	//class tMeshSystemResourceBrowserTree;

	class tMeshSystemPropertiesPanel : public tWxToolsPanelTool
	{
	public:
		tMeshSystemPropertiesPanel( tSigFxMainWindow* fxWindow, tEditableObjectContainer& list, tWxToolsPanel* parent );

		void fUpdateSelectedList( tEditorSelectionList& list );

		void fOnParticleMustBeInRadiusBasedCheckBoxChanged( wxCommandEvent& event );
		void fOnAffectParticleDirectionCheckBoxChanged( wxCommandEvent& event );

	private:
		
		tSigFxMainWindow* mSigFxMainWindow;
		tMeshSystemEmitterType* mMeshSystemEmitterChoice;
		tMeshSystemFlagsCheckListBox* mMeshSystemFlagsCheckList;
		tMeshSystemSyncWithParticleSystemChoice* mMeshSystemSyncWithParticleSystem;
		//tMeshSystemResourceBrowserTree* mMeshSystemResourceBrowser;
		tIgnoreAttractorChecklist* mAttractorIgnoreChecklist;

		tGrowableArray< tSigFxMeshSystem* > mMeshSystemsList;

	};
}

#endif	//__tMeshSystemPropertiesPanel__
