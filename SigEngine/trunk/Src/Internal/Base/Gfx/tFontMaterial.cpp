#include "BasePch.hpp"
#include "tFontMaterial.hpp"
#include "tDevice.hpp"
#include "tMaterialFile.hpp"
#include "tTextureFile.hpp"
#include "tRenderInstance.hpp"
#include "tRenderContext.hpp"
#include "Gui/tRenderableCanvas.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticTexCoord, tVertexElement::cFormat_f32_2 ),
		};
		const tFilePathPtr cFontPath( "Shaders/Engine/Font.mtlb" );
	}
	const tVertexFormat tFontMaterial::cVertexFormat= 
		tVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	tFilePathPtr tFontMaterial::fMaterialFilePath( )
	{
		return cFontPath;
	}

	tFontMaterial::tFontMaterial( )
		: mVsSlot( cVSShaderCount )
		, mPsSlot( cPSShaderCount )
		, mFontMap( 0 )
	{
	}

	tFontMaterial::tFontMaterial( tNoOpTag )
		: tMaterial( cNoOpTag )
	{
	}

	void tFontMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		sigassert( sizeof( tGlyphRenderVertex ) == cVertexFormat.fVertexSize( ) );

		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, 0, mVsSlot );
		mtlFile->fApplyShader( device, 0, mPsSlot );

		// apply font texture map
		fApplyTexture( device, 0, mFontMap );

		// vertex shader constants
		fApplyMatrix4VS( device, cVSWorldToProj,		context.mSceneCamera->fGetWorldToProjection( ) );

	}

	void tFontMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );

		const Gui::tRenderableCanvas* canvas = instance.fRI_RenderableCanvas( );

		const b32 scissorRectEnabled = canvas && canvas->fScissorRectEnabled( );
		if( scissorRectEnabled )
			device->fSetScissorRect( &canvas->fScissorRect( ) ); // set scissor rect if specified

		const b32 dropShadow = canvas && canvas->fDropShadowEnabled( );
		if( dropShadow )
		{
			// shift and apply xform to vertex shader
			Math::tMat3f objToWorld = instance.fRI_ObjectToWorld( );
			objToWorld.fTranslateLocal( Math::tVec3f( ( f32 )canvas->fDropShadowX( ), ( f32 )canvas->fDropShadowY( ), 0.00001f ) );
			fApplyMatrix3VS( device, cVSLocalToWorld, objToWorld );

			// shift and apply rgba to pixel shader
			Math::tVec4f rgbaTint = instance.fRgbaTint( );
			rgbaTint = Math::tVec4f( 0.f, 0.f, 0.f, 0.9f * Math::fSquare( fMin( rgbaTint.fXYZ( ).fMaxMagnitude( ), rgbaTint.w ) ) );
			fApplyVector4PS( device, cPSRgbaTint, rgbaTint );

			// render drop shadow
			renderBatch.fRenderInstance( device );
		}

		// vertex shader constants
		fApplyMatrix3VS( device, cVSLocalToWorld, instance.fRI_ObjectToWorld( ) );

		// set pixel shader constants
		fApplyVector4PS( device, cPSRgbaTint, instance.fRgbaTint( ) );

		// render text
		renderBatch.fRenderInstance( device );

		if( scissorRectEnabled )
			device->fSetScissorRect( 0 ); // clear scissor rect only if we set one previously
	}

	void tFontMaterial::fApplyTexture( const tDevicePtr& device, u32 slot, tLoadInPlaceResourcePtr* texture ) const
	{
		if( texture )
		{
			const tResourcePtr& texResource = texture->fGetResourcePtr( );
			const tTextureFile* texFile = texResource->fCast<tTextureFile>( );
			sigassert( texFile );
			texFile->fApply( device, slot, tTextureFile::cFilterModeWithMip, tTextureFile::cAddressModeWrap, tTextureFile::cAddressModeWrap, tTextureFile::cAddressModeWrap );
		}
	}

}}
