#include "BasePch.hpp"
#include "tDeferredShadingMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tRenderContext.hpp"
#include "tRenderBatch.hpp"
#include "tDrawCall.hpp"
#include "tPostEffectsMaterial.hpp"
#include "tLightEntity.hpp"
#include "tSolidColorSphere.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
		};
		const tFilePathPtr cPath( "Shaders/Engine/DeferredShading.mtlb" );
	}


	///
	/// \section tDeferredShadingMaterial
	///
	devvar( bool, Renderer_Deferred_UseStencil, true ); 
	devvar( bool, Renderer_Deferred_HalfLambert, true ); 
	devvar_clamp( f32, Renderer_Deferred_HalfLambertScale, 0.5f, 0.f, 1.f, 2 ); 
	devvar_clamp( f32, Renderer_Deferred_HalfLambertPow, 2, 1.f, 2.f, 2 );

	devvar( bool, Renderer_Shadows_DepthEps_DeferredPointLightOverride, true );
	Math::tVec4f Renderer_Shadows_DepthEps_DeferredPointLight( 0.0009f, 0.0009f, 0.0009f, 0.0009f );
	devvarptr_p( f32, Renderer_Shadows_DepthEps_DeferredPointLightX, Renderer_Shadows_DepthEps_DeferredPointLight.x, 4 );
	devvarptr_p( f32, Renderer_Shadows_DepthEps_DeferredPointLightY, Renderer_Shadows_DepthEps_DeferredPointLight.y, 4 );
	devvarptr_p( f32, Renderer_Shadows_DepthEps_DeferredPointLightZ, Renderer_Shadows_DepthEps_DeferredPointLight.z, 4 );
	devvarptr_p( f32, Renderer_Shadows_DepthEps_DeferredPointLightW, Renderer_Shadows_DepthEps_DeferredPointLight.w, 4 );


	Math::tVec3f tDeferredShadingMaterial::fDebugUseHalfLambert( )
	{
		return Renderer_Deferred_HalfLambert ? Math::tVec3f( Renderer_Deferred_HalfLambertScale, Renderer_Deferred_HalfLambertPow, 0 ) : Math::tVec3f( 1.f, 1.f, 0 );
	}

	const tVertexFormat tDeferredShadingMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	tFilePathPtr tDeferredShadingMaterial::fMaterialFilePath( )
	{
		return cPath;
	}

	tDeferredShadingMaterial::tDeferredShadingMaterial( )
		: mLastAppliedPS( ~0 )
	{
	}

	tDeferredShadingMaterial::tDeferredShadingMaterial( tNoOpTag )
		: tMaterial( cNoOpTag )
	{
	}

	void tDeferredShadingMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		sigassert( !"This should not be getting called!" );
	}

	void tDeferredShadingMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		sigassert( !"This should not be getting called!" );
	}

	namespace
	{
		b32 fLightSort( tLightEntity* a, tLightEntity* b )
		{
			return a->fLightDesc( ).fLightType( ) < b->fLightDesc( ).fLightType( );
		}

#ifdef platform_xbox360
		const b32 cCanStencil = true;

		void fBeginStencil( tScreen& screen )
		{
			IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );

			d3ddev->SetRenderState( D3DRS_STENCILMASK, 0xFFFFFFFF );
			d3ddev->SetRenderState( D3DRS_STENCILWRITEMASK, 0xFFFFFFFF );
			d3ddev->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );

			// no need to clear using this method
			//u32 stencilClear = 0;
			//u32 clearFlags = D3DCLEAR_STENCIL | D3DCLEAR_HISTENCIL_CULL;
			//d3ddev->Clear( 0, 0, clearFlags, D3DCOLOR( 0 ), 0, stencilClear );
		}

		void fBeginBFStencilMode( tScreen& screen, u32 lightId )
		{
			IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );

			d3ddev->SetPixelShader( NULL );
			d3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, 0 );

			d3ddev->SetRenderState( D3DRS_HISTENCILENABLE, FALSE );
			d3ddev->SetRenderState( D3DRS_HISTENCILWRITEENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_HISTENCILREF, lightId );
			d3ddev->SetRenderState( D3DRS_HISTENCILFUNC, D3DHSCMP_NOTEQUAL );
			d3ddev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );

			d3ddev->SetRenderState( D3DRS_ZENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_STENCILENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_STENCILREF, lightId );
			d3ddev->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
			d3ddev->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE );

		}

		void fBeginFFStencilMode( tScreen& screen )
		{
			// Renable color writes
			IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );

			d3ddev->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_EQUAL );
			d3ddev->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR );
			d3ddev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
		}

		void fBeginStencilTest( tScreen& screen )
		{
			IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );

			// switch to hi-stencil testing and color writting
			d3ddev->FlushHiZStencil( D3DFHZS_SYNCHRONOUS );
			d3ddev->SetRenderState( D3DRS_HISTENCILWRITEENABLE, FALSE );
			d3ddev->SetRenderState( D3DRS_HISTENCILENABLE, TRUE );
			d3ddev->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL );
		}

		void fEndStencilTest( tScreen& screen )
		{
			IDirect3DDevice9* d3ddev = screen.fDevice( )->fGetDevice( );
			d3ddev->SetRenderState( D3DRS_STENCILENABLE, FALSE );
			d3ddev->SetRenderState( D3DRS_HISTENCILENABLE, FALSE );
			d3ddev->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		}

#else
		const b32 cCanStencil = false;

		void fBeginStencil( tScreen& screen )
		{ }
		void fBeginBFStencilMode( tScreen& screen, u32 lightId )
		{ }
		void fBeginFFStencilMode( tScreen& screen )
		{ }
		void fBeginStencilTest( tScreen& screen )
		{ }
		void fEndStencilTest( tScreen& screen )
		{ }
#endif 

		b32 fCameraInsideSphere( const tLightEntity& light, const Gfx::tCamera* camera )
		{
			Math::tSpheref lightS( light.fObjectToWorld( ).fGetTranslation( ), light.fEffectiveRadius( ) + 1.f );
			return lightS.fContains( camera->fGetTripod( ).mEye );
		}
	}

	tDeferredShadingMaterial::tDeferredShadingContext::tDeferredShadingContext( )
		: mLastAppliedBatch( NULL )
	{
		tLens lens;
		lens.fSetScreen( 0.0f, 1.f, 0.f, 1.f, 1.f, 0.f );

		mFullScreenCamera.fSetup( lens, tTripod( Math::tVec3f::cZeroVector, Math::tVec3f::cZAxis, Math::tVec3f::cYAxis ) );
	}

	void tDeferredShadingMaterial::tDeferredShadingContext::fSetScreen( tScreen& screen )
	{
		mRenderState = tRenderState::cDefaultColorOpaque;

		mContext.fFromScreen( screen );
		mContext.mOtherCamera = &mFullScreenCamera;
	}

	void tDeferredShadingMaterial::fResetLightPass( )
	{
		sigassert( sizeof( tPostEffectsRenderVertex ) == cVertexFormat.fVertexSize( ) );
		mLastAppliedPS = ~0;
		mLastAppliedVS = ~0;
		mContext.mLastAppliedBatch = NULL;
	}


	// This is the main rendering implementation for the deferred light pass.
	void tDeferredShadingMaterial::fRenderLightList( tLightEntityList& list, tLightEntityList& shadowLights, tTextureReference& blankShadowMap, tScreen& screen )
	{
		std::sort( list.fBegin( ), list.fEnd( ), fLightSort );

		const tDevicePtr& device = screen.fGetDevice( );
		tRenderContext& rContext = mContext.mContext;

		fBeginStencil( screen );

		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			const tLightEntity& light = *list[ i ];
			b32 fillStencil = false;
			b32 maintainancePass = false;

			if( i == 0 )
			{
				sigassert( light.fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection );

				// first pass is directional light and also writes depth value back to depth buffer
				mContext.mRenderState.fEnableDisable( tRenderState::cAlphaBlend, false );
				mContext.mRenderState.fEnableDisable( tRenderState::cDepthBuffer, true ); 
				mContext.mRenderState.fEnableDisable( tRenderState::cDepthWrite, true );
				mContext.mRenderState.fEnableDisable( tRenderState::cPolyFlipped, true );
				maintainancePass = true;
			}
			if( i == 1 )
			{
				//switch to blending from now on.
				mContext.mRenderState.fEnableDisable( tRenderState::cAlphaBlend, true );
				mContext.mRenderState.fEnableDisable( tRenderState::cDepthWrite, false );
				mContext.mRenderState.fSetDstBlendMode( tRenderState::cBlendOne );
				mContext.mRenderState.fSetSrcBlendMode( tRenderState::cBlendOne );

				if( !Renderer_Deferred_UseStencil )
					mContext.mRenderState.fEnableDisable( tRenderState::cDepthBuffer, false ); 

				mContext.mRenderState.fApply( device, mContext.mContext );
			}

			//determine what geometry we need:
			tRenderBatchData* batchToRender = NULL;

			switch( light.fLightDesc( ).fLightType( ) )
			{
				case tLight::cLightTypeDirection:
				{
					batchToRender = &mContext.mFullScreenQuad;
				}
				break;
				case tLight::cLightTypePoint:
				{
					batchToRender = &mContext.mUnitSphere;

					b32 cameraInsideSphere = fCameraInsideSphere( light, rContext.mSceneCamera );
					if( !cameraInsideSphere && Renderer_Deferred_UseStencil && cCanStencil )
						fillStencil = true;
				}
				break;
			}

			if( batchToRender != mContext.mLastAppliedBatch )
			{
				mContext.mLastAppliedBatch = batchToRender;
				batchToRender->fApplyBatchWithoutMaterial( device, rContext );
			}

			b32 castsShadow = (shadowLights.fFind( &light ) != NULL);
			if( castsShadow )
			{
				light.fSetShadowParams( screen, rContext );
				sigassert( rContext.mShadowMap );
				rContext.mShadowMap->fApply( device, tDeferredShadingMaterial::cTexUnitShadow );
			}
			else
				blankShadowMap.fApply( device, tDeferredShadingMaterial::cTexUnitShadow );

			if( fillStencil )
			{
				fApplyLightVS( true, device, rContext, light ); //depth only vertex shader
				fBeginBFStencilMode( screen, i );		//clears pixel shader, renders backfaces settings stencil values for zfail, object must be in front of light back bounds
				batchToRender->fRenderInstance( device );
				fBeginFFStencilMode( screen );				//renders front faces, invalidating the stencil value for zfail. objects must not obstruct the light front bounds
				batchToRender->fRenderInstance( device );
				fBeginStencilTest( screen );				// flush hi-stencil and begin equal stencil testing

				mLastAppliedPS = ~0; //force bind PS
				mLastAppliedVS = ~0; //force bind VS
			}
			else
			{
				if( !maintainancePass )
					device->fGetDevice( )->SetRenderState( D3DRS_ZENABLE, FALSE );
			}

			fApplyLightVS( false, device, rContext, light );
			fApplyLightPS( device, rContext, light, castsShadow );

			batchToRender->fRenderInstance( device );

			if( fillStencil )
			{
				fEndStencilTest( screen );
			}
		}
	}

	void tDeferredShadingMaterial::fApplyLightVS( b32 depthOnly, const tDevicePtr& device, const tRenderContext& context, const tLightEntity& light )
	{
		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		u32 vsShaderType = ~0;

		switch( light.fLightDesc( ).fLightType( ) )
		{
		case tLight::cLightTypeDirection:
			vsShaderType = cVSShaderQuad;
			break;
		case tLight::cLightTypePoint:
			vsShaderType = depthOnly ? cVSShader3DDepthOnly : cVSShader3D;
			break;
		};

		if( vsShaderType != ~0 )
		{
			if( vsShaderType != mLastAppliedVS )
			{
				mLastAppliedVS = vsShaderType;

				mtlFile->fApplyShader( device, cShaderCategoryVS, vsShaderType );

				// vertex shader constants. camera is the post process, full screen quad camera.
				fApplyMatrix4VS( device, cVSScreenCamera,		context.mOtherCamera->fGetWorldToProjection( ) );
				fApplyVector4VS( device, cVSRenderTargetDims,	context.mRenderTargetDims );
				fApplyVector4VS( device, cVSViewportXform,		context.mViewportTLBR );
			}

			if( vsShaderType == cVSShader3D || vsShaderType == cVSShader3DDepthOnly )
			{
				//we're going to be rendering geometry here
				Math::tMat3f lightXform( light.fEffectiveRadius( ) );
				lightXform.fSetTranslation( light.fObjectToWorld( ).fGetTranslation( ) );
				fApplyMatrix4VS( device, cVSSceneCamera, context.mSceneCamera->fGetWorldToProjection( ) * Math::tMat4f( lightXform ) );
			}
		}
	}

	devvar( bool, Renderer_Shadows_PointLightPCF, false );
	devvar( bool, Renderer_Shadows_NormalShadows, true );

	void tDeferredShadingMaterial::fApplyLightPS( const tDevicePtr& device, const tRenderContext& context, const tLightEntity& light, b32 castsShadows )
	{
		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );
		u32 psShaderType = ~0;

		switch( light.fLightDesc( ).fLightType( ) )
		{
		case tLight::cLightTypeDirection:
			psShaderType
				= castsShadows							? cPSShadersDirectionalLight_Shadowed
				: Renderer_Shadows_NormalShadows		? cPSShadersDirectionalLight_NormalShadowed
				: cPSShadersDirectionalLight;
			break;
		case tLight::cLightTypePoint:
			psShaderType
				= !castsShadows						? cPSShadersPointLight
				: Renderer_Shadows_PointLightPCF	? cPSShadersPointLight_ShadowedPCF
				: cPSShadersPointLight_Shadowed;
			break;
		default:
			sig_nodefault( );
			break;
		};

		if( psShaderType != ~0 )
		{
			if( psShaderType != mLastAppliedPS )
			{
				mLastAppliedPS = psShaderType;

				mtlFile->fApplyShader( device, cShaderCategoryPS, psShaderType );

				fApplyMatrix4PS( device, cPSProjToWorld,		context.mSceneCamera->fGetWorldToProjection( ).fInverse( ) );
				fApplyVector3PS( device, cPSWorldEyePos,		context.mSceneCamera->fGetTripod( ).mEye );
				fApplyVector3PS( device, cPSHalfLambert,		fDebugUseHalfLambert( ) );
				
			}

			tLightShaderConstants constants;
			light.fToShaderConstants( constants, Math::tVec3f::cZeroVector );
			tMaterial::fApplyLight( device, cPSLightVectorFirst, constants );

			if( castsShadows || Renderer_Shadows_NormalShadows )
			{
				const b32 overridePointLightDepthEps
					=	(light.fLightDesc( ).fLightType( ) == tLight::cLightTypePoint)
					&&	Renderer_Shadows_DepthEps_DeferredPointLightOverride;

				const Math::tVec4f depthEps
					= overridePointLightDepthEps ? Renderer_Shadows_DepthEps_DeferredPointLight
					: context.mShadowMapEpsilon;

				fApplyVector4PS( device, cPSShadowMapEpsilon, depthEps );
				fApplyVector4PS( device, cPSShadowMapEpsilon_TexelSize_Amount_Split, Math::tVec4f( depthEps.x, ( f32 )context.mShadowMapTexelSize, context.mShadowAmount, context.mShadowMapCascadeSplit[1] ) );
				fApplyVector4PS( device, cPSShadowMapTarget_Split, Math::tVec4f( context.mShadowMapTarget, context.mShadowMapCascadeSplit[0] ) );
				fApplyMatrix4PS( device, cPSWorldToLightArray, context.mWorldToLightSpace[0], context.mWorldToLightSpace.fCount( ) );
			}
		}
	}


	tDeferredShadingSphere::tDeferredShadingSphere( )
		: mRadius( 1.f )
	{
		fSetPrimTypeOverride( tIndexFormat::cPrimitiveTriangleList );
	}

	void tDeferredShadingSphere::fOnDeviceReset( tDevice* device )
	{
		fGenerate( mRadius );
	}

	void tDeferredShadingSphere::fGenerate( f32 radius )
	{
		mRadius = radius;
		tGrowableArray< tPostEffectsRenderVertex > verts;
		tGrowableArray< u16 > ids;

		tSolidColorSphere::fGenerateData( verts, ids, radius, 0.f );

		fBake( verts, ids, ids.fCount( ) / 3 );
	}

}}
