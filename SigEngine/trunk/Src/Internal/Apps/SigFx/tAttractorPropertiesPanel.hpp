#ifndef __tAttractorPropertiesPanel__
#define __tAttractorPropertiesPanel__
#include "tWxToolsPanel.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "tWxSlapOnRadioBitmapButton.hpp"
#include "FxEditor/tSigFxAttractor.hpp"

namespace Sig
{
	class tSigFxMainWindow;
	class tAttractorTypeChoice;
	class tMustBeInRadiusCheckBox;
	class tAffectsParticleDirectionCheckBox;
	class tAttractorFlagsCheckListBox;

	class tAttractorPropertiesPanel : public tWxToolsPanelTool
	{
	public:
		tAttractorPropertiesPanel( tSigFxMainWindow* fxWindow, tWxToolsPanel* parent );

		void fUpdateSelectedList( tEditorSelectionList& list );

		void fOnParticleMustBeInRadiusBasedCheckBoxChanged( wxCommandEvent& event );
		void fOnAffectParticleDirectionCheckBoxChanged( wxCommandEvent& event );

	private:
		
		tSigFxMainWindow* mSigFxMainWindow;
		tAttractorTypeChoice* mAttractorTypeChoice;
		
		tMustBeInRadiusCheckBox* mParticleMustBeInRadiusCheckBox;
		tAffectsParticleDirectionCheckBox* mAffectParticleDirection;

		tAttractorFlagsCheckListBox*	mFlagsCheckListBox;

		tGrowableArray< tSigFxAttractor* > mAttractorList;
	};
}

#endif	//__tAttractorPropertiesPanel__
