#ifndef __tHeightFieldVertexPaintPanel__
#define __tHeightFieldVertexPaintPanel__
#include "tWxToolsPanel.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tHeightFieldVertexBrushSlider;
	class tHeightFieldPaintCursor;

	class tHeightFieldVertexPaintPanel : public tWxToolsPanelTool
	{
		tEditorAppWindow* mAppWindow;
		tHeightFieldVertexBrushSlider* mSizeSlider;
		tHeightFieldVertexBrushSlider* mStrengthSlider;
		tHeightFieldVertexBrushSlider* mFalloffSlider;
		tHeightFieldVertexBrushSlider* mShapeSlider;
	public:
		tHeightFieldVertexPaintPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent );
		void fAddCursorHotKeys( tHeightFieldPaintCursor* cursor );
		void fOnSlidersChanged( );
		void fUpdateParametersOnCursor( tHeightFieldPaintCursor* cursor );
		void fNudgeCursorSize( f32 delta );
		void fNudgeCursorStrength( f32 delta );
		void fNudgeCursorFalloff( f32 delta );
		void fNudgeCursorShape( f32 delta );
	};
}

#endif//__tHeightFieldVertexPaintPanel__
