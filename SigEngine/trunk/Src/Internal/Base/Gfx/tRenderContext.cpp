#include "BasePch.hpp"
#include "tRenderContext.hpp"
#include "tLightEntity.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{

	devrgbaptr_clamp( Renderer_FlatParticle_Color, tDefaultRenderContext::fInstance( ).mFlatParticleColor, 0.f, +4.f );
	devvarptr_clamp( f32, Renderer_Fog_StartAt, tDefaultRenderContext::fInstance( ).mFogValues.x, 0.f, 1000.f, 1 );
	devvarptr_clamp( f32, Renderer_Fog_Range, tDefaultRenderContext::fInstance( ).mFogValues.y, 0.f, 1000.f, 1 );
	devvarptr_clamp( f32, Renderer_Fog_Min, tDefaultRenderContext::fInstance( ).mFogValues.z, 0.f, 1.f, 3 );
	devvarptr_clamp( f32, Renderer_Fog_Max, tDefaultRenderContext::fInstance( ).mFogValues.w, 0.f, 1.f, 3 );
	devrgbptr_clamp( Renderer_Fog_Color, tDefaultRenderContext::fInstance( ).mFogColor, 0.f, 1.f );
	devvarptr_clamp( f32, Renderer_FogVertical_StartAt, tDefaultRenderContext::fInstance( ).mVerticalFogValues.x, -1000.f, 1000.f, 1 );
	devvarptr_clamp( f32, Renderer_FogVertical_Range, tDefaultRenderContext::fInstance( ).mVerticalFogValues.y, -1000.f, 1000.f, 1 );
	devvarptr_clamp( f32, Renderer_FogVertical_Min, tDefaultRenderContext::fInstance( ).mVerticalFogValues.z, 0.f, 1.f, 3 );
	devvarptr_clamp( f32, Renderer_FogVertical_Max, tDefaultRenderContext::fInstance( ).mVerticalFogValues.w, 0.f, 1.f, 3 );
	devrgbptr_clamp( Renderer_FogVertical_Color, tDefaultRenderContext::fInstance( ).mVerticalFogColor, 0.f, 1.f );

	devrgbptr_clamp( Renderer_RimLight_Color, tDefaultRenderContext::fInstance( ).mRimLightColor, 0.f, +4.f );
	devvarptr_clamp( Math::tVec3f, Renderer_RimLight_Direction, tDefaultRenderContext::fInstance( ).mRimLightDirection, -1.f, +1.f, 2 );


	tDefaultRenderContext::tDefaultRenderContext( )
		: mFlatParticleColor( 1.f, 1.f, 1.f, 1.f )
		, mFogValues( 75.f, 300.f, 0.000f, 0.175f )
		, mFogColor( 0.5f, 0.6f, 0.9f )
		, mVerticalFogValues( 0., 1.f, 0.f, 0.f )
		, mVerticalFogColor( 0.5f, 0.6f, 0.9f )
		, mRimLightColor( 2.5f, 2.0f, 1.5f )
		, mRimLightDirection( 0.0f, -0.3f, -1.0f )
	{
	}
	tDefaultRenderContext& tDefaultRenderContext::fInstance( )
	{
		static tDefaultRenderContext g;
		return g;
	}

	void fSetDefaultFlatParticleColor( const Math::tVec4f& color )
	{
		tDefaultRenderContext::fInstance( ).mFlatParticleColor = color;
	}

	void fSetDefaultFog( const Math::tVec4f& startAt_range_min_max, const Math::tVec3f& rgb )
	{
		tDefaultRenderContext::fInstance( ).mFogValues = startAt_range_min_max;
		tDefaultRenderContext::fInstance( ).mFogColor = rgb;
	}

	void fSetDefaultVerticalFog( const Math::tVec4f& startAt_range_min_max, const Math::tVec3f& rgb )
	{
		tDefaultRenderContext::fInstance( ).mVerticalFogValues = startAt_range_min_max;
		tDefaultRenderContext::fInstance( ).mVerticalFogColor = rgb;
	}

	void fSetDefaultRimLight( const Math::tVec3f& rimLightColor, const Math::tVec3f& rimLightDirection )
	{
		tDefaultRenderContext::fInstance( ).mRimLightColor = rimLightColor;
		tDefaultRenderContext::fInstance( ).mRimLightDirection = rimLightDirection;
	}

	void fSetShadowDefaults( f32 amount, f32 dist, f32 nearPlane, f32 farPlane, f32 vpWidth, f32 vpHeight )
	{
		// shadow ammount needs to come from the light entity.hpp
		//  currently directional lights dont have an entity (in the editor), so the amount is stored here. this is highly ghetto
		//tDefaultRenderContext::fInstance( ).mShadowAmount = fClamp( amount, 0.f, 1.f );
		log_warning( "Gfx.SetShadows script function is decprecated and will have no effect. Remove as soon as possible." );
	}

	void fExportScriptInterface( tScriptVm& vm )
	{
		vm.fNamespace("Gfx").Func(_SC("SetFlatParticleColor"), &fSetDefaultFlatParticleColor);
		vm.fNamespace("Gfx").Func(_SC("SetFog"), &fSetDefaultFog);
		vm.fNamespace("Gfx").Func(_SC("SetVerticalFog"), &fSetDefaultVerticalFog);
		vm.fNamespace("Gfx").Func(_SC("SetRimLight"), &fSetDefaultRimLight);
		vm.fNamespace("Gfx").Func(_SC("SetShadows"), &fSetShadowDefaults);
	}


	tSphericalHarmonics tRenderContext::gSphericalHarmonics; //ONLY FOR DEBUGGING.

	tRenderContext::tRenderContext( )
		: mSceneCamera( NULL )
		, mOtherCamera( NULL )
		, mGlobalFillMode( tRenderState::cGlobalFillDefault )
		, mRenderPassMode( tRenderState::cRenderPassLighting )
		, mRenderTargetDims( 1.f, 1.f, 0.f, 0.f )
		, mViewportTLBR( 0.f, 0.f, 1.f, 1.f )
		, mViewportIndex( -1 )
		, mViewportCount( 0 )
		, mFlatParticleColor( tDefaultRenderContext::fInstance( ).mFlatParticleColor )
		, mFogValues( tDefaultRenderContext::fInstance( ).mFogValues )
		, mFogColor( tDefaultRenderContext::fInstance( ).mFogColor )
		, mVerticalFogValues( tDefaultRenderContext::fInstance( ).mVerticalFogValues )
		, mVerticalFogColor( tDefaultRenderContext::fInstance( ).mVerticalFogColor )
		, mTime( 0.f )
		, mPausableTime( 0.f )
		, mShadowMapRealLayerCount( 0 )
		, mShadowMapLayerCount( 0 )
		, mShadowMapTexelSize( 1 )
		, mShadowMapEpsilon( 0.001f )	// These defaults are never used. These values are set by tLightEntity::fSetShadowParams
		, mShadowAmount( 0.5f )			// These defaults are never used. These values are set by tLightEntity::fSetShadowParams
		, mShadowMapTarget( Math::tVec3f::cZeroVector )
		, mShadowMap( 0 )
		, mShadowGeneratingLight( NULL )
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
		
		//Dont clear the shadow generating light here.
		//mShadowGeneratingLight = NULL;

		// These either, they are being passed through
		//mShadowMapCascadeSplit[ 0 ]
		//mShadowMapCascadeSplit[ 1 ]
	}

	void tRenderContext::fFromLightGroup( const tScreen& screen, const tLightEntityList& lights, const tTextureReference* whiteShadowMap )
	{
		fClearLights( whiteShadowMap );

		if( lights.fCount( ) > 0 )
		{
			const Math::tVec3f eyePos = mSceneCamera ? mSceneCamera->fGetTripod( ).mEye : Math::tVec3f::cZeroVector;

			const tLightEntity& shadowLight = *lights.fFront( );
			if( shadowLight.fCastsShadow( ) )
				shadowLight.fSetShadowParams( screen, *this );

			mLightShaderConstants.mLightCount = lights.fCount( );

			for( u32 i = 0; i < lights.fCount( ); ++i )
			{
				// Assert the expectations of forward rendering shaders WRT light constants
				log_assert( (i != 0) || (lights[ i ]->fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection), "Forward rendering shaders expect the first light to be directional but it is not" );
				log_assert( (i == 0) || (lights[ i ]->fLightDesc( ).fLightType( ) == tLight::cLightTypePoint), "Forward rendering shaders expect all the non-first lights to be point lights, but index " << i << " is not" );

				lights[ i ]->fToShaderConstants( mLightShaderConstants[ i ], eyePos );
			}

			mRimLightConstants.mLightDir = Math::tVec4f( Math::tVec3f( -tDefaultRenderContext::fInstance( ).mRimLightDirection ).fNormalizeSafe( Math::tVec3f::cZAxis ), 0.f );
			mRimLightConstants.mLightColor = Math::tVec4f( tDefaultRenderContext::fInstance( ).mRimLightColor, 0.f );
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

	Math::tVec4f tRenderContext::fComputeNearFarValues( const tCamera& camera )
	{
		const Gfx::tLens& lens = camera.fGetLens( );
		return Math::tVec4f( lens.mNearPlane, lens.mFarPlane, lens.mFarPlane - lens.mNearPlane, 0 );
	}

}}

