#include "BasePch.hpp"
#include "tHeightFieldMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tRenderContext.hpp"
#include "tRenderBatch.hpp"
#include "tDrawCall.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f32_3 ),
			//tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tVertexElement gCompressedVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f16_4 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f16_2 ),
			//tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tFilePathPtr cHeightFieldPath( "Shaders/Engine/HeightField.mtlb" );
	}

	///
	/// \section tLightingPass
	///

	tHeightFieldMaterial::tLightingPass::tLightingPass( )
		: mVS( cVSDiffuse )
		, mPS( cPSDiffuse )
	{
	}

	tHeightFieldMaterial::tLightingPass::tLightingPass( tNoOpTag )
	{
	}

	void tHeightFieldMaterial::tLightingPass::fApplyShared( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const tMaterialFile* mtlFile = mtl.mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		tVS vs = mVS;
		tPS ps = mPS;

		// Optimization to disable normal maps on terrain if viewport count is high
		if( context.mViewportCount > 1 )
		{
			ps = cPSDiffuse;
			if( vs == cVSDiffuseNormalMap )
				vs = cVSDiffuse;
			else if( vs == cVSDiffuseNormalMapCompressed )
				vs = cVSDiffuseCompressed; 
		}

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryLightingVS, vs );
		mtlFile->fApplyShader( device, cShaderCategoryLightingPS, fPixelShaderSlot( ps, context.mLightShaderConstants.mLightCount ) );

		// vertex shader constants
		mtl.fApplyMatrix4VS( device, cVSWorldToProj,		context.mCamera->fGetWorldToProjection( ) );
		mtl.fApplyVector4VS( device, cVSWorldEyePos,		Math::tVec4f( context.mCamera->fGetTripod( ).mEye, context.mCamera->fGetLens( ).mFarPlane ) );
		if( context.mShadowMapRealLayerCount <= 1 )
			mtl.fApplyMatrix4VS( device, cVSWorldToLight,		context.mWorldToLightSpace[ 0 ] );

		u32 numLayers = 1;
		const tTextureFile* diffuseMapAtlas = mtl.mDiffuseMap.fGetTextureFile( );
		if( diffuseMapAtlas )
			numLayers = diffuseMapAtlas->mSubTexCountX * diffuseMapAtlas->mSubTexCountY;

		// pixel shader constants
		mtl.fApplyVector3PS( device, cPSWorldEyePos,		context.mCamera->fGetTripod( ).mEye );
		mtl.fApplyVector4PS( device, cPSWorldSpaceDims_TextureAtlasDims, 
			Math::tVec4f( mtl.mWorldSpaceDims.x, mtl.mWorldSpaceDims.y, ( f32 )mtl.mTextureAtlasDims.x, ( f32 )mtl.mTextureAtlasDims.y ) );
		const Math::tVec2f texAtlasPixelDims( 
			fMax( mtl.mSubDiffuseRectDims.x, mtl.mSubNormalRectDims.x ),
			fMax( mtl.mSubDiffuseRectDims.y, mtl.mSubNormalRectDims.y ) );
		mtl.fApplyVector4PS( device, cPSTextureAtlasPixelDims_LayerCount,
			Math::tVec4f( texAtlasPixelDims.x, texAtlasPixelDims.y, ( f32 )numLayers, 0.f ) );
		mtl.fApplyVector4PS( device, cPSFogValues, context.mFogValues );
		mtl.fApplyVector3PS( device, cPSFogColor, context.mFogColor );
		mtl.fApplyVector4PS( device, cPSShadowMapEpsilon_TexelSize_Amount_Split, Math::tVec4f( 0.0001f/*context.mShadowMapEpsilon*/, ( f32 )context.mShadowMapTexelSize, context.mShadowAmount, context.mShadowMapCascadeSplit[1] ) );
		mtl.fApplyVector4PS( device, cPSShadowMapTarget_Split, Math::tVec4f( context.mShadowMapTarget, context.mShadowMapCascadeSplit[0] ) );
		mtl.fApplyVector4PS( device, cPSTilingFactors, mtl.mTileFactors[0], mtl.mTileFactors.fCount( ) );

		if( context.mShadowMapRealLayerCount > 1 )
			mtl.fApplyMatrix4PS( device, cPSWorldToLightArray, context.mWorldToLightSpace[0], context.mWorldToLightSpace.fCount( ) );

		// set all light vectors to pixel shader
		mtl.fApplyLights( device, cPSLightVectorFirst, context.mLightShaderConstants );

		// apply shadow map
		sigassert( context.mShadowMap );
		context.mShadowMap->fApply( device, 0 );

		// apply material textures
		mtl.mMaskMap.fApply( device, 1 );
		mtl.mMtlIdsMap.fApply( device, 2 );
		mtl.mDiffuseMap.fApply( device, 3 );
		if( ps == cPSDiffuseNormalMap )
			mtl.mNormalMap.fApply( device, 4 );
	}

	void tHeightFieldMaterial::tLightingPass::fApplyInstance( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		// vertex shader constants
		mtl.fApplyObjectToWorldVS( device, cVSLocalToWorld, cVSLocalToWorld, drawCall, context );

		// set pixel shader constants
		mtl.fApplyVector4PS( device, cPSRgbaTint, drawCall.fInstanceRgbaTint( ) );
	}

	u32 tHeightFieldMaterial::tLightingPass::fPixelShaderSlot( tPS shaderSlot, u32 numLights )
	{
		sigassert( numLights < tMaterial::cLightSlotCount );
		return shaderSlot + numLights * cPSCount;
	}


	///
	/// \section tShadowMapPass
	///

	tHeightFieldMaterial::tShadowMapPass::tShadowMapPass( )
		: mVS( cVSDepth )
		, mPS( cPSDepth )
	{
	}

	tHeightFieldMaterial::tShadowMapPass::tShadowMapPass( tNoOpTag )
	{
	}

	void tHeightFieldMaterial::tShadowMapPass::fApplyShared( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const tMaterialFile* mtlFile = mtl.mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		sigassert( mtlFile );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryShadowMapVS, mVS );
		mtlFile->fApplyShader( device, cShaderCategoryShadowMapPS, mPS );

		// vertex shader constants
		mtl.fApplyMatrix4VS( device, cVSWorldToProj,		context.mCamera->fGetWorldToProjection( ) );
	}

	void tHeightFieldMaterial::tShadowMapPass::fApplyInstance( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		// vertex shader constants
		mtl.fApplyObjectToWorldVS( device, cVSLocalToWorld, cVSLocalToWorld, drawCall, context );
	}




	///
	/// \section tHeightFieldMaterial
	///

	const tVertexFormat tHeightFieldMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );
	const tVertexFormat tHeightFieldMaterial::cCompressedVertexFormat( gCompressedVertexFormatElements, array_length( gCompressedVertexFormatElements ) );

	tFilePathPtr tHeightFieldMaterial::fMaterialFilePath( )
	{
		return cHeightFieldPath;
	}

	tHeightFieldMaterial::tHeightFieldMaterial( )
		: mVertexFormatSlot( 0 )
		, mWorldSpaceDims( 256.f )
		, mTextureAtlasDims( 1 )
		, mSubDiffuseRectDims( 512.f )
		, mSubNormalRectDims( 512.f )
	{
		for( u32 i = 0; i < mTileFactors.fCount( ); ++i )
			mTileFactors[ i ] = Math::tVec4f( 0.5f );
	}

	tHeightFieldMaterial::tHeightFieldMaterial( tNoOpTag )
		: tMaterial( cNoOpTag )
		, mLightingPass( cNoOpTag )
		, mShadowMapPass( cNoOpTag )
		, mMaskMap( cNoOpTag )
		, mMtlIdsMap( cNoOpTag )
		, mDiffuseMap( cNoOpTag )
		, mNormalMap( cNoOpTag )
		, mWorldSpaceDims( cNoOpTag )
		, mTextureAtlasDims( cNoOpTag )
		, mSubDiffuseRectDims( cNoOpTag )
		, mSubNormalRectDims( cNoOpTag )
		, mTileFactors( cNoOpTag )
	{
	}

	void tHeightFieldMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		if( context.mRenderPassMode == tRenderState::cRenderPassLighting )
			mLightingPass.fApplyShared( *this, device, context );
		else if( context.mRenderPassMode == tRenderState::cRenderPassShadowMap || context.mRenderPassMode == tRenderState::cRenderPassDepth )
			mShadowMapPass.fApplyShared( *this, device, context );
	}

	void tHeightFieldMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		if( context.mRenderPassMode == tRenderState::cRenderPassLighting )
			mLightingPass.fApplyInstance( *this, device, context, drawCall );
		else if( context.mRenderPassMode == tRenderState::cRenderPassShadowMap || context.mRenderPassMode == tRenderState::cRenderPassDepth )
			mShadowMapPass.fApplyInstance( *this, device, context, drawCall );

		renderBatch.fRenderInstance( device );
	}

}}
