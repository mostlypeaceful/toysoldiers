#include "SigEdPch.hpp"
#include "tResetHeightFieldMaterialButton.hpp"
#include "tEditorAppWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tHeightFieldPaintCursor.hpp"
#include "tHeightFieldMaterialPaintPanel.hpp"
#include "FileSystem.hpp"
#include "Editor/tEditableObjectContainer.hpp"

namespace Sig
{
	class tHeightFieldResetMaterialBrush : public tHeightFieldMaterialPaintCursor
	{
		tResetHeightFieldMaterialButton* mButton;
		b32 mReplacementMode;

	public:

		tHeightFieldResetMaterialBrush( tResetHeightFieldMaterialButton* button, b32 replacementMode = false )
			: tHeightFieldMaterialPaintCursor( button )
			, mButton( button )
			, mReplacementMode( replacementMode )
		{
			if( mReplacementMode )
				fMainWindow( ).fSetStatus( "Reset Terrain [Find And Replace Material]" );
			else
				fMainWindow( ).fSetStatus( "Paint Terrain [Reset Material]" );
		}

		virtual void fPaintMaterial( tPaintMaterialArgs& args )
		{
			sigassert(	args.mMaskTexture.fWidth( ) >= args.mMtlIdsTexture.fWidth( ) &&
						args.mMaskTexture.fHeight( ) >= args.mMtlIdsTexture.fHeight( ) );

			sigassert(	args.mMaskTexture.fWidth( ) % args.mMtlIdsTexture.fWidth( ) == 0 &&
						args.mMaskTexture.fHeight( ) % args.mMtlIdsTexture.fHeight( ) == 0 );

			sigassert(	args.mMaskTexture.fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 &&
						args.mMtlIdsTexture.fFormat( ) == Gfx::tTextureFile::cFormatR5G6B5 );

			tWxRenderPanel* renderPanel = fMainWindow( ).fRenderPanelContainer( )->fGetFocusRenderPanel( );

			// Prevent painting outside of the focus panel.
			if( fMainWindow( ).fRenderPanelContainer( )->fGetActiveRenderPanel( ) != renderPanel )
				return;

			const Input::tMouse& mouse = renderPanel->fGetMouse( );
			const Input::tKeyboard& kb = Input::tKeyboard::fInstance( );
			const b32 shiftHeld = kb.fButtonHeld( Input::tKeyboard::cButtonLShift ) || kb.fButtonHeld( Input::tKeyboard::cButtonRShift );
			const b32 ctrlHeld = kb.fButtonHeld( Input::tKeyboard::cButtonLCtrl ) || kb.fButtonHeld( Input::tKeyboard::cButtonRCtrl );
			const f32 uvRadius = fMax( args.mMaxU - args.mCenterU, args.mMaxV - args.mCenterV );
			const f32 strength = fComputeStrength( );
			const f32 falloff = fComputeFalloff( );
			const f32 shape = fComputeShape( );

			// lock the top mips for both textures
			Gfx::tTextureFile::tLockedMip idsMip = args.mMtlIdsTexture.fLock( );
			Gfx::tTextureFile::tLockedMip maskMip = args.mMaskTexture.fLock( );

			// compute the number of texels in the mask texture per 1 texel in the ids texture; essentially,
			// this single texel in the ids texture will apply to all the texels in the mask texture
			const u32 numMaskTexelsPerMtlIdTexelU = args.mMaskTexture.fWidth( ) / args.mMtlIdsTexture.fWidth( );
			const u32 numMaskTexelsPerMtlIdTexelV = args.mMaskTexture.fHeight( ) / args.mMtlIdsTexture.fHeight( );

			// compute texel indices for both textures (top left, bottom right)
			const Math::tVec2u tlIds = args.mMtlIdsTexture.fTexelIndexClamp( Math::tVec2f( args.mMinU, args.mMinV ) );
			const Math::tVec2u brIds = args.mMtlIdsTexture.fTexelIndexClamp( Math::tVec2f( args.mMaxU, args.mMaxV ) );
			const Math::tVec2u tlMask = args.mMaskTexture.fTexelIndexClamp( Math::tVec2f( args.mMinU, args.mMinV ) );
			const Math::tVec2u brMask = args.mMaskTexture.fTexelIndexClamp( Math::tVec2f( args.mMaxU, args.mMaxV ) );

			u32 firstMat = 0;
			u32 secondMat = 1;
			u32 thirdMat = 2;
			if( mReplacementMode )
			{
				mButton->fFindReplaceMaterials( firstMat, secondMat );
			}
			else
				mButton->fMaterials( firstMat, secondMat, thirdMat );

			// now we're going to iterate over each texel in the ID texture; for each of these texels, we will
			// iterate over the rectangle of texels in the mask texture that are covered by the single ID texel

			u32 tlMaskWindowU = tlMask.x;
			u32 tlMaskWindowV = tlMask.y;

			for( u32 vIds = tlIds.y; vIds <= brIds.y; ++vIds )
			{
				for( u32 uIds = tlIds.x; uIds <= brIds.x; ++uIds, tlMaskWindowU += numMaskTexelsPerMtlIdTexelU )
				{
					b32 painted = false;

					// now we actually paint the amount of the current material into the mask texture
					for( u32 vMask = tlMaskWindowV; vMask < tlMaskWindowV + numMaskTexelsPerMtlIdTexelV; ++vMask )
					for( u32 uMask = tlMaskWindowU; uMask < tlMaskWindowU + numMaskTexelsPerMtlIdTexelU; ++uMask )
					{
						const f32 normMaskU = uMask / ( args.mMaskTexture.fWidth( ) - 1.f );
						const f32 normMaskV = vMask / ( args.mMaskTexture.fHeight( ) - 1.f );
						const b32 shouldPaint = fShouldPaint( args, normMaskU, normMaskV, uvRadius, shape );

						if( !shouldPaint )
							continue;

						painted = true;

						if( !mReplacementMode )
						{
							// stomp the mask and make the first material primary
							u32* maskTexel = maskMip.fGetTexel<u32>( uMask, vMask );
							*maskTexel = Gfx::tTextureVRam::fPackColorR8G8B8A8( 255, 0, 0, 0 );
						}

						// replacement mode preserves the mask weights
					}

					if( !painted )
						continue;

					// write out material ids
					u16* idsTexel = idsMip.fGetTexel<u16>( uIds, vIds );
					if( mReplacementMode )
					{
						Math::tVec3u mtlIds;
						Gfx::tTextureVRam::fUnpackColorR5G6B5( *idsTexel, mtlIds );

						// in replacement mode, firstMat is the "find" and secondMat is the "replace
						if( mtlIds.x == firstMat )
							mtlIds.x = secondMat;
						if( mtlIds.y == firstMat )
							mtlIds.y = secondMat;
						if( mtlIds.z == firstMat )
							mtlIds.z = secondMat;

						// put back the modified materials
						*idsTexel = Gfx::tTextureVRam::fPackColorR5G6B5( mtlIds.x, mtlIds.y, mtlIds.z );
					}
					else
					{
						// stomp the materials and put in desired ids
						*idsTexel = Gfx::tTextureVRam::fPackColorR5G6B5( firstMat, secondMat, thirdMat );
					}
				}

				tlMaskWindowU = tlMask.x; // reset U coordinate (type-writer-style)
				tlMaskWindowV += numMaskTexelsPerMtlIdTexelV;
			}

			// unlock mips
			args.mMtlIdsTexture.fUnlock( );
			args.mMaskTexture.fUnlock( );
		}

	private:

		inline b32 fShouldPaint( tPaintMaterialArgs& args, f32 normMaskU, f32 normMaskV, f32 uvRadius, f32 shape )
		{
			const f32 circleDistToCenter = std::sqrt( Math::fSquare( normMaskU - args.mCenterU ) + Math::fSquare( normMaskV - args.mCenterV ) );
			const f32 squareDistToCenter = fMax( fAbs( normMaskU - args.mCenterU ), fAbs( normMaskV - args.mCenterV ) );
			const f32 distToCenter = Math::fLerp( circleDistToCenter, squareDistToCenter, shape ) - ( 1.f / ( args.mMaskTexture.fWidth( ) - 1.f ) );

			return distToCenter <= uvRadius;
		}

	};



	tResetHeightFieldMaterialButton::tResetHeightFieldMaterialButton(
		tEditorAppWindow* appWindow, 
		tHeightFieldMaterialPaintPanel* buttonContainer, 
		tEditorCursorControllerButtonGroup* parent,
		const char* selectedBitmap,
		const char* deselectedBitmap,
		const char* tooltip,
		b32 replacementMode )
		: tEditorCursorControllerButton( parent, wxBitmap( selectedBitmap ), wxBitmap( deselectedBitmap ), tooltip )
		, mAppWindow( appWindow )
		, mButtonContainer( buttonContainer )
		, mReplacementMode( replacementMode )
	{
	}

	tResetHeightFieldMaterialButton::~tResetHeightFieldMaterialButton( )
	{
	}

	void tResetHeightFieldMaterialButton::fMaterials( u32& firstMat, u32& secondMat, u32& thirdMat ) const
	{
		mButtonContainer->fClearMaterials( firstMat, secondMat, thirdMat );
	}

	void tResetHeightFieldMaterialButton::fFindReplaceMaterials( u32& findMat, u32& replaceMat ) const
	{
		mButtonContainer->fFindReplaceMaterials( findMat, replaceMat );
	}

	tEditorCursorControllerPtr tResetHeightFieldMaterialButton::fCreateCursorController( )
	{
		tHeightFieldResetMaterialBrush* terrainCursor = new tHeightFieldResetMaterialBrush( this, mReplacementMode );
		mButtonContainer->fUpdateParametersOnCursor( terrainCursor );
		mButtonContainer->fAddCursorHotKeys( terrainCursor );

		return tEditorCursorControllerPtr( terrainCursor );
	}

}

