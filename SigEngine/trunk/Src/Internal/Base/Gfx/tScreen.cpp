#include "BasePch.hpp"
#include "tApplication.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "tLightCombo.hpp"
#include "tRenderContext.hpp"
#include "tRenderableEntity.hpp"
#include "tPostEffectsMaterial.hpp"
#include "tDeferredShadingMaterial.hpp"
#include "tDefaultAllocators.hpp"
#include "Fui.hpp"
#include "tRenderToTextureAgent.hpp"
#include "tPotentialVisibilitySet.hpp"
#include "Gui\tFuiCanvas.hpp"

#ifdef sig_devmenu
#include "tUser.hpp"
#include "tGameAppBase.hpp"
#endif//sig_devmenu

namespace Sig { namespace Gfx
{

	devvar_clamp(u32, Renderer_FillMode, 0, 0, 2, 0 );
	devvar( bool, Debug_SceneGraph_BVH_Render, false );
	devvar( bool, Debug_SceneGraph_BVH_RenderObjectsOnly, true );
	devvar_clamp( s32, Debug_SceneGraph_BVH_RenderLevel, -1, -1, 16, 0 );

	devvar( bool, Debug_SceneGraph_BVH_RenderKDTrees, false );
	devvar_clamp( s32, Debug_SceneGraph_BVH_RenderKDTreeLevel, -1, -1, 16, 0 );

	namespace
	{
		static const u32 cDefaultMultiSamplePower = 2;
	}

	devvar_clamp( u32, Renderer_Settings_MultiSample, cDefaultMultiSamplePower, 0, 2, 0 );

	devvar( bool, Renderer_Capture_AutoDevMenuOnOff, true );

	devvar( bool, Renderer_Deferred, true );
	devvar( bool, Renderer_Debug_ShowCutout, false );
	devvar( bool, Renderer_Debug_OnlyDirectional, false );
	devvar( bool, Renderer_Debug_3DFuiCutoutFix, true );

	devvar_clamp( f32, Renderer_Shadows_PointLightFadeSpeed, 8.1f, 0.0001f, 100000.f, 2 );

#ifdef sig_devmenu
	void fPerformCaptureDevCallback( tDevCallback::tArgs& args )
	{
		sigassert( args.mUser );
		const tScreenPtr& screen = args.mUser->fScreen( );
		std::stringstream ss;
		if( screen->fCapturing( ) )
		{
			screen->fEndCaptureDump( );
			ss << "start";
		}
		else
		{
			screen->fBeginCaptureDump( tFilePathPtr( "~captures" ) );
			ss << "stop";
		}
		args.mValueText = ss.str( );

		if( screen->fCapturing( ) && Renderer_Capture_AutoDevMenuOnOff )
		{
			if( args.mUser->fDevMenu( ).fIsActive( ) )
			{
				args.mUser->fDevMenu( ).fDeactivate( *args.mUser );
			}
		}
	}

	devcb( Renderer_Capture_PerformCapture, "start", make_delegate_cfn( tDevCallback::tFunction, fPerformCaptureDevCallback ) );

	namespace
	{
		enum tGBufferDebugDisplayMode
		{
			cGBufferDebugDisplayNone,
			cGBufferDebugDisplay0,
			cGBufferDebugDisplay1,
			cGBufferDebugDisplay2,
			cGBufferDebugDisplay3,
			cGBufferDebugDisplaySpecPower,
			cGBufferDebugDisplaySpecValue,
			cGBufferDebugDisplayShadowMap0Layer0,
			cGBufferDebugDisplayShadowMap0Layer1,
			cGBufferDebugDisplayShadowMap1Layer0,
			cGBufferDebugDisplayShadowMap1Layer1,
			// TODO: Merge SpecPower + SpecValue into a single preview mode without extra buffers
			// TODO: Add depth support

			cGBufferDebugDisplayCount,
		};

		std::map< std::string, tGBufferDebugDisplayMode >& fCreateDebugDisplayLookupTable( )
		{
			static std::map< std::string, tGBufferDebugDisplayMode > table;

			table[ "" ]						= cGBufferDebugDisplayNone;
			table[ "0" ]					= cGBufferDebugDisplay0;
			table[ "1" ]					= cGBufferDebugDisplay1;
			table[ "2" ]					= cGBufferDebugDisplay2;
			table[ "3" ]					= cGBufferDebugDisplay3;

			table[ "None" ]					= cGBufferDebugDisplayNone;
			table[ "Diffuse" ]				= cGBufferDebugDisplay0;
			table[ "Normal" ]				= cGBufferDebugDisplay1;
			table[ "Emissive" ]				= cGBufferDebugDisplay2;
			table[ "Ambient" ]				= cGBufferDebugDisplay3;
			table[ "SpecPower" ]			= cGBufferDebugDisplaySpecPower;
			table[ "SpecValue" ]			= cGBufferDebugDisplaySpecValue;

			table[ "ShadowMap0_Layer0" ]	= cGBufferDebugDisplayShadowMap0Layer0;
			table[ "ShadowMap0_Layer1" ]	= cGBufferDebugDisplayShadowMap0Layer1;
			table[ "ShadowMap1_Layer0" ]	= cGBufferDebugDisplayShadowMap1Layer0;
			table[ "ShadowMap1_Layer1" ]	= cGBufferDebugDisplayShadowMap1Layer1;

			return table;
		}

		s32 gGBufferDebugDisplayMode = cGBufferDebugDisplayNone;

		void fChangeGBufferDebugDisplayMode( tDevCallback::tArgs& args )
		{
			if( args.mEvent == tDevCallback::cEventTypeSetValue )
			{
				static std::map< std::string, tGBufferDebugDisplayMode > displayModes = fCreateDebugDisplayLookupTable( );
				gGBufferDebugDisplayMode = displayModes[ args.mValueText ];
			}
			else
			{
				gGBufferDebugDisplayMode = Math::fModulus( gGBufferDebugDisplayMode + args.fGetIncrementValue( ), (s32)cGBufferDebugDisplayCount );
			}

			switch( gGBufferDebugDisplayMode )
			{
			case cGBufferDebugDisplayNone:				args.mValueText = "None"; break;
			case cGBufferDebugDisplay0:					args.mValueText = "Buffer 0 (Diffuse RGB)"; break;
			case cGBufferDebugDisplay1:					args.mValueText = "Buffer 1 (Normal XYZ)"; break;
			case cGBufferDebugDisplay2:					args.mValueText = "Buffer 2 (Emissive RGB)"; break;
			case cGBufferDebugDisplay3:					args.mValueText = "Buffer 3 (Ambient RGB)"; break;
			case cGBufferDebugDisplaySpecPower:			args.mValueText = "SpecPower (Buffer0.A)"; break;
			case cGBufferDebugDisplaySpecValue:			args.mValueText = "SpecValue (Buffer1.A)"; break;
			case cGBufferDebugDisplayShadowMap0Layer0:	args.mValueText = "ShadowMap 0 Layer 0 (R Z=0.25)"; break;
			case cGBufferDebugDisplayShadowMap0Layer1:	args.mValueText = "ShadowMap 0 Layer 1 (R Z=0.75)"; break;
			case cGBufferDebugDisplayShadowMap1Layer0:	args.mValueText = "ShadowMap 1 Layer 0 (R Z=0.25)"; break;
			case cGBufferDebugDisplayShadowMap1Layer1:	args.mValueText = "ShadowMap 1 Layer 1 (R Z=0.75)"; break;
			default:									args.mValueText = "None"; break;
			}
		}

		devcb( Renderer_Debug_GBufferDebugDisplay, "None", make_delegate_cfn( tDevCallback::tFunction, fChangeGBufferDebugDisplayMode ) );
	}
#endif // defined( sig_devmenu )

	namespace
	{
		struct tRenderableGatherer
		{
			tGrowableArray<tRenderableEntity*> mRenderables;
			void fGatherRecursive( const tEntityPtr& e )
			{
				if( tRenderableEntity* re = e->fDynamicCast<tRenderableEntity>( ) )
					mRenderables.fPushBack( re );
				for(u32 i = 0; i < e->fChildCount( ); ++i)
					fGatherRecursive( e->fChild( i ) );
			}
		};
	}

	/*
		This class manages which point light is currently casting shadows.
		Instead of hard popping, it will:
		 * hold onto the last active light
		 * fade shadows out when it's no longer needed
		 * fade in new light shadows.
	*/
	class tCurrentShadowCastingLightBlender : public tRefCounter
	{
	public:
		tCurrentShadowCastingLightBlender( ) 
			: mFadeValue( 0.f )
			, mTransitioning( false )
		{ }
		
		void fCleanST( )
		{
			if( mNextLight && !mNextLight->fSceneGraph( ) )
			{
				mNextLight.fRelease( );
				mTransitioning = false;
			}

			if( mCurrentLight && !mCurrentLight->fSceneGraph( ) )
			{
				mCurrentLight = mNextLight;
				mTransitioning = false;
			}
		}

		void fFadeToLight( tLightEntity* light )
		{
			if( light != mCurrentLight && light != mNextLight )
			{
				if( mCurrentLight )
				{
					mTransitioning = true;
					if( mNextLight != light )
					{
						mNextLight.fReset( light );
						if( mNextLight )
							mNextLight->fSetShadowFade( 0.f );
					}
				}
				else
				{
					// we had no light so just start fading in.
					mCurrentLight.fReset( light );
					mNextLight.fRelease( );

					mTransitioning = false;
					mFadeValue = 0.f;
				}
			}
		}

		void fStep( f32 dt )
		{
			if( mTransitioning )
			{
				b32 finished = fLerpValueTo( mFadeValue, 0.0f, dt );

				sigassert( mCurrentLight );
				mCurrentLight->fSetShadowFade( mFadeValue );

				if( finished )
				{
					mCurrentLight = mNextLight;
					mTransitioning = false;
				}
			}
			else
			{
				if( mCurrentLight )
				{
					fLerpValueTo( mFadeValue, 1.0f, dt );
					mCurrentLight->fSetShadowFade( mFadeValue );
				}
			}
		}

		tLightEntity* fCurrentLight( ) const
		{
			if( mCurrentLight && mCurrentLight->fSceneGraph( ) )
				return mCurrentLight.fGetRawPtr( );

			return NULL;
		}

	private:
		tRefCounterPtr<tLightEntity> mCurrentLight;
		tRefCounterPtr<tLightEntity> mNextLight;
		f32 mFadeValue;
		b32 mTransitioning;

		// returns true when lerp is finished.
		b32 fLerpValueTo( f32& current, f32 target, f32 dt )
		{
			const f32 maxChange = Renderer_Shadows_PointLightFadeSpeed * dt;
			f32 delta = fClamp( target - current, -maxChange, maxChange );
			current += delta;
			return fEqual( delta, 0.f );
		}
	};

}}

namespace Sig { namespace Gfx
{
	///
	/// \brief Collects all lights intersecting the view volume.
	struct base_export tBuildLightListSceneGraphCallback : public tEntityBVH::tIntersectVolumeCallback<Math::tFrustumf>
	{
		mutable tLightEntityList& mLightList;

		tBuildLightListSceneGraphCallback( tLightEntityList& lightList ) : mLightList( lightList ) { }

		inline void operator()( const Math::tFrustumf& v, tEntityBVH::tObjectPtr i, b32 aabbWhollyContained ) const
		{
			tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i->fOwner( ) );

			sigassert( spatial->fDynamicCast< tLightEntity >( ) );

			tLightEntity* light = static_cast< tLightEntity* >( spatial );
			if( !light->fVisible( ) )
				return;
			if( Renderer_Debug_OnlyDirectional && light->fLightDesc( ).fLightType( ) != tLight::cLightTypeDirection )
				return;
			//if( !fQuickAabbTest( v, i, aabbWhollyContained ) )
			//	return;
			mLightList.fPushBack( light );
		}
	};


	///
	/// \brief Sort the lights by priority
	struct base_export tSortLightList
	{
		inline f32 fPriority( const tLightEntity* l ) const
		{
			// give directional and shadow casting lights highest priority -
			// for point lights, use average of inner/outer radius (i.e., bigger point lights have higher priority)

			f32 o = 0;
			if( l->fCastsShadow( ) )
				o += 1000.f;
			if( l->fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection )
				o += 2000.f; // Forward rendering shaders need the directional light to be first, and directional lights no longer always have shadows.
			else
				o += 0.5f * ( l->fLightDesc( ).fRadii( ).x + l->fLightDesc( ).fRadii( ).y );
			return o;
		}

		inline b32 operator()( const tLightEntity* a, const tLightEntity* b ) const
		{
			const f32 priorityA = fPriority( a );
			const f32 priorityB = fPriority( b );
			return priorityA > priorityB;
		}
	};

}}

namespace Sig { namespace Gfx
{
	tScreenCreationOptions::tScreenCreationOptions( )
		: mWindowHandle( 0 )
		, mFullScreen( false )
		, mBackBufferWidth( 0 )
		, mBackBufferHeight( 0 )
		, mFormat( tRenderTarget::cFormatNull )
		, mVsync( VSYNC_60HZ )
		, mMultiSamplePower( 0 )
		, mShadowMapResolution( 256 )
		, mShadowMapLayerCount( 1 )
		, mAutoDepthStencil( false )
		, mDefaultClearColor( 0 )
	{
#if defined( platform_xbox360 )
		mShadowMapResolution = 1024;
#elif defined( platform_pcdx9 )
		mShadowMapResolution = 2048;
#endif
	}
}}

namespace Sig { namespace Gfx
{
	
	u32 tScreenPlatformBase::fDefaultMultiSamplePower( )
	{
#ifdef target_tools
		return 0; // not supported due to multi screen devices.
#else
		return Renderer_Settings_MultiSample;
#endif
	}

	tScreenPlatformBase::tScreenPlatformBase( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts )
		: mCreateOpts( createOpts )
		, mDevice( device )
		, mSceneGraph( sceneGraph )
		, mWhiteTexture( 0 )
		, mBlackTexture( 0 )
		, mLastClearRGBA( 0.f )
		, mGlobalFillMode( tRenderState::cGlobalFillDefault )
		, mPreSwapTimer( false )
		, mLimitShadowMapLayerCount( ~0 )
		, mWorldRendering( false )
		, mBuildDisplayLists( true )
		, mEnableDeferredShading( true )
		, mShadowLightBlender( NEW tCurrentShadowCastingLightBlender( ) )
		, mRootCanvas( NEW Gui::tCanvasFrame( ) )
		, mWorldSpaceCanvas( NEW Gui::tCanvasFrame( ) )
	{
		// cleanup canvas on destruction
		mRootCanvasPtr = Gui::tCanvasPtr( mRootCanvas );
		mWorldSpaceCanvasPtr = Gui::tCanvasPtr( mWorldSpaceCanvas );
		mRootCanvas->fAddChild( mWorldSpaceCanvasPtr );
	}

	tScreenPlatformBase::~tScreenPlatformBase( )
	{
		fShutdown( );
	}

	tScreen& tScreenPlatformBase::fToScreen( )
	{
		return static_cast<tScreen&>(*this);
	}

	const tScreen& tScreenPlatformBase::fToScreen( ) const
	{
		return static_cast<const tScreen&>(*this);
	}

	Math::tRect tScreenPlatformBase::fComputeSafeRect( ) const
	{
		const Math::tVec2u safeEdge = fComputeGuiSafeEdge( );
		const f32 l = 0.f + safeEdge.x;
		const f32 t = 0.f + safeEdge.y;
		const f32 r = ( f32 )mCreateOpts.mBackBufferWidth - safeEdge.x;
		const f32 b = ( f32 )mCreateOpts.mBackBufferHeight - safeEdge.y;
		return Math::tRect( t, l, b, r );
	}

	void tScreenPlatformBase::fSetViewportCount( u32 count )
	{
		// store the old count
		const u32 oldCount = mViewports.fCount( );

		// resize array
		mViewports.fSetCount( count );

		// now create and set the screen pointer on any new slots
		for( u32 i = oldCount; i < mViewports.fCount( ); ++i )
			mViewports[ i ] = tViewportPtr( NEW tViewport( i, fToScreen( ) ) );

		for( u32 i = 0; i < mViewportDisplayList.fCount( ); ++i )
			mViewportDisplayList[ i ].fReset( );

		mViewportDisplayList.fResize( count );
	}

	void tScreenPlatformBase::fSetViewport( u32 ithVp, const tViewportPtr& vp )
	{
		sigassert( !vp.fNull( ) && "It is invalid to assign a null tViewportPtr to a viewport slot in tScreenPlatformBase" );

		// assign new viewport pointer
		mViewports[ ithVp ] = vp;
	}

	void tScreenPlatformBase::fAddScreenSpaceDrawCall( const tDrawCall& instance )
	{
		mScreenSpaceDL.fInsert( instance );
	}

	void tScreenPlatformBase::fAddWorldSpaceDrawCall( const tRenderableEntityPtr& instance )
	{
		mExplicitWorldObjects.fPushBack( instance );
	}

	void tScreenPlatformBase::fAddWorldSpaceTopDrawCall( const tRenderableEntityPtr& instance )
	{
		mExplicitWorldTopObjects.fPushBack( instance );
	}

	void tScreenPlatformBase::fInitailize( )
	{
		sigassert( mCreateOpts.mResourceDepot );

		fCreateFullScreenQuad( );
		Gfx::tDefaultAllocators::fInstance( ).fCreateAllocators( mDevice, mCreateOpts.mAllocatorSettings );
		Gfx::tDefaultAllocators::fInstance( ).fLoadMaterials( mCreateOpts.mResourceDepot, this );
		fSetupDeferredShading( );
	}

	void tScreenPlatformBase::fShutdown( )
	{
		fExecuteEndOfFrame( );

#ifndef target_tools
		// This typically just crashes. so just dont. :/
		Gfx::tDefaultAllocators::fInstance( ).fUnloadMaterials( this );
#endif

		fToScreen( ).fReleaseSwapChainResources( );
		mViewports.fDeleteArray( );
		mPostEffectMgr.fRelease( );
		mSceneGraph.fRelease( );
		mDevice.fRelease( );
	}

	void tScreenPlatformBase::fResetPreSwapTimer( )
	{
		mPreSwapTimer.fResetElapsedS( 0.f );
		mPreSwapTimer.fStart( );
	}
	
	b32 tScreenPlatformBase::fDeferredShading( ) const
	{
		return mEnableDeferredShading && Renderer_Deferred;
	}


	tScreenPlatformBase::tPerViewPortDisplayListData::tPerViewPortDisplayListData( )
	{ }

	tScreenPlatformBase::tPerViewPortDisplayListData::~tPerViewPortDisplayListData( )
	{ }

	void tScreenPlatformBase::tPerViewPortDisplayListData::fReset( )
	{
		mLightCombo.fInvalidate( );
		mDepthList.fInvalidate( );
		mLightList.fSetCount( 0 );
	}

	void tScreenPlatformBase::fCleanST( )
	{
		mShadowLightBlender->fCleanST( );

		if( mViewports.fCount( ) )
			mSceneGraph->fPotentialVisibilitySet( )->fUpdateForCamera( mViewports[ 0 ]->fRenderCamera( ) );
	}

	void tScreenPlatformBase::fRender( )
	{
		profile_pix( "tScreenPlatformBase::fRender" );
		//fResetPreSwapTimer( );

		// begin scene
		if( !fBeginAllRendering( ) )
			return; // not able to render right now, come back later

		//render
		fBeginGpuProfiling( );
		{
			fRenderRttAgents( );

			mSceneGraph->fPrepareEntityClouds( mViewports.fFront( )->fRenderCamera( ) );
			for( u32 i = 0; i < mViewportDisplayList.fCount( ); ++i )
				mViewportDisplayList[ i ].fReset( );

			fBuildLightLists( );
			fBuildDisplayLists( );
			fBuildCanvasList( );

				mWorldRendering = true;
				fRenderShadowMaps( );
				fRenderWorldDepth( );
				fRenderWorld( );
				mWorldRendering = false;

			fRenderPostEffects( );
			fRenderScreenSpace( );
		}
		fEndGpuProfiling( );

		// end scene, present
		{
			profile_pix( "fEndAllRendering" );
			fEndAllRendering( );
			fExecuteEndOfFrame( );
		}
	}

	void tScreenPlatformBase::fExecuteEndOfFrame( )
	{
		const u32 warnEOFCs = 300;
		if( mEndOfFrameCallbacks.fCount( ) > warnEOFCs )
			log_warning( "Large number (" << mEndOfFrameCallbacks.fCount() << " > " << warnEOFCs << ") of end of frame callbacks in tScreenPlatformBase::fExecuteEndOfFrame !" );

		for( u32 i = 0; i < mEndOfFrameCallbacks.fCount( ); ++i )
			mEndOfFrameCallbacks[ i ]( );
		mEndOfFrameCallbacks.fClear( );
	}

	void tScreenPlatformBase::fBeginGpuProfiling( )
	{
#ifdef sig_profile_xbox_gpu
		{
			IDirect3DDevice9* device = mDevice->fGetDevice( );
			const u32 frameNum = tApplication::fInstance( ).fGetTickCount( );
			device->QueryPerfCounters( mGPUPerfCounterStart[ frameNum % 3 ], D3DPERFQUERY_WAITGPUIDLE );
		}
#endif//sig_profile_xbox_gpu
	}

	void tScreenPlatformBase::fEndGpuProfiling( )
	{
#ifdef sig_profile_xbox_gpu
		{
			IDirect3DDevice9* device = mDevice->fGetDevice( );
			const u32 frameNum = tApplication::fInstance( ).fGetTickCount( );
			device->QueryPerfCounters( mGPUPerfCounterEnd[ frameNum % 3 ], D3DPERFQUERY_WAITGPUIDLE );
		}
#endif//sig_profile_xbox_gpu
	}

#ifdef sig_profile_xbox_gpu
	f32 tScreenPlatformBase::fGetLastGpuTimeMs( ) const
	{
		//NOTE: I used the "Instancing" example in the Microsoft Xbox 360 XDK  to create most of this code
		const u32 frameNum = ( tApplication::fInstance( ).fGetTickCount( ) + 1 ) % 3; //get the next availible frame in our buffered frames
		D3DPERFCOUNTER_VALUES start, end;
		mGPUPerfCounterStart[ frameNum ]->GetValues( &start, 0, 0 );
		mGPUPerfCounterEnd[ frameNum ]->GetValues( &end, 0, 0 );

		//subtract
		u64 diff = end.RBBM[ 0 ].QuadPart - start.RBBM[ 0 ].QuadPart;
		return (f32)diff / 500000.0f;
	}
#endif//sig_profile_xbox_gpu

	void tScreenPlatformBase::fHandleMultiSampleChange( )
	{
#ifdef sig_devmenu
		if( mCreateOpts.mFullScreen )
			return; // TODO currently we don't support changing multi sample mode while in full screen
		if( fDefaultMultiSamplePower( ) != fCreateOpts( ).mMultiSamplePower )
		{
			mCreateOpts.mMultiSamplePower = fDefaultMultiSamplePower( );
			fToScreen( ).fResize( mCreateOpts.mBackBufferWidth, mCreateOpts.mBackBufferHeight );
		}
#endif//sig_devmenu
	}

	const tRenderToTexturePtr& tScreenPlatformBase::fSceneRenderToTexture( ) const 
	{ 
		return mSceneRenderToTexture; 
	}

	void tScreenPlatformBase::fBuildLightLists( )
	{
		profile_pix( "fBuildLightLists" );
		profile( cProfilePerfBuildLightLists );

		// just a list of pointers
		mShadowCastingLightList.fSetCount( 0 );

		// this is the list of all lights from all viewports, but with no duplicates
		mVisibleLightsList.fSetCount( 0 );

		// go through each viewport and gets its light list
		if( mBuildDisplayLists )
		{
			for( u32 ivp = 0; ivp < mViewports.fCount( ); ++ivp )
			{
				tViewport& vp = *mViewports[ ivp ];
				if( vp.fIsVirtual( ) || vp.fIsIgnored( ) )
					continue;

				tLightEntityList& vpLightList = mViewportDisplayList[ ivp ].mLightList;

				// get list of all lights that are on and intersecting view volume
				mSceneGraph->fIntersect( vp.fRenderCamera( ).fGetWorldSpaceFrustum( ), tBuildLightListSceneGraphCallback( vpLightList ), tLightEntity::cSpatialSetIndex );

				// sort lights by priority
				std::sort( vpLightList.fBegin( ), vpLightList.fEnd( ), tSortLightList( ) );

				// add lights in this viewport's list to the unique light list
				for( u32 ilight = 0; ilight < vpLightList.fCount( ); ++ilight )
					mVisibleLightsList.fFindOrAdd( vpLightList[ ilight ] );
			}
		}

		// make sure all lights are sync'd up before rendering;
		// at the same time, extract the lights that cast shadow
		tLightEntity* bestPointLight = NULL;
		f32 bestLightMetric = -Math::cInfinity; //choose highest
		
		/*HACK*/
		Math::tVec3f influencePt = mViewports[ 0 ]->fRenderCamera( ).fGetTripod( ).mLookAt;

		for( u32 i = 0; i < mVisibleLightsList.fCount( ); ++i )
		{			
			tLightEntity* le = mVisibleLightsList[ i ];

			le->fSyncBeforeRender( fToScreen( ) );

			if( le->fCastsShadow( ) )
			{
				if( le->fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection )
					mShadowCastingLightList.fPushBack( mVisibleLightsList[ i ] );
				else if( le->fLightDesc( ).fLightType( ) == tLight::cLightTypePoint )
				{
					f32 distanceInsideLightSurface = le->fEffectiveRadius( ) - (le->fObjectToWorld( ).fGetTranslation( ) - influencePt).fLength( );
					if( distanceInsideLightSurface > bestLightMetric )
					{
						bestLightMetric = distanceInsideLightSurface;
						bestPointLight = le;
					}
				}
			}
		}

		if( bestPointLight )
			mShadowLightBlender->fFadeToLight( bestPointLight );

		mShadowLightBlender->fStep( mSceneGraph->fFrameDeltaTime( ) );

		if( mShadowLightBlender->fCurrentLight( ) )
			mShadowCastingLightList.fPushBack( mShadowLightBlender->fCurrentLight( ) );
	}

	void tScreenPlatformBase::fBuildDisplayLists( )
	{
		profile_pix( "fBuildDisplayLists" );
		profile( cProfilePerfBuildDisplayLists );

		tRenderState::gConsiderCutoutXParent = fDeferredShading( ) && !Renderer_Debug_ShowCutout;

		for( u32 i = 0; i < mViewports.fCount( ); ++i )
		{
			tViewport& vp = *mViewports[ i ];
			tLightEntityList& lights = mViewportDisplayList[ i ].mLightList;
			tLightComboList& combos = mViewportDisplayList[ i ].mLightCombo;
			tWorldSpaceDisplayList& depthDisplayList = mViewportDisplayList[ i ].mDepthList;

			combos.fBuildLightCombos( fToScreen( ), *mSceneGraph, vp, fDeferredShading( ), lights, depthDisplayList, !mWorldDepthTexture.fNull( ), &mShadowCastingLightList );
		}
	}

	void tScreenPlatformBase::fBuildCanvasList( )
	{
		profile_pix( "fBuildCanvasList" );
		profile( cProfilePerfOnRenderCanvas );
		mRootCanvas->fOnRenderCanvas( fToScreen( ) );
	}

	devvar_clamp( u32, Renderer_GPRS_ShadowMapPS, 32, 16, 128-16, 0 );
	void tScreenPlatformBase::fRenderShadowMaps( )
	{
		profile_pix( "fRenderShadowMaps" );
		profile( cProfilePerfRenderShadowMaps );

		mDevice->fSetShaderGPRAllocation( 128 - Renderer_GPRS_ShadowMapPS, Renderer_GPRS_ShadowMapPS );

		// render each shadow map
		for( u32 i = 0; i < mShadowCastingLightList.fCount( ); ++i )
		{
			tLightEntity* shadowLight = mShadowCastingLightList[ i ];
#ifdef sig_profile
			const char* type = "Unknown";
			switch( shadowLight->fLightDesc( ).fLightType( ) )
			{
			case tLight::cLightTypeDirection:	type = "Directional"; break;
			case tLight::cLightTypePoint:		type = "Point"; break;
			default: sig_nodefault( );			type = "Unknown"; break;
			}
			std::stringstream desc;
			desc << "Shadow map for " << type << " LIGHT " << (i+1) << " of " << mShadowCastingLightList.fCount( );
			profile_pix( desc.str().c_str() );
#endif
			
			// render shadow map
			shadowLight->fRenderShadowMap( fToScreen( ) );
		}
	}

	void tScreenPlatformBase::fRenderWorldDepth( )
	{
		if( !mWorldDepthTexture )
			return;

		profile_pix( "fRenderWorldDepth" );

		mDevice->fSetShaderGPRAllocation( 128 - Renderer_GPRS_ShadowMapPS, Renderer_GPRS_ShadowMapPS );

		// set shadow map render targets
		mWorldDepthTexture->fApply( fToScreen( ) );

		// apply full viewport clipbox before clearing
		mWorldDepthTexture->fSetClipBox( fToScreen( ), Math::tVec4f( 0.f, 0.f, 1.f, 1.f ) );

		// clear render targets
		fClearCurrentRenderTargets( true, true, Math::tVec4f(1.f), 0 );

		for( u32 i = 0; i < mViewports.fCount( ); ++i )
		{
			tViewport& vp = *mViewports[ i ];

			if( vp.fIsVirtual( ) || vp.fIsIgnored( ) )
				continue;

			// apply the viewport's clip box
			mWorldDepthTexture->fSetClipBox( fToScreen( ), vp.fClipBox( ).fConvertToLTRB( ) ); 

			// set default render state for depth rendering
			tRenderContext renderContext;
			renderContext.fFromScreen( fToScreen( ) );
			renderContext.mRenderPassMode = tRenderState::cRenderPassDepth;
			renderContext.mSceneCamera = &vp.fRenderCamera( );
			renderContext.mViewportIndex = vp.fViewportIndex( );

			// invalidate render states
			mDevice->fInvalidateLastRenderState( );

			// render display list, using light camera
			tWorldSpaceDisplayList& displayList = mViewportDisplayList[ i ].mDepthList;
			displayList.fRenderAll( fToScreen( ), renderContext );
		}

		// resolve shadow map texture
		mWorldDepthTexture->fResolve( fToScreen( ) );
	}

	devvar_clamp( u32, Renderer_GPRS_WorldVS, 48, 16, 128-16, 0 );
	void tScreenPlatformBase::fRenderWorld( )
	{
		profile_pix( "fRenderWorld" );
		profile( cProfilePerfRenderWorld );

		mDevice->fSetShaderGPRAllocation( Renderer_GPRS_WorldVS, 128-Renderer_GPRS_WorldVS );

		// reset world stats prior to rendering
		mWorldStats.fReset( );

		if( Debug_SceneGraph_BVH_Render )
		{
			// issue debug rendering on the scene graph's BVH and/or object boxes
			if( !mSceneGraph->fWillPauseOnNextFrame( ) )
				mSceneGraph->fVisualizeSpatialSetDebug( Debug_SceneGraph_BVH_RenderLevel, Debug_SceneGraph_BVH_RenderObjectsOnly );
		}
		
		if( Debug_SceneGraph_BVH_RenderKDTrees )
		{
			mSceneGraph->fVisualizeSubObjectSpatialDebug( Debug_SceneGraph_BVH_RenderKDTreeLevel );
		}

		// set the scene render targets
		const tRenderToTexturePtr& renderTarget = ( fDeferredShading( ) ) ? mDeferredRenderToTexture : mSceneRenderToTexture; 
		renderTarget->fApply( fToScreen( ) );

		// set full viewport before clearing
		renderTarget->fSetClipBox( fToScreen( ), Math::tVec4f( 0.f, 0.f, 1.f, 1.f ) );

		// clear render targets
		fClearCurrentRenderTargets( true, true, mCreateOpts.mDefaultClearColor, 0x0, fDeferredShading( ) );

		// render each viewport
		for( u32 i = 0; i < mViewports.fCount( ); ++i )
		{
			tViewport& vp = *mViewports[ i ];

			if( vp.fIsVirtual( ) || vp.fIsIgnored( ) )
				continue;

			renderTarget->fSetClipBox( fToScreen( ), vp.fClipBox( ).fConvertToLTRB( ) );
			fRenderViewport( i );
		}

		// clear explicitly enqueued objects
		mExplicitWorldObjects.fSetCount( 0 );
		mExplicitWorldTopObjects.fSetCount( 0 );
	}

	devvar_clamp( u32, Renderer_GPRS_DeferredVS, 17, 16, 128-16, 0 );
	void tScreenPlatformBase::fDeferredLightPass( const tViewport& vp, tTextureReference& blankShadowMap )
	{
		tDevice::fGetDefaultDevice( )->fSetShaderGPRAllocation( Renderer_GPRS_DeferredVS, 128-Renderer_GPRS_DeferredVS );

		profile_pix( "fDeferredLightPass" );
		sigassert( fDeferredShading( ) );

		const b32 lastTarget = ( &vp == mViewports.fBack( ) );
		tScreen& screen = fToScreen( );

		//mDeferredRenderToTexture->fSetClipBox( fToScreen( ), vp.fClipBox( ).fConvertToLTRB( ) );
		mDeferredRenderToTexture->fResolve( screen, NULL, 0, lastTarget );

		tRenderContext& renderContext = mDeferredShadingMaterial->mContext.mContext;
		renderContext.mRenderTargetDims = Math::tVec4f( (f32)mSceneRenderToTexture->fRenderTarget( )->fWidth( ), (f32)mSceneRenderToTexture->fRenderTarget( )->fHeight( ), 0.f, 0.f );
		renderContext.mViewportTLBR = vp.fClipBox( ).fConvertToLTRB( ); //Math::tVec4f( 0,0,1,1 );
		renderContext.mSceneCamera = &vp.fRenderCamera( );

		mDevice->fInvalidateLastRenderState( );

		// render the Gbuffer to the scene texture.
		mSceneRenderToTexture->fApply( screen );
		mSceneRenderToTexture->fSetClipBox( screen, renderContext.mViewportTLBR );
		screen.fClearCurrentRenderTargets( true, true, mCreateOpts.mDefaultClearColor, 0x0 );

		// apply textures
		tTextureReference& texRef = fToScreen( ).fPostProcessDepthTexture( )->fTexture( 0 );
		texRef.fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeClamp );
		texRef.fApply( mDevice, tDeferredShadingMaterial::cTexUnitDepth );
		for( u32 i = 0; i < mDeferredRenderToTexture->fTargetCount( ); ++i )
			mDeferredRenderToTexture->fTexture( i ).fApply( mDevice, tDeferredShadingMaterial::cTexUnitGbuffer0 + i );

		mDeferredShadingMaterial->fResetLightPass( );
		mDeferredShadingMaterial->fRenderLightList( mVisibleLightsList, mShadowCastingLightList, blankShadowMap, screen );

		// Reset to world GPR allocs
		mDevice->fSetShaderGPRAllocation( Renderer_GPRS_WorldVS, 128-Renderer_GPRS_WorldVS );
	}

	void tScreenPlatformBase::fRenderViewport( u32 vpIndex )
	{
		tViewport& vp = *mViewports[ vpIndex ];
		tScreen& screen = fToScreen( );
		const tSceneGraph& sg = *screen.fSceneGraph( );
		tLightComboList& lightCombo = mViewportDisplayList[ vpIndex ].mLightCombo;

		// use the semi-globally available shared white texture as a default shadow map
		tTextureReference blankShadowMap;
		blankShadowMap.fSetRaw( ( tTextureReference::tPlatformHandle )screen.fBlackTexture( ) );
		blankShadowMap.fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeBorderBlack );

		// set default render state for world/scene rendering (uses scene camera)
		tRenderContext renderContext;
		renderContext.fFromScreen( screen );
		renderContext.mViewportTLBR = vp.fClipBox( ).fConvertToLTRB( );
		renderContext.mSceneCamera = &vp.fRenderCamera( );
		renderContext.mViewportIndex = vpIndex;
		renderContext.mViewportCount = screen.fGetViewportCount( );

		for( s32 v = renderContext.mViewportCount - 1; v >= 0; --v )
		{
			if( screen.fViewport( v )->fIsVirtual( ) )
				--renderContext.mViewportCount;
		}

		// Debug render setup
		if( Renderer_FillMode == 1 )
			renderContext.mGlobalFillMode = tRenderState::cGlobalFillWire;
		else if( Renderer_FillMode == 2 )
			renderContext.mGlobalFillMode = tRenderState::cGlobalFillEdgedFace;

		// add all "explicitly-added" objects to the display list; these are objects that got added 
		// during the course of the last frame, which don't live in the scene graph permanently; this
		// list includes debug geometry, helper objects, etc, unlit usually
		tWorldSpaceDisplayList& displayList = mWorldSpaceDisplayList;
		displayList.fInvalidate( );
		for( u32 i = 0; i < mExplicitWorldObjects.fCount( ); ++i )
			displayList.fInsert( tDrawCall( *mExplicitWorldObjects[ i ], vp.fRenderCamera( ).fCameraDepth( mExplicitWorldObjects[ i ]->fObjectToWorld( ).fGetTranslation( ) ) ) );

		sg.fDebugGeometry( ).fAddToDisplayList( vp, displayList );

		displayList.fSeal( );

		// ACTUAL MAIN RENDERING
		{
			screen.fGetDevice( )->fInvalidateLastRenderState( );

			if( fDeferredShading( ) )
				renderContext.mRenderPassMode = tRenderState::cRenderPassGBuffer;
			else
				renderContext.mRenderPassMode = tRenderState::cRenderPassLighting;

			// Opaque
			{
				profile_pix( "Opaque Objects" );
				screen.fRenderWorldOpaqueBegin( );
				lightCombo.mDisplayList.fOpaque( ).fRender( screen, renderContext );
				displayList.fOpaque( ).fRender( screen, renderContext );
				screen.fRenderWorldOpaqueEnd( );
			}

			if( fDeferredShading( ) )
			{
				profile_pix( "Deferred Lighting Pass" );
				fDeferredLightPass( vp, blankShadowMap );
				renderContext.mRenderPassMode = tRenderState::cRenderPassLighting;
				screen.fGetDevice( )->fInvalidateLastRenderState( );
			}

			// Transparent
			{
				profile_pix( "Transparent Objects" );
				// Render regular xparent objects
				renderContext.fClearLights( &blankShadowMap );
				lightCombo.mDisplayList.fXparent( ).fRender( screen, renderContext );
				displayList.fXparent( ).fRender( screen, renderContext );

				// Render depth only prepass
				tRenderState::fEnableDisableColorWrites( screen.fGetDevice( ), false );
					renderContext.mRenderPassMode = tRenderState::cRenderPassXparentDepthPrepass;
					lightCombo.mDisplayList.fXparentWithDepthPrepass( ).fRender( screen, renderContext );
					displayList.fXparentWithDepthPrepass( ).fRender( screen, renderContext );
				tRenderState::fEnableDisableColorWrites( screen.fGetDevice( ), true );

				// Render depth prepass xparent objects
				renderContext.mRenderPassMode = tRenderState::cRenderPassLighting;
				lightCombo.mDisplayList.fXparentWithDepth( ).fRender( screen, renderContext );
				displayList.fXparentWithDepth( ).fRender( screen, renderContext );
			}
		}

		// Combine our stats
		mWorldStats.fCombine( displayList.fOpaque( ).fGetStats( ) );
		mWorldStats.fCombine( displayList.fXparent( ).fGetStats( ) );
		mWorldStats.fCombine( displayList.fXparentWithDepthPrepass( ).fGetStats( ) );
		mWorldStats.fCombine( displayList.fXparentWithDepth( ).fGetStats( ) );
		mWorldStats.fCombine( lightCombo.mDisplayList.fOpaque( ).fGetStats( ) );
		mWorldStats.fCombine( lightCombo.mDisplayList.fXparent( ).fGetStats( ) );
		mWorldStats.fCombine( lightCombo.mDisplayList.fXparentWithDepthPrepass( ).fGetStats( ) );
		mWorldStats.fCombine( lightCombo.mDisplayList.fXparentWithDepth( ).fGetStats( ) );

		if( mExplicitWorldTopObjects.fCount( ) > 0 )
		{
			// make sure all the lights are cleared
			renderContext.fClearLights( &blankShadowMap );

			// clear depth buffer
			screen.fClearCurrentRenderTargets( false, true, 0.f, 0x0 );

			// get display list of explicitly enqueued world "topmost" objects
			displayList.fInvalidate( );
			for( u32 i = 0; i < mExplicitWorldTopObjects.fCount( ); ++i )
				displayList.fInsert( tDrawCall( *mExplicitWorldTopObjects[ i ], vp.fRenderCamera( ).fCameraDepth( mExplicitWorldTopObjects[ i ]->fObjectToWorld( ).fGetTranslation( ) ) ) );

			// render display list
			displayList.fSeal( );
			displayList.fRenderAll( screen, renderContext );
			
			// Make sure the top most objects get counted in the stats
			mWorldStats.fCombine( displayList.fOpaque( ).fGetStats( ) );
			mWorldStats.fCombine( displayList.fXparent( ).fGetStats( ) );
			mWorldStats.fCombine( displayList.fXparentWithDepthPrepass( ).fGetStats( ) );
			mWorldStats.fCombine( displayList.fXparentWithDepth( ).fGetStats( ) );

			displayList.fInvalidate( );
		}
	}

	void tScreenPlatformBase::fRenderRttAgents( )
	{
		profile_pix( "fRenderRttAgents" );

		for(u32 i = 0; i < mRttAgents.fCount( ); ++i)
			fRenderRttAgent( *mRttAgents[ i ], false );
	}

	void tScreenPlatformBase::fRenderRttAgent( const tRenderToTextureAgent& rtt, b32 immediate )
	{
		tScreen& self = fToScreen( );
		tRenderState::gWriteProperAlphaWhenWritingCutOut = Renderer_Debug_3DFuiCutoutFix;
		tRenderState::gConsiderCutoutXParent = false;

		if( immediate )
			fBeginAllRendering( );

		sigassert( rtt.mRoot );
		sigassert( rtt.mLight );
		//sigassert( rtt.mOutTexture ); // this is ok to be null
		sigassert( rtt.mRtt );
		const u32 texWidth = rtt.mRtt->fWidth( );
		const u32 texHeight = rtt.mRtt->fHeight( );

		//apply our own render target
		rtt.mRtt->fApply( self );

		fSetViewportClipBox( 0, 0, texWidth, texHeight );
		fClearCurrentRenderTargets( true, true, rtt.mClearColor, 0 );

		tRenderContext rc;
		rc.fFromScreen( fToScreen( ) );
		rc.mSceneCamera = &rtt.mCamera;
		rc.mViewportIndex = 0;

		tLightCombo lightCombo;
		lightCombo.mLights.fPushBack( rtt.mLight.fGetRawPtr( ) );

		//gather all renderables
		tRenderableGatherer rg;
		rg.fGatherRecursive( rtt.mRoot );

		//render
		tWorldSpaceDisplayList w;
		for(u32 i = 0; i < rg.mRenderables.fCount( ); ++i)
		{
			tRenderableEntity* re = rg.mRenderables[ i ];
			if( re->fInvisible( ) )
				continue;

			// use: Gfx::tRenderableEntity::fSetForceHighLOD( *rootEnt, true );
			//re->fUpdateLOD( camera.fGetTripod( ).mEye );

			tDrawCall dc = re->fGetDrawCall( rtt.mCamera );
			//sigassert( dc.fValid( ) ); <- failing because some fxmls spawned outside of the world fail to create render instances
			dc.fSetLightCombo( &lightCombo );
			w.fInsert( dc );
		}
		w.fRenderAll( self, rc );

		//finally copy the image from our render target to our texture so it can be used later in the pipeline
		rtt.mRtt->fResolve( self, (IDirect3DTexture9*)rtt.mOutTexture );

		if( immediate )
			fEndAllRendering( );
			
		tRenderState::gWriteProperAlphaWhenWritingCutOut = false;
	}

	devvar_clamp( u32, Renderer_GPRS_PostProcessVS, 32, 16, 128-16, 0 );
	void tScreenPlatformBase::fRenderPostEffects( )
	{
		profile_pix( "fRenderPostEffects" );
		fDebugOverrideViewportEffectSequences( );
		mDevice->fSetShaderGPRAllocation( Renderer_GPRS_PostProcessVS, 128-Renderer_GPRS_PostProcessVS );

		tScreen& self = fToScreen( );

		b32 renderedPostEffects = false;

		s32 firstViewport = -1, lastViewport = -1;
		if( mPostEffectMgr )
			mPostEffectMgr->fDetermineFirstAndLastViewportsWithSequence( self, firstViewport, lastViewport );
		if( firstViewport >= 0 )
		{
			sigassert( lastViewport >= 0 );

			// resolve scene render target
			const tRenderToTexturePtr& sceneRenderTarget = fSceneRenderToTexture( );
			sceneRenderTarget->fResolve( self );

			// no MSAA for full screen post effects
			fDisableMSAA( );

			// render post effects for each viewport
			for( u32 i = 0; i < mViewports.fCount( ); ++i )
			{
				if( !mViewports[ i ]->fIsVirtual( ) && !mViewports[ i ]->fIsIgnored( ) )
				{
					mPostEffectMgr->fRender( self, i, i == firstViewport, i == lastViewport );
					renderedPostEffects = true;
				}
			}

			// re-enable MSAA if there was any
			fReEnableMSAA( );
		}

		if( !renderedPostEffects )
		{
			//log_line( Log::cFlagGraphics, "!renderedPostEffects" );
		}
	}

	void tScreenPlatformBase::fHandleScreenSpaceDummyBatches( const tDrawCall& dc )
	{
		const Gfx::tRenderInstance& renderInstance = dc.fRenderInstance( );
		const Gui::tRenderableCanvas* renderableCanvas = renderInstance.fRI_RenderableCanvas( );
		if( renderableCanvas )
		{
			Gui::tFuiCanvas* fuiCanvas = renderableCanvas->fDynamicCast<Gui::tFuiCanvas>( );
			if( fuiCanvas )
			{
				Fui::tFuiPtr fuiScreen = fuiCanvas->fMovie( );
				if( fuiScreen )
					Fui::tFuiSystem::fInstance( ).fRender( fuiScreen );
			}
		}

		// invalidate render states
		fGetDevice( )->fInvalidateLastRenderState( );
	}

	void tScreenPlatformBase::fRenderScreenSpace( )
	{
		profile_pix( "fRenderScreenSpace" );
		tScreen& self = fToScreen( );

		// no MSAA for screen space gui/hud
		fDisableMSAA( );

		// N.B.! No need to set a render target, we rely on fRenderPostEffects( ) to leave us with a suitable RT.
		// Similarly, we don't resolve after this pass, as the results will be used as the final presentation image.

		const tRenderToTexturePtr& renderTarget = fSceneRenderToTexture( );
		fSetViewportClipBox( 0, 0, renderTarget->fWidth( ), renderTarget->fHeight( ) );
		fClearCurrentRenderTargets( false, true, 0.f, 0x0 );

		// these render on top, after everything else
		for( u32 i = 0; i < mMoviePlayers.fCount( ); ++i )
			mMoviePlayers[ i ]->fRenderFrame( *mDevice );
		mMoviePlayers.fSetCount( 0 );

		// sort the screen space display list
		mScreenSpaceDL.fSeal( );

		// setup render context
		tRenderContext renderContext;
		renderContext.fFromScreen( fToScreen( ) );
		renderContext.mSceneCamera = &mScreenSpaceCamera;
		renderContext.mFogValues = Math::tVec4f( 1000.f, 1000.f, 0.f, 0.f );	//disables fog for screen-space rendereds

		// use the semi-globally available shared white texture as a default shadow map
		tTextureReference whiteShadowMap;
		whiteShadowMap.fSetRaw( ( tTextureReference::tPlatformHandle )fWhiteTexture( ) );
		whiteShadowMap.fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeClamp );

		// setup lights for screen space 3D objects
		tLightEntityList screenSpaceLights; // TODO get these from somewhere
		renderContext.fFromLightGroup( self, screenSpaceLights, &whiteShadowMap );

		// invalidate render states
		fGetDevice( )->fInvalidateLastRenderState( );

		// render all screen draw calls
		if( Fui::fGroupRendering( ) )
			Fui::tFuiSystem::fInstance( ).fRender( Fui::tFuiPtr( ) );
		else
			mScreenSpaceDL.fRender( fToScreen( ), renderContext, make_delegate_memfn( tDisplayList::tDummyHandler, tScreenPlatformBase, fHandleScreenSpaceDummyBatches ) );
		Fui::tFuiSystem::fInstance( ).fOnRenderComplete( );

		// cache screen space display list stats, invalidate screen space display list
		mScreenStats.fReset( );
		mScreenStats.fCombine( mScreenSpaceDL.fGetStats( ) );
		mScreenSpaceDL.fInvalidate( );

		// re-enable MSAA if there was any
		fReEnableMSAA( );
	}

	void tScreenPlatformBase::fRenderWorldOpaqueBegin( )
	{
	}

	void tScreenPlatformBase::fRenderWorldOpaqueEnd( )
	{
	}

	void tScreenPlatformBase::fBeginCaptureDump( const tFilePathPtr& folder, u32 month, u32 day, u32 year, u32 hour, u32 minute, u32 second )
	{
		std::stringstream ss;
		ss << std::setfill( '0' ) << std::setw( 2 );
		ss << month << "(" << day << "(" << year << "_" << hour << "h" << minute << "m" << second << "s";
		std::string subFolder = ss.str( );

		mCaptureData.fReset( NEW tCaptureData );
		mCaptureData->mFolder = tFilePathPtr::fConstructPath( folder, tFilePathPtr( subFolder.c_str( ) ) );
		FileSystem::fCreateDirectory( 	mCaptureData->mFolder );
	}

	void tScreenPlatformBase::fEndCaptureDump( )
	{
		// spit out text file with camera capture data
		const tFilePathPtr cameraDumpPath = tFilePathPtr::fConstructPath( mCaptureData->mFolder, tFilePathPtr( "camera.txt" ) );
		tFileWriter cameraDumpFile( cameraDumpPath );

		if( cameraDumpFile.fIsOpen( ) )
		{
			{
				std::stringstream ss;

				ss  << "format of each line is [ "
					<< "horzFov nearPlaneWidth nearPlaneHeight nearPlane farPlane "
					<< "eye.x eye.y eye.z "
					<< "lookat.x lookat.y lookat.z "
					<< "xaxis.x xaxis.y xaxis.z "
					<< "yaxis.x yaxis.y yaxis.z "
					<< "zaxis.x zaxis.y zaxis.z "
					<< "]\r\n";

				std::string s = ss.str( );
				cameraDumpFile( &s[0], ( u32 )s.length( ) );
			}

			for( u32 i = 0; i < mCaptureData->mCameraFrames.fCount( ); ++i )
			{
				const tCaptureData::tCameraFrame& cameraFrame = mCaptureData->mCameraFrames[ i ];
				for( u32 j = 0; j < cameraFrame.fCount( ); ++j )
				{
					const f32 w = cameraFrame[ j ].fGetLens( ).mRight - cameraFrame[ j ].fGetLens( ).mLeft;
					const f32 h = cameraFrame[ j ].fGetLens( ).mTop - cameraFrame[ j ].fGetLens( ).mBottom;
					const f32 n = cameraFrame[ j ].fGetLens( ).mNearPlane;
					const f32 f = cameraFrame[ j ].fGetLens( ).mFarPlane;
					const f32 fovInDegrees = Math::fToDegrees( 2.f * Math::fAtan( w / n ) );
					const Math::tVec3f eye = cameraFrame[ j ].fGetTripod( ).mEye;
					const Math::tVec3f lat = cameraFrame[ j ].fGetTripod( ).mLookAt;
					const Math::tVec3f x = cameraFrame[ j ].fXAxis( );
					const Math::tVec3f y = cameraFrame[ j ].fYAxis( );
					const Math::tVec3f z = cameraFrame[ j ].fZAxis( );

					std::stringstream ss;

					ss  << "[ "
						<< fovInDegrees << ' ' << w << ' ' << h << ' ' << n << ' ' << f << ' '
						<< eye.x << ' ' << eye.y << ' ' << eye.z << ' ' 
						<< eye.x << ' ' << eye.y << ' ' << eye.z << ' ' 
						<< lat.x << ' ' << lat.y << ' ' << lat.z << ' '
						<< x.x << ' ' << x.y << ' ' << x.z << ' ' 
						<< y.x << ' ' << y.y << ' ' << y.z << ' '
						<< z.x << ' ' << z.y << ' ' << z.z << ' '
						<< "]\r\n";

					std::string s = ss.str( );
					cameraDumpFile( &s[0], ( u32 )s.length( ) );
				}
			}
		}

		mCaptureData.fRelease( );
	}

	b32 tScreenPlatformBase::fCheckForAutoCaptureTermination( ) const
	{
		if( Renderer_Capture_AutoDevMenuOnOff &&
			fCapturing( ) &&
			tApplication::fInstance( ).fIsDevMenuActive( ) )
		{
			devcb_update_value( Renderer_Capture_PerformCapture, "start" );
			return true;
		}
		
		return false;
	}

	tFilePathPtr tScreenPlatformBase::fCreateCaptureDumpPath( const tFilePathPtr& directory, const char* baseName, const char* extension, u32 frameNum ) const
	{
		sigassert( baseName && extension );
		if( *extension == '.' )
			++extension;

		std::stringstream ss;
		ss << baseName << "_" << std::setfill( '0' ) << std::setw( 4 ) << frameNum << "." << extension;
		std::string name = ss.str( );

		tFilePathPtr subDir = tFilePathPtr::fConstructPath( directory, tFilePathPtr( baseName ) );
		FileSystem::fCreateDirectory( subDir );
		return tFilePathPtr::fConstructPath( subDir, tFilePathPtr( name.c_str( ) ) );
	}

	void tScreenPlatformBase::fCaptureCurrentCameras( ) const
	{
		mCaptureData->mCameraFrames.fGrowCount( 1 );
		mCaptureData->mCameraFrames.fBack( ).fNewArray( mViewports.fCount( ) );

		for( u32 i = 0; i < mViewports.fCount( ); ++i )
			mCaptureData->mCameraFrames.fBack( )[ i ] = mViewports[ i ]->fRenderCamera( );
	}

	void tScreenPlatformBase::fDebugOverrideViewportEffectSequences( )
	{
#if defined( sig_devmenu )
		const char* sequence = NULL;
		switch( gGBufferDebugDisplayMode )
		{
		case cGBufferDebugDisplay0:					sequence = "SigEngine.Debug.GBuffer0"; break;
		case cGBufferDebugDisplay1:					sequence = "SigEngine.Debug.GBuffer1"; break;
		case cGBufferDebugDisplay2:					sequence = "SigEngine.Debug.GBuffer2"; break;
		case cGBufferDebugDisplay3:					sequence = "SigEngine.Debug.GBuffer3"; break;
		case cGBufferDebugDisplaySpecPower:			sequence = "SigEngine.Debug.SpecPower"; break;
		case cGBufferDebugDisplaySpecValue:			sequence = "SigEngine.Debug.SpecValue"; break;
		case cGBufferDebugDisplayShadowMap0Layer0:	sequence = "SigEngine.Debug.ShadowMap0.Layer0"; break;
		case cGBufferDebugDisplayShadowMap0Layer1:	sequence = "SigEngine.Debug.ShadowMap0.Layer1"; break;
		case cGBufferDebugDisplayShadowMap1Layer0:	sequence = "SigEngine.Debug.ShadowMap1.Layer0"; break;
		case cGBufferDebugDisplayShadowMap1Layer1:	sequence = "SigEngine.Debug.ShadowMap1.Layer1"; break;
		}
			
		if( !sequence )
			return;

		const tScreenPtr& screen = tGameAppBase::fInstance( ).fScreen( );
		for( u32 i = 0; i < screen->fGetViewportCount( ); ++i )
			screen->fViewport( i )->fSetPostEffectSequence( tStringPtr( sequence ) );
#endif
	}

	void tScreenPlatformBase::fCreateFullScreenQuad( )
	{
		//
		// create vertex buffer: four verts for two triangles
		//
		tPostEffectsRenderVertex sysMemVerts[] =
		{
			tPostEffectsRenderVertex( Math::tVec3f( 0.f, 0.f, 0.f ) ),
			tPostEffectsRenderVertex( Math::tVec3f( 0.f, 1.f, 0.f ) ),
			tPostEffectsRenderVertex( Math::tVec3f( 1.f, 1.f, 0.f ) ),
			tPostEffectsRenderVertex( Math::tVec3f( 1.f, 0.f, 0.f ) ),
		};
		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysTextures ) );
		mFullScreenQuadVB.fAllocate( mDevice, tPostEffectsMaterial::cVertexFormat, 4, 0 );
		Memory::tHeap::fResetVramContext( );
		mFullScreenQuadVB.fBufferData( sysMemVerts, array_length( sysMemVerts ) );
			
		//
		// create index buffer: six indices for two indexed triangles
		// Reverse wound.
		//
		u16 sysMemIds[] =
		{
			0, 2, 1,
			0, 3, 2,
		};
		Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );
		mFullScreenQuadIB.fAllocate( mDevice, tIndexFormat( tIndexFormat::cStorageU16, tIndexFormat::cPrimitiveTriangleList ), 6, 2, 0 );
		Memory::tHeap::fResetVramContext( );
		mFullScreenQuadIB.fBufferData( sysMemIds, array_length( sysMemIds ) );
	}

	void tScreenPlatformBase::fSetupDeferredShading( )
	{
		if( !fDeferredShading( ) )
			return;

		tDefaultAllocators& defAlloc = tDefaultAllocators::fInstance( );
		mDeferredShadingMaterial.fReset( NEW tDeferredShadingMaterial( ) );
		mDeferredShadingMaterial->fSetMaterialFileResourcePtrOwned( defAlloc.mDeferredShadingMaterialFile );

		tRenderToTexture::tFormat formats = 
			tRenderToTexture::tFormat( tRenderTarget::cFormatRGBA8 ) // Diffuse
			.fNext( tRenderTarget::cFormatRGBA8 )					 // normal map
			.fNext( tRenderTarget::cFormatRGBA8 )                    // emission
			.fNext( tRenderTarget::cFormatRGBA8 );                   // ambient
		mDeferredRenderToTexture.fReset( NEW tRenderToTexture( mDevice, mSceneRenderToTexture->fWidth( ), mSceneRenderToTexture->fHeight( ), formats, tRenderTarget::cFormatD24FS8, 0 ) );
		
		// don't do any filtering
		for( u32 i = 0; i < formats.fCount( ); ++i )
		{
			mDeferredRenderToTexture->fTexture( i ).fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeClamp );
		}

		//mLightSphere.fSetRenderStateOverride( &mImpl->mRenderStateOverride );
		mLightSphere.fReset( NEW tDeferredShadingSphere( ) );
		mLightSphere->fResetDeviceObjects( mDevice, tMaterialPtr( mDeferredShadingMaterial.fGetRawPtr( ) ), defAlloc.mPostEffectsAndDeferredGeomAllocator, defAlloc.mIndexAllocator );
		mLightSphere->fGenerate( 1.f );

#ifdef platform_xbox360 //ghettoooo
		mDeferredRenderToTexture->fSetDepthResolveTexture( fToScreen( ).fPostProcessDepthTexture( ) );
#endif

		tScreen& screen = fToScreen( );
		tDeferredShadingMaterial::tDeferredShadingContext& context = mDeferredShadingMaterial->mContext;
		context.fSetScreen( screen );

		// setup render batch data descriptor
		tRenderBatchData& batchQuad = context.mFullScreenQuad;
		batchQuad.mRenderState = &context.mRenderState;
		batchQuad.mVertexFormat = &screen.fFullScreenQuadVB( ).fVertexFormat( );
		batchQuad.mGeometryBuffer = &screen.fFullScreenQuadVB( );
		batchQuad.mIndexBuffer = &screen.fFullScreenQuadIB( );
		batchQuad.mVertexCount = screen.fFullScreenQuadVB( ).fVertexCount( );
		batchQuad.mBaseVertexIndex = 0;
		batchQuad.mPrimitiveCount = screen.fFullScreenQuadIB( ).fPrimitiveCount( );
		batchQuad.mBaseIndexIndex = 0;
		batchQuad.mPrimitiveType = screen.fFullScreenQuadIB( ).fIndexFormat( ).mPrimitiveType;
		batchQuad.mMaterial = mDeferredShadingMaterial.fGetRawPtr( );

		tRenderBatchData& batchSphere = context.mUnitSphere;
		batchSphere = mLightSphere->fGetRenderBatch( )->fBatchData( );
		batchSphere.mRenderState = &context.mRenderState;
	}
		

}}

