//------------------------------------------------------------------------------
// \file tManipulateObjectPanel.hpp - 23 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tManipulateObjectPanel__
#define __tManipulateObjectPanel__
#include "tWxToolsPanel.hpp"
#include "tEditorHotKey.hpp"

namespace Sig
{
	class tGizmoCursorButton;
	class tEditorCursorControllerButtonGroup;

	///
	/// \class tManipulateObjectPanel
	/// \brief 
	class tManipulateObjectPanel : public tWxToolsPanelTool 
	{

	public:

		tManipulateObjectPanel( tWxToolsPanel * parent );

		virtual void fOnTick( );
		void fSetSelectionCursor( );

	private:

		tFixedArray<tGizmoCursorButton*,3> mGizmos;
		tEditorCursorControllerButtonGroup* mButtonGroup;

		tEditorHotKeyPtr mSelectHotKey;
		tEditorHotKeyPtr mTranslateHotKey;
		tEditorHotKeyPtr mRotateHotKey;
		tEditorHotKeyPtr mScaleHotKey;
	};
}

#endif//__tManipulateObjectPanel__
