#ifndef __tRenderContext__
#define __tRenderContext__
#include "tLightEntity.hpp"
#include "tRenderState.hpp"
#include "tTextureReference.hpp"
#include "tSphericalHarmonics.hpp" // only temporarily, for debugging.

namespace Sig { namespace Gfx
{
	class tCamera;

	struct base_export tDefaultRenderContext
	{
		Math::tVec4f mFlatParticleColor;
		Math::tVec4f mFogValues;
		Math::tVec3f mFogColor;
		Math::tVec4f mVerticalFogValues;
		Math::tVec3f mVerticalFogColor;
		Math::tVec3f mRimLightColor;
		Math::tVec3f mRimLightDirection;

		tDefaultRenderContext( );
		static tDefaultRenderContext& fInstance( );
	};

	void fExportScriptInterface( tScriptVm& vm );


	///
	/// \brief Encapsulates common render data that is global or semi-global.
	class base_export tRenderContext
	{
	public:
		const tCamera*					mSceneCamera; // either 3d light or world camera
		const tCamera*					mOtherCamera; // post processor or screen camera

		// global render state
		tRenderState::tGlobalFillMode	mGlobalFillMode;
		tRenderState::tRenderPassMode	mRenderPassMode;

		// current viewport xform and back buffer dims
		Math::tVec4f					mRenderTargetDims;
		Math::tVec4f					mViewportTLBR;
		u32								mViewportIndex;
		u32								mViewportCount;

		// fog stuffs
		Math::tVec4f					mFlatParticleColor;
		Math::tVec4f					mFogValues;
		Math::tVec3f					mFogColor;
		Math::tVec4f					mVerticalFogValues;
		Math::tVec3f					mVerticalFogColor;

		// time stuffs
		f32								mTime;
		f32								mPausableTime;

		// light stuffs
		tRimLightShaderConstants		mRimLightConstants;
		tLightShaderConstantsArray		mLightShaderConstants;
		static tSphericalHarmonics		gSphericalHarmonics; //ONLY FOR DEBUGGING.

		// shadow map stuffs
		tFixedArray<Math::tMat4f,Gfx::tMaterial::cMaxShadowLayers> mWorldToLightSpace;
		tFixedArray<Math::tMat4f,Gfx::tMaterial::cMaxShadowLayers> mViewToLightSpace;
		u32								mShadowMapRealLayerCount;
		u32								mShadowMapLayerCount;
		u32								mShadowMapTexelSize;
		Math::tVec4f					mShadowMapEpsilon;
		f32								mShadowAmount;
		Math::tVec3f					mShadowMapTarget;
		tFixedArray<f32,Gfx::tMaterial::cMaxShadowLayers-1> mShadowMapCascadeSplit;
		const tTextureReference*		mShadowMap;
		const tLightEntity*				mShadowGeneratingLight;

		// default textures
		tTextureReference				mWhiteTexture;
		tTextureReference				mBlackTexture;

	public:
		tRenderContext( );
		void fClearLights( const tTextureReference* whiteShadowMap );
		void fFromLightGroup( const tScreen& screen, const tLightEntityList& lights, const tTextureReference* whiteShadowMap );
		void fFromScreen( const tScreen& screen );

		static Math::tVec4f fComputeNearFarValues( const tCamera& camera );
	};

}}

#endif//__tRenderContext__

