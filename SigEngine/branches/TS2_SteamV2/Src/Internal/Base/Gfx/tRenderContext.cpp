#include "BasePch.hpp"
#include "tRenderContext.hpp"
#include "tLightEntity.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		static f32 gDefaultShadowAmount = 0.6f;
		static f32 gDefaultShadowEpsilon = 0.0015f;
		static Math::tVec4f gDefaultFlatParticleColor = Math::tVec4f( 1.f, 1.f, 1.f, 1.f );
		static Math::tVec4f gDefaultFogValues = Math::tVec4f( 75.f, 300.f, 0.000f, 0.175f );
		static Math::tVec3f gDefaultFogColor = Math::tVec3f( 0.5f, 0.6f, 0.9f );
		static Math::tVec3f gDefaultRimLightColor = Math::tVec3f( 2.5f, 2.0f, 1.5f );
		static Math::tVec3f gDefaultRimLightDirection = Math::tVec3f( 0.0f, -0.3f, -1.0f );
	}

	devvarptr_clamp( f32, Renderer_Shadows_Epsilon, gDefaultShadowEpsilon, -0.2f, 0.2f, 5 );
	devvarptr_clamp( f32, Renderer_Shadows_Amount, gDefaultShadowAmount, 0.f, 1.0f, 3 );

	devvarptr_clamp( Math::tVec4f, Renderer_FlatParticle_Color, gDefaultFlatParticleColor, 0.f, +4.f, 2 );
	devvarptr_clamp( f32, Renderer_Fog_StartAt, gDefaultFogValues.x, 0.f, 1000.f, 1 );
	devvarptr_clamp( f32, Renderer_Fog_Range, gDefaultFogValues.y, 0.f, 1000.f, 1 );
	devvarptr_clamp( f32, Renderer_Fog_Min, gDefaultFogValues.z, 0.f, 1.f, 3 );
	devvarptr_clamp( f32, Renderer_Fog_Max, gDefaultFogValues.w, 0.f, 1.f, 3 );
	devrgbptr_clamp( Renderer_Fog_Color, gDefaultFogColor, 0.f, 1.f, 3 );

	devvarptr_clamp( Math::tVec3f, Renderer_RimLight_Color, gDefaultRimLightColor, 0.f, +4.f, 2 );
	devvarptr_clamp( Math::tVec3f, Renderer_RimLight_Direction, gDefaultRimLightDirection, -1.f, +1.f, 2 );

	void fSetDefaultFlatParticleColor( const Math::tVec4f& color )
	{
		gDefaultFlatParticleColor = color;
	}

	void fSetDefaultFog( const Math::tVec4f& startAt_range_min_max, const Math::tVec3f& rgb )
	{
		gDefaultFogValues = startAt_range_min_max;
		gDefaultFogColor = rgb;
	}

	void fGetDefaultFog( Math::tVec4f& values, Math::tVec3f& rgb )
	{
		values = gDefaultFogValues;
		rgb = gDefaultFogColor;
	}

	void fSetDefaultRimLight( const Math::tVec3f& rimLightColor, const Math::tVec3f& rimLightDirection )
	{
		gDefaultRimLightColor = rimLightColor;
		gDefaultRimLightDirection = rimLightDirection;
	}

	void fSetDefaultShadowAmount( f32 amount )
	{
		gDefaultShadowAmount = fClamp( amount, 0.f, 1.f );
	}

	void fSetDefaultShadowEpsilon( f32 epsilon )
	{
		gDefaultShadowEpsilon = epsilon;
	}

	void fSetShadowDefaults( f32 amount, f32 dist, f32 nearPlane, f32 farPlane, f32 vpWidth, f32 vpHeight )
	{
		fSetDefaultShadowAmount( amount );
		tLightEntity::fSetShadowMapDefaults( dist, nearPlane, farPlane, vpWidth, vpHeight );
	}

	void fExportScriptInterface( tScriptVm& vm )
	{
		vm.fNamespace("Gfx").Func(_SC("SetFlatParticleColor"), &fSetDefaultFlatParticleColor);
		vm.fNamespace("Gfx").Func(_SC("SetFog"), &fSetDefaultFog);
		vm.fNamespace("Gfx").Func(_SC("SetRimLight"), &fSetDefaultRimLight);
		vm.fNamespace("Gfx").Func(_SC("SetShadows"), &fSetShadowDefaults);
	}

	tRenderContext::tRenderContext( )
		: mCamera( 0 )
		, mGlobalFillMode( tRenderState::cGlobalFillDefault )
		, mRenderPassMode( tRenderState::cRenderPassLighting )
		, mRenderTargetDims( 1.f, 1.f, 0.f, 0.f )
		, mViewportTLBR( 0.f, 0.f, 1.f, 1.f )
		, mViewportIndex( -1 )
		, mViewportCount( 0 )
		, mFlatParticleColor( gDefaultFlatParticleColor )
		, mFogValues( gDefaultFogValues )
		, mFogColor( gDefaultFogColor )
		, mTime( 0.f )
		, mPausableTime( 0.f )
		, mShadowMapRealLayerCount( 0 )
		, mShadowMapLayerCount( 0 )
		, mShadowMapTexelSize( 1 )
		, mShadowMapEpsilon( Renderer_Shadows_Epsilon )
		, mShadowAmount( gDefaultShadowAmount )
		, mShadowMapTarget( Math::tVec3f::cZeroVector )
		, mShadowMap( 0 )
	{
		mWorldToLightSpace.fFill( Math::tMat4f::cIdentity );
		mViewToLightSpace.fFill( Math::tMat4f::cIdentity );
		fZeroOut( mShadowMapCascadeSplit );
	}

	void tRenderContext::fClearLights( const tTextureReference* whiteShadowMap )
	{
		mLightShaderConstants.mLightCount = 0;
		fZeroOut( mRimLightConstants );
		fZeroOut( mWorldToLightSpace );
		fZeroOut( mViewToLightSpace );
		mShadowMap = whiteShadowMap;
		mShadowMapLayerCount = 0;
		mShadowMapTexelSize = 1;
	}

	void tRenderContext::fFromLightGroup( const tScreen& screen, const tLightEntityList& lights, const tTextureReference* whiteShadowMap )
	{
		fClearLights( whiteShadowMap );

		if( lights.fCount( ) > 0 )
		{
			const Math::tVec3f eyePos = mCamera ? mCamera->fGetTripod( ).mEye : Math::tVec3f::cZeroVector;

			const tLightEntity& shadowLight = *lights.fFront( );
			if( shadowLight.fCastsShadow( ) )
			{
				mShadowMapRealLayerCount = shadowLight.fShadowLayerCount( );
				mShadowMapLayerCount = fMin( mShadowMapRealLayerCount, screen.fLimitShadowMapLayerCount( ) );

				for( u32 i = 0; i < mShadowMapLayerCount; ++i )
				{
					mWorldToLightSpace[ i ] = shadowLight.fCamera( i ).fGetWorldToProjection( );
					mViewToLightSpace[ i ] = shadowLight.fCamera( i ).fGetCameraToProjection( );
				}
				mShadowMap = screen.fShadowMap0( ).fGetRawPtr( );
				mShadowMapTexelSize = shadowLight.fShadowMapResolution( );
				mShadowMapTarget = shadowLight.fShadowMapTarget( );

				s32 splitCount = mShadowMapLayerCount - 1;
				for( s32 i = 0; i < splitCount; ++i )
					mShadowMapCascadeSplit[ i ] = shadowLight.fShadowMapCascadeSplit( i );
			}

			mLightShaderConstants.mLightCount = lights.fCount( );

			for( u32 i = 0; i < lights.fCount( ); ++i )
				lights[ i ]->fToShaderConstants( mLightShaderConstants[ i ], eyePos );

			mRimLightConstants.mLightDir = Math::tVec4f( Math::tVec3f( -gDefaultRimLightDirection ).fNormalizeSafe( Math::tVec3f::cZAxis ), 0.f );
			mRimLightConstants.mLightColor = Math::tVec4f( gDefaultRimLightColor, 0.f );
		}
	}

	void tRenderContext::fFromScreen( const tScreen& screen )
	{
		mGlobalFillMode = screen.fGetGlobalFillMode( );
		mTime = screen.fSceneGraph( )->fElapsedTime( );
		mPausableTime = screen.fSceneGraph( )->fPausableTime( );
		mWhiteTexture.fSetRaw( screen.fWhiteTexture( ) );
		mBlackTexture.fSetRaw( screen.fBlackTexture( ) );
	}

}}

