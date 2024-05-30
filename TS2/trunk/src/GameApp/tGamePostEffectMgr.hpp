#ifndef __tGamePostEffectMgr__
#define __tGamePostEffectMgr__
#include "Gfx/tPostEffect.hpp"

namespace Sig
{

	class tGamePostEffectsData : public tRefCounter
	{
	public:
		tGamePostEffectsData( )
			: mDof( Math::tVec4f::cOnesVector )
			, mSaturation( Math::tVec3f::cOnesVector )
			, mContrast( Math::tVec3f::cOnesVector )
			, mExposure( Math::tVec3f::cOnesVector )
			, mUnitTint( Math::tVec4f::cZeroVector )
		{ }

		tGamePostEffectsData( const Math::tVec4f& dof, const Math::tVec3f& sat, const Math::tVec3f& contrast, const Math::tVec3f& exposure, const Math::tVec4f& characterTint )
			: mDof( dof )
			, mSaturation( sat )
			, mContrast( contrast )
			, mExposure( exposure )
			, mUnitTint( characterTint )
		{ }

		/* Depth of field usage, from shader:
			float middleDepth = gDepthBlendValues.x; 
			float focalPlane = gDepthBlendValues.y; 
			float blurScale = gDepthBlendValues.z;
			float minBlur = gDepthBlendValues.w;
		*/
		Math::tVec4f mDof;
		Math::tVec3f mSaturation;
		Math::tVec3f mContrast;
		Math::tVec3f mExposure;
		Math::tVec4f mUnitTint;

		Gfx::tPostEffectsMaterial::tParametersPtr mFilmGrainOverride;

		Gfx::tPostEffectsMaterial::tParameters* fFilmGrainOverride( );
	};
	typedef tRefCounterPtr<tGamePostEffectsData> tGamePostEffectsDataPtr;

	class tGamePostEffectManager : public Gfx::tPostEffectManager
	{
		struct tPerViewportData
		{
			Gfx::tPostEffectsMaterialPtr mPauseCamDofMtl;
			Gfx::tPostEffectsMaterialPtr mPauseCamSaturationMtl;
			Gfx::tPostEffectsMaterialPtr mGameCamDofMtl;
			Gfx::tPostEffectsMaterialPtr mGameCamSaturationMtl;
			Gfx::tPostEffectsMaterialPtr mFilmGrainDofMtl;
			Gfx::tPostEffectsMaterialPtr mFilmGrainMaterial;
			Gfx::tPostEffectsMaterialPtr mDamageOverlayMaterial;

			f32 mFilmGrainFadeOutDuration;
			f32 mFilmGrainFadeOutTimer;

			f32 mFilmGrainPulseDuration;
			f32 mFilmGrainPulseTimer;
			f32 mFilmGrainPulseStrength;
			f32 mFilmGrainPulseBaseStrength;
			f32 mFilmGrainLastPulse;
			b32 mFilmGrainOnOverride;
			f32 mFilmGrainExposureMult;
			f32 mGlobalExposureMult;

			tPerViewportData( );
			void fOnTick( tGamePostEffectManager& mgr, f32 dt, b32 paused );
			b32 fFilmGrainActive( ) const;
			void fBeginFilmGrainFadeOut( f32 fadeOutTime );
			void fFilmGrainPulse( f32 zeroToOneStrength, f32 duration );
			void fClearFilmGrainPulse( );

			tGamePostEffectsData mDefaultGamecamData;
			tGamePostEffectsData mCurrentGameCamData;
			tGrowableArray<tGamePostEffectsData> mDataStack;

			u32 mIndex;
		};

		Gfx::tRenderToTexturePtr	mHalfSizeRt;
		Gfx::tRenderToTexturePtr	mQuarterSizeRt;
		Gfx::tRenderToTexturePtr	mQuarterSizeRtAlt;

		Gfx::tTextureReference		mDamageEffectTexture;
		Gfx::tTextureReference		mInGameOverlayTexture;
		tHashTable<tStringPtr, Gfx::tTextureReference> mFilmGrainTexture;

		tFixedArray<tPerViewportData,2> mPerViewportData;

		void fAddFilmGrainTexture( const tStringPtr& key, const tFilePathPtr& path );
		void fPushFilmGrainData( const Gfx::tPostEffectsMaterial::tParameters& params, u32 viewportIndex );
		void fResetFilmGranData( u32 viewportIndex );
		void fReconfigureFilmGrainEffect( u32 viewportIndex );

	public:
		const tGamePostEffectsData& fDefaultGameCamData( u32 viewportIndex ) const { return mPerViewportData[ viewportIndex ].mDefaultGamecamData; }
		tGamePostEffectsData& fDefaultGameCamData( u32 viewportIndex ) { return mPerViewportData[ viewportIndex ].mDefaultGamecamData; }
		tGamePostEffectsData& fCurrentGameCamData( u32 viewportIndex ) { return mPerViewportData[ viewportIndex ].mCurrentGameCamData; }
		const tGamePostEffectsData& fCurrentGameCamData( u32 viewportIndex ) const { return mPerViewportData[ viewportIndex ].mCurrentGameCamData; }

		void fPushEffectsData( const tGamePostEffectsData& data, u32 viewportIndex );
		void fPopEffectsData( u32 viewportIndex );
		void fResetEffectsData( ); //fail safe after level unload

		// These will switch between current gamecam values and the devmenu values.
		Math::tVec4f fCurrentGameCamDof( u32 viewportIndex ) const;
		Math::tVec3f fCurrentGameCamSaturation( u32 viewportIndex ) const;
		Math::tVec3f fCurrentGameCamContrast( u32 viewportIndex ) const;
		Math::tVec3f fCurrentGameCamExposure( u32 viewportIndex ) const;
		Math::tVec4f fCurrentGameCamUnitTint( u32 viewportIndex ) const;

		static const tStringPtr& fSeqNameGameCam( u32 ithVp );
		static const tStringPtr& fSeqNamePauseCam( u32 ithVp );
		static const tStringPtr& fSeqNameFilmGrain( u32 ithVp );

		enum tInGameFilmGrainMode
		{
			cInGameFilmGrainNone,
			cInGameFilmGrainNormal,
			cInGameFilmGrainColorized,
		};

		b32						mFrontEndMode;
		tInGameFilmGrainMode	mInGameFilmGrainMode;

	public:
		explicit tGamePostEffectManager( const tResourcePtr& postEffectsMtlFile );
		virtual ~tGamePostEffectManager( );
		
		void fOnTick( f32 dt, b32 paused );
		
		b32 fFilmGrainActive( u32 ithVp ) const;
		void fBeginFilmGrainFadeOut( u32 ithVp, f32 fadeOutTime );
		void fFilmGrainPulse( u32 ithVp, f32 zeroToOneStrength, f32 duration );
		void fClearFilmGrainPulse( u32 ithVp );

		void fSetGlobalExposureMult( u32 ithVp, f32 mult );
		void fSetFilmGrainExposureMult( u32 ithVp, f32 mult );

		virtual void fDestroyRenderTargets( );
		virtual void fCreateRenderTargets( Gfx::tScreen& screen );

		static void fExportScriptInterface( tScriptVm& vm );

	private:
		void fSetupRenderTargets( Gfx::tScreen& screen );
		void fSetupPauseCam( Gfx::tScreen& screen );
		void fSetupGameCam( Gfx::tScreen& screen );
		void fSetupFilmGrain( Gfx::tScreen& screen );
		void fSetupSoftFocus( Gfx::tScreen& screen );
	};

	typedef tRefCounterPtr< tGamePostEffectManager > tGamePostEffectManagerPtr;
}

#endif//__tGamePostEffectMgr__
