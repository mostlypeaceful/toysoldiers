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

namespace Sig { namespace Gui
{
	class tCanvas;
}}

namespace Sig { namespace Gfx
{
	struct base_export tScreenCreationOptions
	{
		u64 mWindowHandle;
		b32 mFullScreen;
		u32 mBackBufferWidth;
		u32 mBackBufferHeight;
		u32 mVsync; // zero is no vsync, 1 is 60 hz, 2 is 30 hz
		u32 mMultiSamplePower;
		u32 mShadowMapResolution;
		u32 mShadowMapLayerCount;

		tScreenCreationOptions( );
	};

	class tScreen;
	class tScreenWorkerThread;
	class tLightEntityList;

	define_smart_ptr( base_export, tRefCounterPtr, tScreen );

	class base_export tScreenPlatformBase : public tUncopyable, public tRefCounter
	{
		friend class tViewport;
		friend class tRenderToTexture;
		friend class tRenderToTexturePlatformBase;
	public:

		typedef Sig::byte* tPlatformTextureHandle;
		typedef tGrowableArray< tViewportPtr > tViewportArray;

	protected:
		tScreenCreationOptions			mCreateOpts;
		tDevicePtr						mDevice;
		tSceneGraphPtr					mSceneGraph;
		tScreenWorkerThread*			mWorkerThread;
		tRenderToTexturePtr				mSceneRenderToTexture;
		tRenderToTexturePtr				mScreenSpaceRenderToTexture;
		tRenderToTexturePtr				mWorldDepthTexture;
		tRenderToTexturePtr				mShadowMap0;
		tRenderToTexturePtr				mCurrentTarget;
		tPostEffectManagerPtr			mPostEffectMgr;
		tPlatformTextureHandle			mWhiteTexture;
		tPlatformTextureHandle			mBlackTexture;
		tGeometryBufferVRam				mFullScreenQuadVB;
		tIndexBufferVRam				mFullScreenQuadIB;
		Math::tVec4f					mRgbaClear;
		tViewportArray					mViewports;
		tRenderableEntityList			mExplicitWorldObjects;
		tRenderableEntityList			mExplicitWorldTopObjects;
		tScreenSpaceDisplayList			mScreenSpaceDL;
		tCamera							mScreenSpaceCamera;
		tRenderState::tGlobalFillMode	mGlobalFillMode;
		tDisplayStats					mWorldStats, mScreenStats;
		Time::tStopWatch				mPreSwapTimer;
		tGrowableArray<tMoviePlayerPtr> mMoviePlayers; //Rendered for one frame and then cleared
		b32								mLimitShadowMapLayerCount;

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

		tScreenPlatformBase( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts );
		~tScreenPlatformBase( );

		static u32 fDefaultMultiSamplePower( );

		void                        fSetupScreenSpaceCamera( );

		inline const tDevicePtr&	fGetDevice( ) const { return mDevice; }
		inline const tSceneGraphPtr& fSceneGraph( ) const { return mSceneGraph; }
		void						fSetSceneGraph( const tSceneGraphPtr& sg ) { mSceneGraph = sg; }
		const tRenderToTexturePtr&	fSceneRenderToTexture( ) const { return mSceneRenderToTexture; }
		const tRenderToTexturePtr&	fScreenSpaceRenderToTexture( ) const { return mScreenSpaceRenderToTexture; }
		const tRenderToTexturePtr&  fWorldDepthTexture( ) const { return mWorldDepthTexture; }
		const tRenderToTexturePtr&	fShadowMap0( ) const { return mShadowMap0; }
		const tRenderToTexturePtr&  fPostProcessDepthTexture( ) const { return mWorldDepthTexture; } // N.B., this might be overridden in the platform-specific implementation
		void						fSetPostEffectMgr( const tPostEffectManagerPtr& pem ) { mPostEffectMgr = pem; }
		void						fSetRgbaClearColor( f32 r, f32 g, f32 b, f32 a = 1.f ) { mRgbaClear.x = r; mRgbaClear.y = g; mRgbaClear.z = b; mRgbaClear.w = a; }
		const Math::tVec4f&			fRgbaClearColor( ) const { return mRgbaClear; }
		inline u32					fLimitShadowMapLayerCount( ) const { return mLimitShadowMapLayerCount; }
		inline void					fSetLimitShadowMapLayerCount( u32 layerCount ) { mLimitShadowMapLayerCount = layerCount; }

		const tScreenCreationOptions& fCreateOpts( ) const { return mCreateOpts; }
		f32							fAspectRatio( ) const 
		{
			return ( f32 )mCreateOpts.mBackBufferWidth / ( f32 )mCreateOpts.mBackBufferHeight;
		}

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

		void						fSetViewportClipBox( u32 x, u32 y, u32 width, u32 height, f32 minZ = 0.f, f32 maxZ = 1.f ); // defined by tScreen (platform-specific)
		Math::tVec2u				fComputeGuiSafeEdge( ) const;
		Math::tRect					fComputeSafeRect( ) const;

		void						fDisableMSAA( ); // defined by tScreen (platform-specific)
		void						fReEnableMSAA( ); // defined by tScreen (platform-specific)

		void						fClearCurrentRenderTargets( b32 clearColor, b32 clearDepth, const Math::tVec4f& rgbaClear, f32 zClear, u32 stencilClear ); // defined by tScreen (platform-specific)

		tCamera&					fGetScreenSpaceCamera( ) { return mScreenSpaceCamera; }
		const tCamera&				fGetScreenSpaceCamera( ) const { return mScreenSpaceCamera; }

		void						fSetGlobalFillMode( tRenderState::tGlobalFillMode gfm ) { mGlobalFillMode = gfm; }
		tRenderState::tGlobalFillMode fGetGlobalFillMode( ) const { return mGlobalFillMode; }

		const tDisplayStats&		fGetWorldStats( ) const { return mWorldStats; }
		const tDisplayStats&		fGetScreenStats( ) const { return mScreenStats; }
		const Time::tStopWatch&		fPreSwapTimer( ) const { return mPreSwapTimer; }
		void						fResetPreSwapTimer( );

		void						fAddScreenSpaceDrawCall( const tDrawCall& instance );
		void						fAddWorldSpaceDrawCall( const tRenderableEntityPtr& instance );
		void						fAddWorldSpaceTopDrawCall( const tRenderableEntityPtr& instance );

		void						fShutdown( );

		b32							fCapturing( ) const { return !mCaptureData.fNull( ); }

		void						fRender( Gui::tCanvas* canvasRoot = 0 );

		Math::tRect fCalcViewportClipBox();

	protected:

		void fHandleMultiSampleChange( );
		b32  fBeginAllRendering( ); // defined by tScreen (platform-specific); returns false if rendering for this frame should not continue
		void fEndAllRendering( ); // defined by tScreen (platform-specific)
		void fBuildLightLists( 
			tDynamicArray<tLightEntityList>&		viewportLightLists, 
			tLightEntityList&						shadowCastingLightList );
		void fRenderShadowMaps( tLightEntityList& shadowCastingLightList );
		void fRenderWorldDepth( tDynamicArray<tWorldSpaceDisplayList>& viewportDepthDisplayLists );
		void fRenderWorld( tDynamicArray<tLightComboList>& viewportLightCombos );
		void fRenderPostEffects( );
		void fRenderScreenSpace( );

		// capture related...
		void fBeginCaptureDump( const tFilePathPtr& folder, u32 month, u32 day, u32 year, u32 hour, u32 minute, u32 second );
		void fEndCaptureDump( );
		b32 fCheckForAutoCaptureTermination( ) const;
		tFilePathPtr fCreateCaptureDumpPath( const tFilePathPtr& directory, const char* baseName, const char* extension, u32 frameNum ) const;
		void fCaptureCurrentCameras( ) const;

	private:

		void fCreateFullScreenQuad( );
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
