#include "GameAppPch.hpp"
#include "tGamePostEffectMgr.hpp"
#include "Gfx/tScreen.hpp"
#include "tResourceDepot.hpp"

namespace Sig
{

	

	// pause cam
	devvar( Math::tVec4f, Renderer_PostEffects_PauseCam_DepthOfField, Math::tVec4f( 0.975f, 0.400f, 10.000f, 0.250f ) );

	// game cam
	devvar( bool, Renderer_PostEffects_GameCam_UseDevMenuValues, false );
	devvar( Math::tVec4f, Renderer_PostEffects_GameCam_DOF, Math::tVec4f( 0.994f, 0.465f, 3.9f, 0.5f ) );
	devrgb_clamp( Renderer_PostEffects_GameCam_Saturation, Math::tVec3f( 1.0f, 1.0f, 1.0f ), 0.f, 3.f, 2 );
	devrgb_clamp( Renderer_PostEffects_GameCam_Contrast, Math::tVec3f( 1.0f, 1.0f, 1.0f ), 0.f, 3.f, 2 );
	devrgb_clamp( Renderer_PostEffects_GameCam_Exposure, Math::tVec3f( 1.f ), -10.f, +10.f, 2 );
	devrgb_clamp( Renderer_PostEffects_GameCam_CharacterTint, Math::tVec3f( 0.f, 0.f, 0.f ), 0.f, 2.f, 2 );


	// film grain (front-end)
	devrgb_clamp( Renderer_PostEffects_FilmGrain_FrontEnd_Exposure, Math::tVec3f( 1.4f, 1.3f, 1.24f ), -10.f, +10.f, 2 );
	devrgb_clamp( Renderer_PostEffects_FilmGrain_FrontEnd_Saturation, Math::tVec3f( 1.0f, 1.0f, 1.0f ), 0.f, 3.f, 2 );

	// film grain (common)
	devvar_clamp( f32, Renderer_PostEffects_HealthEffect_PulseSmooth, 0.25f, 0.f, +1.f, 3 );
	devrgba_clamp( Renderer_PostEffects_HealthEffect_Tint, Math::tVec4f( 1.0f, 1.0f, 1.0f, 1.0f ), 0.f, 4.f, 2 );

	namespace
	{
		static const tStringPtr cSeqNameNull;
		static const tStringPtr cSeqNameGameCam[]= { tStringPtr( "postEffectGameCam0" ), tStringPtr( "postEffectGameCam1" ) };
		static const tStringPtr cSeqNamePauseCam[]= { tStringPtr( "postEffectPauseCam0" ), tStringPtr( "postEffectPauseCam1" ) };
		static const tStringPtr cSeqNameFilmGrain[]= { tStringPtr( "postEffectFilmGrain0" ), tStringPtr( "postEffectFilmGrain1" ) };
		static tRandom gSubjectiveRand;
	}

	tGamePostEffectManager::tPerViewportData::tPerViewportData( )
		: mFilmGrainFadeOutDuration( -1.f )
		, mFilmGrainFadeOutTimer( 0.f )
		, mFilmGrainPulseDuration( -1.f )
		, mFilmGrainPulseTimer( 0.f )
		, mFilmGrainPulseStrength( 0.f )
		, mFilmGrainPulseBaseStrength( 0.f )
		, mFilmGrainLastPulse( 1.f )
		, mIndex( 0 )
		, mFilmGrainOnOverride( false )
		, mFilmGrainExposureMult( 1.f )
		, mGlobalExposureMult( 1.f )
	{
	}

	void tGamePostEffectManager::tPerViewportData::fOnTick( tGamePostEffectManager& mgr, f32 dt, b32 paused )
	{
		if( mPauseCamDofMtl )
			mPauseCamDofMtl->mTargetDepthValues = Renderer_PostEffects_PauseCam_DepthOfField;
		if( mGameCamDofMtl )
			mGameCamDofMtl->mTargetDepthValues = mgr.fCurrentGameCamDof( mIndex );
		if( mFilmGrainDofMtl )
			mFilmGrainDofMtl->mTargetDepthValues = paused ? Renderer_PostEffects_PauseCam_DepthOfField : mgr.fCurrentGameCamDof( mIndex );
		if( mPauseCamSaturationMtl )
		{
			mPauseCamSaturationMtl->mSaturation = Math::tVec4f( mgr.fCurrentGameCamSaturation( mIndex ), 1.f );
			mPauseCamSaturationMtl->mRgbaTint0 = Math::tVec4f( mgr.fCurrentGameCamExposure( mIndex ), 1.f ) * mGlobalExposureMult;
		}
		if( mGameCamSaturationMtl )
		{
			mGameCamSaturationMtl->mSaturation = Math::tVec4f( mgr.fCurrentGameCamSaturation( mIndex ), 1.f );
			mGameCamSaturationMtl->mRgbaTint0 = Math::tVec4f( mgr.fCurrentGameCamExposure( mIndex ), 1.f ) * mGlobalExposureMult;
		}
		if( mFilmGrainMaterial )
		{
			tRandom& rand = gSubjectiveRand;

			const b32 colorized = mgr.mFrontEndMode || (mgr.mInGameFilmGrainMode == cInGameFilmGrainColorized);
			const Math::tVec3f baseSaturation = colorized ? Renderer_PostEffects_FilmGrain_FrontEnd_Saturation : mFilmGrainMaterial->mParameters.fSaturation( );
			const Math::tVec3f baseExposure = colorized ? Renderer_PostEffects_FilmGrain_FrontEnd_Exposure : mFilmGrainMaterial->mParameters.fExposure( );

			Math::tVec4f saturation, exposure; f32 damage; f32 flicker;
			if( mFilmGrainFadeOutTimer < mFilmGrainFadeOutDuration )
			{
				const f32 t = mFilmGrainFadeOutTimer / mFilmGrainFadeOutDuration;
				mFilmGrainFadeOutTimer += dt;

				flicker = 1.f - t;
				saturation = Math::tVec4f( Math::fLerp( baseSaturation, mgr.fCurrentGameCamSaturation( mIndex ), t ), 1.f );
				exposure = Math::tVec4f( Math::fLerp( baseExposure, mgr.fCurrentGameCamExposure( mIndex ), t ), 1.f );
				damage = 0.f;
			}
			else if( mFilmGrainPulseStrength > 1.f || mFilmGrainPulseTimer < mFilmGrainPulseDuration )
			{
				f32 t;
				if( mFilmGrainPulseStrength > 1.f )
				{
					t = 0.f;
					damage = 0.f;
				}
				else
				{
					t = 1.f - fMin( 1.f, rand.fFloatInRange( 0.f, 1.5f * mFilmGrainPulseStrength ) );
					t = Math::fLerp( mFilmGrainLastPulse, t, ( f32 )Renderer_PostEffects_HealthEffect_PulseSmooth );
					mFilmGrainLastPulse = t;

					//mFilmGrainPulseTimer += dt;

					if( mFilmGrainPulseTimer >= mFilmGrainPulseDuration )
					{
						const f32 thresholdToTurnOffAt = 0.125f;
						const f32 thresholdToNeverTurnOffAt = 0.80f;
						if( mFilmGrainPulseStrength < thresholdToTurnOffAt )
						{
							if( mFilmGrainPulseBaseStrength >= thresholdToNeverTurnOffAt )
								mFilmGrainPulseTimer = 0.f; // just keep going at this low level of strength
							else
							{
								mFilmGrainPulseStrength = 0.f;
								mFilmGrainPulseDuration = 2.f;
								mFilmGrainPulseTimer = 0.f;
							}
						}
						else
						{
							mFilmGrainPulseStrength *= 0.5f;
							mFilmGrainPulseDuration *= 0.5f;
							mFilmGrainPulseTimer = 0.f;
						}
					}

					damage = Math::fLerp( 1.f, 0.f, t );
				}

				flicker = 1.f - t;
				saturation = Math::tVec4f( Math::fLerp( baseSaturation, ( Math::tVec3f )mgr.fCurrentGameCamSaturation( mIndex ), t ), 1.f );
				exposure = Math::tVec4f( Math::fLerp( baseExposure, ( Math::tVec3f )mgr.fCurrentGameCamExposure( mIndex ), t ), 1.f );
			}
			else
			{
				flicker = 1.f;
				saturation = Math::tVec4f( baseSaturation, 1.f );
				exposure = Math::tVec4f( baseExposure, 1.f );
				damage = 0.f;
			}

			mFilmGrainMaterial->fStepFilmGrain( dt, rand, flicker );
			mFilmGrainMaterial->mSaturation = saturation;
			mFilmGrainMaterial->mRgbaTint0 = exposure * mFilmGrainExposureMult * mGlobalExposureMult;

			const Math::tVec4f healthEffectTint = Renderer_PostEffects_HealthEffect_Tint;
			mDamageOverlayMaterial->mRgbaTint1 = Math::tVec4f( healthEffectTint.fXYZ( ), healthEffectTint.w * damage );
		}
	}

	b32 tGamePostEffectManager::tPerViewportData::fFilmGrainActive( ) const
	{
		if( mFilmGrainOnOverride || mFilmGrainFadeOutTimer < mFilmGrainFadeOutDuration )
			return true;
		if( mFilmGrainPulseStrength > 1.f || mFilmGrainPulseTimer < mFilmGrainPulseDuration )
			return true;
		return false;
	}
	void tGamePostEffectManager::tPerViewportData::fBeginFilmGrainFadeOut( f32 fadeOutTime )
	{
		mFilmGrainFadeOutDuration = fadeOutTime;
		mFilmGrainFadeOutTimer = 0.f;

		mFilmGrainPulseDuration = -1.f;
		mFilmGrainPulseTimer = 0.f;
		mFilmGrainPulseStrength = 0.f;
		mFilmGrainPulseBaseStrength = 0.f;
		mFilmGrainLastPulse = 1.f;
	}

	void tGamePostEffectManager::tPerViewportData::fFilmGrainPulse( f32 zeroToOneStrength, f32 duration )
	{
		mFilmGrainPulseTimer = 0.f;
		mFilmGrainPulseDuration = duration;
		mFilmGrainPulseStrength = zeroToOneStrength;
		mFilmGrainPulseBaseStrength = zeroToOneStrength;
	}

	void tGamePostEffectManager::tPerViewportData::fClearFilmGrainPulse( )
	{
		mFilmGrainPulseTimer = 0.f;
		mFilmGrainPulseDuration = 0.f;
		mFilmGrainPulseStrength = 0.f;
		mFilmGrainPulseBaseStrength = 0.f;
	}


	Gfx::tPostEffectsMaterial::tParameters* tGamePostEffectsData::fFilmGrainOverride( )
	{
		if( !mFilmGrainOverride )
			mFilmGrainOverride.fReset( NEW Gfx::tPostEffectsMaterial::tParameters( ) );

		return mFilmGrainOverride.fGetRawPtr( );
	}

	tGamePostEffectManager::tGamePostEffectManager( const tResourcePtr& postEffectsMtlFile )
		: Gfx::tPostEffectManager( postEffectsMtlFile )
		, mFrontEndMode( false )
		, mInGameFilmGrainMode( cInGameFilmGrainNone )
	{
		for( u32 i = 0; i < mPerViewportData.fCount( ); ++i )
		{
			mPerViewportData[ i ].mIndex = i;
			mPerViewportData[ i ].mDefaultGamecamData = tGamePostEffectsData( Renderer_PostEffects_GameCam_DOF, Renderer_PostEffects_GameCam_Saturation, Renderer_PostEffects_GameCam_Contrast, Renderer_PostEffects_GameCam_Exposure, Math::tVec4f(Renderer_PostEffects_GameCam_CharacterTint, 0.f) );
			fPushEffectsData( mPerViewportData[ i ].mDefaultGamecamData, i );
		}

		fAddFilmGrainTexture( tStringPtr::cNullPtr, tFilePathPtr( "gui\\Textures\\PostEffects\\filmgrain_g.tgab" ) );
		//fAddFilmGrainTexture( tStringPtr("video_noise"), tFilePathPtr( "gui\\Textures\\PostEffects\\videonoise01_g.tgab" ) );

		const tFilePathPtr damageTexturePath = tFilePathPtr( "gui\\Textures\\PostEffects\\damage_g.pngb" );
		tResourcePtr damageEffectResource = postEffectsMtlFile->fGetOwner( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( damageTexturePath ), this );
		mDamageEffectTexture.fSetDynamic( damageEffectResource.fGetRawPtr( ) );
		mDamageEffectTexture.fSetSamplingModes( Gfx::tTextureFile::cFilterModeNoMip, Gfx::tTextureFile::cAddressModeClamp );


		const tFilePathPtr inGameOverlayTexturePath = tFilePathPtr( "gui\\Textures\\PostEffects\\gameoverlay_g.tgab" );
		tResourcePtr inGameOverlayResource = postEffectsMtlFile->fGetOwner( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( inGameOverlayTexturePath ), this );
		mInGameOverlayTexture.fSetDynamic( inGameOverlayResource.fGetRawPtr( ) );
		mInGameOverlayTexture.fSetSamplingModes( Gfx::tTextureFile::cFilterModeNoMip, Gfx::tTextureFile::cAddressModeClamp );
	}

	tGamePostEffectManager::~tGamePostEffectManager( )
	{
		tHashTable<tStringPtr, Gfx::tTextureReference>::tConstIteratorNoNullOrRemoved it( mFilmGrainTexture.fBegin( ), mFilmGrainTexture.fEnd( ) );
		for( ; it != mFilmGrainTexture.fEnd( ); ++it )
		{
			if( (*it).mValue.fGetDynamic( ) )
				(*it).mValue.fGetDynamic( )->fUnload( this );
		}

		if( mDamageEffectTexture.fGetDynamic( ) )
			mDamageEffectTexture.fGetDynamic( )->fUnload( this );
		if( mInGameOverlayTexture.fGetDynamic( ) )
			mInGameOverlayTexture.fGetDynamic( )->fUnload( this );
	}
	
	void tGamePostEffectManager::fAddFilmGrainTexture( const tStringPtr& key, const tFilePathPtr& path )
	{
		tResourcePtr filmGrainResource = mPostEffectsMaterialFile->fGetOwner( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( path ), this );
		Gfx::tTextureReference mat;
		mat.fSetDynamic( filmGrainResource.fGetRawPtr( ) );
		mat.fSetSamplingModes( Gfx::tTextureFile::cFilterModeNoMip, Gfx::tTextureFile::cAddressModeWrap );

		mFilmGrainTexture.fInsert( key, mat );
	}

	Math::tVec4f tGamePostEffectManager::fCurrentGameCamDof( u32 viewportIndex ) const
	{
		return Renderer_PostEffects_GameCam_UseDevMenuValues ? Renderer_PostEffects_GameCam_DOF : mPerViewportData[ viewportIndex ].mCurrentGameCamData.mDof;
	}
	Math::tVec3f tGamePostEffectManager::fCurrentGameCamSaturation( u32 viewportIndex ) const
	{
		return Renderer_PostEffects_GameCam_UseDevMenuValues ? Renderer_PostEffects_GameCam_Saturation : mPerViewportData[ viewportIndex ].mCurrentGameCamData.mSaturation;
	}
	Math::tVec3f tGamePostEffectManager::fCurrentGameCamContrast( u32 viewportIndex ) const
	{
		return Renderer_PostEffects_GameCam_UseDevMenuValues ? Renderer_PostEffects_GameCam_Contrast : mPerViewportData[ viewportIndex ].mCurrentGameCamData.mContrast;
	}
	Math::tVec3f tGamePostEffectManager::fCurrentGameCamExposure( u32 viewportIndex ) const
	{
		return Renderer_PostEffects_GameCam_UseDevMenuValues ? Renderer_PostEffects_GameCam_Exposure : mPerViewportData[ viewportIndex ].mCurrentGameCamData.mExposure;
	}
	Math::tVec4f tGamePostEffectManager::fCurrentGameCamUnitTint( u32 viewportIndex ) const
	{
		return Renderer_PostEffects_GameCam_UseDevMenuValues ? Math::tVec4f( Renderer_PostEffects_GameCam_CharacterTint, 0.f ) : mPerViewportData[ viewportIndex ].mCurrentGameCamData.mUnitTint;
	}

	void tGamePostEffectManager::fPushEffectsData( const tGamePostEffectsData& data, u32 viewportIndex )
	{
		mPerViewportData[ viewportIndex ].mDataStack.fPushBack( data );
		mPerViewportData[ viewportIndex ].mCurrentGameCamData = mPerViewportData[ viewportIndex ].mDataStack.fBack( );

		if( data.mFilmGrainOverride )
			fPushFilmGrainData( *data.mFilmGrainOverride, viewportIndex );
		else
			fResetFilmGranData( viewportIndex );
	}

	void tGamePostEffectManager::fPopEffectsData( u32 viewportIndex )
	{
		if( mPerViewportData[ viewportIndex ].mFilmGrainOnOverride )
			fResetFilmGranData( viewportIndex );

		mPerViewportData[ viewportIndex ].mDataStack.fPopBack( );
		if( mPerViewportData[ viewportIndex ].mDataStack.fCount( ) > 0 )
		{
			tGamePostEffectsData& data = mPerViewportData[ viewportIndex ].mDataStack.fBack( );
			mPerViewportData[ viewportIndex ].mCurrentGameCamData = data;
			if( data.mFilmGrainOverride )
				fPushFilmGrainData( *data.mFilmGrainOverride, viewportIndex );
		}
	}

	void tGamePostEffectManager::fResetEffectsData( )
	{
		for( u32 i = 0; i < mPerViewportData.fCount( ); ++i )
		{
			mPerViewportData[ i ].mFilmGrainExposureMult = 1.f;
			mPerViewportData[ i ].mGlobalExposureMult = 1.f;
			mPerViewportData[ i ].mDataStack.fSetCount( 0 );
			fPushEffectsData( mPerViewportData[ i ].mDefaultGamecamData, i );
		}
	}

	void tGamePostEffectManager::fPushFilmGrainData( const Gfx::tPostEffectsMaterial::tParameters& params, u32 viewportIndex )
	{
		tPerViewportData& data = mPerViewportData[ viewportIndex ];

		if( data.mFilmGrainMaterial )
		{
			data.mFilmGrainMaterial->mParameters = params;
			data.mFilmGrainOnOverride = true;
			fReconfigureFilmGrainEffect( viewportIndex );
		}
	}

	void tGamePostEffectManager::fResetFilmGranData( u32 viewportIndex )
	{
		tPerViewportData& data = mPerViewportData[ viewportIndex ];

		if( data.mFilmGrainMaterial )
		{
			data.mFilmGrainMaterial->mParameters = Gfx::tPostEffectsMaterial::tParameters( );
			data.mFilmGrainOnOverride = false;
			fReconfigureFilmGrainEffect( viewportIndex );
		}
	}

	void tGamePostEffectManager::fReconfigureFilmGrainEffect( u32 viewportIndex )
	{	
		Gfx::tTextureReference* tRef = mFilmGrainTexture.fFind( mPerViewportData[ viewportIndex ].mFilmGrainMaterial->mParameters.mTextureKey );
		if( !tRef ) 
			return;

		tResource* filmGrainResource = tRef->fGetDynamic( );
		if( !filmGrainResource || !filmGrainResource->fLoaded( ) ) 
			return;

		Gfx::tTextureFile* filmGrain = filmGrainResource->fCast< Gfx::tTextureFile >( );

		fConfigureFilmGrainInputs( mPerViewportData[ viewportIndex ].mFilmGrainMaterial.fGetRawPtr( )
			, *tRef
			, filmGrain->mWidth
			, filmGrain->mHeight );
	}

	void tGamePostEffectManager::fSetupSoftFocus( Gfx::tScreen& screen )
	{
		Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );

		const Gfx::tRenderToTexturePtr  nullRtt;
		const Gfx::tRenderToTexturePtr& sceneRtt = screen.fSceneRenderToTexture( );
		const Gfx::tRenderToTexturePtr& screenSpaceRtt = screen.fScreenSpaceRenderToTexture( );

		fAddDownsample2x2Pass( sequence, sceneRtt, mHalfSizeRt );
		fAddDownsample2x2Pass( sequence, mHalfSizeRt, mQuarterSizeRt );
		fAddGaussBlurPass( sequence, mQuarterSizeRt, mQuarterSizeRtAlt );

		fAddSequence( screen, tStringPtr( "gameCam" ), sequence );
	}

	const tStringPtr& tGamePostEffectManager::fSeqNameGameCam( u32 ithVp )
	{
		if( ithVp >= array_length( cSeqNameGameCam ) ) return cSeqNameNull;
		return cSeqNameGameCam[ ithVp ];
	}
	const tStringPtr& tGamePostEffectManager::fSeqNamePauseCam( u32 ithVp )
	{
		if( ithVp >= array_length( cSeqNamePauseCam ) ) return cSeqNameNull;
		return cSeqNamePauseCam[ ithVp ];
	}
	const tStringPtr& tGamePostEffectManager::fSeqNameFilmGrain( u32 ithVp )
	{
		if( ithVp >= array_length( cSeqNameFilmGrain ) ) return cSeqNameNull;
		return cSeqNameFilmGrain[ ithVp ];
	}

	void tGamePostEffectManager::fDestroyRenderTargets( )
	{
		for( u32 i = 0; i < mPerViewportData.fCount( ); ++i )
			mPerViewportData[ i ] = tPerViewportData( );

		mHalfSizeRt.fRelease( );
		mQuarterSizeRt.fRelease( );
		mQuarterSizeRtAlt.fRelease( );
		Gfx::tPostEffectManager::fDestroyRenderTargets( );
	}

	void tGamePostEffectManager::fCreateRenderTargets( Gfx::tScreen& screen )
	{
		fSetupRenderTargets( screen );
		//fSetupSoftFocus( screen );
		fSetupPauseCam( screen );
		fSetupGameCam( screen );
		fSetupFilmGrain( screen );
	}

	void tGamePostEffectManager::fOnTick( f32 dt, b32 paused )
	{
		for( u32 i = 0; i < mPerViewportData.fCount( ); ++i )
			mPerViewportData[ i ].fOnTick( *this, dt, paused );
	}

	b32 tGamePostEffectManager::fFilmGrainActive( u32 ithVp ) const
	{
		if( ithVp >= mPerViewportData.fCount( ) ) return false;
		return mPerViewportData[ ithVp ].fFilmGrainActive( );
	}

	void tGamePostEffectManager::fBeginFilmGrainFadeOut( u32 ithVp, f32 fadeOutTime )
	{
		if( mInGameFilmGrainMode != cInGameFilmGrainNone )
			return;

		if( ithVp < mPerViewportData.fCount( ) )
			mPerViewportData[ ithVp ].fBeginFilmGrainFadeOut( fadeOutTime );
	}

	void tGamePostEffectManager::fFilmGrainPulse( u32 ithVp, f32 zeroToOneStrength, f32 duration )
	{
		if( mInGameFilmGrainMode != cInGameFilmGrainNone )
			return;

		if( ithVp < mPerViewportData.fCount( ) )
			mPerViewportData[ ithVp ].fFilmGrainPulse( zeroToOneStrength, duration );
	}

	void tGamePostEffectManager::fClearFilmGrainPulse( u32 ithVp )
	{
		if( mInGameFilmGrainMode != cInGameFilmGrainNone )
			return;

		if( ithVp < mPerViewportData.fCount( ) )
			mPerViewportData[ ithVp ].fClearFilmGrainPulse( );
	}

	void tGamePostEffectManager::fSetupRenderTargets( Gfx::tScreen& screen )
	{
		const Gfx::tRenderToTexturePtr& sceneDefRtt = screen.fSceneRenderToTexture( );

		// we can safely re-use the world depth texture depth target
		Gfx::tRenderTargetPtr sharedDepthTarget = screen.fScreenSpaceRenderToTexture( )->fDepthTarget( );

		mHalfSizeRt.fReset( NEW Gfx::tRenderToTexture( 
			screen.fGetDevice( ), 
			sceneDefRtt->fRenderTarget( )->fWidth( ) / 2, 
			sceneDefRtt->fRenderTarget( )->fHeight( ) / 2,
			Gfx::tRenderTarget::cFormatRGBA8,
			sharedDepthTarget ) );
		fAddRenderTarget( mHalfSizeRt );

		mQuarterSizeRt.fReset( NEW Gfx::tRenderToTexture( 
			screen.fGetDevice( ), 
			sceneDefRtt->fRenderTarget( )->fWidth( ) / 4, 
			sceneDefRtt->fRenderTarget( )->fHeight( ) / 4,
			Gfx::tRenderTarget::cFormatRGBA8,
			sharedDepthTarget ) );
		fAddRenderTarget( mQuarterSizeRt );

		mQuarterSizeRtAlt.fReset( NEW Gfx::tRenderToTexture( 
			screen.fGetDevice( ), 
			sceneDefRtt->fRenderTarget( )->fWidth( ) / 4, 
			sceneDefRtt->fRenderTarget( )->fHeight( ) / 4,
			Gfx::tRenderTarget::cFormatRGBA8,
			sharedDepthTarget ) );
		fAddRenderTarget( mQuarterSizeRtAlt );
	}

	void tGamePostEffectManager::fSetupPauseCam( Gfx::tScreen& screen )
	{
		const Gfx::tRenderToTexturePtr  nullRtt;
		const Gfx::tRenderToTexturePtr& sceneRtt = screen.fSceneRenderToTexture( );
		const Gfx::tRenderToTexturePtr& screenSpaceRtt = screen.fScreenSpaceRenderToTexture( );

		tResource* inGameOverlayResource = mInGameOverlayTexture.fGetDynamic( );
		if( !inGameOverlayResource || !inGameOverlayResource->fLoaded( ) ) 
			return;

		for( u32 i = 0; i < mPerViewportData.fCount( ); ++i )
		{
			tPerViewportData& perVpData = mPerViewportData[ i ];
			Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );

			fAddDownsample2x2Pass( sequence, sceneRtt, mHalfSizeRt );
			fAddDownsample2x2Pass( sequence, mHalfSizeRt, mQuarterSizeRt );
			fAddGaussBlurPass( sequence, mQuarterSizeRt, mQuarterSizeRtAlt );
			perVpData.mPauseCamDofMtl = fAddDepthOfFieldWithOverlayPass( sequence, mInGameOverlayTexture, inGameOverlayResource->fCast< Gfx::tTextureFile >( )->mWidth, inGameOverlayResource->fCast< Gfx::tTextureFile >( )->mHeight, 
				sceneRtt, mQuarterSizeRt, screen.fPostProcessDepthTexture( ), screenSpaceRtt, Renderer_PostEffects_PauseCam_DepthOfField );
			perVpData.mPauseCamSaturationMtl = fAddSaturationPass( sequence, screenSpaceRtt, nullRtt, fCurrentGameCamSaturation( i ) );

			fAddSequence( screen, fSeqNamePauseCam( i ), sequence );
		}
	}

	void tGamePostEffectManager::fSetupGameCam( Gfx::tScreen& screen )
	{
		const Gfx::tRenderToTexturePtr  nullRtt;
		const Gfx::tRenderToTexturePtr& sceneRtt = screen.fSceneRenderToTexture( );
		const Gfx::tRenderToTexturePtr& screenSpaceRtt = screen.fScreenSpaceRenderToTexture( );

		tResource* inGameOverlayResource = mInGameOverlayTexture.fGetDynamic( );
		if( !inGameOverlayResource || !inGameOverlayResource->fLoaded( ) ) 
			return;

		for( u32 i = 0; i < mPerViewportData.fCount( ); ++i )
		{
			tPerViewportData& perVpData = mPerViewportData[ i ];
			Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );

			fAddDownsample2x2Pass( sequence, sceneRtt, mHalfSizeRt );
			fAddDownsample2x2Pass( sequence, mHalfSizeRt, mQuarterSizeRt );
			fAddGaussBlurPass( sequence, mQuarterSizeRt, mQuarterSizeRtAlt );
			perVpData.mGameCamDofMtl = fAddDepthOfFieldWithOverlayPass( sequence, mInGameOverlayTexture, inGameOverlayResource->fCast< Gfx::tTextureFile >( )->mWidth, inGameOverlayResource->fCast< Gfx::tTextureFile >( )->mHeight, 
				sceneRtt, mQuarterSizeRt, screen.fPostProcessDepthTexture( ), screenSpaceRtt, fCurrentGameCamDof( i ) );
			perVpData.mGameCamSaturationMtl = fAddSaturationPass( sequence, screenSpaceRtt, nullRtt, fCurrentGameCamSaturation( i ) );

			fAddSequence( screen, fSeqNameGameCam( i ), sequence );
		}
	}

	void tGamePostEffectManager::fSetupFilmGrain( Gfx::tScreen& screen )
	{
		Gfx::tTextureReference* tRef = mFilmGrainTexture.fFind( tStringPtr::cNullPtr );
		if( !tRef ) 
			return;
		tResource* filmGrainResource = tRef->fGetDynamic( );
		if( !filmGrainResource || !filmGrainResource->fLoaded( ) ) 
			return;
		tResource* damageEffectResource = mDamageEffectTexture.fGetDynamic( );
		if( !damageEffectResource || !damageEffectResource->fLoaded( ) ) 
			return;
		tResource* inGameOverlayResource = mInGameOverlayTexture.fGetDynamic( );
		if( !inGameOverlayResource || !inGameOverlayResource->fLoaded( ) ) 
			return;

		const Gfx::tRenderToTexturePtr  nullRtt;
		const Gfx::tRenderToTexturePtr& sceneRtt = screen.fSceneRenderToTexture( );
		const Gfx::tRenderToTexturePtr& screenSpaceRtt = screen.fScreenSpaceRenderToTexture( );
		const b32 renderToSelf = screenSpaceRtt->fCanRenderToSelf( );

		for( u32 i = 0; i < mPerViewportData.fCount( ); ++i )
		{
			tPerViewportData& perVpData = mPerViewportData[ i ];
			Gfx::tPostEffectSequencePtr sequence( NEW Gfx::tPostEffectSequence );

			fAddDownsample2x2Pass( sequence, sceneRtt, mHalfSizeRt );
			fAddDownsample2x2Pass( sequence, mHalfSizeRt, mQuarterSizeRt );
			fAddGaussBlurPass( sequence, mQuarterSizeRt, mQuarterSizeRtAlt );
			perVpData.mFilmGrainDofMtl = fAddDepthOfFieldWithOverlayPass( sequence, mInGameOverlayTexture, inGameOverlayResource->fCast< Gfx::tTextureFile >( )->mWidth, inGameOverlayResource->fCast< Gfx::tTextureFile >( )->mHeight, 
				sceneRtt, mQuarterSizeRt, screen.fPostProcessDepthTexture( ), screenSpaceRtt, fCurrentGameCamDof( i ) );

			perVpData.mFilmGrainMaterial = fAddFilmGrainPass( sequence, screenSpaceRtt, *tRef, 
				filmGrainResource->fCast< Gfx::tTextureFile >( )->mWidth, filmGrainResource->fCast< Gfx::tTextureFile >( )->mHeight, renderToSelf ? screenSpaceRtt : sceneRtt );
			perVpData.mDamageOverlayMaterial = fAddBlendUsingSource1AlphaPass( sequence, renderToSelf ? screenSpaceRtt : sceneRtt, mDamageEffectTexture, 
				damageEffectResource->fCast< Gfx::tTextureFile >( )->mWidth, damageEffectResource->fCast< Gfx::tTextureFile >( )->mHeight, screenSpaceRtt );

			if( !renderToSelf )
			{
				// unfortunately we have to copy back to main target
				fAddCopyPass( sequence, screenSpaceRtt, sceneRtt );
			}

			perVpData.mFilmGrainMaterial->mSaturation = Math::tVec4f( perVpData.mFilmGrainMaterial->mParameters.fSaturation( ), 1.f );
			perVpData.mFilmGrainMaterial->mRgbaTint0 = Math::tVec4f( perVpData.mFilmGrainMaterial->mParameters.fExposure( ), 1.f );
			perVpData.mDamageOverlayMaterial->mRgbaTint1 = Renderer_PostEffects_HealthEffect_Tint;
			perVpData.mDamageOverlayMaterial->mRgbaTint1.w = 0.f; // start transparent

			fAddSequence( screen, fSeqNameFilmGrain( i ), sequence );
		}
	}

	void tGamePostEffectManager::fSetGlobalExposureMult( u32 ithVp, f32 mult )
	{
		mPerViewportData[ ithVp ].mGlobalExposureMult = mult;
	}

	void tGamePostEffectManager::fSetFilmGrainExposureMult( u32 ithVp, f32 mult )
	{
		mPerViewportData[ ithVp ].mFilmGrainExposureMult = mult;
	}
}

#include "tGameApp.hpp"

namespace Sig
{
	static void fSetPostEffectSaturation( const Math::tVec3f& saturation )
	{
		tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( 0 ).mSaturation = saturation;
		tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( 1 ).mSaturation = saturation;
	}
	static void fSetPostEffectContrast( const Math::tVec3f& contrast )
	{
		tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( 0 ).mContrast = contrast;
		tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( 1 ).mContrast = contrast;
	}
	static void fSetPostEffectExposure( const Math::tVec3f& exposure )
	{
		tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( 0 ).mExposure = exposure;
		tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( 1 ).mExposure = exposure;
	}
	static void fSetPostEffectDof( const Math::tVec4f& dof )
	{
		tGameApp::fInstance( ).fPostEffectsManager( )->fDefaultGameCamData( 0 ).mDof = dof;
		tGameApp::fInstance( ).fPostEffectsManager( )->fDefaultGameCamData( 1 ).mDof = dof;
		tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( 0 ).mDof = dof;
		tGameApp::fInstance( ).fPostEffectsManager( )->fCurrentGameCamData( 1 ).mDof = dof;
	}

	void tGamePostEffectManager::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<Gfx::tPostEffectsMaterial::tParameters, Sqrat::DefaultAllocator<Gfx::tPostEffectsMaterial::tParameters>> classDesc( vm.fSq( ) );
			classDesc				
				.Var(_SC("TextureKey"),		&Gfx::tPostEffectsMaterial::tParameters::mTextureKey)
				.Var(_SC("Exposure"),		&Gfx::tPostEffectsMaterial::tParameters::mExposure)
				.Var(_SC("Saturation"),		&Gfx::tPostEffectsMaterial::tParameters::mSaturation)
				.Var(_SC("GrainFreq"),		&Gfx::tPostEffectsMaterial::tParameters::mGrainFreq)
				.Var(_SC("GrainScale"),		&Gfx::tPostEffectsMaterial::tParameters::mGrainScale)
				.Var(_SC("GrainSpeed"),		&Gfx::tPostEffectsMaterial::tParameters::mGrainSpeed)
				.Var(_SC("HairsFreq"),		&Gfx::tPostEffectsMaterial::tParameters::mHairsFreq)
				.Var(_SC("HairsScale"),		&Gfx::tPostEffectsMaterial::tParameters::mHairsScale)
				.Var(_SC("HairsSpeed"),		&Gfx::tPostEffectsMaterial::tParameters::mHairsSpeed)
				.Var(_SC("LinesFreq"),		&Gfx::tPostEffectsMaterial::tParameters::mLinesFreq)
				.Var(_SC("LinesScale"),		&Gfx::tPostEffectsMaterial::tParameters::mLinesScale)
				.Var(_SC("LinesSpeed"),		&Gfx::tPostEffectsMaterial::tParameters::mLinesSpeed)
				.Var(_SC("SmudgeFreq"),		&Gfx::tPostEffectsMaterial::tParameters::mSmudgeFreq)
				.Var(_SC("SmudgeScale"),	&Gfx::tPostEffectsMaterial::tParameters::mSmudgeScale)
				.Var(_SC("SmudgeSpeed"),	&Gfx::tPostEffectsMaterial::tParameters::mSmudgeSpeed)
				;
			vm.fRootTable( ).Bind( _SC("GamePostEffectsDataFilmGrain"), classDesc );
		}

		{
			Sqrat::Class<tGamePostEffectsData, Sqrat::DefaultAllocator<tGamePostEffectsData>> classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("Dof"),		&tGamePostEffectsData::mDof)
				.Var(_SC("Saturation"),	&tGamePostEffectsData::mSaturation)
				.Var(_SC("Contrast"),	&tGamePostEffectsData::mContrast)
				.Var(_SC("Exposure"),	&tGamePostEffectsData::mExposure)
				.Var(_SC("UnitTint"),	&tGamePostEffectsData::mUnitTint)
				.Prop(_SC("FilmGrainOverride"), &tGamePostEffectsData::fFilmGrainOverride)
				;
			vm.fRootTable( ).Bind( _SC("GamePostEffectsData"), classDesc );
		}

		{
			Sqrat::Class<tGamePostEffectManager, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.StaticFunc( "SetSaturation", &fSetPostEffectSaturation )
				.StaticFunc( "SetContrast", &fSetPostEffectContrast )
				.StaticFunc( "SetExposure", &fSetPostEffectExposure )
				.StaticFunc( "SetDepthOfField", &fSetPostEffectDof )
				;
			vm.fRootTable( ).Bind(_SC("PostEffects"), classDesc );
		}
	}

}
