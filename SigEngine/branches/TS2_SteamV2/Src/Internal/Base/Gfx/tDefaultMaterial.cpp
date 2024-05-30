#include "BasePch.hpp"
#include "tDefaultMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tRenderBatch.hpp"
#include "tRenderContext.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tFilePathPtr cMaterialPath( "Shaders/Engine/Default.mtlb" );
	}
	const tVertexFormat tDefaultMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	tFilePathPtr tDefaultMaterial::fMaterialFilePath( )
	{
		return cMaterialPath;
	}

	tDefaultMaterial::tDefaultMaterial( )
	{
	}

	tDefaultMaterial::tDefaultMaterial( tNoOpTag )
		: tMaterial( cNoOpTag )
	{
	}

	void tDefaultMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, 0, cShaderSlotVS );
		mtlFile->fApplyShader( device, 0, cShaderSlotPS );

		// set camera->projection matrix
		fApplyMatrix4VS( device, cVSWorldToProj, context.mCamera->fGetWorldToProjection( ) );
		fApplyVector4VS( device, cVSWorldEyePos, Math::tVec4f( context.mCamera->fGetTripod( ).mEye, context.mCamera->fGetLens( ).mFarPlane ) );
	}

	void tDefaultMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		fApplyObjectToWorldVS( device, cVSLocalToWorld, cVSLocalToWorld, drawCall, context );

		renderBatch.fRenderInstance( device );
	}

}}
