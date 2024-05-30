#ifndef __tHeightFieldMaterialPaintPanel__
#define __tHeightFieldMaterialPaintPanel__
#include "tWxToolsPanel.hpp"
#include "tWxSlapOnTextBox.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tEditorCursorControllerButtonGroup;
	class tHeightFieldMaterialBrushSlider;
	class tHeightFieldPaintCursor;
	class tHeightFieldMaterialButton;
	class tWxSlapOnComboBox;
	class tWxSlapOnCheckBox;

	namespace Sigml { class tFile; }


	class tHeightFieldMaterialPaintPanel : public tWxToolsPanelTool
	{
		class tAtlasPathTextBox : public tWxSlapOnTextBox
		{
			tHeightFieldMaterialPaintPanel* mOwner;
		public:
			tAtlasPathTextBox( tHeightFieldMaterialPaintPanel* owner, wxWindow* parent, const char* label ) 
				: tWxSlapOnTextBox( parent, label, 150 ), mOwner( owner ) { }
			virtual void fOnControlUpdated( ) { mOwner->fOnAtlasChanged( this ); }
			void fOnBrowseForAtlas( wxCommandEvent& );
		};

		friend class tAtlasPathTextBox;

		tEditorAppWindow* mAppWindow;
		tWxSlapOnCheckBox* mHeightFieldMaterial;
		tAtlasPathTextBox* mDiffuseMapBrowser;
		tAtlasPathTextBox* mNormalMapBrowser;
		tEditorCursorControllerButtonGroup* mMaterialGroup;
		tDynamicArray<tHeightFieldMaterialButton*> mMaterialButtons;
		tFilePathPtr mDiffuseMapAtlas;
		tFilePathPtr mNormalMapAtlas;
		tHeightFieldMaterialBrushSlider* mTilingSlider;
		tHeightFieldMaterialBrushSlider* mSizeSlider;
		tHeightFieldMaterialBrushSlider* mStrengthSlider;
		tHeightFieldMaterialBrushSlider* mFalloffSlider;
		tHeightFieldMaterialBrushSlider* mShapeSlider;
		tHeightFieldMaterialBrushSlider* mLuminositySlider;
		tWxSlapOnComboBox* mFirstClearMat;
		tWxSlapOnComboBox* mSecondClearMat;
		tWxSlapOnComboBox* mThirdClearMat;
		tWxSlapOnComboBox* mFindMat;
		tWxSlapOnComboBox* mReplaceMat;
		wxArrayString mHFMaterialChoices;

	public:
		tHeightFieldMaterialPaintPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent );
		~tHeightFieldMaterialPaintPanel( );

		void fClearAtlases( );
		void fToSigmlFile( Sigml::tFile& sigml );
		void fFromSigmlFile( const Sigml::tFile& sigml );
		void fAcquireHeightFieldMaterialTileFactors( tDynamicArray<f32>& tilingFactors );

		void fAddCursorHotKeys( tHeightFieldPaintCursor* cursor );
		void fOnSlidersChanged( tHeightFieldMaterialBrushSlider* slider = 0 );
		void fUpdateParametersOnCursor( tHeightFieldPaintCursor* cursor );
		void fNudgeCursorSize( f32 delta );
		void fNudgeCursorStrength( f32 delta );
		void fNudgeCursorFalloff( f32 delta );
		void fNudgeCursorShape( f32 delta );
		void fSetTilingSlider( f32 value );
		f32 fTilingSlider( ) const;

		void fClearMaterials( u32& firstMat, u32& secondMat, u32& thirdMat ) const;
		void fFindReplaceMaterials( u32& findMat, u32& replaceMat ) const;

		void fOnHeightFieldMaterialChange( );

	private:
		void fClearAtlases( b32 diffuse, b32 normal );
		void fRefreshAtlases( b32 diffuse, b32 normal );
		void fOnAtlasChanged( tAtlasPathTextBox* control );
		void fOnClearTerrainTextures( wxCommandEvent& event );
	};

}

#endif//__tHeightFieldMaterialPaintPanel__
