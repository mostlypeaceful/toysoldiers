#include "BasePch.hpp"
#include "tFullBrightMaterial.hpp"
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
			tVertexElement( tVertexElement::cSemanticTexCoord, tVertexElement::cFormat_f32_2 ),
		};
		const tFilePathPtr cFullBrightPath( "Shaders/Engine/FullBright.mtlb" );
	}
	const tVertexFormat tFullBrightMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	tFilePathPtr tFullBrightMaterial::fMaterialFilePath( )
	{
		return cFullBrightPath;
	}

	tFullBrightMaterial::tFullBrightMaterial( )
		: mShaderSlotVS( cShaderSlotVS )
		, mShaderSlotPS( cShaderSlotPS )
	{
		mRenderState = tRenderState::cDefaultColorTransparent;
	}

	tFullBrightMaterial::tFullBrightMaterial( tNoOpTag )
		: mColorMap( cNoOpTag )
	{
	}

	void tFullBrightMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		sigassert( sizeof( tFullBrightRenderVertex ) == cVertexFormat.fVertexSize( ) );

		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		u32 ps = (context.mRenderPassMode == tRenderState::cRenderPassGBuffer) ? cShaderSlotPS_Gbuffer : mShaderSlotPS;

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, 0, mShaderSlotVS );
		mtlFile->fApplyShader( device, 0, ps );

		// vertex shader constants
		fApplyMatrix4VS( device, cVSWorldToProj, context.mSceneCamera->fGetWorldToProjection( ) );
		fApplyVector4VS( device, cVSWorldEyePos, Math::tVec4f( context.mSceneCamera->fGetTripod( ).mEye, context.mSceneCamera->fGetLens( ).mFarPlane ) );

		// apply textures
		mColorMap.fApply( device, 0 );
	}

	void tFullBrightMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
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
