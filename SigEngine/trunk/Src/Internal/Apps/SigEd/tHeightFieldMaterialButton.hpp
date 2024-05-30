#ifndef __tHeightFieldMaterialButton__
#define __tHeightFieldMaterialButton__
#include "tEditorDialog.hpp"
#include "tTextureSysRam.hpp"
#include "tEditorCursorControllerButton.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tHeightFieldMaterialButton;
	class tHeightFieldMaterialPaintPanel;

	class tHeightFieldMaterialButton : public tEditorCursorControllerButton
	{
		friend class tEditorAppWindow;
	private:
		tEditorAppWindow* mAppWindow;
		tHeightFieldMaterialPaintPanel* mButtonContainer;
		u32 mMaterialIndex;
		f32 mTilingFactor;
		tFilePathPtr mDiffuseMapPath;
		tFilePathPtr mNormalMapPath;

	public:
		tHeightFieldMaterialButton( tEditorAppWindow* appWindow, tHeightFieldMaterialPaintPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent, u32 mtlIndex );
		~tHeightFieldMaterialButton( );
		u32 fMaterialIndex( ) const;
		virtual tEditorCursorControllerPtr fCreateCursorController( );

		void fResetMaps( );
		void fSetDiffuseMapPath( const tFilePathPtr& path );
		void fSetNormalMapPath( const tFilePathPtr& path );
		void fSyncMaterialProperties( b32 syncGuiToMaterial );
		void fSetTileFactor( f32 tileFactor ) { mTilingFactor = tileFactor; }
		f32  fTileFactor( ) const { return mTilingFactor; }

		inline tEditorAppWindow* fEditorWindow( ) const { return mAppWindow; }
		inline const tFilePathPtr& fGetDiffuseMapPath( ) const { return mDiffuseMapPath; }
		inline const tFilePathPtr& fGetNormalMapPath( ) const { return mNormalMapPath; }
	};

}

#endif//__tHeightFieldMaterialButton__
