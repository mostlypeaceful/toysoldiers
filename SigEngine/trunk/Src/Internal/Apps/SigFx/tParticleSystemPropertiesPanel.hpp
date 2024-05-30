#ifndef __tParticleSystemPropertiesPanel__
#define __tParticleSystemPropertiesPanel__

#include "tWxToolsPanel.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"
#include "tWxSlapOnRadioBitmapButton.hpp"
#include "FxEditor/tSigFxAttractor.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnChoice.hpp"

namespace Sig
{

	class tCameraDepthSpinner;
	class tUpdateSpeedSpinner;
	class tLodFactorSlider;
	class tGhostParticleFrequencySpinner;
	class tGhostParticleLifetimeSpinner;
	//class tBurstModeCheckBox;
	class tSystemFlagsCheckListBox;
	class tBlendOpChoice;
	class tSrcBlendChoice;
	class tDstBlendChoice;
	class tSortModeChoice;
	class tEmitterShapeChoice;
	class tSigFxMainWindow;


	class tIgnoreAttractorChecklist :  public tWxSlapOnControl
	{
		wxCheckListBox* mCheckListBox;
		tEditableObjectContainer& mObjectList;
		tSigFxMainWindow* mSigFxMainWindow;

	public:

		tIgnoreAttractorChecklist( wxWindow* parent, const char* label, tEditableObjectContainer& list, tSigFxMainWindow* window );
		virtual void fEnableControl( );
		virtual void fDisableControl( );
		void fOnControlUpdated( wxCommandEvent& event );
		void fRefresh( );
	};



	class tParticleSystemPropertiesPanel : public tWxToolsPanelTool
	{
	public:

		tParticleSystemPropertiesPanel( tWxToolsPanel* parent, tEditableObjectContainer& list, tSigFxMainWindow* window );
		void fUpdateSelectedList( tEditorSelectionList& list );

	private:
		
		tEmitterShapeChoice* mEmitterShapeChoice;
		tBlendOpChoice* mBlendOpChoice;
		tSrcBlendChoice* mSrcBlendChoice;
		tDstBlendChoice* mDstBlendChoice;
		//tBurstModeCheckBox* mBurstModeCheckBox;
		tSystemFlagsCheckListBox* mSystemFlagsCheckList;
		tCameraDepthSpinner* mCameraDepthSpinner;
		tUpdateSpeedSpinner* mUpdateSpeedSpinner;
		tLodFactorSlider* mLodFactorSpinner;
		tGhostParticleFrequencySpinner* mGhostParticleFrequencySpinner;
		tGhostParticleLifetimeSpinner* mGhostParticleLifetimeSpinner;

		tSortModeChoice* mSortModeChoice;
		tIgnoreAttractorChecklist* mAttractorIgnoreChecklist;

		tGrowableArray< tSigFxParticleSystem* > mParticleSystemsList;
		tEditableObjectContainer& mObjectList;
		tSigFxMainWindow* mSigFxMainWindow;
	};
}

#endif	//__tParticleSystemPropertiesPanel__
