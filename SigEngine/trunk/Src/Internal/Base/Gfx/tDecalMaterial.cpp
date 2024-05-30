#include "BasePch.hpp"
#include "tDecalMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tRenderBatch.hpp"
#include "tRenderInstance.hpp"
#include "tRenderContext.hpp"
#include "Gui/tRenderableCanvas.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticTangent, tVertexElement::cFormat_f32_4 ),
			tVertexElement( tVertexElement::cSemanticTexCoord, tVertexElement::cFormat_f32_2 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};
		const tVertexElement gCompressedVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f16_4 ),
			tVertexElement( tVertexElement::cSemanticNormal, tVertexElement::cFormat_f16_4 ),
			tVertexElement( tVertexElement::cSemanticTangent, tVertexElement::cFormat_f16_4 ),
			tVertexElement( tVertexElement::cSemanticTexCoord, tVertexElement::cFormat_f16_2 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
		};

		const tFilePathPtr cDecalPath( "Shaders/Engine/Decal.mtlb" );
	}

	///
	/// \section tDecalMaterial
	///

	devvar( bool, Lod_DecalShadows, true );
	devvar( bool, Lod_DecalNormals, true );

	devvar( bool, Renderer_Shadows_DepthEps_DecalTwoSidedOverride, true );
	Math::tVec4f Renderer_Shadows_DepthEps_DecalTwoSided( 0.001f, 0.001f, 0.001f, 0.001f );
	devvarptr_p( f32, Renderer_Shadows_DepthEps_DecalTwoSidedX, Renderer_Shadows_DepthEps_DecalTwoSided.x, 4 );
	devvarptr_p( f32, Renderer_Shadows_DepthEps_DecalTwoSidedY, Renderer_Shadows_DepthEps_DecalTwoSided.y, 4 );
	devvarptr_p( f32, Renderer_Shadows_DepthEps_DecalTwoSidedZ, Renderer_Shadows_DepthEps_DecalTwoSided.z, 4 );
	devvarptr_p( f32, Renderer_Shadows_DepthEps_DecalTwoSidedW, Renderer_Shadows_DepthEps_DecalTwoSided.w, 4 );

	const tVertexFormat tDecalMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	void tDecalMaterial::fApplySharedTex( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		if( !mDiffuseMap.fGetTextureFile( ) )
			return;

		u32 ps = mPS, vs = mVS;

		// Remove shadows
		if( !Lod_DecalShadows || !renderBatch.fBehaviorRecieveShadow( ) )
		{
			if( ps == cPSDiffuseShadow )
				ps = cPSDiffuse;
			else if( ps == cPSDiffuseNormalShadow )
				ps = cPSDiffuseNormal;
		}

		// Remove normals
		if( !Lod_DecalNormals || renderBatch.fBehaviorNoNormalMaps( ) )
		{
			if( ps == cPSDiffuseNormal )
				ps = cPSDiffuse;
			else if( ps == cPSDiffuseNormalShadow )
				ps = cPSDiffuseShadow;

			if( vs == cVSDiffuseNormal )
				vs = cVSDiffuse;
		}

		const b32 shadows = ( ps == cPSDiffuseShadow || ps == cPSDiffuseNormalShadow );

		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		sigassert( mtlFile );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryVS, vs );
		mtlFile->fApplyShader( device, cShaderCategoryPS, fPixelShaderSlot( ps, context.mLightShaderConstants.mLightCount ) );

		// vertex shader constants
		fApplyMatrix4VS( device, cVSWorldToProj,		context.mSceneCamera->fGetWorldToProjection( ) );
		fApplyVector4VS( device, cVSWorldEyePos,		Math::tVec4f( context.mSceneCamera->fGetTripod( ).mEye, context.mSceneCamera->fGetLens( ).mFarPlane ) );
		if( context.mShadowMapRealLayerCount <= 1 )
			fApplyMatrix4VS( device, cVSWorldToLight,		context.mWorldToLightSpace[ 0 ] );

		// set pixel shader constants
		const b32 overrideTwoSidedDepthEps
			=	fGetRenderState( ).fQuery( tRenderState::cPolyTwoSided )
			&&	Renderer_Shadows_DepthEps_DecalTwoSidedOverride;

		const Math::tVec4f epsilon
			=	overrideTwoSidedDepthEps ? Renderer_Shadows_DepthEps_DecalTwoSided
			:	context.mShadowMapEpsilon;

		fApplyVector4PS( device, cPSDiffuseUvXform,		mDiffuseUvXform );
		fApplyVector4PS( device, cPSNormalUvXform,		mNormalUvXform );
		fApplyVector4PS( device, cPSBumpDepth_SpecSize_Opacity_BackFaceFlip,	mBumpDepth_SpecSize_Opacity_BackFaceFlip );
		fApplyVector4PS( device, cPSFogValues,			context.mFogValues );
		fApplyVector3PS( device, cPSFogColor,			context.mFogColor );
		fApplyVector3PS( device, cPSWorldEyePos,		context.mSceneCamera->fGetTripod( ).mEye );

		if( shadows )
		{
			fApplyVector4PS( device, cPSShadowMapEpsilon,	epsilon );
			fApplyVector4PS( device, cPSShadowMapEpsilon_TexelSize_Amount_Split, Math::tVec4f( epsilon[0], ( f32 )context.mShadowMapTexelSize, context.mShadowAmount, context.mShadowMapCascadeSplit[1] ) );
			fApplyVector4PS( device, cPSShadowMapTarget_Split, Math::tVec4f( context.mShadowMapTarget, context.mShadowMapCascadeSplit[0] ) );
		}

		if( context.mShadowMapRealLayerCount > 1 )
			fApplyMatrix4PS( device, cPSWorldToLightArray, context.mWorldToLightSpace[0], context.mWorldToLightSpace.fCount( ) );

		// set all light vectors to pixel shader
		fApplyLights( device, cPSLightVectorFirst, context.mLightShaderConstants );

		// apply shadow map
		if( shadows )
		{
			sigassert( context.mShadowMap );
			context.mShadowMap->fApply( device, 0 );
		}

		// apply material textures: order matters!
		mDiffuseMap.fApply( device, 1 );

		if( ( ps == cPSDiffuseNormal || ps == cPSDiffuseNormalShadow ) && mNormalMap.fGetTextureFile( ) )
			mNormalMap.fApply( device, 2 );
	}

	void tDecalMaterial::fApplyInstanceTex( const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		// vertex shader constants
		fApplyObjectToWorldVS( device, cVSLocalToWorldPos, cVSLocalToWorldNorm, drawCall, context );

		// set pixel shader constants
		fApplyVector4PS( device, cPSRgbaTint, drawCall.fInstanceRgbaTint( ) );
	}

	void tDecalMaterial::fApplySharedFB( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		sigassert( sizeof( tDecalRenderVertex ) == cVertexFormat.fVertexSize( ) );

		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryVS, mVS );
		mtlFile->fApplyShader( device, cShaderCategoryPS, mPS );

		// vertex shader constants
		fApplyMatrix4VS( device, cVSWorldToProj, context.mSceneCamera->fGetWorldToProjection( ) );
		fApplyVector4VS( device, cVSWorldEyePos, Math::tVec4f( context.mSceneCamera->fGetTripod( ).mEye, context.mSceneCamera->fGetLens( ).mFarPlane ) );

		// apply textures
		mDiffuseMap.fApply( device, 1 );
	}

	void tDecalMaterial::fApplyInstanceFB( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );

		// vertex shader constants
		fApplyMatrix3VS( device, cVSLocalToWorldPos, instance.fRI_ObjectToWorld( ) );

		// set pixel shader constants
		fApplyVector4PS( device, cPSRgbaTint, drawCall.fInstanceRgbaTint( ) );

		renderBatch.fRenderInstance( device );
	}

	u32 tDecalMaterial::fPixelShaderSlot( u32 shaderSlot, u32 numLights )
	{
		sigassert( numLights < tMaterial::cLightSlotCount );
		return shaderSlot + numLights * cPSNumLit;
	}

	tFilePathPtr tDecalMaterial::fMaterialFilePath( )
	{
		return cDecalPath;
	}

	tDecalMaterial::tDecalMaterial( )
		: mVS( cVSFullBright )
		, mPS( cPSFullBright )
		, mDiffuseUvXform( 1.f, 1.f, 0.f, 0.f )
		, mNormalUvXform( 1.f, 1.f, 0.f, 0.f )
		, mBumpDepth_SpecSize_Opacity_BackFaceFlip( 1.f, 0.f, 1.f, 0.f )
	{
	}

	tDecalMaterial::tDecalMaterial( tNoOpTag )
		: tMaterial( cNoOpTag )
		, mDiffuseMap( cNoOpTag )
		, mNormalMap( cNoOpTag )
		, mDiffuseUvXform( cNoOpTag )
		, mNormalUvXform( cNoOpTag )
		, mBumpDepth_SpecSize_Opacity_BackFaceFlip( cNoOpTag )
	{
	}

	b32 tDecalMaterial::fIsLit( ) const
	{
		return mPS != cPSFullBright;
	}

	void tDecalMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		if( context.mRenderPassMode != tRenderState::cRenderPassLighting )
			return;

		if( mPS == cPSFullBright )
		{
			fApplySharedFB( device, context, renderBatch );
		}
		else
		{
			// Both diffuse and diffuse+norm use the same set up and application.
			fApplySharedTex( device, context, renderBatch );
		}
	}

	void tDecalMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		if( context.mRenderPassMode != tRenderState::cRenderPassLighting )
			return;

		if( mPS == cPSFullBright )
			fApplyInstanceFB( device, context, renderBatch, drawCall );
		else
		{
			// Both diffuse and diffuse+norm use the same set up and application.
			fApplyInstanceTex( device, context, drawCall );
		}

		renderBatch.fRenderInstance( device );
	}

	void tDecalMaterial::fSetAcceptsLights( b32 acceptLights )
	{
		if( !acceptLights )
		{
			mPS = cPSFullBright;
			mVS = cVSFullBright;
			return;
		}

		if( mNormalMap.fGetTextureFile( ) )
		{
			mPS = cPSDiffuseNormalShadow;
			mVS = cVSDiffuseNormal;
			return;
		}

		mPS = cPSDiffuseShadow;
		mVS = cVSDiffuse;
	}

}}
