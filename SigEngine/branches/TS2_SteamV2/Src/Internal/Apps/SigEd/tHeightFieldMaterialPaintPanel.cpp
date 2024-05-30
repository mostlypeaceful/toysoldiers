#include "SigEdPch.hpp"
#include "tHeightFieldMaterialPaintPanel.hpp"
#include "tHeightFieldPaintCursor.hpp"
#include "tHeightFieldMaterialButton.hpp"
#include "tResetHeightFieldMaterialButton.hpp"
#include "tWxToolsPanelSlider.hpp"
#include "tEditorAppWindow.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Tatml.hpp"
#include "tWxSlapOnComboBox.hpp"

namespace Sig
{
	namespace
	{
		const f32 cNudgeAmount = 0.1f;
	}
	
	class tHeightFieldMaterialBrushSlider : public tWxToolsPanelSlider
	{
		tHeightFieldMaterialPaintPanel* mParent;
	public:
		tHeightFieldMaterialBrushSlider( tHeightFieldMaterialPaintPanel* parent, wxWindow* windowParent, const char* labelName, f32 initialValue = 0.5f )
			: tWxToolsPanelSlider( windowParent, labelName, &parent->fGuiApp( ).fMainWindow( ), initialValue )
			, mParent( parent ) { }
		virtual void fOnValueChanged( ) { mParent->fOnSlidersChanged( this ); }
	};




	class tTerrainCursorMaterialSizeIncHotKey : public tEditorHotKey
	{
		tHeightFieldMaterialPaintPanel* mOwner;
	public:
		tTerrainCursorMaterialSizeIncHotKey( tHeightFieldMaterialPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton1, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorSize( +cNudgeAmount ); }
	};
	class tTerrainCursorMaterialSizeDecHotKey : public tEditorHotKey
	{
		tHeightFieldMaterialPaintPanel* mOwner;
	public:
		tTerrainCursorMaterialSizeDecHotKey( tHeightFieldMaterialPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton1, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorSize( -cNudgeAmount ); }
	};

	class tTerrainCursorMaterialStrengthIncHotKey : public tEditorHotKey
	{
		tHeightFieldMaterialPaintPanel* mOwner;
	public:
		tTerrainCursorMaterialStrengthIncHotKey( tHeightFieldMaterialPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton2, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorStrength( +cNudgeAmount ); }
	};
	class tTerrainCursorMaterialStrengthDecHotKey : public tEditorHotKey
	{
		tHeightFieldMaterialPaintPanel* mOwner;
	public:
		tTerrainCursorMaterialStrengthDecHotKey( tHeightFieldMaterialPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton2, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorStrength( -cNudgeAmount ); }
	};

	class tTerrainCursorMaterialFalloffIncHotKey : public tEditorHotKey
	{
		tHeightFieldMaterialPaintPanel* mOwner;
	public:
		tTerrainCursorMaterialFalloffIncHotKey( tHeightFieldMaterialPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton3, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorFalloff( +cNudgeAmount ); }
	};
	class tTerrainCursorMaterialFalloffDecHotKey : public tEditorHotKey
	{
		tHeightFieldMaterialPaintPanel* mOwner;
	public:
		tTerrainCursorMaterialFalloffDecHotKey( tHeightFieldMaterialPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton3, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorFalloff( -cNudgeAmount ); }
	};

	class tTerrainCursorMaterialShapeIncHotKey : public tEditorHotKey
	{
		tHeightFieldMaterialPaintPanel* mOwner;
	public:
		tTerrainCursorMaterialShapeIncHotKey( tHeightFieldMaterialPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton4, 0 ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorShape( +cNudgeAmount ); }
	};
	class tTerrainCursorMaterialShapeDecHotKey : public tEditorHotKey
	{
		tHeightFieldMaterialPaintPanel* mOwner;
	public:
		tTerrainCursorMaterialShapeDecHotKey( tHeightFieldMaterialPaintPanel* owner ) 
			: tEditorHotKey( owner->fGuiApp( ).fHotKeys( ), Input::tKeyboard::cButton4, tEditorHotKey::cOptionShift ), mOwner( owner ) { }
		virtual void fFire( ) const { mOwner->fNudgeCursorShape( -cNudgeAmount ); }
	};

	tHeightFieldMaterialPaintPanel::tHeightFieldMaterialPaintPanel( tEditorAppWindow* appWindow, tWxToolsPanel* parent )
		: tWxToolsPanelTool( parent, "Terrain Material Painting", "Terrain Material Painting", "TerrainPaint" )
		, mAppWindow( appWindow )
		, mDiffuseMapBrowser( 0 )
		, mNormalMapBrowser( 0 )
		, mMaterialGroup( 0 )
		, mTilingSlider( 0 )
		, mSizeSlider( 0 )
		, mStrengthSlider( 0 )
		, mFalloffSlider( 0 )
		, mShapeSlider( 0 )
		, mLuminositySlider( 0 )
	{
		tWxSlapOnGroup* browseGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Atlases", false );
		browseGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		mDiffuseMapBrowser = new tAtlasPathTextBox( this, browseGroup->fGetMainPanel( ), "DiffuseMap" );
		wxButton* browseDiffuse = new wxButton( browseGroup->fGetMainPanel( ), wxID_ANY, "...", wxDefaultPosition, wxSize( 22, 20 ) );
		browseDiffuse->SetForegroundColour( wxColour( 0x22, 0x22, 0xff ) );
		mDiffuseMapBrowser->fAddWindowToSizer( browseDiffuse, true );
		browseDiffuse->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tHeightFieldMaterialPaintPanel::tAtlasPathTextBox::fOnBrowseForAtlas ), NULL, mDiffuseMapBrowser );

		mNormalMapBrowser = new tAtlasPathTextBox( this, browseGroup->fGetMainPanel( ), "NormalMap" );
		wxButton* browseNormal = new wxButton( browseGroup->fGetMainPanel( ), wxID_ANY, "...", wxDefaultPosition, wxSize( 22, 20 ) );
		browseNormal->SetForegroundColour( wxColour( 0x22, 0x22, 0xff ) );
		mNormalMapBrowser->fAddWindowToSizer( browseNormal, true );
		browseNormal->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tHeightFieldMaterialPaintPanel::tAtlasPathTextBox::fOnBrowseForAtlas ), NULL, mNormalMapBrowser );

		browseGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		mMaterialGroup = new tEditorCursorControllerButtonGroup( this, "Material Brushes", false, 4 );

		tWxSlapOnGroup* mtlPropsGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Material Properties", false );
		mTilingSlider = new tHeightFieldMaterialBrushSlider( this, mtlPropsGroup->fGetMainPanel( ), "Tiling", 0.5f );
		mtlPropsGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		tWxSlapOnGroup* propsGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Brush Properties", false );
		mSizeSlider = new tHeightFieldMaterialBrushSlider( this, propsGroup->fGetMainPanel( ), "Size", 0.25f );
		mStrengthSlider = new tHeightFieldMaterialBrushSlider( this, propsGroup->fGetMainPanel( ), "Strength", 0.50f );
		mFalloffSlider = new tHeightFieldMaterialBrushSlider( this, propsGroup->fGetMainPanel( ), "Focus", 0.25f );
		mShapeSlider = new tHeightFieldMaterialBrushSlider( this, propsGroup->fGetMainPanel( ), "Shape", 0.00f );
		mLuminositySlider = new tHeightFieldMaterialBrushSlider( this, propsGroup->fGetMainPanel( ), "Luminance", 1.0f );
		propsGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		tEditorCursorControllerButtonGroup* resetBrushGroup = new tEditorCursorControllerButtonGroup( this, "Special Brushes", false );

		new tResetHeightFieldMaterialButton( mAppWindow, this, resetBrushGroup );
		new tResetHeightFieldMaterialButton( 
			mAppWindow, 
			this, 
			resetBrushGroup, 
			"PaintTerrainMaterialFindReplaceSel", 
			"PaintTerrainMaterialFindReplaceDeSel",
			"Find And Replace Brush - searches for the specified material and fills it with the replacement",
			true );

		tWxSlapOnGroup* clearGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Reset Materials", false );
		mFirstClearMat = new tWxSlapOnComboBox( clearGroup->fGetMainPanel( ), "First Material", 0, wxCB_READONLY | wxCB_DROPDOWN );
		mSecondClearMat = new tWxSlapOnComboBox( clearGroup->fGetMainPanel( ), "Second Material", 0, wxCB_READONLY | wxCB_DROPDOWN );
		mThirdClearMat = new tWxSlapOnComboBox( clearGroup->fGetMainPanel( ), "Third Material", 0, wxCB_READONLY | wxCB_DROPDOWN );
		wxButton* cleanButton = new wxButton( clearGroup->fGetMainPanel( ), wxID_ANY, "Fill", wxDefaultPosition, wxSize( mFirstClearMat->fLabelWidth( ), wxDefaultSize.y ) );
		cleanButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tHeightFieldMaterialPaintPanel::fOnClearTerrainTextures ), NULL, this );
		clearGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 4 );
		clearGroup->fGetMainPanel( )->GetSizer( )->Add( cleanButton, 0, wxALIGN_RIGHT | wxRIGHT, 76 );
		clearGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );

		tWxSlapOnGroup* findReplaceGroup = new tWxSlapOnGroup( fGetMainPanel( ), "Find And Replace Materials", false );
		mFindMat = new tWxSlapOnComboBox( findReplaceGroup->fGetMainPanel( ), "Find Material", 0, wxCB_READONLY | wxCB_DROPDOWN );
		mReplaceMat = new tWxSlapOnComboBox( findReplaceGroup->fGetMainPanel( ), "Replace Material", 0, wxCB_READONLY | wxCB_DROPDOWN );
	}

	tHeightFieldMaterialPaintPanel::~tHeightFieldMaterialPaintPanel( )
	{
	}

	void tHeightFieldMaterialPaintPanel::fClearAtlases( )
	{
		if( mMaterialButtons.fCount( ) > 0 )
			mAppWindow->fSetSelectionCursor( );
		fClearAtlases( true, true );
		mDiffuseMapBrowser->fSetValue( "" );
		mNormalMapBrowser->fSetValue( "" );
	}

	void tHeightFieldMaterialPaintPanel::fToSigmlFile( Sigml::tFile& sigml )
	{
		sigml.mDiffuseMapAtlas = mDiffuseMapAtlas;
		sigml.mNormalMapAtlas = mNormalMapAtlas;
		fAcquireHeightFieldMaterialTileFactors( sigml.mHeightFieldMaterialTileFactors );
	}

	void tHeightFieldMaterialPaintPanel::fFromSigmlFile( const Sigml::tFile& sigml )
	{
		fClearAtlases( true, true );

		mDiffuseMapAtlas = sigml.mDiffuseMapAtlas;
		mNormalMapAtlas = sigml.mNormalMapAtlas;

		mDiffuseMapBrowser->fSetValue( mDiffuseMapAtlas.fCStr( ) );
		mNormalMapBrowser->fSetValue( mNormalMapAtlas.fCStr( ) );

		fRefreshAtlases( true, true );

		for( u32 i = 0; i < mMaterialButtons.fCount( ); ++i )
			if( i < sigml.mHeightFieldMaterialTileFactors.fCount( ) )
				mMaterialButtons[ i ]->fSetTileFactor( sigml.mHeightFieldMaterialTileFactors[ i ] );
	}

	void tHeightFieldMaterialPaintPanel::fAcquireHeightFieldMaterialTileFactors( tDynamicArray<f32>& tilingFactors )
	{
		tilingFactors.fNewArray( mMaterialButtons.fCount( ) );
		for( u32 i = 0; i < tilingFactors.fCount( ); ++i )
			tilingFactors[ i ] = mMaterialButtons[ i ]->fTileFactor( );
	}

	void tHeightFieldMaterialPaintPanel::fAddCursorHotKeys( tHeightFieldPaintCursor* cursor )
	{
		if( !cursor )
			return;
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorMaterialSizeIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorMaterialSizeDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorMaterialStrengthIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorMaterialStrengthDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorMaterialFalloffIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorMaterialFalloffDecHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorMaterialShapeIncHotKey( this ) ) );
		cursor->fAddHotKey( tEditorHotKeyPtr( new tTerrainCursorMaterialShapeDecHotKey( this ) ) );
	}

	void tHeightFieldMaterialPaintPanel::fOnSlidersChanged( tHeightFieldMaterialBrushSlider* slider )
	{
		tEditorCursorControllerPtr cursor = fGuiApp( ).fCurrentCursor( );
		fUpdateParametersOnCursor( dynamic_cast<tHeightFieldPaintCursor*>( cursor.fGetRawPtr( ) ) );
	}

	void tHeightFieldMaterialPaintPanel::fUpdateParametersOnCursor( tHeightFieldPaintCursor* cursorBase )
	{
		tHeightFieldMaterialPaintCursor* cursor = dynamic_cast< tHeightFieldMaterialPaintCursor* >( cursorBase );
		if( !cursor )
			return;
		cursor->fSetSize( mSizeSlider->fGetValue( ) );
		cursor->fSetStrength( mStrengthSlider->fGetValue( ) );
		cursor->fSetFalloff( mFalloffSlider->fGetValue( ) );
		cursor->fSetShape( mShapeSlider->fGetValue( ) );
		cursor->fSetLuminosity( mLuminositySlider->fGetValue( ) );
	}

	void tHeightFieldMaterialPaintPanel::fNudgeCursorSize( f32 delta )
	{
		mSizeSlider->fSetValue( mSizeSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	void tHeightFieldMaterialPaintPanel::fNudgeCursorStrength( f32 delta )
	{
		mStrengthSlider->fSetValue( mStrengthSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	void tHeightFieldMaterialPaintPanel::fNudgeCursorFalloff( f32 delta )
	{
		mFalloffSlider->fSetValue( mFalloffSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	void tHeightFieldMaterialPaintPanel::fNudgeCursorShape( f32 delta )
	{
		mShapeSlider->fSetValue( mShapeSlider->fGetValue( ) + delta );
		fOnSlidersChanged( );
	}

	void tHeightFieldMaterialPaintPanel::fSetTilingSlider( f32 value )
	{
		mTilingSlider->fSetValue( value );
	}
	f32 tHeightFieldMaterialPaintPanel::fTilingSlider( ) const
	{
		return mTilingSlider->fGetValue( );
	}

	void tHeightFieldMaterialPaintPanel::fClearMaterials( u32& firstMat, u32& secondMat, u32& thirdMat ) const
	{
		firstMat = mFirstClearMat->fGetSelection( );
		secondMat = mSecondClearMat->fGetSelection( );
		thirdMat = mThirdClearMat->fGetSelection( );
	}

	void tHeightFieldMaterialPaintPanel::fFindReplaceMaterials( u32& findMat, u32& replaceMat ) const
	{
		findMat = mFindMat->fGetSelection( );
		replaceMat = mReplaceMat->fGetSelection( );
	}

	void tHeightFieldMaterialPaintPanel::fClearAtlases( b32 diffuse, b32 normal )
	{
		mParent->Freeze( );

		if( diffuse )
		{
			mMaterialButtons.fDeleteArray( );
			mMaterialGroup->fDeleteButtons( );
			
			// Clear out all the clearing boxes.
			mFirstClearMat->fClearChoices( );
			mSecondClearMat->fClearChoices( );
			mThirdClearMat->fClearChoices( );

			mFindMat->fClearChoices( );
			mReplaceMat->fClearChoices( );

			mDiffuseMapAtlas = tFilePathPtr( );

			tResourcePtr& res = mAppWindow->fGuiApp( ).fEditableObjects( ).fSharedHeightFieldDiffuseMap( );
			if( res )
				res->fUnload( this );
		}

		if( normal )
		{
			mNormalMapAtlas = tFilePathPtr( );

			tResourcePtr& res = mAppWindow->fGuiApp( ).fEditableObjects( ).fSharedHeightFieldNormalMap( );
			if( res )
				res->fUnload( this );
		}

		mParent->fUpdateScrollBars( );
		mParent->Layout( );
		mParent->Update( );
		mParent->Thaw( );
	}

	void tHeightFieldMaterialPaintPanel::fRefreshAtlases( b32 diffuse, b32 normal )
	{
		mParent->Freeze( );

		tEditableObjectContainer& editableObjects = mAppWindow->fGuiApp( ).fEditableObjects( );

		if( diffuse )
		{
			tResourcePtr& diffuseMapResource = editableObjects.fSharedHeightFieldDiffuseMap( );
			if( diffuseMapResource )
				diffuseMapResource->fUnload( this );

			b32 success = false;
			if( mDiffuseMapAtlas.fLength( ) > 0 )
			{
				Tatml::tFile tatml;
				if( tatml.fLoadXml( ToolsPaths::fMakeResAbsolute( mDiffuseMapAtlas ) ) )
				{
					// Copy in all the texture names in order so the drop downs can select them.
					const u32 numMtls = tatml.mTexturePaths.fCount( );
					mMaterialButtons.fNewArray( numMtls );
					for( u32 imtl = 0; imtl < numMtls; ++imtl )
					{
						tHeightFieldMaterialButton* mtlButton = new tHeightFieldMaterialButton( mAppWindow, this, mMaterialGroup, imtl );
						mMaterialButtons[ imtl ] = mtlButton;
						mtlButton->fSetDiffuseMapPath( tatml.mTexturePaths[ imtl ] );

						std::string texName = StringUtil::fNameFromPath( tatml.mTexturePaths[ imtl ].fCStr( ) );
						mFirstClearMat->fAddString( texName );
						mSecondClearMat->fAddString( texName );
						mThirdClearMat->fAddString( texName );

						mFindMat->fAddString( texName );
						mReplaceMat->fAddString( texName );
					}

					// Set default picks for the materials.
					if( numMtls > 3 )
					{
						mFirstClearMat->fSetSelection( 0 );
						mSecondClearMat->fSetSelection( 1 );
						mThirdClearMat->fSetSelection( 2 );

						mFindMat->fSetSelection( 0 );
						mReplaceMat->fSetSelection( 1 );
					}
					else
					{
						mFirstClearMat->fSetSelection( 0 );
						mSecondClearMat->fSetSelection( 0 );
						mThirdClearMat->fSetSelection( 0 );

						mFindMat->fSetSelection( 0 );
						mReplaceMat->fSetSelection( 0 );
					}

					diffuseMapResource = editableObjects.fGetResourceDepot( )->fQuery( tResourceId::fMake<Gfx::tTextureFile>( Tatml::fTatmlPathToTatb( mDiffuseMapAtlas ) ) );
					diffuseMapResource->fLoadDefault( this );
					diffuseMapResource->fBlockUntilLoaded( );
					success = true;
				}
			}
			if( !success )
				fClearAtlases( true, false );
		}

		if( normal )
		{
			tResourcePtr& normalMapResource = editableObjects.fSharedHeightFieldNormalMap( );
			if( normalMapResource )
				normalMapResource->fUnload( this );

			b32 success = false;
			if( mNormalMapAtlas.fLength( ) > 0 )
			{
				Tatml::tFile tatml;
				if( tatml.fLoadXml( ToolsPaths::fMakeResAbsolute( mNormalMapAtlas ) ) )
				{
					normalMapResource = editableObjects.fGetResourceDepot( )->fQuery( tResourceId::fMake<Gfx::tTextureFile>( Tatml::fTatmlPathToTatb( mNormalMapAtlas ) ) );
					normalMapResource->fLoadDefault( this );
					normalMapResource->fBlockUntilLoaded( );
					success = true;
				}
			}
			if( !success )
				fClearAtlases( false, true );
		}

		editableObjects.fOnSharedHeightFieldMapsModified( );

		mParent->fUpdateScrollBars( );
		mParent->Layout( );
		mParent->Update( );
		mParent->Thaw( );
	}

	void tHeightFieldMaterialPaintPanel::fOnAtlasChanged( tAtlasPathTextBox* control )
	{
		if( control == mDiffuseMapBrowser )
		{
			fClearAtlases( true, false );
			mDiffuseMapAtlas = tFilePathPtr( control->fGetValue( ) );
			fRefreshAtlases( true, false );
		}
		else if( control == mNormalMapBrowser )
		{
			fClearAtlases( false, true );
			mNormalMapAtlas = tFilePathPtr( control->fGetValue( ) );
			fRefreshAtlases( false, true );
		}

		mAppWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
	}

	void tHeightFieldMaterialPaintPanel::fOnClearTerrainTextures( wxCommandEvent& event )
	{
		if( mFirstClearMat->fGetSelection( ) == -1 ||
			mSecondClearMat->fGetSelection( ) == -1 ||
			mThirdClearMat->fGetSelection( ) == -1 )
			return;

		mAppWindow->fCleanHeightFieldMaterials( mFirstClearMat->fGetSelection( ), mSecondClearMat->fGetSelection( ), mThirdClearMat->fGetSelection( ) );
	}

	void tHeightFieldMaterialPaintPanel::tAtlasPathTextBox::fOnBrowseForAtlas( wxCommandEvent& )
	{
		const std::string ext = Tatml::fGetFileExtension( );

		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			mOwner->fGetMainPanel( ), 
			"Select Texture Atlas File",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxEmptyString,
			wxString( "*" + ext ),
			wxFD_OPEN ) );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			const tFilePathPtr absPath = tFilePathPtr( openFileDialog->GetPath( ).c_str( ) );
			const tFilePathPtr relPath = ToolsPaths::fMakeResRelative( absPath );
			fSetValue( relPath.fCStr( ) );
			mOwner->fOnAtlasChanged( this );
		}
	}

}
