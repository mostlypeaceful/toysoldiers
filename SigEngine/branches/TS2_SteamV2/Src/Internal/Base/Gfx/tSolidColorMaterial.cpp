#include "BasePch.hpp"
#include "tSolidColorMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tRenderInstance.hpp"
#include "tRenderContext.hpp"
#include "tDevice.hpp"
#include "Gui/tRenderableCanvas.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tFilePathPtr cSolidColorPath( "Shaders/Engine/SolidColor.mtlb" );
	}
	const tVertexFormat tSolidColorMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	tFilePathPtr tSolidColorMaterial::fMaterialFilePath( )
	{
		return cSolidColorPath;
	}

	tSolidColorMaterial::tSolidColorMaterial( )
	{
		mRenderState = tRenderState::cDefaultColorOpaque;
	}

	tSolidColorMaterial::tSolidColorMaterial( tNoOpTag )
	{
	}

	tSolidColorMaterial::tSolidColorMaterial( const tResourcePtr& solidColorMaterialFile )
	{
		mRenderState = tRenderState::cDefaultColorOpaque;
		fSetMaterialFileResourcePtrOwned( solidColorMaterialFile );
	}

	void tSolidColorMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		sigassert( sizeof( tSolidColorRenderVertex ) == cVertexFormat.fVertexSize( ) );

		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, 0, cShaderSlotVS );
		mtlFile->fApplyShader( device, 0, cShaderSlotPS );

		// vertex shader constants
		fApplyMatrix4VS( device, cVSWorldToProj, context.mCamera->fGetWorldToProjection( ) );
		fApplyVector4VS( device, cVSWorldEyePos, Math::tVec4f( context.mCamera->fGetTripod( ).mEye, context.mCamera->fGetLens( ).mFarPlane ) );
	}

	void tSolidColorMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );

		const Gui::tRenderableCanvas* canvas = instance.fRI_RenderableCanvas( );

		const b32 scissorRectEnabled = canvas && canvas->fScissorRectEnabled( );
		if( scissorRectEnabled )
			device->fSetScissorRect( &canvas->fScissorRect( ) ); // set scissor rect if specified

		// vertex shader constants
		fApplyObjectToWorldVS( device, cVSLocalToWorld, cVSLocalToWorld, drawCall, context );

		// set pixel shader constants
		fApplyVector4PS( device, cPSRgbaTint, drawCall.fInstanceRgbaTint( ) );

		renderBatch.fRenderInstance( device );

		if( scissorRectEnabled )
			device->fSetScissorRect( 0 ); // clear scissor rect only if we set one previously
	}

}}
