#include "SigEdPch.hpp"
#include "tHeightFieldMaterialButton.hpp"
#include "tEditorAppWindow.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tHeightFieldPaintCursor.hpp"
#include "tHeightFieldMaterialPaintPanel.hpp"
#include "FileSystem.hpp"
#include "Editor/tEditableObjectContainer.hpp"

namespace Sig
{
	class tHeightFieldStandardMaterialBrush : public tHeightFieldMaterialPaintCursor
	{
		tHeightFieldMaterialButton* mMaterial;
	public:

		typedef Math::tVec4f tMaskSample;

		tHeightFieldStandardMaterialBrush( tHeightFieldMaterialButton* button )
			: tHeightFieldMaterialPaintCursor( button )
			, mMaterial( button )
		{
			fMainWindow( ).fSetStatus( "Paint Terrain [Material]" );

			mMaterial->fSyncMaterialProperties( true );
		}

		virtual void fOnTick( )
		{
			mMaterial->fSyncMaterialProperties( false );

			tHeightFieldMaterialPaintCursor::fOnTick( );
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
			const Math::tVec2f uvRadius = Math::tVec2f( args.mMaxU - args.mCenterU, args.mMaxV - args.mCenterV );
			const f32 strength = fComputeStrength( );
			const f32 falloff = fComputeFalloff( );
			const f32 shape = fComputeShape( );

			// get material source data (from gui)
			tHeightFieldMaterialButton* material = mMaterial;
			sigassert( material );
			// current material index
			const u32 paintedMtlIndex = material->fMaterialIndex( );

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


			// before we start painting, we need to find which channel we're going to paint to; this channel needs to
			// remain consistent across all adjacent texels
			f32 evict = 1.f;
			const u32 channel = fComputeProperChannelForId( 
				paintedMtlIndex, 
				tlIds, 
				brIds, 
				tlMask, 
				brMask, 
				idsMip,
				maskMip,
				evict );

			// now we're going to iterate over each texel in the ID texture; for each of these texels, we will
			// iterate over the rectangle of texels in the mask texture that are covered by the single ID texel

			u32 tlMaskWindowU = tlMask.x;
			u32 tlMaskWindowV = tlMask.y;

			for( u32 vIds = tlIds.y; vIds <= brIds.y; ++vIds )
			{
				for( u32 uIds = tlIds.x; uIds <= brIds.x; ++uIds, tlMaskWindowU += numMaskTexelsPerMtlIdTexelU )
				{
					// seek to ids texel
					u16* idsTexel = idsMip.fGetTexel<u16>( uIds, vIds );

					// unpack to get material ids
					Math::tVec3u mtlIds;
					Gfx::tTextureVRam::fUnpackColorR5G6B5( *idsTexel, mtlIds );

					// set this material in the material ids texture
					mtlIds.fAxis( channel ) = paintedMtlIndex;

					// create the target mask color vector; i.e., if we applied 100% strength in one paint action,
					// then this would be the new value of the mask at each texel; instead, we'll blend to this value.
					tMaskSample targetMask = tMaskSample::cZeroVector;
					targetMask.fAxis( channel ) = fGetLuminosity( );

					// as we iterate over the texels in the mask, we will compute 
					// the sum of all the mask samples in the current rectangle after modification
					tMaskSample summedMaskSamples = tMaskSample::cZeroVector;

					// additionally, we'll track whether we actually modified any texels in the mask
					// if not, then we'll skip any modifications to this texel in the IDs texture
					f32 summedPaintStrength = 0.f;

					// now we actually paint the amount of the current material into the mask texture
					for( u32 vMask = tlMaskWindowV; vMask < tlMaskWindowV + numMaskTexelsPerMtlIdTexelV; ++vMask )
					for( u32 uMask = tlMaskWindowU; uMask < tlMaskWindowU + numMaskTexelsPerMtlIdTexelU; ++uMask )
					{
						const f32 normMaskU = uMask / ( args.mMaskTexture.fWidth( ) - 1.f );
						const f32 normMaskV = vMask / ( args.mMaskTexture.fHeight( ) - 1.f );
						const f32 paintStrength = fComputePaintStrength( args, normMaskU, normMaskV, uvRadius, shape, falloff, strength );

						if( paintStrength == 0.f )
							continue;

						summedPaintStrength += paintStrength;

						// sample this mask texel
						u32* maskTexel = maskMip.fGetTexel<u32>( uMask, vMask );

						// unpack to easy-to-use float values
						Math::tVec4f mask;
						Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *maskTexel, mask );

						// if we're evicting an old material, we default the mask to 0.f
						mask.fAxis( channel ) *= evict;

						// lerp to new mask value
						Math::tVec4f newMask = Math::fLerp( mask, targetMask, paintStrength );

						// make sure that we apply enough of a delta that our
						// floating point precision isn't lost on the texture's integer bit depth
						const f32 minDelta = 0.004f; // this should correspond to 1 / (max integer value of channel)
						tMaskSample maskDelta = newMask - mask;
						for( u32 iaxis = 0; iaxis < mtlIds.cDimension; ++iaxis )
						{
							const f32 sign = fSign( maskDelta.fAxis( iaxis ) );
							const f32 mag = fAbs( maskDelta.fAxis( iaxis ) );
							if( mag && mag < minDelta )
								maskDelta.fAxis( iaxis ) = sign * minDelta;
						}

						// re-apply delta now that it's been properly quantized
						newMask = fClamp( mask + maskDelta, tMaskSample::cZeroVector, tMaskSample::cOnesVector );

						// track mask sum
						summedMaskSamples += newMask;

						// pack and write new mask texel
						*maskTexel = Gfx::tTextureVRam::fPackColorR8G8B8A8( newMask );
					}

					if( summedPaintStrength == 0.f )
						continue;

					// store back material ids
					*idsTexel = Gfx::tTextureVRam::fPackColorR5G6B5( mtlIds.x, mtlIds.y, mtlIds.z );
				}

				tlMaskWindowU = tlMask.x; // reset U coordinate (type-writer-style)
				tlMaskWindowV += numMaskTexelsPerMtlIdTexelV;
			}

			// unlock mips
			args.mMtlIdsTexture.fUnlock( );
			args.mMaskTexture.fUnlock( );
		}

	private:

		u32 fComputeProperChannelForId( 
			u32 currentMaterialIndex, 
			const Math::tVec2u& tlIds, 
			const Math::tVec2u& brIds, 
			const Math::tVec2u& tlMask, 
			const Math::tVec2u& brMask, 
			const Gfx::tTextureFile::tLockedMip& idsMip, 
			const Gfx::tTextureFile::tLockedMip& maskMip, 
			f32& evict ) const
		{
			for( u32 vIds = tlIds.y; vIds <= brIds.y; ++vIds )
			{
				for( u32 uIds = tlIds.x; uIds <= brIds.x; ++uIds )
				{
					// seek to ids texel
					u16* idsTexel = idsMip.fGetTexel<u16>( uIds, vIds );

					// unpack to get material ids
					Math::tVec3u mtlIds;
					Gfx::tTextureVRam::fUnpackColorR5G6B5( *idsTexel, mtlIds );

					// see if this ID is already present in this texel, and if it is, return it
					for( u32 iaxis = 0; iaxis < mtlIds.cDimension; ++iaxis )
					{
						if( mtlIds.fAxis( iaxis ) == currentMaterialIndex )
						{
							evict = 1.f;
							return iaxis;
						}
					}
				}
			}

			// we're evicting an old material
			evict = 0.f;

			// compute the sum of all the mask samples in the current rectangle
			tMaskSample summedMaskSamples = tMaskSample::cZeroVector;
			for( u32 vMask = tlMask.y; vMask <= brMask.y; ++vMask )
			{
				for( u32 uMask = tlMask.x; uMask <= brMask.x; ++uMask )
				{
					tMaskSample sample;
					Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *maskMip.fGetTexel<u32>( uMask, vMask ), sample );
					summedMaskSamples += sample;
				}
			}

			// now we use the channel in the summed sample vector with the smallest value
			const u32 iWeakestAxis = Math::tVec3f( summedMaskSamples.x, summedMaskSamples.y, summedMaskSamples.z ).fMinAxisIndex( );
			return iWeakestAxis;
		}

		inline f32 fComputePaintStrength( tPaintMaterialArgs& args, f32 normMaskU, f32 normMaskV, const Math::tVec2f& uvRadius, f32 shape, f32 falloff, f32 strength )
		{
			// compute paint strength (as a function of the gui sliders, i.e., how big the radius is,
			// whether we're circle/square, overall strength, etc).
			const f32 circleDistToCenter = std::sqrt( Math::fSquare( ( normMaskU - args.mCenterU ) / uvRadius.x ) + Math::fSquare( ( normMaskV - args.mCenterV ) / uvRadius.y ) );
			const f32 squareDistToCenter = fMax( fAbs( normMaskU - args.mCenterU ) / uvRadius.x, fAbs( normMaskV - args.mCenterV ) / uvRadius.y );
			const f32 distToCenter = Math::fLerp( circleDistToCenter, squareDistToCenter, shape ) - ( 1.f / ( args.mMaskTexture.fWidth( ) - 1.f ) );
			const f32 distToCenterNorm = fClamp( distToCenter, 0.f, 1.f ); 
			const f32 distBasedStrength = 1.f - std::powf( distToCenterNorm, falloff );
			const f32 paintStrength = strength * distBasedStrength * ( 10.f * args.mDt );
			sigassert( paintStrength >= 0.f );
			const f32 threshold = 1.f/255.f;
			return paintStrength > threshold ? ( ( paintStrength - threshold ) / ( 1.f - threshold ) ) : 0.f;
		}

		template<class tVecType>
		tFixedArray<u32,3> fComputeChannelReOrdering( tVecType v ) const
		{
			tFixedArray<u32,3> reOrderedChannels;
			sigassert( reOrderedChannels.cDimension <= v.cDimension );

			for( u32 iaxis = 0; iaxis < reOrderedChannels.cDimension; ++iaxis )
			{
				reOrderedChannels[ iaxis ] = v.fMaxAxisIndex( );
				v.fAxis( reOrderedChannels[ iaxis ] ) = -Math::cInfinity;
			}

			return reOrderedChannels;
		}

		template<class tVecType>
		tVecType fSwizzle( const tVecType& v, const tFixedArray<u32,3>& swizzle ) const
		{
			sigassert( swizzle.cDimension <= v.cDimension );
			tVecType o = v;
			for( u32 i = 0; i < swizzle.cDimension; ++i )
				o.fAxis( i ) = v.fAxis( swizzle[ i ] );
			return o;
		}


	};



	tHeightFieldMaterialButton::tHeightFieldMaterialButton( tEditorAppWindow* appWindow, tHeightFieldMaterialPaintPanel* buttonContainer, tEditorCursorControllerButtonGroup* parent, u32 mtlIndex )
		: tEditorCursorControllerButton( parent, wxBitmap( "PaintTerrainMaterialSel" ), wxBitmap( "PaintTerrainMaterialDeSel" ), "" )
		, mAppWindow( appWindow )
		, mButtonContainer( buttonContainer )
		, mMaterialIndex( mtlIndex )
		, mTilingFactor( 0.5f )
		, mDiffuseMapPath( "" )
		, mNormalMapPath( "" )
	{
	}

	tHeightFieldMaterialButton::~tHeightFieldMaterialButton( )
	{
	}

	u32 tHeightFieldMaterialButton::fMaterialIndex( ) const
	{
		return mMaterialIndex;
	}

	tEditorCursorControllerPtr tHeightFieldMaterialButton::fCreateCursorController( )
	{
		tHeightFieldMaterialPaintCursor* terrainCursor = new tHeightFieldStandardMaterialBrush( this );
		mButtonContainer->fUpdateParametersOnCursor( terrainCursor );
		mButtonContainer->fAddCursorHotKeys( terrainCursor );

		return tEditorCursorControllerPtr( terrainCursor );
	}

	void tHeightFieldMaterialButton::fResetMaps( )
	{
		if( mDiffuseMapPath.fLength( ) == 0 && mNormalMapPath.fLength( ) == 0 )
			return; // already reset

		// invalid bmp type or file not specified, use default button images
		fGetSelectedBitmap( ) = wxBitmap( "PaintTerrainMaterialSel" );
		fGetDeSelectedBitmap( ) = wxBitmap( "PaintTerrainMaterialDeSel" );
		
		mDiffuseMapPath = tFilePathPtr("");
		mNormalMapPath = tFilePathPtr("");

		if( fIsSelected( ) )
			fUpdateAllBitmaps( fGetSelectedBitmap( ) );
		else
			fUpdateAllBitmaps( fGetDeSelectedBitmap( ) );
	}

	void tHeightFieldMaterialButton::fSetDiffuseMapPath( const tFilePathPtr& path )
	{
		std::string diffuseMapName = StringUtil::fStripExtension( StringUtil::fNameFromPath( path.fCStr( ) ).c_str( ) );
		SetToolTip( diffuseMapName.c_str( ) );
		const tFilePathPtr fullPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectResFolder( ), path );
		mDiffuseMapPath = path;

		int bmpType = -1;

		if( FileSystem::fFileExists( fullPath ) )
		{
			if( StringUtil::fCheckExtension( path.fCStr( ), ".tga" ) )
				bmpType = wxBITMAP_TYPE_TGA;
			else if( StringUtil::fCheckExtension( path.fCStr( ), ".png" ) )
				bmpType = wxBITMAP_TYPE_PNG;
			else if( StringUtil::fCheckExtension( path.fCStr( ), ".jpg" ) || StringUtil::fCheckExtension( path.fCStr( ), ".jpeg" ) )
				bmpType = wxBITMAP_TYPE_JPEG;
			else if( StringUtil::fCheckExtension( path.fCStr( ), ".bmp" ) )
				bmpType = wxBITMAP_TYPE_BMP;
		}

		if( bmpType < 0 )
		{
			// invalid bmp type or file not specified, use default button images
			fGetSelectedBitmap( ) = wxBitmap( "PaintTerrainMaterialSel" );
			fGetDeSelectedBitmap( ) = wxBitmap( "PaintTerrainMaterialDeSel" );
		}
		else
		{
			// load the bitmap using the proper format type, and scale down to button size
			wxImage bmpImage = wxImage( fullPath.fCStr( ), bmpType ).Scale( fMax( 16, fGetSelectedBitmap( ).GetWidth( ) ), fMax( 16, fGetSelectedBitmap( ).GetHeight( ) ) );

			// set the de-selected bitmap
			fGetDeSelectedBitmap( ) = wxBitmap( bmpImage );

			// create band around the border for the "selected" bitmap
			const u32 bandPixelCount = 4;
			for( s32 y = 0; y < bmpImage.GetHeight( ); ++y )
			{
				for( u32 iband = 0; iband < bandPixelCount; ++iband )
				{
					const u32 c = fMin<u32>( bmpImage.GetHeight( ) - y - 1, y, iband ) * 64;
					bmpImage.SetRGB( iband, y, c, c, 255u );
					bmpImage.SetRGB( bmpImage.GetWidth( ) - ( 1 + iband ), y, c, c, 255u );
				}
			}
			for( s32 x = 0; x < bmpImage.GetWidth( ); ++x )
			{
				for( u32 iband = 0; iband < bandPixelCount; ++iband )
				{
					const u32 c = fMin<u32>( bmpImage.GetWidth( ) - x - 1, x, iband ) * 64;
					bmpImage.SetRGB( x, iband, c, c, 255u );
					bmpImage.SetRGB( x, bmpImage.GetHeight( ) - ( 1 + iband ), c, c, 255u );
				}
			}

			// set the selected bitmap
			fGetSelectedBitmap( ) = wxBitmap( bmpImage );
		}

		if( fIsSelected( ) )
			fUpdateAllBitmaps( fGetSelectedBitmap( ) );
		else
			fUpdateAllBitmaps( fGetDeSelectedBitmap( ) );
	}

	void tHeightFieldMaterialButton::fSetNormalMapPath( const tFilePathPtr& path )
	{
		mNormalMapPath = path;
	}

	void tHeightFieldMaterialButton::fSyncMaterialProperties( b32 syncGuiToMaterial )
	{
		if( syncGuiToMaterial )
			mButtonContainer->fSetTilingSlider( mTilingFactor );
		else
		{
			const f32 pre = mTilingFactor;
			mTilingFactor = mButtonContainer->fTilingSlider( );
			if( pre != mTilingFactor ) // value changed, notify everyone
			{
				mAppWindow->fRefreshHeightFieldMaterialTileFactors( );
				mAppWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( );
			}
		}
	}

}

