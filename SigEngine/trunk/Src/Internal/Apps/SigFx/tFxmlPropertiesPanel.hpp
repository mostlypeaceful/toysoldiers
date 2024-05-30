#ifndef __tFxmlPropertiesPanel__
#define __tFxmlPropertiesPanel__

#include "tWxToolsPanel.hpp"
#include "tWxSlapOnCheckBox.hpp"

namespace Sig
{
	class tSigFxMainWindow;
	class tRandomStartTimeCheckbox;

	// This should probably be a Global Properties dialog to match SigEd but time is short.
	// These properties should be set on Fxml Deserialize event time possibility chance.
	class tFxmlPropertiesPanel : public tWxToolsPanelTool
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tRandomStartTimeCheckbox* mRandomStartTimeCheckbox;

	public:
		tFxmlPropertiesPanel( tWxToolsPanel* parent, tSigFxMainWindow* window );
		void fUpdateSelectedList( tEditorSelectionList& list );

		void fSetRandomStartTime( b32 set );
		b32 fRandomStartTime( ) const;

	private:
	};
}

#endif	//__tFxmlPropertiesPanel__
