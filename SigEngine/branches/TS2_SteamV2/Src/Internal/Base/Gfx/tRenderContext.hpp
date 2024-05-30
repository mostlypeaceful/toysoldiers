#ifndef __tRenderContext__
#define __tRenderContext__
#include "tLightEntity.hpp"
#include "tRenderState.hpp"

namespace Sig { namespace Gfx
{
	class tCamera;
	class tRenderToTexture;

	void fSetDefaultFlatParticleColor( const Math::tVec4f& color );
	base_export void fSetDefaultFog( const Math::tVec4f& startAt_range_min_max, const Math::tVec3f& rgb );
	base_export void fGetDefaultFog( Math::tVec4f& values, Math::tVec3f& rgb );
	void fSetDefaultShadowAmount( f32 amount );
	void fSetDefaultShadowEpsilon( f32 epsilon );
	void fExportScriptInterface( tScriptVm& vm );

	///
	/// \brief Encapsulates common render data that is global or semi-global.
	class base_export tRenderContext
	{
	public:
		// current camera (may correspond to the scene, or a light)
		const tCamera*					mCamera;

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

		// time stuffs
		f32								mTime;
		f32								mPausableTime;

		// light stuffs
		tRimLightShaderConstants		mRimLightConstants;
		tLightShaderConstantsArray		mLightShaderConstants;

		// shadow map stuffs
		tFixedArray<Math::tMat4f,Gfx::tMaterial::cMaxShadowLayers> mWorldToLightSpace;
		tFixedArray<Math::tMat4f,Gfx::tMaterial::cMaxShadowLayers> mViewToLightSpace;
		u32								mShadowMapRealLayerCount;
		u32								mShadowMapLayerCount;
		u32								mShadowMapTexelSize;
		f32								mShadowMapEpsilon;
		f32								mShadowAmount;
		Math::tVec3f					mShadowMapTarget;
		tFixedArray<f32,Gfx::tMaterial::cMaxShadowLayers-1> mShadowMapCascadeSplit;
		const tTextureReference*		mShadowMap;

		// default textures
		tTextureReference				mWhiteTexture;
		tTextureReference				mBlackTexture;

	public:
		tRenderContext( );
		void fClearLights( const tTextureReference* whiteShadowMap );
		void fFromLightGroup( const tScreen& screen, const tLightEntityList& lights, const tTextureReference* whiteShadowMap );
		void fFromScreen( const tScreen& screen );
	};

}}

#endif//__tRenderContext__

