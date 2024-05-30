#include "BasePch.hpp"
#include "tPhongMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tDrawCall.hpp"
#include "tRenderBatch.hpp"
#include "tRenderContext.hpp"
#include "tSkinMap.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElementsPosNormalColor[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tVertexElement gVertexFormatElementsPosColorUv0[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticTexCoord, tVertexElement::cFormat_f32_2 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tVertexElement gVertexFormatElementsPosNormalColorUv0[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticTexCoord, tVertexElement::cFormat_f32_2 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tVertexElement gVertexFormatElementsPosNormalTangentColorUv0[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticTangent, tVertexElement::cFormat_f32_4 ),
			tVertexElement( tVertexElement::cSemanticTexCoord, tVertexElement::cFormat_f32_2 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};

		const tVertexElement gVertexFormatElementsPosNormalColor_Skinned[]=
		{
			gVertexFormatElementsPosNormalColor[ 0 ],
			gVertexFormatElementsPosNormalColor[ 1 ],
			gVertexFormatElementsPosNormalColor[ 2 ],
			tVertexElement( tVertexElement::cSemanticBoneWeights, tVertexElement::cFormat_u8_4_Normalized ),
			tVertexElement( tVertexElement::cSemanticBoneIndices, tVertexElement::cFormat_u8_4 ),
		};
		const tVertexElement gVertexFormatElementsPosColorUv0_Skinned[]=
		{
			gVertexFormatElementsPosColorUv0[ 0 ],
			gVertexFormatElementsPosColorUv0[ 1 ],
			gVertexFormatElementsPosColorUv0[ 2 ],
			tVertexElement( tVertexElement::cSemanticBoneWeights, tVertexElement::cFormat_u8_4_Normalized ),
			tVertexElement( tVertexElement::cSemanticBoneIndices, tVertexElement::cFormat_u8_4 ),
		};
		const tVertexElement gVertexFormatElementsPosNormalColorUv0_Skinned[]=
		{
			gVertexFormatElementsPosNormalColorUv0[ 0 ],
			gVertexFormatElementsPosNormalColorUv0[ 1 ],
			gVertexFormatElementsPosNormalColorUv0[ 2 ],
			gVertexFormatElementsPosNormalColorUv0[ 3 ],
			tVertexElement( tVertexElement::cSemanticBoneWeights, tVertexElement::cFormat_u8_4_Normalized ),
			tVertexElement( tVertexElement::cSemanticBoneIndices, tVertexElement::cFormat_u8_4 ),
		};
		const tVertexElement gVertexFormatElementsPosNormalTangentColorUv0_Skinned[]=
		{
			gVertexFormatElementsPosNormalTangentColorUv0[ 0 ],
			gVertexFormatElementsPosNormalTangentColorUv0[ 1 ],
			gVertexFormatElementsPosNormalTangentColorUv0[ 2 ],
			gVertexFormatElementsPosNormalTangentColorUv0[ 3 ],
			gVertexFormatElementsPosNormalTangentColorUv0[ 4 ],
			tVertexElement( tVertexElement::cSemanticBoneWeights, tVertexElement::cFormat_u8_4_Normalized ),
			tVertexElement( tVertexElement::cSemanticBoneIndices, tVertexElement::cFormat_u8_4 ),
		};
		const tFilePathPtr cPhongPath( "Shaders/Engine/Phong.mtlb" );
	}



	///
	/// \section tLightingPass
	///

	tPhongMaterial::tLightingPass::tLightingPass( )
		: mVS( cVSCount )
		, mPS( cPSCount )
	{
	}

	tPhongMaterial::tLightingPass::tLightingPass( tNoOpTag )
	{
	}

	void tPhongMaterial::tLightingPass::fSetSkinned( b32 skinned )
	{
		if( skinned && mVS < cVSNonInstancedCount / 2)
			mVS = tEnum<tVS,u16>( tVS( mVS + cVSNonInstancedCount / 2) );
	}

	void tPhongMaterial::tLightingPass::fApplyShared( const tPhongMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const tMaterialFile* mtlFile = mtl.mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		sigassert( mtlFile );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryLightingVS, mVS );
		mtlFile->fApplyShader( device, cShaderCategoryLightingPS, fPixelShaderSlot( mPS, context.mLightShaderConstants.mLightCount ) );

		// apply shadow map
		sigassert( context.mShadowMap );
		context.mShadowMap->fApply( device, 0 );

		// apply material textures: order matters!
		u32 texSlot = 1;
		mtl.fApplyTexture( device, texSlot, mtl.mDiffuseMap );
		mtl.fApplyTexture( device, texSlot, mtl.mSpecColorMap );
		mtl.fApplyTexture( device, texSlot, mtl.mEmissiveMap );
		mtl.fApplyTexture( device, texSlot, mtl.mNormalMap );

		// vertex shader constants
		mtl.fApplyMatrix4VS( device, cVSWorldToProj,		context.mCamera->fGetWorldToProjection( ) );
		mtl.fApplyVector4VS( device, cVSWorldEyePos,		Math::tVec4f( context.mCamera->fGetTripod( ).mEye, context.mCamera->fGetLens( ).mFarPlane ) );
		if( context.mShadowMapRealLayerCount <= 1 )
			mtl.fApplyMatrix4VS( device, cVSWorldToLight,		context.mWorldToLightSpace[ 0 ] );

		// set pixel shader constants
		const f32 epsilon = mtl.fGetRenderState( ).fQuery( tRenderState::cPolyTwoSided ) ? 0.001f : context.mShadowMapEpsilon;
		mtl.fApplyVector4PS( device, cPSDiffuseUvXform,		mtl.mDiffuseUvXform );
		mtl.fApplyVector4PS( device, cPSSpecColorUvXform,	mtl.mSpecColorUvXform );
		mtl.fApplyVector4PS( device, cPSEmissiveUvXform,	mtl.mEmissiveUvXform );
		mtl.fApplyVector4PS( device, cPSNormalUvXform,		mtl.mNormalUvXform );
		mtl.fApplyVector4PS( device, cPSDiffuseColor,		mtl.mDiffuseColor );
		mtl.fApplyVector4PS( device, cPSSpecColor,			mtl.mSpecColor );
		mtl.fApplyVector4PS( device, cPSEmissiveColor,		mtl.mEmissiveColor );
		mtl.fApplyVector4PS( device, cPSBumpDepth_SpecSize_Opacity_BackFaceFlip,	mtl.mBumpDepth_SpecSize_Opacity_BackFaceFlip );
		mtl.fApplyVector4PS( device, cPSFogValues,			context.mFogValues );
		mtl.fApplyVector3PS( device, cPSFogColor,			context.mFogColor );
		mtl.fApplyVector3PS( device, cPSWorldEyePos,		context.mCamera->fGetTripod( ).mEye );
		mtl.fApplyVector4PS( device, cPSShadowMapEpsilon_TexelSize_Amount_Split, Math::tVec4f( epsilon, ( f32 )context.mShadowMapTexelSize, context.mShadowAmount, context.mShadowMapCascadeSplit[1] ) );
		mtl.fApplyVector4PS( device, cPSShadowMapTarget_Split, Math::tVec4f( context.mShadowMapTarget, context.mShadowMapCascadeSplit[0] ) );

		if( context.mShadowMapRealLayerCount > 1 )
			mtl.fApplyMatrix4PS( device, cPSWorldToLightArray, context.mWorldToLightSpace[0], context.mWorldToLightSpace.fCount( ) );

		// set all light vectors to pixel shader
		mtl.fApplyLights( device, cPSLightVectorFirst, context.mLightShaderConstants );
	}

	void tPhongMaterial::tLightingPass::fApplyInstance( const tPhongMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );

		// vertex shader constants
		mtl.fApplyObjectToWorldVS( device, cVSLocalToWorldPos, cVSLocalToWorldNorm, drawCall, context );

		const tSkinMap* skinMap = instance.fRI_SkinMap( );
		if( skinMap )
		{
			u32 numEntries;
			tSkinMap::tScratchMatrixPalette palette;

			skinMap->fFillScratchMatrixPalette( palette, numEntries );

			mtl.fApplyMatrixPaletteVS( device, cVSMatrixPalette, palette.fBegin( ), numEntries );
		}

		// set pixel shader constants
		mtl.fApplyVector4PS( device, cPSRgbaTint, drawCall.fInstanceRgbaTint( ) );
	}

	u32 tPhongMaterial::tLightingPass::fPixelShaderSlot( tPS shaderSlot, u32 numLights )
	{
		sigassert( numLights < tMaterial::cLightSlotCount );
		return shaderSlot + numLights * cPSCount;
	}



	///
	/// \section tShadowMapPass
	///

	tPhongMaterial::tShadowMapPass::tShadowMapPass( )
		: mVS( cVSCount )
		, mPS( cPSCount )
	{
	}

	tPhongMaterial::tShadowMapPass::tShadowMapPass( tNoOpTag )
	{
	}

	void tPhongMaterial::tShadowMapPass::fSetSkinned( b32 skinned )
	{
		if( skinned && mVS < cVSNonInstancedCount / 2)
			mVS = tEnum<tVS,u16>( tVS( mVS + cVSNonInstancedCount / 2) );
	}

	void tPhongMaterial::tShadowMapPass::fApplyShared( const tPhongMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const tMaterialFile* mtlFile = mtl.mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		sigassert( mtlFile );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryShadowMapVS, mVS );
		mtlFile->fApplyShader( device, cShaderCategoryShadowMapPS, mPS );

		// vertex shader constants
		mtl.fApplyMatrix4VS( device, cVSWorldToProj,		context.mCamera->fGetWorldToProjection( ) );

		if( mPS == cPSDepthCutOut )
		{
			u32 texSlot = 0;
			mtl.fApplyTexture( device, texSlot, mtl.mDiffuseMap );
		}
	}

	void tPhongMaterial::tShadowMapPass::fApplyInstance( const tPhongMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );

		// vertex shader constants
		mtl.fApplyObjectToWorldVS( device, cVSLocalToWorldPos, cVSLocalToWorldNorm, drawCall, context );

		const tSkinMap* skinMap = instance.fRI_SkinMap( );
		if( skinMap )
		{
			u32 numEntries;
			tSkinMap::tScratchMatrixPalette palette;

			skinMap->fFillScratchMatrixPalette( palette, numEntries );

			mtl.fApplyMatrixPaletteVS( device, cVSMatrixPalette, palette.fBegin( ), numEntries );
		}
	}


	///
	/// \section tPhongMaterial
	///

	const tVertexFormat tPhongMaterial::cVertexFormats[cVertexFormatCount]=
	{
		tVertexFormat( gVertexFormatElementsPosNormalColor, array_length( gVertexFormatElementsPosNormalColor ) ),
		tVertexFormat( gVertexFormatElementsPosColorUv0, array_length( gVertexFormatElementsPosColorUv0 ) ),
		tVertexFormat( gVertexFormatElementsPosNormalColorUv0, array_length( gVertexFormatElementsPosNormalColorUv0 ) ),
		tVertexFormat( gVertexFormatElementsPosNormalTangentColorUv0, array_length( gVertexFormatElementsPosNormalTangentColorUv0 ) ),

		tVertexFormat( gVertexFormatElementsPosNormalColor_Skinned, array_length( gVertexFormatElementsPosNormalColor_Skinned ) ),
		tVertexFormat( gVertexFormatElementsPosColorUv0_Skinned, array_length( gVertexFormatElementsPosColorUv0_Skinned ) ),
		tVertexFormat( gVertexFormatElementsPosNormalColorUv0_Skinned, array_length( gVertexFormatElementsPosNormalColorUv0_Skinned ) ),
		tVertexFormat( gVertexFormatElementsPosNormalTangentColorUv0_Skinned, array_length( gVertexFormatElementsPosNormalTangentColorUv0_Skinned ) ),
	};

	tFilePathPtr tPhongMaterial::fMaterialFilePath( )
	{
		return cPhongPath;
	}

	tPhongMaterial::tPhongMaterial( )
		: mVertexFormatSlot( 0 )
		, mDiffuseMap( 0 )
		, mSpecColorMap( 0 )
		, mEmissiveMap( 0 )
		, mNormalMap( 0 )
		, mDiffuseUvXform( 1.f, 1.f, 0.f, 0.f )
		, mSpecColorUvXform( 1.f, 1.f, 0.f, 0.f )
		, mEmissiveUvXform( 1.f, 1.f, 0.f, 0.f )
		, mNormalUvXform( 1.f, 1.f, 0.f, 0.f )
		, mDiffuseColor( 1.f )
		, mSpecColor( 0.f )
		, mEmissiveColor( 0.f )
		, mBumpDepth_SpecSize_Opacity_BackFaceFlip( 1.f, 0.f, 1.f, 0.f )
	{
	}

	tPhongMaterial::tPhongMaterial( tNoOpTag )
		: tMaterial( cNoOpTag )
		, mLightingPass( cNoOpTag )
		, mShadowMapPass( cNoOpTag )
		, mDiffuseUvXform( cNoOpTag )
		, mSpecColorUvXform( cNoOpTag )
		, mEmissiveUvXform( cNoOpTag )
		, mNormalUvXform( cNoOpTag )
		, mDiffuseColor( cNoOpTag )
		, mSpecColor( cNoOpTag )
		, mEmissiveColor( cNoOpTag )
		, mBumpDepth_SpecSize_Opacity_BackFaceFlip( cNoOpTag )
	{
	}

	void tPhongMaterial::fSetSkinned( b32 skinned )
	{
		mLightingPass.fSetSkinned( skinned );
		mShadowMapPass.fSetSkinned( skinned );
		mVertexFormatSlot = ( u32 )mLightingPass.mVS;
	}

	b32 tPhongMaterial::fIsLit( ) const
	{
		return mLightingPass.mPS != tLightingPass::cPSPosColor_EmissiveMapUv0;
	}

	b32 tPhongMaterial::fSupportsInstancing( ) const
	{
		return 
			!( mMaterialFlags & cFaceMask ) && // we don't support billboarding/facing instances
			 ( mLightingPass.mVS < tLightingPass::cVSNonInstancedCount / 2 ) && // or skinned instances
			 ( mShadowMapPass.mVS < tShadowMapPass::cVSNonInstancedCount / 2 );
	}

	void tPhongMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		if( context.mRenderPassMode == tRenderState::cRenderPassLighting )
			mLightingPass.fApplyShared( *this, device, context );
		else if( context.mRenderPassMode == tRenderState::cRenderPassShadowMap || context.mRenderPassMode == tRenderState::cRenderPassDepth )
			mShadowMapPass.fApplyShared( *this, device, context );
	}

	void tPhongMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		if( context.mRenderPassMode == tRenderState::cRenderPassLighting )
			mLightingPass.fApplyInstance( *this, device, context, drawCall );
		else if( context.mRenderPassMode == tRenderState::cRenderPassShadowMap || context.mRenderPassMode == tRenderState::cRenderPassDepth )
			mShadowMapPass.fApplyInstance( *this, device, context, drawCall );

		renderBatch.fRenderInstance( device );
	}

}}
