#ifndef __tManipulateObjectPanel__
#define __tManipulateObjectPanel__
#include "tWxToolsPanel.hpp"
#include "tEditorHotKey.hpp"

namespace Sig
{
	class tSelectObjectCursorButton;
	class tEditorCursorControllerButtonGroup;

	class tManipulateObjectPanel : public tWxToolsPanelTool
	{
		tEditorCursorControllerButtonGroup* mButtonGroup;
		tSelectObjectCursorButton* mSelectObjectButton;
		tEditorHotKeyPtr mSelectHotKey;
		tEditorHotKeyPtr mTranslateHotKey;
		tEditorHotKeyPtr mRotateHotKey;
		tEditorHotKeyPtr mScaleHotKey;
	public:
		tManipulateObjectPanel( tWxToolsPanel* parent );
		void fSetSelectionCursor( );
		virtual void fOnTick( );
	};
}

#endif//__tManipulateObjectPanel__
