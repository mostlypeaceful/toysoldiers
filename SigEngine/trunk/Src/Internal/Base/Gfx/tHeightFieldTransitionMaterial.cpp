#include "BasePch.hpp"
#include "tHeightFieldTransitionMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tRenderContext.hpp"
#include "tRenderBatch.hpp"
#include "tDrawCall.hpp"

namespace Sig { namespace Gfx
{
	tFilePathPtr tHeightFieldTransitionMaterial::fMaterialFilePath( )
	{
		return tFilePathPtr( "Shaders/Engine/HeightFieldTransition.mtlb" );
	}

	//------------------------------------------------------------------------------
	// tLightingPass
	//------------------------------------------------------------------------------
	tHeightFieldTransitionMaterial::tLightingPass::tLightingPass( )
		: mVS( cVSDiffuse )
		, mPS( cPSDiffuse )
	{
	}
	//------------------------------------------------------------------------------
	tHeightFieldTransitionMaterial::tLightingPass::tLightingPass( tNoOpTag )
	{
	}
	//------------------------------------------------------------------------------
	void tHeightFieldTransitionMaterial::tLightingPass::fApplyShared( 
		const tHeightFieldTransitionMaterial& mtl, 
		const tDevicePtr& device, 
		const tRenderContext& context,
		const tRenderBatchData& renderBatch ) const
	{
		const tMaterialFile* mtlFile = mtl.mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		b32 deferredPass = (context.mRenderPassMode == tRenderState::cRenderPassGBuffer);

		tVS vs = mVS;
		tPS ps = mPS;

		// Remove shadows
		if( !fLod_TerrainShadows( ) || !renderBatch.fBehaviorRecieveShadow( ) )
		{
			if( ps == cPSDiffuseNormalMapShadow )
				ps = cPSDiffuseNormalMap;
			else if( ps == cPSDiffuseShadow )
				ps = cPSDiffuse;
		}

		// Optimization to disable normal maps on terrain if viewport count is high
		if( !fLod_TerrainNormals( ) || renderBatch.fBehaviorNoNormalMaps( ) || context.mViewportCount > 1 )
		{
			if( ps == cPSDiffuseNormalMapShadow )
				ps = cPSDiffuseShadow;
			else if( ps == cPSDiffuseNormalMap )
				ps = cPSDiffuse;

			if( vs == cVSDiffuseNormalMap )
				vs = cVSDiffuse;
			else if( vs == cVSDiffuseNormalMapCompressed )
				vs = cVSDiffuseCompressed; 
		}

		const b32 normals = ( ps == cPSDiffuseNormalMap || ps == cPSDiffuseNormalMapShadow );
		const b32 shadows = !deferredPass && ( ps == cPSDiffuseShadow || ps == cPSDiffuseNormalMapShadow );

		//multi-object shader offset
		if( renderBatch.mUserFlags == tHeightFieldTransitionMaterial::cMultiObjectShader )
		{
			switch( ps )
			{
			case cPSDiffuse: ps = cPSDiffuseMultiObject; break;
			case cPSDiffuseShadow: ps = cPSDiffuseShadowMultiObject; break;
			case cPSDiffuseNormalMap: ps = cPSDiffuseNormalMapMultiObject; break;
			case cPSDiffuseNormalMapShadow: ps = cPSDiffuseNormalMapShadowMultiObject; break;
			default: sig_nodefault( );
			};
		}

		if( deferredPass )
		{
			switch( ps )
			{
				//default shader
			case cPSDiffuse:
			case cPSDiffuseShadow:
				ps = cPSDiffuseGBuffer;
				break;
			case cPSDiffuseNormalMap:
			case cPSDiffuseNormalMapShadow:
				ps = cPSDiffuseNormalMapGBuffer;
				break;
				//multi-object shader
			case cPSDiffuseMultiObject:
			case cPSDiffuseShadowMultiObject:
				ps = cPSDiffuseGBufferMultiObject;
				break;
			case cPSDiffuseNormalMapMultiObject:
			case cPSDiffuseNormalMapShadowMultiObject:
				ps = cPSDiffuseNormalMapGBufferMultiObject;
				break;
			}

			vs = (tVS)(mVS + (cVSDiffuseGBuffer - cVSDiffuse));

			ps = fPixelShaderSlot( ps, 0 );
		}
		else
			ps = fPixelShaderSlot( ps, context.mLightShaderConstants.mLightCount );


		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryLightingVS, vs );
		mtlFile->fApplyShader( device, cShaderCategoryLightingPS, ps );

		// vertex shader constants
		mtl.fApplyMatrix4VS( device, cVSWorldToProj,		context.mSceneCamera->fGetWorldToProjection( ) );
		mtl.fApplyVector4VS( device, cVSWorldEyePos,		Math::tVec4f( context.mSceneCamera->fGetTripod( ).mEye, context.mSceneCamera->fGetLens( ).mFarPlane ) );
		if( context.mShadowMapRealLayerCount <= 1 )
			mtl.fApplyMatrix4VS( device, cVSWorldToLight,		context.mWorldToLightSpace[ 0 ] );

		u32 numLayers = 1;
		const tTextureFile* diffuseMapAtlas = mtl.mDiffuseMap.fGetTextureFile( );
		if( diffuseMapAtlas )
			numLayers = diffuseMapAtlas->mSubTexCountX * diffuseMapAtlas->mSubTexCountY;

		// pixel shader constants
		mtl.fApplyVector3PS( device, cPSWorldEyePos,		context.mSceneCamera->fGetTripod( ).mEye );
		mtl.fApplyVector4PS( device, cPSWorldSpaceDims_TextureAtlasMaxPixelDims, Math::tVec4f( 
			mtl.mWorldSpaceDims.x,
			mtl.mWorldSpaceDims.y, 
			fMax( mtl.mSubDiffuseRectDims.x, mtl.mSubNormalRectDims.x ), 
			fMax( mtl.mSubDiffuseRectDims.y, mtl.mSubNormalRectDims.y ) ) );
		mtl.fApplyVector4PS( device, cPSDiffuseCount_NormalCount, mtl.mDiffuseCount_NormalCount );
		mtl.fApplyVector4PS( device, cPSFogValues, context.mFogValues );
		mtl.fApplyVector3PS( device, cPSFogColor, context.mFogColor );

		if( shadows )
		{
			mtl.fApplyVector4PS( device, cPSShadowMapEpsilon, context.mShadowMapEpsilon );
			mtl.fApplyVector4PS( device, cPSShadowMapEpsilon_TexelSize_Amount_Split, Math::tVec4f( context.mShadowMapEpsilon[0], ( f32 )context.mShadowMapTexelSize, context.mShadowAmount, context.mShadowMapCascadeSplit[1] ) );
			mtl.fApplyVector4PS( device, cPSShadowMapTarget_Split, Math::tVec4f( context.mShadowMapTarget, context.mShadowMapCascadeSplit[0] ) );
		}

		mtl.fApplyVector4PS( device, cPSTilingFactors, mtl.mTileFactors[0], mtl.mTileFactors.fCount( ) );

		mtl.fApplyVector4PS( device, cPSTime, Math::tVec4f( context.mTime, context.mPausableTime, 0.f, 1.f ), 1 );

		tTransitionObjectMaterialHelper::fApplyDefaultId( cPSDefaultAlignment, device, mtl );
		tTransitionObjectMaterialHelper::fApplyEdgeColors( cPSAlignmentColors, device, mtl );

		if( context.mShadowMapRealLayerCount > 1 )
			mtl.fApplyMatrix4PS( device, cPSWorldToLightArray, context.mWorldToLightSpace[0], context.mWorldToLightSpace.fCount( ) );

		// set all light vectors to pixel shader
		mtl.fApplyLights( device, cPSLightVectorFirst, context.mLightShaderConstants );

		// apply shadow map
		if( shadows )
		{
			sigassert( context.mShadowMap );
			context.mShadowMap->fApply( device, 0 );
		}

		// apply material textures
		mtl.mMaskMap.fApply( device, 1 );
		mtl.mMtlIdsMap.fApply( device, 2 );
		mtl.mDiffuseMap.fApply( device, 3 );
		if( normals )
			mtl.mNormalMap.fApply( device, 4 );
	}

	//------------------------------------------------------------------------------
	void tHeightFieldTransitionMaterial::tLightingPass::fApplyInstance( const tHeightFieldTransitionMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );

		// vertex shader constants
		mtl.fApplyObjectToWorldVS( device, cVSLocalToWorld, cVSLocalToWorld, drawCall, context );

		// set pixel shader constants
		mtl.fApplyVector4PS( device, cPSRgbaTint, drawCall.fInstanceRgbaTint( ) );

		mtl.fApplyVector4PS( device, cPSSphericalHarmonics, context.gSphericalHarmonics.mFactors[ 0 ], context.gSphericalHarmonics.mFactors.fCount( ) );		
		//mtl.fApplyVector4PS( device, cPSSphericalHarmonics, drawCall.fRenderInstance( ).fSphericalHarmonics( ).mFactors[ 0 ], drawCall.fRenderInstance( ).fSphericalHarmonics( ).mFactors.fCount( ) );		

		if( instance.fRenderBatch( )->fBatchData( ).mUserFlags == tHeightFieldTransitionMaterial::cMultiObjectShader )
			tTransitionObjectMaterialHelper::fApplyTransitionObjects( cPSTransitionObjects, device, mtl, instance );
	}

	//------------------------------------------------------------------------------
	tHeightFieldTransitionMaterial::tLightingPass::tPS tHeightFieldTransitionMaterial::tLightingPass::fPixelShaderSlot( tPS shaderSlot, u32 numLights )
	{
		sigassert( numLights < tMaterial::cLightSlotCount );
		return (tPS)( shaderSlot + numLights * cPSCount );
	}

	//------------------------------------------------------------------------------
	// tShadowMapPass
	//------------------------------------------------------------------------------
	tHeightFieldTransitionMaterial::tShadowMapPass::tShadowMapPass( )
		: mVS( cVSDepth )
		, mPS( cPSDepth )
	{
	}
	//------------------------------------------------------------------------------
	tHeightFieldTransitionMaterial::tShadowMapPass::tShadowMapPass( tNoOpTag )
	{
	}
	//------------------------------------------------------------------------------
	void tHeightFieldTransitionMaterial::tShadowMapPass::fApplyShared( const tHeightFieldTransitionMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const tMaterialFile* mtlFile = mtl.mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		sigassert( mtlFile );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryShadowMapVS, mVS );
		mtlFile->fApplyShader( device, cShaderCategoryShadowMapPS, mPS );

		// vertex shader constants
		mtl.fApplyMatrix4VS( device, cVSWorldToProj, context.mSceneCamera->fGetWorldToProjection( ) );
	}
	//------------------------------------------------------------------------------
	void tHeightFieldTransitionMaterial::tShadowMapPass::fApplyInstance( const tHeightFieldTransitionMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		// vertex shader constants
		mtl.fApplyObjectToWorldVS( device, cVSLocalToWorld, cVSLocalToWorld, drawCall, context );
	}


	//------------------------------------------------------------------------------
	// tHeightFieldTransitionMaterial
	//------------------------------------------------------------------------------
	tHeightFieldTransitionMaterial::tHeightFieldTransitionMaterial( )
	{
	}
	//------------------------------------------------------------------------------
	tHeightFieldTransitionMaterial::tHeightFieldTransitionMaterial( tNoOpTag )
		: tHeightFieldMaterialBase( cNoOpTag )
		, mLightingPass( cNoOpTag )
		, mShadowMapPass( cNoOpTag )
	{
	}
	//------------------------------------------------------------------------------
	void tHeightFieldTransitionMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		switch( context.mRenderPassMode )
		{
		case tRenderState::cRenderPassGBuffer:
		case tRenderState::cRenderPassLighting:
			mLightingPass.fApplyShared( *this, device, context, renderBatch );
			break;
		case tRenderState::cRenderPassShadowMap:
		case tRenderState::cRenderPassDepth:
			mShadowMapPass.fApplyShared( *this, device, context );
			break;
		default: sig_nodefault( );
		};
	}
	//------------------------------------------------------------------------------
	void tHeightFieldTransitionMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		switch( context.mRenderPassMode )
		{
		case tRenderState::cRenderPassGBuffer:
		case tRenderState::cRenderPassLighting:
			mLightingPass.fApplyInstance( *this, device, context, drawCall );
			break;
		case tRenderState::cRenderPassShadowMap:
		case tRenderState::cRenderPassDepth:
			mShadowMapPass.fApplyInstance( *this, device, context, drawCall );
			break;
		default: sig_nodefault( );
		};

		renderBatch.fRenderInstance( device );
	}
	//------------------------------------------------------------------------------
	void tHeightFieldTransitionMaterial::fSetLightingPassToUseNormalMap( )
	{
		mLightingPass.mVS = tLightingPass::cVSDiffuseNormalMap;
		mLightingPass.mPS = tLightingPass::cPSDiffuseNormalMap;
	}
	//------------------------------------------------------------------------------

}}//Sig::Gfx
