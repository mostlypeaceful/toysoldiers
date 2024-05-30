#include "BasePch.hpp"
#include "tApplication.hpp"
#include "tScreenWorkerThread.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "tLightCombo.hpp"
#include "tRenderContext.hpp"
#include "tRenderableEntity.hpp"
#include "tPostEffectsMaterial.hpp"
#include "Gui/tCanvas.hpp"
#include "tProfiler.hpp"

#ifdef sig_devmenu
#include "tUser.hpp"
#endif//sig_devmenu

namespace Sig { namespace Gfx
{
	devvar( bool, Debug_SceneGraph_BVH_Render, false );
	devvar( bool, Debug_SceneGraph_BVH_RenderObjectsOnly, true );
	devvar_clamp( s32, Debug_SceneGraph_BVH_RenderLevel, -1, -1, 16, 0 );

	devvar( bool, Debug_SceneGraph_BVH_RenderKDTrees, false );
	devvar_clamp( s32, Debug_SceneGraph_BVH_RenderKDTreeLevel, -1, -1, 16, 0 );

	namespace
	{
		static const u32 cDefaultMultiSamplePower = 
#if defined( platform_xbox360 )
			1;
#else
			2;
#endif
	}

	devvar_clamp( u32, Renderer_Settings_MultiSample, cDefaultMultiSamplePower, 0, 2, 0 );

	devvar( bool, Renderer_Capture_AutoDevMenuOnOff, true );

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
#endif//sig_devmenu
	devcb( Renderer_Capture_PerformCapture, "start", make_delegate_cfn( tDevCallback::tFunction, fPerformCaptureDevCallback ) );
}}

namespace Sig { namespace Gfx
{
	///
	/// \brief Collects all lights intersecting the view volume.
	struct base_export tBuildLightListSceneGraphCallback : public tEntityBVH::tIntersectVolumeCallback<Math::tFrustumf>
	{
		tLightEntityList& mLightList;

		tBuildLightListSceneGraphCallback( tLightEntityList& lightList ) : mLightList( lightList ) { }

		inline void operator()( const Math::tFrustumf& v, tEntityBVH::tObjectPtr i, b32 aabbWhollyContained ) const
		{
			tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );

			sigassert( spatial->fDynamicCast< tLightEntity >( ) );

			tLightEntity* light = static_cast< tLightEntity* >( spatial );
			if( light->fOn( ) )
				if( fQuickAabbTest( v, i, aabbWhollyContained ) )
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
				o += 10000.f;
			
			if( l->fLightDesc( ).fLightType( ) == tLight::cLightTypeDirection )
			{
				o += 1000000.f;
			}
			else
			{
				const f32 scale = l->fObjectToWorld( ).fGetScale(  ).fMax( );
				const f32 maxRadius = fMax( l->fLightDesc( ).fRadii( ).x, l->fLightDesc( ).fRadii( ).y );
				o += scale * maxRadius;
			}
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
	{
		fZeroOut( this );
		mVsync = true;
		mShadowMapLayerCount = 1;
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
		return Renderer_Settings_MultiSample;
	}

	tScreenPlatformBase::tScreenPlatformBase( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts )
		: mCreateOpts( createOpts )
		, mDevice( device )
		, mSceneGraph( sceneGraph )
		, mWorkerThread( NEW tScreenWorkerThread( ) )
		, mWhiteTexture( 0 )
		, mBlackTexture( 0 )
		, mRgbaClear( 0.f )
		, mGlobalFillMode( tRenderState::cGlobalFillDefault )
		, mPreSwapTimer( false )
		, mLimitShadowMapLayerCount( ~0 )
	{
		fCreateFullScreenQuad( );
		fSetupScreenSpaceCamera( );
	}
	void tScreenPlatformBase::fSetupScreenSpaceCamera( )
	{
		// always 1280x720 even if the screen rez is higher
		mScreenSpaceCamera.fSetup(
			Gfx::tLens( 0.0f, 1.f, 0, 1280.0f, 720.0f, 0.0f, Gfx::tLens::cProjectionScreen ),
			Gfx::tTripod( Math::tVec3f::cZeroVector, Math::tVec3f::cZAxis, Math::tVec3f::cYAxis ) );
	}
	tScreenPlatformBase::~tScreenPlatformBase( )
	{
		fShutdown( );
		delete mWorkerThread;
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
		const f32 r = 1280.0f - safeEdge.x;
		const f32 b = 720.0f - safeEdge.y;
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
			mViewports[ i ] = tViewportPtr( NEW tViewport( i ) );
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

	void tScreenPlatformBase::fShutdown( )
	{
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

	void tScreenPlatformBase::fRender( Gui::tCanvas* canvasRoot )
	{
		//fResetPreSwapTimer( );

		// begin scene
		if( !fBeginAllRendering( ) )
			return; // not able to render right now, come back later

		mSceneGraph->fPrepareEntityClouds( mViewports.fFront( )->fRenderCamera( ) );

		// retrieve lists of lights in each viewport, as well as the combined list of all shadow-casting lights
		tDynamicArray< tLightComboList > viewportLightCombos( mViewports.fCount( ) );
		tDynamicArray< tWorldSpaceDisplayList >	viewportDepthDisplayLists( mViewports.fCount( ) );
		tLightEntityList shadowCastingLightList;
		tDynamicArray<tLightEntityList> viewportLightLists;
		fBuildLightLists( viewportLightLists, shadowCastingLightList );
		mWorkerThread->fBeginWork( mSceneGraph.fGetRawPtr( ), &mViewports, &viewportLightLists, &viewportLightCombos, &viewportDepthDisplayLists, !mWorldDepthTexture.fNull( ) );

		// render shadow maps on main thread while worker thread generates display lists
		fRenderShadowMaps( shadowCastingLightList );

		// wait for display list generation to complete
		mWorkerThread->fCompleteWork( );

		// kick off canvas display list generation while we do some other stuff
		if( canvasRoot )
			mWorkerThread->fBeginWork( static_cast<tScreen*>( this ), canvasRoot );

		// render world depth of the world objects
		fRenderWorldDepth( viewportDepthDisplayLists );

		// render the world space objects
		fRenderWorld( viewportLightCombos );

		// world rendering is done, time to apply any post-effects
		fRenderPostEffects( );

		// wait for canvas display list generation to complete
		if( canvasRoot )
			mWorkerThread->fCompleteWork( );

		// render all screen space objects (2D gui stuff)
		fRenderScreenSpace( );

		// end scene, present
		fEndAllRendering( );
	}

	void tScreenPlatformBase::fHandleMultiSampleChange( )
	{
#ifdef sig_devmenu
		if( mCreateOpts.mFullScreen )
			return; // TODO currently we don't support changing multi sample mode while in full screen
		if( Renderer_Settings_MultiSample != fCreateOpts( ).mMultiSamplePower )
		{
			mCreateOpts.mMultiSamplePower = Renderer_Settings_MultiSample;
			fToScreen( ).fResize( mCreateOpts.mBackBufferWidth, mCreateOpts.mBackBufferHeight );
		}
#endif//sig_devmenu
	}

	void tScreenPlatformBase::fBuildLightLists( 
		tDynamicArray<tLightEntityList>&		viewportLightLists, 
		tLightEntityList&						shadowCastingLightList )
	{
		profile( cProfilePerfBuildLightLists );

		// this is the list of all lights from all viewports, but with no duplicates
		tLightEntityList uniqueLightList;

		// these are the lists of lights per viewport
		viewportLightLists.fNewArray( mViewports.fCount( ) );

		// go through each viewport and gets its light list
		for( u32 ivp = 0; ivp < mViewports.fCount( ); ++ivp )
		{
			tViewport& vp = *mViewports[ ivp ];
			if( vp.fIsVirtual( ) || vp.fIsIgnored( ) )
				continue;

			tLightEntityList& vpLightList = viewportLightLists[ ivp ];

			// get list of all lights that are on and intersecting view volume
			mSceneGraph->fIntersect( vp.fRenderCamera( ).fGetWorldSpaceFrustum( ), tBuildLightListSceneGraphCallback( vpLightList ), tLightEntity::cSpatialSetIndex );

			// sort lights by priority
			std::sort( vpLightList.fBegin( ), vpLightList.fEnd( ), tSortLightList( ) );

			// add lights in this viewport's list to the unique light list
			for( u32 ilight = 0; ilight < vpLightList.fCount( ); ++ilight )
				uniqueLightList.fFindOrAdd( vpLightList[ ilight ] );
		}

		// make sure all lights are sync'd up before rendering;
		// at the same time, extract the lights that cast shadow
		for( u32 i = 0; i < uniqueLightList.fCount( ); ++i )
		{
			uniqueLightList[ i ]->fSyncBeforeRender( );

			if( uniqueLightList[ i ]->fCastsShadow( ) )
				shadowCastingLightList.fPushBack( uniqueLightList[ i ] );
		}
	}

	void tScreenPlatformBase::fRenderShadowMaps( tLightEntityList& shadowCastingLightList )
	{
		profile( cProfilePerfRenderShadowMaps );

		mDevice->fSetShaderGPRAllocation( true, false );

		// render each shadow map
		for( u32 i = 0; i < shadowCastingLightList.fCount( ); ++i )
		{
			tLightEntity* shadowLight = shadowCastingLightList[ i ];

			// render shadow map
			shadowLight->fRenderShadowMap( fToScreen( ) );
		}
	}

	void tScreenPlatformBase::fRenderWorldDepth( tDynamicArray<tWorldSpaceDisplayList>& viewportDepthDisplayLists )
	{
		if( !mWorldDepthTexture )
			return;

		mDevice->fSetShaderGPRAllocation( true, false );

		// set shadow map render targets
		mWorldDepthTexture->fApply( fToScreen( ) );

		// apply full viewport clipbox before clearing
		Math::tRect clipBox = fToScreen( ).fCalcViewportClipBox( );
		mWorldDepthTexture->fSetClipBox( fToScreen( ), Math::tVec4f( clipBox.mL, clipBox.mT, clipBox.mR, clipBox.mB ) );

		// clear render targets
		fClearCurrentRenderTargets( true, true, Math::tVec4f(1.f), 1.f, 0 );

		for( u32 i = 0; i < mViewports.fCount( ); ++i )
		{
			tViewport& vp = *mViewports[ i ];

			if( vp.fIsVirtual( ) || vp.fIsIgnored( ) )
				continue;

			// apply the viewport's clip box
			mWorldDepthTexture->fSetClipBox( fToScreen( ), vp.f16_9AdjustedClipBox( ).fConvertToLTRB( ) );

			const tCamera& camera = vp.fRenderCamera( );

			// set default render state for depth rendering
			tRenderContext renderContext;
			renderContext.fFromScreen( fToScreen( ) );
			renderContext.mRenderPassMode = tRenderState::cRenderPassDepth;
			renderContext.mCamera = &camera;
			renderContext.mViewportIndex = vp.fViewportIndex( );

			// invalidate render states
			mDevice->fInvalidateLastRenderState( );

			// render display list, using light camera
			tWorldSpaceDisplayList& displayList = viewportDepthDisplayLists[ i ];
			displayList.fRenderAll( fToScreen( ), renderContext );
		}

		// resolve shadow map texture
		mWorldDepthTexture->fResolve( fToScreen( ) );
	}

	void tScreenPlatformBase::fRenderWorld( tDynamicArray<tLightComboList>& viewportLightCombos )
	{
		profile( cProfilePerfRenderWorld );

		mDevice->fSetShaderGPRAllocation( true, true );

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
		mSceneRenderToTexture->fApply( fToScreen( ) );

		// set full viewport before clearing
		mSceneRenderToTexture->fSetClipBox( fToScreen( ), Math::tVec4f( 0.f, 0.f, 1.f, 1.f ) );

		// clear render targets
		fClearCurrentRenderTargets( true, true, fRgbaClearColor( ), 1.f, 0x0 );

		// render each viewport
		for( u32 i = 0; i < mViewports.fCount( ); ++i )
		{
			tViewport& vp = *mViewports[ i ];

			if( vp.fIsVirtual( ) || vp.fIsIgnored( ) )
				continue;

			mSceneRenderToTexture->fSetClipBox( fToScreen( ), vp.f16_9AdjustedClipBox( ).fConvertToLTRB( ) );
			vp.fRenderLitWorld( fToScreen( ), viewportLightCombos[ i ], mExplicitWorldObjects, mExplicitWorldTopObjects, mWorldStats );
		}

		// clear explicitly enqueued objects
		mExplicitWorldObjects.fSetCount( 0 );
		mExplicitWorldTopObjects.fSetCount( 0 );
	}

	void tScreenPlatformBase::fRenderPostEffects( )
	{
		mDevice->fSetShaderGPRAllocation( false, true );

		tScreen& self = fToScreen( );

		b32 renderedPostEffects = false;

		s32 firstViewport = -1, lastViewport = -1;
		if( mPostEffectMgr )
			mPostEffectMgr->fDetermineFirstAndLastViewportsWithSequence( self, firstViewport, lastViewport );
		if( firstViewport >= 0 )
		{
			sigassert( lastViewport >= 0 );

			// resolve scene render target
			mSceneRenderToTexture->fResolve( self );

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

	Math::tRect tScreenPlatformBase::fCalcViewportClipBox()
	{
		float currentAspectRatio = (float)fCreateOpts().mBackBufferWidth / (float)fCreateOpts().mBackBufferHeight;
		float requiredAspectRatio = 1280.0f / 720.0f;
		float hClip = 0.0f;
		float vClip = 0.0f;
		if (currentAspectRatio > requiredAspectRatio)
		{
			hClip = (1.0f - (requiredAspectRatio / currentAspectRatio)) * 0.5f;
		}
		else
		{
			vClip = (1.0f - (currentAspectRatio / requiredAspectRatio)) * 0.5f;
		}
		return Math::tRect( 0.0f+vClip, 0.0f+hClip, 1.0f-vClip, 1.0f-hClip );
	}

	void tScreenPlatformBase::fRenderScreenSpace( )
	{
		tScreen& me = fToScreen( );

		// no MSAA for screen space gui/hud
		fDisableMSAA( );

		// N.B.! No need to set a render target, we rely on fRenderPostEffects( ) to leave us with a suitable RT.
		// Similarly, we don't resolve after this pass, as the results will be used as the final presentation image.
		Math::tRect clipBox = fCalcViewportClipBox( );
		int renderWidth = mSceneRenderToTexture->fWidth( );
		int renderHeight = mSceneRenderToTexture->fHeight( );
		int x = (int)(clipBox.mL * (f32)renderWidth);
		int y = (int)(clipBox.mT * (f32)renderHeight);
		int w = (int)(clipBox.fWidth( ) * (f32)renderWidth);
		int h = (int)(clipBox.fHeight( ) * (f32)renderHeight);
		fSetViewportClipBox( x, y, w, h );

		fClearCurrentRenderTargets( false, true, 0.f, 1.f, 0x0 );

		// sort the screen space display list
		mScreenSpaceDL.fSeal( );

		// setup render context
		tRenderContext renderContext;
		renderContext.fFromScreen( fToScreen( ) );
		renderContext.mCamera = &mScreenSpaceCamera;
		renderContext.mFogValues = Math::tVec4f( 1000.f, 1000.f, 0.f, 0.f );	//disables fog for screen-space rendereds

		// use the semi-globally available shared white texture as a default shadow map
		tTextureReference whiteShadowMap;
		whiteShadowMap.fSetRaw( ( tTextureReference::tPlatformHandle )fWhiteTexture( ) );
		whiteShadowMap.fSetSamplingModes( tTextureFile::cFilterModeNone, tTextureFile::cAddressModeClamp );

		// setup lights for screen space 3D objects
		tLightEntityList screenSpaceLights; // TODO get these from somewhere
		renderContext.fFromLightGroup( me, screenSpaceLights, &whiteShadowMap );

		// invalidate render states
		fGetDevice( )->fInvalidateLastRenderState( );

		// render all screen draw calls
		mScreenSpaceDL.fRender( fToScreen( ), renderContext );

		// cache screen space display list stats, invalidate screen space display list
		mScreenStats.fReset( );
		mScreenStats.fCombine( mScreenSpaceDL.fGetStats( ) );
		mScreenSpaceDL.fInvalidate( );

		// re-enable MSAA if there was any
		fReEnableMSAA( );

		// these render on top, after everything else
		for( u32 i = 0; i < mMoviePlayers.fCount( ); ++i )
			mMoviePlayers[ i ]->fRenderFrame( *mDevice );
		mMoviePlayers.fSetCount( 0 );

		// clear borders...
		if (w < renderWidth)
		{
			// vert strips
			fSetViewportClipBox( 0, y, x, h );
			fClearCurrentRenderTargets( false, true, 0.f, 1.f, 0x0 );
			fSetViewportClipBox( renderWidth-x, y, x, h );
			fClearCurrentRenderTargets( false, true, 0.f, 1.f, 0x0 );
		}
		else if (h < renderHeight)
		{
			// horiz strips
			fSetViewportClipBox( x, 0, w, y );
			fClearCurrentRenderTargets( true, false, 0.f, 1.f, 0x0 );
			fSetViewportClipBox( x, renderHeight-y, w, y );
			fClearCurrentRenderTargets( true, false, 0.f, 1.f, 0x0 );
		}


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


	void tScreenPlatformBase::fCreateFullScreenQuad( )
	{
		//
		// create vertex buffer: four verts for two triangles
		//
		tPostEffectsRenderVertex sysMemVerts[] =
		{
			tPostEffectsRenderVertex( Math::tVec2f( 0.f, 0.f ) ),
			tPostEffectsRenderVertex( Math::tVec2f( 0.f, 1.f ) ),
			tPostEffectsRenderVertex( Math::tVec2f( 1.f, 1.f ) ),
			tPostEffectsRenderVertex( Math::tVec2f( 1.f, 0.f ) ),
		};
		mFullScreenQuadVB.fAllocate( mDevice, tPostEffectsMaterial::cVertexFormat, 4, 0 );
		mFullScreenQuadVB.fBufferData( sysMemVerts, array_length( sysMemVerts ) );
			
		//
		// create index buffer: six indices for two indexed triangles
		//
		u16 sysMemIds[] =
		{
			0, 1, 2,
			2, 0, 3,
		};
		mFullScreenQuadIB.fAllocate( mDevice, tIndexFormat( tIndexFormat::cStorageU16, tIndexFormat::cPrimitiveTriangleList ), 6, 2, 0 );
		mFullScreenQuadIB.fBufferData( sysMemIds, array_length( sysMemIds ) );
	}

}}

