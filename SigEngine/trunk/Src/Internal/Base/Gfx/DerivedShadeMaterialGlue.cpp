#include "BasePch.hpp"
#include "DerivedShadeMaterialGlue.hpp"
#include "tShadeMaterial.hpp"
#include "tRenderContext.hpp"
#include "tDrawCall.hpp"
#include "tSkinMap.hpp"
#include "tTransitionObjectMaterialHelper.hpp"

#include "Gfx/tDeferredShadingMaterial.hpp" //debugging, temporary

namespace Sig { namespace Gfx
{
	//register_rtti_factory( tVsGlueModelToWorld, true ); // HAXOR/WORKAROUND see tMaterialFile.cpp for instantiation of this macro
	void tVsGlueModelToWorld::fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		mtl.fApplyObjectToWorldVS( device, mPRegister, mNRegister, drawCall, context );
	}

	register_rtti_factory( tVsGlueModelToView, true );
	void tVsGlueModelToView::fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );
		const Math::tMat3f localToView = context.mSceneCamera->fGetWorldToCamera( ) * instance.fRI_ObjectToWorld( );
		mtl.fApplyMatrix3VS( device, mRegister, localToView );
	}

	register_rtti_factory( tVsGlueWorldToProjection, true );
	void tVsGlueWorldToProjection::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyMatrix4VS( device, mRegister, context.mSceneCamera->fGetWorldToProjection( ) );
	}

	register_rtti_factory( tVsGlueViewToProjection, true );
	void tVsGlueViewToProjection::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyMatrix4VS( device, mRegister, context.mSceneCamera->fGetCameraToProjection( ) );
	}

	register_rtti_factory( tVsGlueViewToWorld, true );
	void tVsGlueViewToWorld::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyMatrix3VS( device, mRegister, context.mSceneCamera->fLocalToWorld( ) );
	}

	register_rtti_factory( tVsGlueWorldEyePos, true );
	void tVsGlueWorldEyePos::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4VS( device, mRegister, Math::tVec4f( context.mSceneCamera->fGetTripod( ).mEye, context.mSceneCamera->fGetLens( ).mFarPlane ) );
	}

	register_rtti_factory( tVsGlueWorldToLight, true );
	void tVsGlueWorldToLight::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyMatrix4VS( device, mRegister, context.mWorldToLightSpace[ 0 ] );
	}

	register_rtti_factory( tVsGlueViewToLight, true );
	void tVsGlueViewToLight::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyMatrix4VS( device, mRegister, context.mViewToLightSpace[ 0 ] );
	}

	register_rtti_factory( tVsGlueSkinningPalette, true );
	void tVsGlueSkinningPalette::fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		sigassert_is_main_thread( ); // if this assert is ever hit then the static tScratchMatrixPalette optimization must be removed
		const tRenderInstance& instance = drawCall.fRenderInstance( );
		const tSkinMap* skinMap = instance.fRI_SkinMap( );
		if( skinMap )
		{
			if( skinMap->fInOrder( ) )
			{
				mtl.fApplyMatrixPaletteVS( device, mRegister, skinMap->fBegin( ), skinMap->fCount( ) );
			}
			else
			{
				u32 numEntries;
				static tSkinMap::tScratchMatrixPalette palette;

				skinMap->fFillScratchMatrixPalette( palette, numEntries );

				mtl.fApplyMatrixPaletteVS( device, mRegister, palette.fBegin( ), numEntries );
			}
		}
	}

	register_rtti_factory( tVsGlueShadowMapSplits, true );
	void tVsGlueShadowMapSplits::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4VS( device, mRegister, Math::tVec4f( context.mShadowMapCascadeSplit[0], context.mShadowMapCascadeSplit[1], 0.f, 1.f ) );
	}

	register_rtti_factory( tVsGlueInstancingData, true );
	void tVsGlueInstancingData::fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		const tRenderBatchData& data = drawCall.fRenderBatch( )->fBatchData( );
		const u32 indexCountPerInstance = data.mIndexBuffer->fIndexCountPerInstance( );
		const Math::tVec3f& scalePos = context.mSceneCamera->fGetTripod( ).mEye; //instances scaled up/down in patches to create a similar effect of them fading in
		f32 scaleDistSq = Math::fSquare( drawCall.fRenderInstance( ).fRI_FarFadeOutDistance( ) ); //shader expects this to be pre-squared.
		mtl.fApplyVector4VS( device, mRegister, Math::tVec4f( (f32)indexCountPerInstance, scalePos.x, scalePos.z, scaleDistSq ) );
	}

	register_rtti_factory( tPsGlueBackFacePlusShadowMapParams, true );
	void tPsGlueBackFacePlusShadowMapParams::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const Math::tVec4f epsilon = context.mShadowMapEpsilon;
		mtl.fApplyVector4PS( device, mRegister, Math::tVec4f( glueVals.fBackFaceFlip( ), epsilon[ 0 ], ( f32 )context.mShadowMapTexelSize, context.mShadowAmount ) );
		mtl.fApplyVector4PS( device, mEpsRegister, epsilon );
	}

	register_rtti_factory( tPsGlueShadowMapTargetPos, true );
	void tPsGlueShadowMapTargetPos::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4PS( device, mRegister, Math::tVec4f( context.mShadowMapTarget, 0.f ) );
	}

	register_rtti_factory( tPsGlueShadowMapSplits, true );
	void tPsGlueShadowMapSplits::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4PS( device, mRegister, Math::tVec4f( context.mShadowMapCascadeSplit[0], context.mShadowMapCascadeSplit[1], 0.f, 1.f ) );
	}

	register_rtti_factory( tPsGlueWorldToLightSpaceArray, true );
	void tPsGlueWorldToLightSpaceArray::fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		mtl.fApplyMatrix4PS( device, mRegister, context.mWorldToLightSpace[0], context.mWorldToLightSpace.fCount( ) );
	}

	register_rtti_factory( tPsGlueFog, true );
	void tPsGlueFog::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4PS( device, mPRegister, context.mFogValues );
		mtl.fApplyVector3PS( device, mCRegister, context.mFogColor );
	}

	register_rtti_factory( tPsGlueFlatParticleColor, true );
	void tPsGlueFlatParticleColor::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4PS( device, mRegister, context.mFlatParticleColor );
	}

	register_rtti_factory( tPsGlueTime, true );
	void tPsGlueTime::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4PS( device, mRegister, Math::tVec4f( context.mTime, context.mPausableTime, 0.f, 1.f ) );
	}

	register_rtti_factory( tPsGlueInstanceTint, true );
	void tPsGlueInstanceTint::fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		mtl.fApplyVector4PS( device, mRegister, drawCall.fInstanceRgbaTint( ) );
	}

	register_rtti_factory( tPsGlueDynamicVec4, true );
	void tPsGlueDynamicVec4::fApplyInstance( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );
		const tStringPtr& name = glueVals.fFindString( mNameIndex );
		const Math::tVec4f v = instance.fRI_DynamicVec4( name, context.mViewportIndex );
		mtl.fApplyVector4PS( device, mRegister, v );
	}

	register_rtti_factory( tPsGlueRimLightParams, true );
	void tPsGlueRimLightParams::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyRimLight( device, mRegister, context.mRimLightConstants );
	}

	register_rtti_factory( tPsGlueLightParams, true );
	void tPsGlueLightParams::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		sigassert( context.mLightShaderConstants.mLightCount <= mNumLights );
		mtl.fApplyLights( device, mRegister, context.mLightShaderConstants );
	}

	register_rtti_factory( tPsGlueShadowMap, true );
	void tPsGlueShadowMap::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		sigassert( context.mShadowMap );
		context.mShadowMap->fApply( device, mRegister );
	}

	register_rtti_factory( tPsGlueWorldEyePos, true );
	void tPsGlueWorldEyePos::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4PS( device, mRegister, Math::tVec4f( context.mSceneCamera->fGetTripod( ).mEye, context.mSceneCamera->fGetLens( ).mFarPlane ) );
	}

	register_rtti_factory( tPsGlueMaterialVector, true );
	void tPsGlueMaterialVector::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const Math::tVec4f* v = glueVals.fFindVector( mVectorIndex );
		mtl.fApplyVector4PS( device, mRegister, v ? *v : Math::tVec4f::cOnesVector );
	}

	register_rtti_factory( tPsGlueMaterialSampler, true );
	void tPsGlueMaterialSampler::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const tTextureReference& tr = glueVals.fFindSampler( mSamplerIndex, context );
		tr.fApply( device, mRegister );
	}

	//------------------------------------------------------------------------------
	// tPsGlueMaterialAtlasSampler
	//------------------------------------------------------------------------------
	register_rtti_factory( tPsGlueMaterialAtlasSampler, true );
	void tPsGlueMaterialAtlasSampler::fApplyShared( 
			const tMaterial& mtl, 
			const tShadeMaterialGlueValues& glueVals, 
			const tDevicePtr& device, 
			const tRenderContext& context ) const
	{
		Math::tVec4f atlasInfo;
		const tTextureReference & tr = glueVals.fFindAtlasSampler( mSamplerIndex, context, atlasInfo );
		
		if( !tr.fApply( device, mSamplerRegister ) )
		{
			log_warning_nospam( "Failed to apply texture atlas - setting white texture" );
			context.mWhiteTexture.fApply( device, mSamplerRegister );
			atlasInfo = Math::tVec4f( 1 );
		}

		if( const tTextureFile * file = tr.fGetTextureFile( ) )
		{
			Math::tVec4f info;
			info.x = file->mSubTexCountX;
			info.y = file->mSubTexCountY;
			info.z = file->mSubTexWidth;
			info.w = file->mSubTexHeight;

			mtl.fApplyVector4PS( device, mVectorRegister, info );
		}
		else
		{
			mtl.fApplyVector4PS( device, mVectorRegister, atlasInfo );
		}
	}

	//------------------------------------------------------------------------------
	// tPsGlueTransitionDefaultId
	//------------------------------------------------------------------------------
	register_rtti_factory( tPsGlueTransitionDefaultId, true );
	void tPsGlueTransitionDefaultId::fApplyShared( 
		const tMaterial& mtl, 
		const tShadeMaterialGlueValues& glueVals, 
		const tDevicePtr& device, 
		const tRenderContext& context ) const
	{
		tTransitionObjectMaterialHelper::fApplyDefaultId( mRegisterIndex, device, mtl );
	}

	//------------------------------------------------------------------------------
	// tPsGlueTransitionEdgeColors
	//------------------------------------------------------------------------------
	register_rtti_factory( tPsGlueTransitionEdgeColors, true );
	void tPsGlueTransitionEdgeColors::fApplyShared( 
		const tMaterial& mtl, 
		const tShadeMaterialGlueValues& glueVals, 
		const tDevicePtr& device, 
		const tRenderContext& context ) const
	{
		tTransitionObjectMaterialHelper::fApplyEdgeColors( mRegisterIndex, device, mtl );
	}

	//------------------------------------------------------------------------------
	// tPsGlueTransitionObjects
	//------------------------------------------------------------------------------
	register_rtti_factory( tPsGlueTransitionObjects, true );
	void tPsGlueTransitionObjects::fApplyInstance( 
		const tMaterial& mtl, 
		const tShadeMaterialGlueValues& glueVals, 
		const tDevicePtr& device, 
		const tRenderContext& context, 
		const tDrawCall& drawCall ) const
	{
		tTransitionObjectMaterialHelper::fApplyTransitionObjects( 
			mRegisterIndex, device, mtl, drawCall.fRenderInstance( ) );
	}

	//------------------------------------------------------------------------------
	// tPsGlueHalfLambert
	//------------------------------------------------------------------------------
	register_rtti_factory( tPsGlueHalfLambert, true );
	void tPsGlueHalfLambert::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector3PS( device, mRegister, tDeferredShadingMaterial::fDebugUseHalfLambert( ) );
	}

	//------------------------------------------------------------------------------
	// tPsGlueSphericalHarmonics
	//------------------------------------------------------------------------------
	register_rtti_factory( tPsGlueSphericalHarmonics, true );
	void tPsGlueSphericalHarmonics::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		// testing
		mtl.fApplyVector4PS( device, mRegister, context.gSphericalHarmonics.mFactors[ 0 ], context.gSphericalHarmonics.mFactors.fCount( ) );
	}

	//register_rtti_factory( tPsGlueSphericalHarmonics, true );
	//void tPsGlueSphericalHarmonics::fApplyInstance( 
	//	const tMaterial& mtl, 
	//	const tShadeMaterialGlueValues& glueVals, 
	//	const tDevicePtr& device, 
	//	const tRenderContext& context, 
	//	const tDrawCall& drawCall ) const
	//{
	//	//mtl.fApplyVector4PS( device, mRegister, drawCall.fRenderInstance( ).fSphericalHarmonics( ).mFactors[ 0 ], drawCall.fRenderInstance( ).fSphericalHarmonics( ).mFactors.fCount( ) );

	//	// testing
	//	mtl.fApplyVector4PS( device, mRegister, context.gSphericalHarmonics.mFactors[ 0 ], context.gSphericalHarmonics.mFactors.fCount( ) );
	//}


	

}} // ::Sig::Gfx

