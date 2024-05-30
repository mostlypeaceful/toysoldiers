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

	const tVertexFormat tDecalMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	void tDecalMaterial::fApplySharedTex( const tDevicePtr& device, const tRenderContext& context ) const
	{
		if( !mDiffuseMap.fGetTextureFile( ) )
			return;

		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		sigassert( mtlFile );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, cShaderCategoryVS, mVS );
		mtlFile->fApplyShader( device, cShaderCategoryPS, fPixelShaderSlot( mPS, context.mLightShaderConstants.mLightCount ) );

		// vertex shader constants
		fApplyMatrix4VS( device, cVSWorldToProj,		context.mCamera->fGetWorldToProjection( ) );
		fApplyVector4VS( device, cVSWorldEyePos,		Math::tVec4f( context.mCamera->fGetTripod( ).mEye, context.mCamera->fGetLens( ).mFarPlane ) );
		if( context.mShadowMapRealLayerCount <= 1 )
			fApplyMatrix4VS( device, cVSWorldToLight,		context.mWorldToLightSpace[ 0 ] );

		// set pixel shader constants
		const f32 epsilon = fGetRenderState( ).fQuery( tRenderState::cPolyTwoSided ) ? 0.001f : context.mShadowMapEpsilon;
		fApplyVector4PS( device, cPSDiffuseUvXform,		mDiffuseUvXform );
		fApplyVector4PS( device, cPSNormalUvXform,		mNormalUvXform );
		fApplyVector4PS( device, cPSBumpDepth_SpecSize_Opacity_BackFaceFlip,	mBumpDepth_SpecSize_Opacity_BackFaceFlip );
		fApplyVector4PS( device, cPSFogValues,			context.mFogValues );
		fApplyVector3PS( device, cPSFogColor,			context.mFogColor );
		fApplyVector3PS( device, cPSWorldEyePos,		context.mCamera->fGetTripod( ).mEye );
		fApplyVector4PS( device, cPSShadowMapEpsilon_TexelSize_Amount_Split, Math::tVec4f( epsilon, ( f32 )context.mShadowMapTexelSize, context.mShadowAmount, context.mShadowMapCascadeSplit[1] ) );
		fApplyVector4PS( device, cPSShadowMapTarget_Split, Math::tVec4f( context.mShadowMapTarget, context.mShadowMapCascadeSplit[0] ) );

		if( context.mShadowMapRealLayerCount > 1 )
			fApplyMatrix4PS( device, cPSWorldToLightArray, context.mWorldToLightSpace[0], context.mWorldToLightSpace.fCount( ) );

		// set all light vectors to pixel shader
		fApplyLights( device, cPSLightVectorFirst, context.mLightShaderConstants );

		// apply shadow map
		sigassert( context.mShadowMap );
		context.mShadowMap->fApply( device, 0 );

		// apply material textures: order matters!
		mDiffuseMap.fApply( device, 1 );

		if( mNormalMap.fGetTextureFile( ) )
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
		fApplyMatrix4VS( device, cVSWorldToProj, context.mCamera->fGetWorldToProjection( ) );
		fApplyVector4VS( device, cVSWorldEyePos, Math::tVec4f( context.mCamera->fGetTripod( ).mEye, context.mCamera->fGetLens( ).mFarPlane ) );

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

	u32 tDecalMaterial::fPixelShaderSlot( tPS shaderSlot, u32 numLights )
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
			fApplySharedTex( device, context );
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
			mPS = cPSDiffuseNormal;
			mVS = cVSDiffuseNormal;
			return;
		}

		mPS = cPSDiffuse;
		mVS = cVSDiffuse;
	}

}}
