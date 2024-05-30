#include "BasePch.hpp"
#include "DerivedShadeMaterialGlue.hpp"
#include "tShadeMaterial.hpp"
#include "tRenderContext.hpp"
#include "tDrawCall.hpp"
#include "tSkinMap.hpp"
#include "tVarProperty.hpp"

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
		const Math::tMat3f localToView = context.mCamera->fGetWorldToCamera( ) * instance.fRI_ObjectToWorld( );
		mtl.fApplyMatrix3VS( device, mRegister, localToView );
	}

	register_rtti_factory( tVsGlueWorldToProjection, true );
	void tVsGlueWorldToProjection::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyMatrix4VS( device, mRegister, context.mCamera->fGetWorldToProjection( ) );
	}

	register_rtti_factory( tVsGlueViewToProjection, true );
	void tVsGlueViewToProjection::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyMatrix4VS( device, mRegister, context.mCamera->fGetCameraToProjection( ) );
	}

	register_rtti_factory( tVsGlueViewToWorld, true );
	void tVsGlueViewToWorld::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyMatrix3VS( device, mRegister, context.mCamera->fLocalToWorld( ) );
	}

	register_rtti_factory( tVsGlueWorldEyePos, true );
	void tVsGlueWorldEyePos::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		mtl.fApplyVector4VS( device, mRegister, Math::tVec4f( context.mCamera->fGetTripod( ).mEye, context.mCamera->fGetLens( ).mFarPlane ) );
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
		const tRenderInstance& instance = drawCall.fRenderInstance( );
		const tSkinMap* skinMap = instance.fRI_SkinMap( );
		if( skinMap )
		{
			u32 numEntries;
			tSkinMap::tScratchMatrixPalette palette;

			skinMap->fFillScratchMatrixPalette( palette, numEntries );

			mtl.fApplyMatrixPaletteVS( device, mRegister, palette.fBegin( ), numEntries );
		}
	}

	register_rtti_factory( tPsGlueBackFacePlusShadowMapParams, true );
	void tPsGlueBackFacePlusShadowMapParams::fApplyShared( const tMaterial& mtl, const tShadeMaterialGlueValues& glueVals, const tDevicePtr& device, const tRenderContext& context ) const
	{
		const f32 epsilon = context.mShadowMapEpsilon;
		mtl.fApplyVector4PS( device, mRegister, Math::tVec4f( glueVals.fBackFaceFlip( ), epsilon, ( f32 )context.mShadowMapTexelSize, context.mShadowAmount ) );
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
		mtl.fApplyVector4PS( device, mRegister, Math::tVec4f( context.mCamera->fGetTripod( ).mEye, context.mCamera->fGetLens( ).mFarPlane ) );
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
}}

