#include "BasePch.hpp"
#include "tPostEffectsMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tRenderInstance.hpp"
#include "tRenderContext.hpp"
#include "tRandom.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
		};
		const tFilePathPtr cPostFxPath( "Shaders/Engine/PostEffects.mtlb" );
	}
	const tVertexFormat tPostEffectsMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	tFilePathPtr tPostEffectsMaterial::fMaterialFilePath( )
	{
		return cPostFxPath;
	}

	tPostEffectsMaterial::tPostEffectsMaterial( )
		: mInputCount( 0 )
		, mRgbaTint0( Math::tVec4f::cOnesVector )
		, mRgbaTint1( Math::tVec4f::cOnesVector )
		, mRgbaTint2( Math::tVec4f::cOnesVector )
		, mRgbaTint3( Math::tVec4f::cOnesVector )
		, mRgbaBlend( 0.5f )
		, mTargetDepthValues( 0.95f, 0.5f, 2.f, 0.f )
		, mSaturation( Math::tVec4f::cOnesVector )
		, mFilmGrainFlicker( Math::tVec4f::cOnesVector )
		, mFilmGrainOffsets0( Math::tVec4f::cZeroVector )
		, mFilmGrainOffsets1( Math::tVec4f::cZeroVector )
		, mContrast( Math::tVec4f::cOnesVector )
		, mTransformCutOff( Math::tVec4f::cOnesVector )
		, mTransformAdd( Math::tVec4f::cZeroVector )
		, mTransformMul( Math::tVec4f::cOnesVector )
		, mBlurScale( Math::tVec2f::cOnesVector )
	{
	}

	tPostEffectsMaterial::tPostEffectsMaterial( const tResourcePtr& postEffectsMtlFile )
		: mInputCount( 0 )
		, mRgbaTint0( Math::tVec4f::cOnesVector )
		, mRgbaTint1( Math::tVec4f::cOnesVector )
		, mRgbaTint2( Math::tVec4f::cOnesVector )
		, mRgbaTint3( Math::tVec4f::cOnesVector )
		, mRgbaBlend( 0.5f )
		, mTargetDepthValues( 0.95f, 0.5f, 2.f, 0.f )
		, mSaturation( Math::tVec4f::cOnesVector )
		, mFilmGrainFlicker( Math::tVec4f::cOnesVector )
		, mFilmGrainOffsets0( Math::tVec4f::cZeroVector )
		, mFilmGrainOffsets1( Math::tVec4f::cZeroVector )
		, mContrast( Math::tVec4f::cOnesVector )
		, mTransformCutOff( Math::tVec4f::cOnesVector )
		, mTransformAdd( Math::tVec4f::cZeroVector )
		, mTransformMul( Math::tVec4f::cOnesVector )
		, mBlurScale( Math::tVec2f::cOnesVector )
	{
		fSetMaterialFileResourcePtrOwned( postEffectsMtlFile );
	}

	void tPostEffectsMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		sigassert( sizeof( tPostEffectsRenderVertex ) == cVertexFormat.fVertexSize( ) );

		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, 0, mVS );
		mtlFile->fApplyShader( device, 1, mPS );

		// vertex shader constants
		fApplyMatrix4VS( device, cVSWorldToProj,		context.mOtherCamera->fGetWorldToProjection( ) );
		fApplyVector4VS( device, cVSRenderTargetDims,	context.mRenderTargetDims );
		fApplyVector4VS( device, cVSViewportXform,		context.mViewportTLBR );

		// pixel shader constants
		switch( mPS )
		{
		case cPShaderCopy:
			{
				sigassert( mInputCount == 1 );
				fApplyVector4PS( device, cPSRgbaTint0, mRgbaTint0 );
			}
			break;
		case cPShaderAdd:
			{
				sigassert( mInputCount == 2 );
				fApplyVector4PS( device, cPSRgbaTint0, mRgbaTint0 );
				fApplyVector4PS( device, cPSRgbaTint1, mRgbaTint1 );
			}
			break;
		case cPShaderSwizzle3D:
			fApplyVector4PS( device, cPSTransformAdd, mTransformAdd );
			// INTENTIONAL FALL THROUGH
		case cPShaderSwizzle:
			{
				sigassert( mInputCount == 1 );
				fApplyVector4PS( device, cPSRgbaTint0, mRgbaTint0 );
				fApplyVector4PS( device, cPSRgbaTint1, mRgbaTint1 );
				fApplyVector4PS( device, cPSRgbaTint2, mRgbaTint2 );
				fApplyVector4PS( device, cPSRgbaTint3, mRgbaTint3 );
			}
			break;
		case cPShaderSample2x2:
			{
				sigassert( mInputCount == 1 );
				const f32 uOffset = 0.5f/mInputs[0].mWidth;
				const f32 vOffset = 0.5f/mInputs[0].mHeight;
				const Math::tVec4f sampleOffsets[]=
				{
					Math::tVec4f( +uOffset, +vOffset, 0.f, 1.f ),
					Math::tVec4f( -uOffset, +vOffset, 0.f, 1.f ),
					Math::tVec4f( -uOffset, -vOffset, 0.f, 1.f ),
					Math::tVec4f( +uOffset, -vOffset, 0.f, 1.f ),
				};
				fApplyVector4PS( device, cPSSampleOffsets, sampleOffsets[0], array_length( sampleOffsets ) );
			}
			break;
		case cPShaderGaussBlurH:
		case cPShaderGaussBlurV:
			{
				sigassert( mInputCount == 1 );
				const f32 uScale = ( mPS == cPShaderGaussBlurH ) ? ( 1.f/mInputs[0].mWidth * mBlurScale.x  ) : 0.f;
				const f32 vScale = ( mPS == cPShaderGaussBlurV ) ? ( 1.f/mInputs[0].mHeight * mBlurScale.y ) : 0.f;

				const Math::tVec4f sampleOffsets[]=
				{
					Math::tVec4f( uScale * -6.f, vScale * -6.f, 0.f, 0.002216f ),
					Math::tVec4f( uScale * -5.f, vScale * -5.f, 0.f, 0.008764f ),
					Math::tVec4f( uScale * -4.f, vScale * -4.f, 0.f, 0.026995f ),
					Math::tVec4f( uScale * -3.f, vScale * -3.f, 0.f, 0.064759f ),
					Math::tVec4f( uScale * -2.f, vScale * -2.f, 0.f, 0.120985f ),
					Math::tVec4f( uScale * -1.f, vScale * -1.f, 0.f, 0.176033f ),
					Math::tVec4f( uScale * +0.f, vScale * +0.f, 0.f, 0.199471f ),
					Math::tVec4f( uScale * +1.f, vScale * +1.f, 0.f, 0.176033f ),
					Math::tVec4f( uScale * +2.f, vScale * +2.f, 0.f, 0.120985f ),
					Math::tVec4f( uScale * +3.f, vScale * +3.f, 0.f, 0.064759f ),
					Math::tVec4f( uScale * +4.f, vScale * +4.f, 0.f, 0.026995f ),
					Math::tVec4f( uScale * +5.f, vScale * +5.f, 0.f, 0.008764f ),
					Math::tVec4f( uScale * +6.f, vScale * +6.f, 0.f, 0.002216f ),
				};
				fApplyVector4PS( device, cPSSampleOffsets, sampleOffsets[0], array_length( sampleOffsets ) );
			}
			break;
		case cPShaderBlend:
			{
				sigassert( mInputCount == 2 );
				fApplyVector4PS( device, cPSRgbaBlend, mRgbaBlend );
				fApplyVector4PS( device, cPSRgbaTint0, mRgbaTint0 );
				fApplyVector4PS( device, cPSRgbaTint1, mRgbaTint1 );
			}
			break;
		case cPShaderBlendUsingSource1Alpha:
			{
				sigassert( mInputCount == 2 );
				fApplyVector4PS( device, cPSRgbaTint0, mRgbaTint0 );
				fApplyVector4PS( device, cPSRgbaTint1, mRgbaTint1 );
			}
			break;
		case cPShaderBlendUsingDepth:
		case cPShaderBlendUsingDepthAndFog:
		case cPShaderBlendUsingDepthAndOverlay:
		case cPShaderBlendUsingDepthAndOverlayAndFog:
		case cPShaderBlendUsingDepthAndSaturate:
		case cPShaderBlendUsingDepthAndFogAndSaturate:
		case cPShaderBlendUsingDepthAndOverlayAndSaturate:
		case cPShaderBlendUsingDepthAndOverlayAndFogAndSaturate:
			{
				const b32 hasFog =
					mPS == cPShaderBlendUsingDepthAndFog ||
					mPS == cPShaderBlendUsingDepthAndOverlayAndFog ||
					mPS == cPShaderBlendUsingDepthAndFogAndSaturate ||
					mPS == cPShaderBlendUsingDepthAndOverlayAndFogAndSaturate;

				const b32 hasOverlay =
					mPS == cPShaderBlendUsingDepthAndOverlay ||
					mPS == cPShaderBlendUsingDepthAndOverlayAndFog ||
					mPS == cPShaderBlendUsingDepthAndOverlayAndSaturate ||
					mPS == cPShaderBlendUsingDepthAndOverlayAndFogAndSaturate;

				const b32 hasSaturation =
					mPS == cPShaderBlendUsingDepthAndSaturate ||
					mPS == cPShaderBlendUsingDepthAndFogAndSaturate ||
					mPS == cPShaderBlendUsingDepthAndOverlayAndSaturate ||
					mPS == cPShaderBlendUsingDepthAndOverlayAndFogAndSaturate;

				sigassert( mInputCount == 3 + (hasOverlay ? 1 : 0) );

				fApplyVector4PS( device, cPSTargetDepthValues, mTargetDepthValues );
				fApplyVector4PS( device, cPSRgbaTint0, mRgbaTint0 );
				fApplyVector4PS( device, cPSRgbaTint1, mRgbaTint1 );

				if( hasFog )
				{
					fApplyMatrix4PS( device, cPSProjToView,			context.mSceneCamera->fGetCameraToProjection( ).fInverse( ) );
					fApplyMatrix4PS( device, cPSProjToWorld,		context.mSceneCamera->fGetWorldToProjection( ).fInverse( ) );
					fApplyVector4PS( device, cPSFogParams,			context.mFogValues );
					fApplyVector3PS( device, cPSFogColor,			context.mFogColor );
					fApplyVector4PS( device, cPSVerticalFogParams,	context.mVerticalFogValues );
					fApplyVector3PS( device, cPSVerticalFogColor,	context.mVerticalFogColor );
					fApplyVector4PS( device, cPSNearFarPlanes,		tRenderContext::fComputeNearFarValues( *context.mSceneCamera ) );
				}

				if( hasSaturation )
				{
					fApplyVector4PS( device, cPSSaturation,	mSaturation );
					fApplyVector4PS( device, cPSContrast,	mContrast );
					fApplyVector4PS( device, cPSRgbaTint2,	mRgbaTint2 );
				}
			}
			break;
		case cPShaderSaturation:
			{
				sigassert( mInputCount == 1 );
				fApplyVector4PS( device, cPSRgbaTint0, mRgbaTint0 );
				fApplyVector4PS( device, cPSSaturation, mSaturation );
				fApplyVector4PS( device, cPSContrast, mContrast );
			}
			break;
		case cPShaderFilmGrain:
			{
				sigassert( mInputCount == 2 );
				fApplyVector4PS( device, cPSRgbaTint0, mRgbaTint0 );
				fApplyVector4PS( device, cPSSaturation, mSaturation );
				fApplyVector4PS( device, cPSFilmGrainFlicker, mFilmGrainFlicker );
				fApplyVector4PS( device, cPSFilmGrainOffsets0, mFilmGrainOffsets0 );
				fApplyVector4PS( device, cPSFilmGrainOffsets1, mFilmGrainOffsets1 );
			}
			break;
		case cPShaderTransform:
			{
				sigassert( mInputCount == 1 );
				fApplyVector4PS( device, cPSTransformAdd, mTransformAdd );
				fApplyVector4PS( device, cPSTransformMul, mTransformMul );
			}
			break;
		case cPShaderFilterHighPass:
			{
				sigassert( mInputCount == 1 );
				fApplyVector4PS( device, cPSTransformCutoff, mTransformCutOff );
				fApplyVector4PS( device, cPSTransformMul, mTransformMul );
			}
			break;
		}

		// apply input textures
		for( u32 i = 0; i < mInputCount; ++i )
		{
			sigassert( mInputs[ i ].mTexture );
			mInputs[ i ].mTexture->fApply( device, i );
		}

		renderBatch.fRenderInstance( device );

		tTextureReference::fClearBoundTextures( device, 0, mInputCount );
	}

	void tPostEffectsMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		sigassert( !"shouldn't be here: tPostEffectsMaterial::fApplyInstance" );
	}

	void tPostEffectsMaterial::fClearInputs( )
	{
		for( u32 i = 0; i < mInputCount; ++i )
			mInputs[ i ] = tInput( );
		mInputCount = 0;
	}

	void tPostEffectsMaterial::fSetInputCount( u32 count )
	{
		for( u32 i = count; i < mInputCount; ++i )
			mInputs[ i ] = tInput( );
		mInputCount = count;
	}

	void tPostEffectsMaterial::fAddInput( const tRenderToTexturePtr& rtt )
	{
		sigassert( mInputCount < mInputs.fCount( ) );
		mInputs[ mInputCount++ ] = tInput( &rtt->fTexture( ), rtt->fRenderTarget( )->fWidth( ), rtt->fRenderTarget( )->fHeight( ) );
	}

	void tPostEffectsMaterial::fAddInput( const tTextureReference* texture, u32 width, u32 height )
	{
		sigassert( mInputCount < mInputs.fCount( ) );
		mInputs[ mInputCount++ ] = tInput( texture, width, height );
	}

	// arranges in alphabetical order so they match the dev menu
	// in .w
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_Grain_FlickerFreq, 0.14f, 0.f, +100.f, 2 );
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_Grain_FlickerScale, 0.9f, -100.f, +100.f, 2 );
	devvar_clamp( Math::tVec2f, Renderer_PostEffects_FilmGrain_Grain_Speed, Math::tVec2f( 3.8f, 8.0f ), -100.f, +100.f, 2 );

	// in .y
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_Hairs_FlickerFreq, 0.07f, 0.f, +100.f, 2 );
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_Hairs_FlickerScale, 0.7f, -100.f, +100.f, 2 );
	devvar_clamp( Math::tVec2f, Renderer_PostEffects_FilmGrain_Hairs_Speed, Math::tVec2f( -5.f, 16.f ), -100.f, +100.f, 2 );

	// in .z
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_Lines_FlickerFreq, 0.45f, 0.f, +100.f, 2 );
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_Lines_FlickerScale, -0.3f, -100.f, +100.f, 2 );
	devvar_clamp( Math::tVec2f, Renderer_PostEffects_FilmGrain_Lines_Speed, Math::tVec2f( -10.f, -25.f ), -100.f, +100.f, 2 );

	// in .x
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_Smudges_FlickerFreq, 0.9f, 0.f, +100.f, 2 );
	devvar_clamp( f32, Renderer_PostEffects_FilmGrain_Smudges_FlickerScale, 0.3f, -100.f, +100.f, 2 );
	devvar_clamp( Math::tVec2f, Renderer_PostEffects_FilmGrain_Smudges_Speed, Math::tVec2f( 4.f, 23.f ), -100.f, +100.f, 2 );


	devvar( bool, Renderer_PostEffects_FilmGrain_UseDevVarValues, false );

	devrgb_clamp( Renderer_PostEffects_FilmGrain_Saturation, Math::tVec3f( 0.0f, 0.0f, 0.0f ), 0.f, 3.f );
	devrgb_clamp( Renderer_PostEffects_FilmGrain_Exposure, Math::tVec3f( 1.70f, 1.50f, 1.35f ), 0.f, +10.f );




	tPostEffectsMaterial::tParameters::tParameters( )
		: mExposure( Renderer_PostEffects_FilmGrain_Exposure )
		, mSaturation( Renderer_PostEffects_FilmGrain_Saturation )
		, mGrainFreq( Renderer_PostEffects_FilmGrain_Grain_FlickerFreq )
		, mGrainScale( Renderer_PostEffects_FilmGrain_Grain_FlickerScale )
		, mGrainSpeed( Renderer_PostEffects_FilmGrain_Grain_Speed )
		, mHairsFreq( Renderer_PostEffects_FilmGrain_Hairs_FlickerFreq )
		, mHairsScale( Renderer_PostEffects_FilmGrain_Hairs_FlickerScale )
		, mHairsSpeed( Renderer_PostEffects_FilmGrain_Hairs_Speed )
		, mLinesFreq( Renderer_PostEffects_FilmGrain_Lines_FlickerFreq )
		, mLinesScale( Renderer_PostEffects_FilmGrain_Lines_FlickerScale )
		, mLinesSpeed( Renderer_PostEffects_FilmGrain_Lines_Speed )
		, mSmudgeFreq( Renderer_PostEffects_FilmGrain_Smudges_FlickerFreq )
		, mSmudgeScale( Renderer_PostEffects_FilmGrain_Smudges_FlickerScale )
		, mSmudgeSpeed( Renderer_PostEffects_FilmGrain_Smudges_Speed )
	{ }

	const Math::tVec3f& tPostEffectsMaterial::tParameters::fExposure( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Exposure;
		else return mExposure;
	}

	const Math::tVec3f& tPostEffectsMaterial::tParameters::fSaturation( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Saturation;
		else return mSaturation;
	}


	f32 tPostEffectsMaterial::tParameters::fGrainFreq( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Grain_FlickerFreq;
		else return mGrainFreq;
	}

	f32 tPostEffectsMaterial::tParameters::fGrainScale( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Grain_FlickerScale;
		else return mGrainScale;
	}

	const Math::tVec2f&	tPostEffectsMaterial::tParameters::fGrainSpeed( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Grain_Speed;
		else return mGrainSpeed;
	}

	f32 tPostEffectsMaterial::tParameters::fHairsFreq( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Hairs_FlickerFreq;
		else return mHairsFreq;
	}

	f32 tPostEffectsMaterial::tParameters::fHairsScale( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Hairs_FlickerScale;
		else return mHairsScale;
	}

	const Math::tVec2f&	tPostEffectsMaterial::tParameters::fHairsSpeed( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Hairs_Speed;
		else return mHairsSpeed;
	}

	f32 tPostEffectsMaterial::tParameters::fLinesFreq( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Lines_FlickerFreq;
		else return mLinesFreq;
	}

	f32 tPostEffectsMaterial::tParameters::fLinesScale( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Lines_FlickerScale;
		else return mLinesScale;
	}

	const Math::tVec2f&	tPostEffectsMaterial::tParameters::fLinesSpeed( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Lines_Speed;
		else return mLinesSpeed;
	}

	f32 tPostEffectsMaterial::tParameters::fSmudgeFreq( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Smudges_FlickerFreq;
		else return mSmudgeFreq;
	}

	f32 tPostEffectsMaterial::tParameters::fSmudgeScale( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Smudges_FlickerScale;
		else return mSmudgeScale;
	}

	const Math::tVec2f&	tPostEffectsMaterial::tParameters::fSmudgeSpeed( ) const
	{
		if( Renderer_PostEffects_FilmGrain_UseDevVarValues ) return Renderer_PostEffects_FilmGrain_Smudges_Speed;
		else return mSmudgeSpeed;
	}

	void tPostEffectsMaterial::fStepFilmGrain( f32 dt, tRandom& rand, f32 scale )
	{
		// smudges
		{
			if( rand.fFloatZeroToOne( ) < 0.75f * Renderer_PostEffects_FilmGrain_Smudges_FlickerFreq )		mFilmGrainFlicker.x = 0.f;
			else if( rand.fFloatZeroToOne( ) < 0.55f * Renderer_PostEffects_FilmGrain_Smudges_FlickerFreq )	mFilmGrainFlicker.x = -rand.fFloatZeroToOne( );
			else																							mFilmGrainFlicker.x = 1.f;
																											mFilmGrainFlicker.x *= Renderer_PostEffects_FilmGrain_Smudges_FlickerScale;

			const Math::tVec2f uvDelta = ( dt * rand.fFloatZeroToOne( ) * rand.fFloatMinusOneToOne( ) ) * (Math::tVec2f)Renderer_PostEffects_FilmGrain_Smudges_Speed;
			mFilmGrainOffsets0.x += uvDelta.x;
			mFilmGrainOffsets0.y += uvDelta.y;
		}

		// hairs/scratches
		{
			if( rand.fFloatZeroToOne( ) < 0.25f * Renderer_PostEffects_FilmGrain_Hairs_FlickerFreq )		mFilmGrainFlicker.y = 0.f;
			else if( rand.fFloatZeroToOne( ) < 0.55f * Renderer_PostEffects_FilmGrain_Hairs_FlickerFreq )	mFilmGrainFlicker.y = -rand.fFloatZeroToOne( );
			else																							mFilmGrainFlicker.y = 1.f;
																											mFilmGrainFlicker.y *= Renderer_PostEffects_FilmGrain_Hairs_FlickerScale;

			const Math::tVec2f uvDelta = ( dt * rand.fFloatZeroToOne( ) * rand.fFloatMinusOneToOne( ) ) * (Math::tVec2f)Renderer_PostEffects_FilmGrain_Hairs_Speed;
			mFilmGrainOffsets0.z += uvDelta.x;
			mFilmGrainOffsets0.w += uvDelta.y;
		}

		// vertical lines
		{
			if( rand.fFloatZeroToOne( ) < 0.125f * Renderer_PostEffects_FilmGrain_Lines_FlickerFreq  )		mFilmGrainFlicker.z = 0.f;
			else if( rand.fFloatZeroToOne( ) < 0.25f * Renderer_PostEffects_FilmGrain_Lines_FlickerFreq  )	mFilmGrainFlicker.z = -rand.fFloatZeroToOne( );
			else																							mFilmGrainFlicker.z = 1.f;
																											mFilmGrainFlicker.z *= Renderer_PostEffects_FilmGrain_Lines_FlickerScale;

			const Math::tVec2f uvDelta = ( dt * rand.fFloatZeroToOne( ) * rand.fFloatMinusOneToOne( ) ) * (Math::tVec2f)Renderer_PostEffects_FilmGrain_Lines_Speed;
			mFilmGrainOffsets1.x += uvDelta.x;
			mFilmGrainOffsets1.y += uvDelta.y;
		}

		// grain
		{
																											mFilmGrainFlicker.w = rand.fFloatInRange( 0.125f, 0.25f );
			if( rand.fFloatZeroToOne( ) < 0.125f * Renderer_PostEffects_FilmGrain_Grain_FlickerFreq )		mFilmGrainFlicker.w *= -rand.fFloatZeroToOne( );
																											mFilmGrainFlicker.w *= Renderer_PostEffects_FilmGrain_Grain_FlickerScale;

			const Math::tVec2f uvDelta = ( dt * rand.fFloatZeroToOne( ) * rand.fFloatMinusOneToOne( ) ) * (Math::tVec2f)Renderer_PostEffects_FilmGrain_Grain_Speed;
			mFilmGrainOffsets1.z += uvDelta.x;
			mFilmGrainOffsets1.w += uvDelta.y;
		}

		mFilmGrainFlicker *= scale;
	}

}}
