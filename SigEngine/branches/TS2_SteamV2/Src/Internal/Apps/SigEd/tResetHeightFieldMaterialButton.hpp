#ifndef __tResetHeightFieldMaterialButton__
#define __tResetHeightFieldMaterialButton__
#include "tEditorDialog.hpp"
#include "tTextureSysRam.hpp"
#include "tEditorCursorControllerButton.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tResetHeightFieldMaterialButton;
	class tHeightFieldMaterialPaintPanel;

	class tResetHeightFieldMaterialButton : public tEditorCursorControllerButton
	{
		friend class tEditorAppWindow;
	private:
		tEditorAppWindow* mAppWindow;
		tHeightFieldMaterialPaintPanel* mButtonContainer;
		b32 mReplacementMode;

	public:
		tResetHeightFieldMaterialButton( 
			tEditorAppWindow* appWindow, 
			tHeightFieldMaterialPaintPanel* buttonContainer, 
			tEditorCursorControllerButtonGroup* parent, 
			const char* selectedBitmap = "PaintTerrainMaterialResetSel",
			const char* deselectedBitmap = "PaintTerrainMaterialResetDeSel",
			const char* tooltip = "Reset Brush - resets materials in the order of the reset materials specified below",
			b32 replacementMode = false );
		~tResetHeightFieldMaterialButton( );
		void fMaterials( u32& firstMat, u32& secondMat, u32& thirdMat ) const;
		void fFindReplaceMaterials( u32& findMat, u32& replaceMat ) const;
		virtual tEditorCursorControllerPtr fCreateCursorController( );


		inline tEditorAppWindow* fEditorWindow( ) const { return mAppWindow; }
	};

}

#endif//__tResetHeightFieldMaterialButton__
