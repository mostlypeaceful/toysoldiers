#ifndef __tCreateNewEffectPanel__
#define __tCreateNewEffectPanel__
#include "tWxToolsPanel.hpp"

namespace Sig
{
	class tPlacedEffectsListBox;
	class tSigFxMainWindow;

	class tCreateNewEffectPanel : public tWxToolsPanelTool
	{
		tPlacedEffectsListBox* mEffectsChoice;

	public:
		tCreateNewEffectPanel( tWxToolsPanel* parent, tEditableObjectContainer& list, tSigFxMainWindow& mainWindow );
		void fRefresh( );
		void fUpdateSelectedList( tEditorSelectionList& list );
		void tCreateNewEffectPanel::fOnShowHideAll( wxCommandEvent& event );
	};
}

#endif//__tCreateNewEffectPanel__
