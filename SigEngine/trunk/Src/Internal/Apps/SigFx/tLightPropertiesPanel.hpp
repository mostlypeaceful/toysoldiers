#ifndef __tLightPropertiesPanel__
#define __tLightPropertiesPanel__
#include "tWxToolsPanel.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "tWxSlapOnRadioBitmapButton.hpp"
#include "FxEditor/tSigFxLight.hpp"

namespace Sig
{
	class tSigFxMainWindow;
	class tCastsShadowsCheckBox;
	class tShadowIntensitySpinner;

	class tLightPropertiesPanel : public tWxToolsPanelTool
	{
	public:
		tLightPropertiesPanel( tSigFxMainWindow* fxWindow, tWxToolsPanel* parent );

		void fUpdateSelectedList( tEditorSelectionList& list );

		void fOnParticleMustBeInRadiusBasedCheckBoxChanged( wxCommandEvent& event );
		void fOnAffectParticleDirectionCheckBoxChanged( wxCommandEvent& event );

	private:

		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxLight* > mLightList;

		tCastsShadowsCheckBox* mCastsShadowsCheckBox;
		tShadowIntensitySpinner* mShadowIntensitySpinner;
	};
}

#endif	//__tLightPropertiesPanel__