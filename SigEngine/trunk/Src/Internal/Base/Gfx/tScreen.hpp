#ifndef __tScreen__
#define __tScreen__
#include "tDevice.hpp"
#include "tSceneGraph.hpp"
#include "tPostEffect.hpp"
#include "tViewport.hpp"
#include "tMoviePlayer.hpp"
#include "tGeometryBufferVRam.hpp"
#include "tIndexBufferVRam.hpp"
#include "tRenderableEntity.hpp"
#include "tLightCombo.hpp"
#include "tLightEntity.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gui/tCanvas.hpp"

#if !defined( build_release ) && defined( platform_xbox360 )
#define sig_profile_xbox_gpu
#endif// !defined( build_release ) && defined( platform_xbox360 )

namespace Sig
{
	namespace Gui
	{
		class tCanvas;
	}//Gui
	namespace Gfx
	{
		class tScreen;
		class tLightEntityList;
		class tDeferredShadingMaterial;
		class tDeferredShadingSphere;
	}//Gfx
}//Sig

namespace Sig { namespace Gfx
{
	struct base_export tScreenCreationOptions
	{
		u64 mWindowHandle;
		b32 mFullScreen;
		u32 mBackBufferWidth;
		u32 mBackBufferHeight;
		tRenderTarget::tFormat mFormat;
		tVsync mVsync;
		u32 mMultiSamplePower;
		u32 mShadowMapResolution;
		u32 mShadowMapLayerCount;
		b32 mAutoDepthStencil;
		Math::tVec4f mDefaultClearColor;

		tResourceDepotPtr mResourceDepot;
		Gfx::tDefaultAllocatorSettings mAllocatorSettings;

		tScreenCreationOptions( );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tScreen );

	class tRenderToTextureAgent;
	typedef tRefCounterPtr< tRenderToTextureAgent > tRenderToTextureAgentPtr;

	class tCurrentShadowCastingLightBlender;
	typedef tRefCounterPtr< tCurrentShadowCastingLightBlender > tCurrentShadowCastingLightBlenderPtr;

	class base_export tScreenPlatformBase : public tUncopyable, public tRefCounter
	{
		friend class tViewport;
		friend class tRenderToTexture;
		friend class tRenderToTexturePlatformBase;
	public:

		typedef Sig::byte* tPlatformTextureHandle;
		typedef tGrowableArray< tViewportPtr > tViewportArray;

		struct tPerViewPortDisplayListData
		{
			tLightComboList			mLightCombo;
			tWorldSpaceDisplayList	mDepthList;
			tLightEntityList		mLightList;

			tPerViewPortDisplayListData( );
			~tPerViewPortDisplayListData( );
			void fReset( );
		};
		typedef tDynamicArray< tPerViewPortDisplayListData > tViewportDisplayLists;

	protected:
		tScreenCreationOptions			mCreateOpts;
		tDevicePtr						mDevice;
		tSceneGraphPtr					mSceneGraph;
		tRenderToTexturePtr				mSceneRenderToTexture;
		tRenderToTexturePtr				mScreenSpaceRenderToTexture;
		tRenderToTexturePtr				mDeferredRenderToTexture;
		tRenderToTexturePtr				mWorldDepthTexture;
		tFixedArray<tRenderToTexturePtr, 2> mShadowMaps;
		tRenderToTexturePtr				mCurrentTarget;
		tPostEffectManagerPtr			mPostEffectMgr;
		tPlatformTextureHandle			mWhiteTexture;
		tPlatformTextureHandle			mBlackTexture;
		tGeometryBufferVRam				mFullScreenQuadVB;
		tIndexBufferVRam				mFullScreenQuadIB;
		Math::tVec4f					mLastClearRGBA;
		tViewportArray					mViewports;
		tRenderableEntityList			mExplicitWorldObjects;
		tRenderableEntityList			mExplicitWorldTopObjects;
		tScreenSpaceDisplayList			mScreenSpaceDL;
		tCamera							mScreenSpaceCamera;
		tRenderState::tGlobalFillMode	mGlobalFillMode;
		tDisplayStats					mWorldStats, mScreenStats;
		Time::tStopWatch				mPreSwapTimer;
		tGrowableArray<tMoviePlayerPtr> mMoviePlayers; //Rendered for one frame and then cleared
		Gui::tCanvasFrame*				mRootCanvas;
		Gui::tCanvasPtr					mRootCanvasPtr;
		Gui::tCanvasFrame*				mWorldSpaceCanvas;
		Gui::tCanvasPtr					mWorldSpaceCanvasPtr;
		b32								mLimitShadowMapLayerCount;
		b32								mWorldRendering;
		b32								mBuildDisplayLists;
		b32								mEnableDeferredShading;
		tWorldSpaceDisplayList			mWorldSpaceDisplayList;

#ifdef sig_profile_xbox_gpu
		D3DPerfCounters*				mGPUPerfCounterStart[3];
		D3DPerfCounters*				mGPUPerfCounterEnd[3];
#endif//sig_profile_xbox_gpu

		tViewportDisplayLists			mViewportDisplayList;
		tLightEntityList				mShadowCastingLightList;
		tCurrentShadowCastingLightBlenderPtr mShadowLightBlender;

		// Deferred shading stuff
		tLightEntityList				mVisibleLightsList;
		tRefCounterPtr<tDeferredShadingMaterial>	mDeferredShadingMaterial;
		tRefCounterPtr<tDeferredShadingSphere>		mLightSphere;

		struct tCaptureData : public tRefCounter
		{
			typedef tDynamicArray< tCamera > tCameraFrame;
			typedef tGrowableArray< tCameraFrame > tCameraFrameArray;

			tFilePathPtr		mFolder;
			u32					mFrame;
			tCameraFrameArray	mCameraFrames;

			tCaptureData( ) : mFrame( 0 ) { }
		};

		typedef tRefCounterPtr< tCaptureData > tCaptureDataPtr;
		mutable tCaptureDataPtr mCaptureData;

	public:
		typedef tDelegate< void () > tEndOfFrameCallback;
	private:
		tGrowableArray< tEndOfFrameCallback > mEndOfFrameCallbacks;

	public:
		
		tGrowableArray< tRenderToTextureAgentPtr > mRttAgents;

	public:

		tScreenPlatformBase( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts );
		~tScreenPlatformBase( );

		static u32 fDefaultMultiSamplePower( );

		inline const tDevicePtr&	fGetDevice( ) const { return mDevice; }
		inline const tSceneGraphPtr& fSceneGraph( ) const { return mSceneGraph; }
		void						fSetSceneGraph( const tSceneGraphPtr& sg ) { mSceneGraph = sg; }
		const tRenderToTexturePtr&	fSceneRenderToTexture( ) const;
		const tRenderToTexturePtr&	fScreenSpaceRenderToTexture( ) const { return mScreenSpaceRenderToTexture; }
		const tRenderToTexturePtr&  fWorldDepthTexture( ) const { return mWorldDepthTexture; }
		const tRenderToTexturePtr&	fShadowMap( u32 index ) const { return mShadowMaps[ index ]; }
		const tRenderToTexturePtr&	fDeferredRenderToTexture( ) const { return mDeferredRenderToTexture; } // for debugging
		const tRenderToTexturePtr&  fPostProcessDepthTexture( ) const { return mWorldDepthTexture; } // N.B., this might be overridden in the platform-specific implementation
		void						fSetPostEffectMgr( const tPostEffectManagerPtr& pem ) { mPostEffectMgr = pem; }
		void						fSetRgbaClearColor( const Math::tVec4f& clearRGBA ) { mCreateOpts.mDefaultClearColor = clearRGBA; }
		
		inline u32					fLimitShadowMapLayerCount( ) const { return mLimitShadowMapLayerCount; }
		inline void					fSetLimitShadowMapLayerCount( u32 layerCount ) { mLimitShadowMapLayerCount = layerCount; }

		const tScreenCreationOptions& fCreateOpts( ) const { return mCreateOpts; }
		f32							fAspectRatio( ) const { return ( f32 )mCreateOpts.mBackBufferWidth / ( f32 )mCreateOpts.mBackBufferHeight; }

		tScreen&					fToScreen( );
		const tScreen&				fToScreen( ) const;

		void						fQueueMoviePlayer( const tMoviePlayerPtr& playsOneFrame ) { mMoviePlayers.fPushBack( playsOneFrame ); }

		tPlatformTextureHandle		fWhiteTexture( ) const { return mWhiteTexture; }
		tPlatformTextureHandle		fBlackTexture( ) const { return mBlackTexture; }
		const tGeometryBufferVRam&	fFullScreenQuadVB( ) const { return mFullScreenQuadVB; }
		const tIndexBufferVRam&		fFullScreenQuadIB( ) const { return mFullScreenQuadIB; }

		void						fSetViewportCount( u32 count );
		u32							fGetViewportCount( ) const { return mViewports.fCount( ); }
		void						fSetViewport( u32 ithVp, const tViewportPtr& vp );
		const tViewportPtr&			fViewport( u32 ithVp ) const { return mViewports[ ithVp ]; }

		void						fSetViewportClipBox( u32 x, u32 y, u32 width, u32 height );
		Math::tVec2u				fComputeGuiSafeEdge( ) const;
		Math::tRect					fComputeSafeRect( ) const;

		void						fDisableMSAA( ); // defined by tScreen (platform-specific)
		void						fReEnableMSAA( ); // defined by tScreen (platform-specific)

		void						fClearCurrentRenderTargets( b32 clearColor, b32 clearDepth, const Math::tVec4f& rgbaClear, u32 stencilClear, b32 gBufferClear = false ); // defined by tScreen (platform-specific)
		const Math::tVec4f&			fLastRgbaClearColor( ) const { return mLastClearRGBA; }

		tCamera&					fGetScreenSpaceCamera( ) { return mScreenSpaceCamera; }
		const tCamera&				fGetScreenSpaceCamera( ) const { return mScreenSpaceCamera; }

		void						fSetGlobalFillMode( tRenderState::tGlobalFillMode gfm ) { mGlobalFillMode = gfm; }
		tRenderState::tGlobalFillMode fGetGlobalFillMode( ) const { return mGlobalFillMode; }

		void						fSetBuildDisplayLists( b32 build ) { mBuildDisplayLists = build; }
		b32							fGetBuildDisplayLists( ) const { return mBuildDisplayLists; }

		void						fSetEnableDeferredShading( b32 enable ) { mEnableDeferredShading = enable; }
		b32							fGetEnableDeferredShading( ) const { return mEnableDeferredShading; }

		const tDisplayStats&		fGetWorldStats( ) const { return mWorldStats; }
		const tDisplayStats&		fGetScreenStats( ) const { return mScreenStats; }
		const Time::tStopWatch&		fPreSwapTimer( ) const { return mPreSwapTimer; }
		void						fResetPreSwapTimer( );

		// The very root, used for everything
		Gui::tCanvasFrame&			fRootCanvas( ) { return *mRootCanvas; }
		const Gui::tCanvasFrame&	fRootCanvas( ) const { return *mRootCanvas; }

		// World space is the root for lense flares and other system level screen space stuff. The ui should draw on top of this.
		Gui::tCanvasFrame&			fWorldSpaceCanvas( ) { return *mWorldSpaceCanvas; }
		const Gui::tCanvasFrame&	fWorldSpaceCanvas( ) const { return *mWorldSpaceCanvas; }

		void						fAddScreenSpaceDrawCall( const tDrawCall& instance );
		void						fAddWorldSpaceDrawCall( const tRenderableEntityPtr& instance );
		void						fAddWorldSpaceTopDrawCall( const tRenderableEntityPtr& instance );

		void						fShutdown( );

		b32							fCapturing( ) const { return !mCaptureData.fNull( ); }

		b32							fDeviceCheck( );
		void						fCleanST( );
		void						fRender( );

		void						fEnqueueEndOfFrame( const tEndOfFrameCallback& callback ) { mEndOfFrameCallbacks.fPushBack( callback ); }
		void						fExecuteEndOfFrame( );

#ifdef sig_profile_xbox_gpu
		f32							fGetLastGpuTimeMs( ) const;
#endif//sig_profile_xbox_gpu

	protected:
		void fInitailize( );
		void fHandleMultiSampleChange( );
		b32  fBeginAllRendering( ); // defined by tScreen (platform-specific); returns false if rendering for this frame should not continue
		void fEndAllRendering( ); // defined by tScreen (platform-specific)
		void fBuildLightLists( );
		void fBuildDisplayLists( );
		void fBuildCanvasList( );

		void fRenderShadowMaps( );
		void fRenderWorldDepth( );
		void fRenderWorld( );
		void fRenderViewport( u32 vpIndex );
		void fRenderRttAgents( );
		void fRenderPostEffects( );
		void fRenderScreenSpace( );

		void fBeginGpuProfiling( );
		void fEndGpuProfiling( );

		// capture related...
		void fBeginCaptureDump( const tFilePathPtr& folder, u32 month, u32 day, u32 year, u32 hour, u32 minute, u32 second );
		void fEndCaptureDump( );
		b32 fCheckForAutoCaptureTermination( ) const;
		tFilePathPtr fCreateCaptureDumpPath( const tFilePathPtr& directory, const char* baseName, const char* extension, u32 frameNum ) const;
		void fCaptureCurrentCameras( ) const;

	public: // but just bearly
		void fGetWorldDepthSetup( f32& minZ, f32& maxZ );
		void fRenderWorldOpaqueBegin( );
		void fRenderWorldOpaqueEnd( );
		b32	 fDeferredShading( ) const;
		void fSetupDeferredShading( );
		void fRenderRttAgent( const tRenderToTextureAgent& rtt, b32 immediate );

	private:
		void fDebugOverrideViewportEffectSequences( );
		void fCreateFullScreenQuad( );
		void fDeferredLightPass( const tViewport& vp, tTextureReference& blankShadowMap );
		void fHandleScreenSpaceDummyBatches( const tDrawCall& dc );
	};

}}


#if defined( platform_pcdx9 )
#	include "tScreen_pcdx9.hpp"
#elif defined( platform_xbox360 )
#	include "tScreen_xbox360.hpp"
#elif defined( platform_ios )
#	include "tScreen_ios.hpp"
#else
#	error Invalid platform for tScreen defined!
#endif

#endif//__tScreen__
