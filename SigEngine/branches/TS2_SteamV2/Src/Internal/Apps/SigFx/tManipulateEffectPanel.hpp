#ifndef __tManipulateEffectPanel__
#define __tManipulateEffectPanel__
#include "tWxToolsPanel.hpp"
#include "tEditorHotKey.hpp"
#include "FX/tFxGraph.hpp"
#include "tSigFxKeyline.hpp"

namespace Sig
{
	class tSelectObjectCursorButton;
	class tEditorCursorControllerButtonGroup;

	class tManipulateEffectPanel : public tWxToolsPanelTool
	{
		tEditorCursorControllerButtonGroup* mButtonGroup;
		tSelectObjectCursorButton* mSelectObjectButton;
		tEditorHotKeyPtr mSelectHotKey;
		tEditorHotKeyPtr mTranslateHotKey;
		tEditorHotKeyPtr mRotateHotKey;
		tEditorHotKeyPtr mScaleHotKey;
		
		tEditorHotKeyPtr mPositionOffsetHotKey;
		tEditorHotKeyPtr tEffectScaleHotKey;
		tEditorHotKeyPtr tEffectFollowMouseHotKey;

		tSigFxKeyline* mTheKeyline;

	public:
		tManipulateEffectPanel( tWxToolsPanel* parent, tSigFxKeyline* Keyline );
		~tManipulateEffectPanel( );
		void fSetSelectionCursor( );
		virtual void fOnTick( );
	};
}

#endif//__tManipulateEffectPanel__
