#include "BasePch.hpp"
#include "tGameAppBase.hpp"
#include "Memory/tMainMemoryProvider.hpp"
#include "Memory/tResourceMemoryProvider.hpp"
#include "Memory/tExternalHeap.hpp"
#include "tCmdLineOption.hpp"
#include "tDataTableFile.hpp"
#include "Scripts/tScriptFile.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tDebugFreeCamera.hpp"
#include "FX/tFxFile.hpp"
#include "tFxFileRefEntity.hpp"
#include "FileSystem.hpp"
#include "tProfiler.hpp"
#include "AI/tSigAIGoal.hpp"
#include "tSync.hpp"
#include "memory/tHeap.hpp"
#include "gfx/tMaterialFile.hpp"
#include "memory/DebugMemory.hpp"
#include "Physics/tPhysicsWorld.hpp"
#include "Gui/tFont.hpp"
#include "tTtfFile.hpp"
#include "Gfx/tGroundCoverCloud.hpp"
#include "Fui.hpp"
#include "tGameMode.hpp"
#include "tXmlFile.hpp"
#include "tGameEffects.hpp"
#include "tProjectFile.hpp"
#include "tPlatformDebugging.hpp"
#include "Allocators.hpp"

namespace Sig
{
	devvar( bool, Debug_Watcher_Visible, false );
#if defined( build_debug ) || defined( build_internal )
	devvar( bool, Resources_AutoReload, true );
#else
	devvar( bool, Resources_AutoReload, false );
#endif
	devvar( bool, Renderer_Settings_FullScreen, false );
	devvar( u32, Renderer_Settings_DefaultFullScreenResX, 1280 );
	devvar( u32, Renderer_Settings_DefaultFullScreenResY, 720 );
	devvar( bool, Debug_Memory_DumpMeshesAndTextures, false );
	devvar( bool, Debug_Memory_DumpAnims, false );
	devvar( bool, Debug_Memory_DumpWithTrail, false );
	devvar( bool, Debug_Memory_DisplayMainMemPooled, false );
	devvar( bool, Debug_Stats_Render, false );
	devvar( bool, Game_HideHudRoot, false );
	devvar_clamp( f32, Debug_Stats_PositionX, 20.f, 0.f, 1280.f, 0 );
	devvar_clamp( f32, Debug_Warn_PositionX, 400.0f, 0.f, 1280.f, 0 );
	devvar_clamp( f32, Debug_Warn_PositionY, 666.f, 0.f, 720.f, 0 );
	devvar( bool, Debug_Warn_Enable, true );
	devrgba_clamp( Debug_Stats_TextColor, Math::tVec4f( 0.75f, 0.75f, 1.f, 0.75f ), 0.f, 1.f );

	devvar( bool, Geometry_Memory_NoLimit, false );
	devvar( u32, Geometry_Memory_Target, 35 );
	devvar( u32, Geometry_Memory_Scratch, 5 );

	devvar( bool, Hack_QuitBeforeCrash, true );

#ifdef sig_devmenu
	void fAppMemoryDump( tDevCallback::tArgs& args )
	{
		tGameAppBase::fInstance( ).fDumpMemory( "DevMenu", false, NULL );
	}
	void fAppMemoryDumpLog( tDevCallback::tArgs& args )
	{
		tGameAppBase::fInstance( ).fDumpMemory( "DevMenu", true, NULL );
	}
	void fAAACheatsReadLicense( tDevCallback::tArgs& args )
	{
		tGameAppBase::fInstance( ).fReadLicense( );
	}
#endif//sig_devmenu
	devcb( Debug_Memory_HeapDump, "0", make_delegate_cfn( tDevCallback::tFunction, fAppMemoryDump ) );
	devcb( Debug_Memory_HeapDumpJustLog, "0", make_delegate_cfn( tDevCallback::tFunction, fAppMemoryDumpLog ) );
	devcb( AAACheats_ReadLicense, "0", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsReadLicense ) );

	namespace
	{
		static const f32 cWorldCanvasZPos   = 0.9f;
		static const f32 cRootHudCanvasZPos = 0.7f;
		static const f32 cLoadingCanvasZPos = 0.5f;
		static const f32 cWarningCanvasZPos = 0.3f;
		static const f32 cStatusCanvasZPos  = 0.1f;
	}

	tGameAppBase* tGameAppBase::gGameAppBase=0;

	//------------------------------------------------------------------------------
	// 
	//------------------------------------------------------------------------------
	tGameAppBase::tGameAppBase( )
		: mIsFullVersion( false )
		, mSimulate( true )
		, mInGameSimulationState( false )
// debug runs too slow to maintain a good frame rate, but at the same time 
// we want to be able to debug "real" situations (i.e. at target frame rate)
#ifdef build_debug
		, mFixedTimeStep( true )
#else
		, mFixedTimeStep( false )
#endif
		, mBuildStamp( 0 )
		, mLastLevelLoadTime( 0.f )
        , mSoundSystem( 0 )
	{
		gGameAppBase = this;
		Allocators::fInit();

#ifdef sig_devmenu
		Log::fAddOutputFunction( fCaptureWarnings );
#endif // sig_devmenu
	}

	//------------------------------------------------------------------------------
	tGameAppBase::~tGameAppBase( )
	{
		Allocators::fShutdown();
	}

	//------------------------------------------------------------------------------
	u64 tGameAppBase::fComputeBuildStamp( ) const
	{
		u64 timestamp = 0;

#ifdef sig_devmenu
		Time::tStopWatch thisCheck;

		const tFilePathPtr buildStampPath = tFilePathPtr::fConstructPath( mGameRoot, tFilePathPtr( "zzz.buildstamp" ) );
		timestamp =  FileSystem::fGetLastModifiedTimeStamp( buildStampPath );

		const f32 t = thisCheck.fGetElapsedMs( );
		const f32 msPerFrame = 1000/30.0f;
		if( t > msPerFrame )
			log_warning_nospam_time( "Performance warning: Resources_AutoReload enabled, took " << (int)t << "ms / " << (int)(t/msPerFrame) << " entire frames!", 60, +30 );
#endif//sig_devmenu

		return timestamp;
	}

	//------------------------------------------------------------------------------
	b32 tGameAppBase::fShouldDoSingleStep( ) const
	{
#ifdef sig_devmenu
		if( !fSceneGraph( )->fIsPaused( ) )
			return false;
		for( u32 i = 0; i < fLocalUsers( ).fCount( ); ++i )
		{
			if( fLocalUsers( )[ i ]->fIsLocal( ) )
			{
				const Input::tGamepad& gp = fLocalUsers( )[ i ]->fRawGamepad( );
				if( gp.fButtonHeld( Input::tGamepad::cButtonSelect ) && gp.fButtonDown( Input::tGamepad::cButtonRShoulder ) )
					return true;
			}
		}
#endif//sig_devmenu
		return false;
	}

	//------------------------------------------------------------------------------
	void tGameAppBase::fCaptureWarnings( const char * msg, u32 flag )
	{
#ifdef sig_devmenu

		if( !Debug_Warn_Enable )
			return;

		if( !StringUtil::fStrStrI( msg, "WARNING" ) )
			return;

		const f32 cWarningLife = 1.f;
		std::string finalMsg = StringUtil::fTrim( msg, "\n", false, true );

		// First we test if the warning is spam
		if( fInstance( ).mWarningLog.fCount( ) && fInstance( ).mWarningLog.fBack( ).mWarning == finalMsg )
		{
			fInstance( ).mWarningLog.fBack( ).mCount += 1;
			fInstance( ).mWarningLog.fBack( ).mLifeLeft = cWarningLife;
		}
		else
		{

			tWarningMessage warnMsg; 
			warnMsg.mCount = 1;
			warnMsg.mLifeLeft = cWarningLife; // How long to force display of this error before moving along
			warnMsg.mWarning = finalMsg;
				
			fInstance( ).mWarningLog.fPushBack( warnMsg );
		}

#endif // sig_devmenu
	}

	//------------------------------------------------------------------------------
	devvar_clamp( f32, Debug_Time_Factor, 1.0f, 0.01f, 10.0f, 2 );
	devvar_clamp( u32, Debug_Time_SleepPerFrame, 0, 0, 1000, 0 );

	void tGameAppBase::fTickApp( )
	{
		profile_pix("tGameAppBase::fTickApp");
		b32 fixedTimeStep = mFixedTimeStep;

#ifdef sig_devmenu
		const b32 singleStep = fShouldDoSingleStep( );
		if( singleStep || fScreen( )->fCapturing( ) )
			fixedTimeStep = true;
		if( singleStep )
			fSceneGraph( )->fPauseNextFrame( false );
		if( Debug_Time_SleepPerFrame > 0 )
			fSleep( Debug_Time_SleepPerFrame );
#endif//sig_devmenu

		// run the simulation
		const f32 baseDt = fixedTimeStep ? fGetOptions( ).mFixedTimeStep : fMin( 1 / 12.f, fGetFrameDeltaTime( ) );
		const f32 dt = baseDt * Debug_Time_Factor;

#ifdef sig_devmenu
		// Update our warning logs
		for( u32 w = 0; w < mWarningLog.fCount( ); ++w )
		{
			tWarningMessage & msg = mWarningLog[ w ];
			if( msg.mLifeLeft > 0.f )
				msg.mLifeLeft -= dt;
		}
		if( mWarningLog.fCount( ) > 1024 )
			mWarningLog.fEraseOrdered( 0, mWarningLog.fCount( ) - 1024 );
		while( mWarningLog.fCount( ) > cMaxWarningMsgs )
		{
			if( mWarningLog.fFront( ).mLifeLeft < 0 ) mWarningLog.fPopFront( );
			else break;
		}
#endif//sig_devmenu

#if defined( platform_debugging_enabled )
		tPlatformDebugger::fInstance( ).fDoWork( );
		tPlatformDebugger::fInstance( ).fExecuteButtonInterceptsForFrame( );
#endif//#if defined( platform_debugging_enabled )

		if( mSimulate )
		{
			sync_event_v( dt );
			sync_d_event_v( tRandom::fObjectiveRand( ).fState( ) );

			fSceneGraph( )->fAdvanceTime( fModifySceneGraphDT( dt ) );
			fTickPlayers( dt );
			tGameEffects::fInstance( ).fTickST( dt );
		}

		fTickRender( dt );
		fTickAudio( dt );
		fAutoReloadResources( );

		tScriptVm::fInstance( ).fLogStackTop( "tGameAppBase::fTickApp", 1 );

#ifdef sig_devmenu
		if( singleStep )
			fSceneGraph( )->fPauseNextFrame( true );
#endif//sig_devmenu

	}

	//------------------------------------------------------------------------------
	void tGameAppBase::fStartupApp( )
	{
		mBuildStamp = fComputeBuildStamp( );

		fPreLoadFilePackages( );
		fStartupProjectSettings( );
		fStartupScript( );
		fStartupSceneGraph( );
		fStartupGfx( );
		fStartupProfiler( );
		fStartupSfx( );
		fSetupPostEffects( ); //these will block
		fLoadInitalizationGameResources( );
		fLoadPermaLoadedGameResources( );
		fStartupUsers( );

		// configure exclusive threads
		//  don't run logic on the Audio thread
		mSceneGraph->fDisallowLogicThread( mStartupParams.mAudioOptions.mThreadIndex );

#if defined( platform_debugging_enabled )
		// Start listening for debug connections
		tPlatformDebugger::fInstance( );
#endif

		// determine command line options
		const std::string cmdLineBuffer = fGetCmdLine( );
		const tCmdLineOption previewMode( "preview", cmdLineBuffer );
		const tCmdLineOption replayMode( "replay", cmdLineBuffer );

		if( previewMode.fFound( ) )
		{
			const tFilePathPtr previewPath = tFilePathPtr( previewMode.fGetTypedOption<std::string>( ) );
			log_line( 0, "Running PREVIEW on file [" << previewPath << "]" );
			fPreview( previewPath );
		}
		else if( replayMode.fFound( ) )
		{
			tFilePathPtr replayPath( replayMode.fGetTypedOption<std::string>( ) );
			if( !replayPath.fLength( ) )
			{
				log_line( 0, "No replay file specified searching for lone file" );

				tFilePathPtrList list;
				FileSystem::fGetFileNamesInFolder( list, mGameRoot, true, false, tFilePathPtr( ".rply" ) );

				if( list.fCount( ) == 1 )
					replayPath = list[ 0 ];
				else
					log_warning( "No valid replay file could be found" );
			}

			log_line( 0, "Running REPLAY on file [" << replayPath << "]" );
			fReplay( replayPath );
		}
		else
		{
			// run for rillz, with front end, logo screens, etc.
			fStartGame( );
		}
	}
	void tGameAppBase::fShutdownApp( )
	{
		if( Hack_QuitBeforeCrash )
			_exit( 0 );

		fRootCanvas( ).fClearChildren( );
		mRootHudCanvas = Gui::tCanvasPtr( );
		mLoadingCanvas = Gui::tCanvasPtr( );
		mWarningCanvas = Gui::tCanvasPtr( );
		mStatusCanvas = Gui::tCanvasPtr( );
		mSceneGraph->fClear( );
		mSceneGraph.fRelease( );
		mAppState.fRelease( );

		for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
			mLocalUsers[ i ]->fSetViewport( Gfx::tScreenPtr( ), Gfx::tViewportPtr( ) );
		mLocalUsers.fSetCount( 0 );

#ifdef sig_devmenu
		mFpsText.fRelease( );
		mStatsText.fRelease( );
		mDebugTextRightSide.fRelease( );
		mWarningText.fRelease( );
#endif // sig_devmenu

		Gfx::tDefaultAllocators& defAllocators = Gfx::tDefaultAllocators::fInstance( );
		defAllocators.fDeallocate( );
		defAllocators.fUnloadMaterials( this );
		defAllocators = Gfx::tDefaultAllocators( );

		mScreen->fShutdown( );
		mScreen.fRelease( );

		tScriptVm::fInstance( ).fGarbageCollect( );
		tScriptVm::fInstance( ).fReset( );

		Fui::tFuiSystem::fInstance( ).fShutdown( );

		mDevice.fRelease( );
		Gfx::tDevice::fSetDefaultDevice( Gfx::tDevicePtr( ) );

		mSoundSystem.fRelease( );
		Audio::tSystem::fSetDefaultSystem( Audio::tSystemPtr( ) );

		const u32 globalEntCount = tEntity::fGlobalEntityCount( );
		log_line( 0, globalEntCount << " entities left in tGameAppBase::fShutdownApp." );
	}

	void tGameAppBase::fChangeViewportCount( 
		u32 newCount, u32 localUserCount, b32 updateUsers, tUserArray * remoteUsers )
	{
		// create viewports
		mScreen->fSetViewportCount( newCount );

		// update sound system
		if( mSoundSystem )
			mSoundSystem->fSetListenerMask( ( 1 << localUserCount ) - 1 );

		if( updateUsers )
		{
			// update existing users
			u32 localFound = 0;
			const u32 localCount = mLocalUsers.fCount( );
			for( u32 i = 0; i < localCount && localFound < localUserCount; ++i )
			{
				tUserPtr user = mLocalUsers[ i ];
				if( !user->fIsUsedByGame( ) )
					continue;

				const s32 vpId = fClamp<s32>( localFound, 0, mScreen->fGetViewportCount( ) - 1 );
				Gfx::tViewportPtr vp = mScreen->fViewport( vpId );

				// Set it if we own the index, it may be passed to other local users
				// without it getting set
				if( localFound == vpId )
					vp->fSetIsVirtual( false );

				user->fSetViewport( mScreen, vp );
				++localFound;
			}

			sigassert( localFound == localUserCount && "Not enough used local users for user viewport setup" );

			u32 remoteFound = 0;
			const u32 remoteCount = remoteUsers ? remoteUsers->fCount( ) : 0;
			for( u32 i = 0; i < remoteCount; ++i )
			{
				tUserPtr user = (*remoteUsers)[ i ];
				sigassert( user && "Null remote users are not allowed" );

				if( !user->fIsUsedByGame( ) )
					continue;

				const s32 vpId = fClamp<s32>( remoteFound + localUserCount, 0, mScreen->fGetViewportCount( ) - 1 );
				Gfx::tViewportPtr vp = mScreen->fViewport( vpId );

				// If the remote user owns the index, then make sure to set the
				// viewport as virtual or shit will hit the fan
				if( vpId == remoteFound + localUserCount )
					vp->fSetIsVirtual( true );

				user->fSetViewport( mScreen, vp );
				++remoteFound;
			}
		}
	}

	void tGameAppBase::fResetViewportCameras( )
	{
		Gfx::tCamera defaultCamera;
		const u32 viewportCount = mScreen->fGetViewportCount( );
		for( u32 v = 0; v < viewportCount; ++v )
			mScreen->fViewport( v )->fSetCameras( defaultCamera );
	}

	Gui::tCanvasPtr& tGameAppBase::fHudLayer( const std::string& layerName )
	{
		if( layerName == "" )
			return fRootHudCanvas( );

		Gui::tCanvasPtr* findResult = mHudLayers.fFind( layerName );
		if( findResult )
			return *findResult;
		else
		{
			Gui::tCanvasPtr& newLayer = *mHudLayers.fInsert( layerName, Gui::tCanvasPtr( Sqrat::Object( NEW Gui::tCanvasFrame( ) ) ) );
			fRootHudCanvas( ).fToCanvasFrame( ).fAddChild( newLayer );
			return newLayer;
		}
	}
	Sqrat::Object tGameAppBase::fHudLayerFromScript( const std::string& layerName )
	{
		return fHudLayer( layerName ).fScriptObject( );
	}
	void tGameAppBase::fStartupProjectSettings( )
	{
		mProjectXml = fAddAlwaysLoadedResource( tResourceId::fMake<tXmlFile>( tFilePathPtr( "Project.xml" ) ), true );
		sigassert( mProjectXml && "Project.xml needs to be loaded!" );

		tXmlFile* file = mProjectXml->fCast<tXmlFile>( );

		tProjectFile::fInstance( ).fLoadXml( file->fBegin( ) );

		Gfx::tLightEntity::fSetupShadowMapping( );
		Gfx::tRenderableEntity::fSetProjectLODEnable( tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mEnableLOD );
	}
	void tGameAppBase::fTryLoadPackage( const tFilePathPtr& root, const tFilePathPtr& name )
	{
		const tFilePathPtr packageName = tFilePathPtr::fConstructPath( root, name );
		if( FileSystem::fFileExists( packageName ) )
		{
			tFilePackagePtr package( NEW tFilePackage );
			package->fLoad( this, *fResourceDepot( ), name );
			package->fBlockUntilLoaded( );
			if( package->fLoaded( ) )
			{
				mPermaLoadedFilePackages.fPushBack( package );
				mFilePackageProvider->fAddPackage( *package );

				log_line( 0, "               (((((                               )))))" );
				log_line( 0, "***************(((((    running from '" << packageName << "'    )))))***************" );
				log_line( 0, "               (((((                               )))))" );
			}
		}
	}
	void tGameAppBase::fStartupScript( )
	{
		tScriptVm& vm = tScriptVm::fInstance( );
		vm.fExportCommonScriptInterfaces( );
		fExportGameScriptInterfaces( vm );
		tScriptVm::fInstance( ).fLogStackTop( "tGameAppBase::fStartupScript", 1 );
	}
	void tGameAppBase::fStartupSceneGraph( )
	{
		// create scene graph
#ifdef sig_usequadtree
		log_line( 0, "Using QUADTREE" );
		fSceneGraph( )->fSetSpatialBounds( mStartupParams.mWorldHalfAxisLength, 0.f, 0.f, 0.f, 6 );
#else//sig_usequadtree
		log_line( 0, "Using OCTREE" );
		fSceneGraph( )->fSetSpatialBounds( mStartupParams.mWorldHalfAxisLength, 0.f, mStartupParams.mWorldHalfAxisLength / 2.f + 50.f, 0.f );
#endif//sig_usequadtree
	}
	void tGameAppBase::fStartupGfx( )
	{
		// setup screen creation params
		Gfx::tScreenCreationOptions screenCreateOpts;
		screenCreateOpts.mDefaultClearColor = mStartupParams.mRgbaClearColor;
		screenCreateOpts.mResourceDepot = mResourceDepot;

		fFillOutScreenCreateOpts( screenCreateOpts );
		sigassert( screenCreateOpts.mShadowMapLayerCount <= Gfx::tMaterial::cMaxShadowLayers );

		fCreateDevice( screenCreateOpts );
		sigassert( mDevice );
		Gfx::tDevice::fSetDefaultDevice( mDevice );
		screenCreateOpts.mAutoDepthStencil = mDevice->fSingleScreenDevice( );
		mScreen.fReset( NEW Gfx::tScreen( mDevice, fSceneGraph( ), screenCreateOpts ) );
		mSceneGraph->fSetScreen( mScreen.fGetRawPtr( ) );
		
		Gfx::tLens lens;
		lens.fSetScreen( 0.0f, 1.f, 0.f, ( f32 )screenCreateOpts.mBackBufferWidth, ( f32 )screenCreateOpts.mBackBufferHeight, 0.f );
		mScreen->fGetScreenSpaceCamera( ).fSetup( lens, Gfx::tTripod( Math::tVec3f::cZeroVector, Math::tVec3f::cZAxis, Math::tVec3f::cYAxis ) );

		// setup debug geometry
		fSceneGraph( )->fDebugGeometry( ).fResetDeviceObjects( *fResourceDepot( ), mDevice );

		mPostEffectsMaterialFile = fAddAlwaysLoadedResource( tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tPostEffectsMaterial::fMaterialFilePath( ) ), true );

		// load default font (block until its loaded)
		mDefaultFont = fAddAlwaysLoadedResource( tResourceId::fMake<Gui::tFont>( Gui::tFont::fDevFontPath( ) ), true );

#ifdef sig_devmenu
		mFpsText.fReset( NEW_TYPED( Gui::tFpsText )( ) );
		mFpsText->fSetFont( mDefaultFont );
		
		mStatsText.fReset( NEW_TYPED( Gui::tText )( ) );
		mStatsText->fSetFont( mDefaultFont );
		
		mDebugTextRightSide.fReset( NEW_TYPED( Gui::tText )( ) );
		mDebugTextRightSide->fSetFont( mDefaultFont );
		
		mWarningText.fReset( NEW_TYPED( Gui::tText )( ) );
		mWarningText->fSetFont( mDefaultFont );
#endif // sig_devmenu

		// show the window now that gfx is ready
		tApplication::fShowWindow( screenCreateOpts.mBackBufferWidth, screenCreateOpts.mBackBufferHeight );

		// create viewports
		fChangeViewportCount( mStartupParams.mViewportCount, mStartupParams.mViewportCount, false );

		// create/add HUD root
		Gui::tCanvasFrame* canvasFrame;

		fScreen( )->fWorldSpaceCanvas( ).fSetZPos( cWorldCanvasZPos );

		canvasFrame = NEW Gui::tCanvasFrame( );
		canvasFrame->fSetZPos( cRootHudCanvasZPos );
		mRootHudCanvas = Gui::tCanvasPtr( Sqrat::Object( canvasFrame ) );
		fRootCanvas( ).fAddChild( mRootHudCanvas );

		canvasFrame = NEW Gui::tCanvasFrame( );
		canvasFrame->fSetZPos( cLoadingCanvasZPos );
		mLoadingCanvas = Gui::tCanvasPtr( Sqrat::Object( canvasFrame ) );
		fRootCanvas( ).fAddChild( mLoadingCanvas );

		canvasFrame = NEW Gui::tCanvasFrame( );
		canvasFrame->fSetZPos( cWarningCanvasZPos );
		mWarningCanvas = Gui::tCanvasPtr( Sqrat::Object( canvasFrame ) );
		fRootCanvas( ).fAddChild( mWarningCanvas );

		canvasFrame = NEW Gui::tCanvasFrame( );
		canvasFrame->fSetZPos( cStatusCanvasZPos );
		mStatusCanvas = Gui::tCanvasPtr( Sqrat::Object( canvasFrame ) );
		fRootCanvas( ).fAddChild( mStatusCanvas );
		
		//create Flash UI system
		Fui::tFuiSystem::fInstance( ).fInit( mDevice, mStartupParams.mFuiOptions );
	}
	void tGameAppBase::fStartupProfiler( )
	{
		profile_init( *mScreen );
	}
	void tGameAppBase::fCreateDevice( Gfx::tScreenCreationOptions& screenCreateOpts )
	{
#if defined( platform_xbox360 )
		screenCreateOpts.mWindowHandle = fGetWindowHandleGeneric( );
		screenCreateOpts.mVsync = Gfx::VSYNC_30HZ;
		screenCreateOpts.mMultiSamplePower = Gfx::tScreen::fDefaultMultiSamplePower( );
		Gfx::tDisplayModeList displayModes;
		Gfx::tDevice::fEnumerateDisplayModes( displayModes );
		const u32 bestMatch = displayModes.fFindClosestMatch( Gfx::tDisplayMode( Renderer_Settings_DefaultFullScreenResX, Renderer_Settings_DefaultFullScreenResY ) );
		sigassert( bestMatch < displayModes.fCount( ) );

		screenCreateOpts.mFullScreen = true;
		screenCreateOpts.mBackBufferWidth = displayModes[ bestMatch ].mBackBufferWidth;
		screenCreateOpts.mBackBufferHeight = displayModes[ bestMatch ].mBackBufferHeight;
		screenCreateOpts.mAutoDepthStencil = true;
		mDevice.fReset( NEW Gfx::tDevice( screenCreateOpts ) );
#elif defined( platform_ios )
		screenCreateOpts.mWindowHandle = fGetWindowHandleGeneric( );
		screenCreateOpts.mVsync = Gfx::VSYNC_30HZ;
		screenCreateOpts.mMultiSamplePower = Gfx::tScreen::fDefaultMultiSamplePower( );
		Gfx::tDisplayModeList displayModes;
		Gfx::tDevice::fEnumerateDisplayModes( displayModes );
		const u32 bestMatch = displayModes.fFindClosestMatch( Gfx::tDisplayMode( Renderer_Settings_DefaultFullScreenResX, Renderer_Settings_DefaultFullScreenResY ) );
		sigassert( bestMatch < displayModes.fCount( ) );
		
		screenCreateOpts.mFullScreen = true;
		screenCreateOpts.mBackBufferWidth = displayModes[ bestMatch ].mBackBufferWidth;
		screenCreateOpts.mBackBufferHeight = displayModes[ bestMatch ].mBackBufferHeight;
		screenCreateOpts.mAutoDepthStencil = true;
		mDevice.fReset( NEW Gfx::tDevice( screenCreateOpts ) );
#elif defined( platform_pcdx9 ) || defined( platform_pcdx10 )
		screenCreateOpts.mWindowHandle = fGetWindowHandleGeneric( );
		screenCreateOpts.mVsync = Gfx::VSYNC_60HZ;
		screenCreateOpts.mMultiSamplePower = Gfx::tScreen::fDefaultMultiSamplePower( );
		screenCreateOpts.mAutoDepthStencil = true;
		screenCreateOpts.mAllocatorSettings = mStartupParams.mMemory;

		if( Renderer_Settings_FullScreen )
		{
			Gfx::tDisplayModeList displayModes;
			Gfx::tDevice::fEnumerateDisplayModes( displayModes );
			const u32 bestMatch = displayModes.fFindClosestMatch( Gfx::tDisplayMode( Renderer_Settings_DefaultFullScreenResX, Renderer_Settings_DefaultFullScreenResY ) );
			sigassert( bestMatch < displayModes.fCount( ) );

			screenCreateOpts.mFullScreen = true;
			screenCreateOpts.mBackBufferWidth = displayModes[ bestMatch ].mBackBufferWidth;
			screenCreateOpts.mBackBufferHeight = displayModes[ bestMatch ].mBackBufferHeight;
			mDevice.fReset( NEW Gfx::tDevice( screenCreateOpts ) );
		}
		else
		{
			screenCreateOpts.mFullScreen = false;
			screenCreateOpts.mBackBufferWidth = mStartupParams.mScreenWidth;
			screenCreateOpts.mBackBufferHeight = mStartupParams.mScreenHeight;
			mDevice.fReset( NEW Gfx::tDevice( screenCreateOpts ) );
		}
#endif// defined( platform_xbox360 )
	}
	void tGameAppBase::fStartupUsers( )
	{
		//fReadLicense( );

		// create the user objects and assign viewport(s) to the user(s);
		// this code will handle the case of multiple users being assigned to a single viewport,
		// as well as a 1-1 correspondence bewteen users and viewports (i.e., 2 users, 2 viewports);
		// any other cases probably need to be handled by custom game code
		mLocalUsers.fSetCount( tUser::cMaxLocalUsers );
		for( u32 i = 0; i < tUser::cMaxLocalUsers; ++i )
		{
			const s32 viewportId = fMax( 0, fMin<s32>( mScreen->fGetViewportCount( ) - 1, i ) );
			tUser * user = NEW tUser( mScreen->fCreateOpts( ).mWindowHandle, i );
			user->fSetViewport( mScreen, mScreen->fViewport( viewportId ) );
			user->fDevMenu( ).fInitDevMenu( mDefaultFont );
			user->fDevMenu( ).fSetDevMenuStart( fGetDevMenuStart() );

			mLocalUsers[ i ].fReset( user  );
		}

	}
	Math::tRect tGameAppBase::fViewportRect( u32 ithViewport )
	{
		if( mScreen && ithViewport < mScreen->fGetViewportCount( ) )
		{
			const Gfx::tViewportPtr& vp = mScreen->fViewport( ithViewport );
			return vp->fComputeRect( );
		}
		
		return Math::tRect( );
	}
	Math::tRect tGameAppBase::fViewportSafeRect( u32 ithViewport )
	{
		if( mScreen && ithViewport < mScreen->fGetViewportCount( ) )
		{
			const Gfx::tViewportPtr& vp = mScreen->fViewport( ithViewport );
			return vp->fComputeSafeRect( );
		}

		return Math::tRect( );
	}
	b32 tGameAppBase::fLocStringExists( const tStringPtr& stringId ) const
	{
		for( u32 i = 0; i < mLocTextFileList.fCount(); ++i )
		{
			const tLocalizationFile* locOverrideFile = mLocTextFileList[i]->fCast< tLocalizationFile >( );
			sigcheckfail( locOverrideFile, continue );
			if( locOverrideFile->fStringIndexFromId( stringId ) >= 0 )
				return true;
		}
		return false;
	}
	const tLocalizedString& tGameAppBase::fLocString( const tStringPtr& stringId ) const
	{
		for( s32 i = mLocTextFileList.fCount() - 1; i >= 0; --i )
		{
			const tLocalizationFile* locOverrideFile = mLocTextFileList[i]->fCast< tLocalizationFile >( );
			sigcheckfail( locOverrideFile, continue );
			if( i == 0 || locOverrideFile->fStringIndexFromId( stringId ) >= 0 )
				return locOverrideFile->fStringFromId( stringId );
		}
		return tLocalizedString::fEmptyString();
	}
	void tGameAppBase::fCreateLocOverride( tResourceLoadList2* rll )
	{
		const tResourceId rid = tResourceId::fMake< tLocalizationFile >( tFilePathPtr::fConstructPath( mLocFolder, tFilePathPtr( "text_override.locml" ) ) );
		if( rll )
		{
			rll->fAdd( rid );
		}
		else
		{
			mLocTextFileList.fPushBack( fResourceDepot( )->fQuery( rid ) );
		}
	}
	void tGameAppBase::fClearLocOverride( )
	{
		mLocTextFileList.fResize( 0 );
		fCreatePreloadLocTextResources( mLocFolder );
	}

	void tGameAppBase::fCreatePreloadLocTextResources( const tFilePathPtr& locFolder )
	{
		tFilePathPtrList files;

		std::stringstream ss;
		for( u32 i = 0; true; ++i )
		{
			ss.str( "" );
			
			ss << "text";
			if( i ) ss << "_" << i;
			ss << ".locml";

			const tResourceId resId = tResourceId::fMake< tLocalizationFile >( 
				tFilePathPtr::fConstructPath( locFolder, tFilePathPtr( ss.str( ) ) ) );

			if( !mResourceDepot->fResourceExists( resId.fGetPath( ) ) )
				break;
			
			mLocTextFileList.fPushBack( fAddAlwaysLoadedResource( resId, true ) );
		}
	}
	void tGameAppBase::fLoadLocResources( const tFilePathPtr& locFolder, const tDynamicArray<tStringPtr>& fontNames )
	{
		mLocFolder = locFolder;
		fCreatePreloadLocTextResources( locFolder );
		mLocResFile = fAddAlwaysLoadedResource( tResourceId::fMake< tLocalizationFile >( tFilePathPtr::fConstructPath( locFolder, tFilePathPtr( "res.locml" ) ) ), true );

		mFonts.fNewArray( fontNames.fCount( ) );

		// get fonts from loc file
		const tLocalizationFile* resLocFile = mLocResFile->fCast< tLocalizationFile >( );
		sigassert( resLocFile );
		for( u32 i = 0; i < resLocFile->fPathCount( ); ++i )
		{
			const tFilePathPtr fontPath = resLocFile->fPathFromIndex( i );
			
			u32 fontType = ~0;
			const tStringPtr pathId = resLocFile->fPathIdFromIndex( i );

			for( u32 j = 0; j < fontNames.fCount( ); ++j )
			{
				if( pathId == fontNames[ j ] )
				{
					fontType = j;
					break;
				}
			}

			if( fontType != ~0 )
			{
				if( StringUtil::fCheckExtension( fontPath.fCStr( ), ".fntml" ) )
				{
					log_line( 0, "Loading gui font: index = " << fontType << ", path = " << fontPath );
					mFonts[ fontType ] = fAddAlwaysLoadedResource( tResourceId::fMake< Gui::tFont >( fontPath ), true );
				}
				else if( tTtfFile::fIsOperablePath( fontPath ) )
				{
					log_line( 0, "Loading fui font: index = " << fontType << ", path = " << fontPath );
					mFonts[ fontType ] = fAddAlwaysLoadedResource( tResourceId::fMake< tTtfFile >( fontPath ), true );
					if( mFonts[ fontType ] )
					{
						if( const tTtfFile* file = mFonts[ fontType ]->fCast< const tTtfFile >( ) )
						{
							Fui::tFuiSystem::fInstance( ).fInstallTrueTypeFont
								( fontNames[ fontType ]
								, (void*)file->fData( ).fBegin( )
								, Fui::tFuiSystem::cFontFlag_None );
						}
					}
				}
			}
		}

		// set font mapping callback to facilitate use of text objects from script
		Gfx::tDefaultAllocators& defGuiAllocators = Gfx::tDefaultAllocators::fInstance( );
		defGuiAllocators.mFontFromId = &fFontFromId;
	}
	void tGameAppBase::fTickRender( f32 dt )
	{
		profile_pix("tGameAppBase::fTickRender");
#ifdef sig_devmenu
		fPrintDebugStats( );
		fPrintRightSideDebugStats( );
#endif

		{
			profile_render( *mScreen );

			mScreen->fResetPreSwapTimer( );

			//if( Gfx::tGeometryBufferVRamAllocator::fBufferLockingEnabled( ) )
			//if( mSimulate )
			{
				fTickCanvas( dt );
			}


			Gfx::tGeometryBufferVRamAllocator::fGlobalPreRender( );
			Gfx::tIndexBufferVRamAllocator::fGlobalPreRender( );

			// Before we kick the corender give the screen a chance to clean up
			mScreen->fCleanST( );

			if( mSimulate )
				mSceneGraph->fKickCoRenderMTRunList( );

			mScreen->fRender( );

			Gfx::tGeometryBufferVRamAllocator::fGlobalPostRender( );
			Gfx::tIndexBufferVRamAllocator::fGlobalPostRender( );
		}

#ifdef sig_devmenu
		mFpsText->fPostRender( ); //used in fPrintDebugStats()
#endif
	}
	void tGameAppBase::fTickCanvas( f32 dt )
	{

		{
			profile_pix( "tGameAppBase::fTickCanvas (FUI)" );
			profile( cProfilePerfOnTickFUI );
			Fui::tFuiSystem::fInstance( ).fTick( );
		}
		{
			profile( cProfilePerfOnTickCanvas );
			if( mSimulate )
			{
				profile_pix( "tGameAppBase::fTickCanvas (Root Canvas)" );
				fRootCanvas( ).fOnTickCanvas( dt );
				fRootCanvas( ).fSetInvisible( Game_HideHudRoot );
				// The status canvas is a child of the root
			}
			else 
			{
				profile_pix( "tGameAppBase::fTickCanvas (Status Canvas)" );
				// If we're not simulating we only tick the status canvas
				// Eventually we'll want a "status canvas" for the fui system
				mStatusCanvas.fOnTickCanvas( dt );
			}
		}	
	}
	void tGameAppBase::fTickAudio( f32 dt )
	{
		profile_pix("fTickAudio");

		if_devmenu( if( Audio::tSystem::fGlobalDisable( ) ) return; )

		if( mSoundSystem )
			mSoundSystem->fUpdateAudio( dt );
	}

	void tGameAppBase::fFadeToBlack( f32 seconds )
	{
		Sqrat::Function f( tScriptVm::fInstance( ).fRootTable( ), "FadeToBlack" );
		if( !f.IsNull( ) )
			f.Execute( seconds );
	}

	void tGameAppBase::fFadeFromBlack( f32 seconds )
	{
		Sqrat::Function f( tScriptVm::fInstance( ).fRootTable( ), "FadeFromBlack" );
		if( !f.IsNull( ) )
			f.Execute( seconds );
	}

	b32 tGameAppBase::fBlackCanvasFullAlpha( )
	{
		b32 result = true;
		Sqrat::Function f( tScriptVm::fInstance( ).fRootTable( ), "BlackCanvasFullAlpha" );
		if( !f.IsNull( ) )
			result = f.Evaluate<b32>( );
		return result;
	}

	b32 tGameAppBase::fBlackCanvasZeroAlpha( )
	{
		b32 result = true;
		Sqrat::Function f( tScriptVm::fInstance( ).fRootTable( ), "BlackCanvasZeroAlpha" );
		if( !f.IsNull( ) )
			result = f.Evaluate<b32>( );
		return result;
	}

#ifdef sig_devmenu

	namespace
	{
		///
		/// \class tResourceStorageLogData
		/// \brief 
		struct tResourceStorageLogData
		{
			f32 mSort;
			tFilePathPtr mPath;
			Math::tVec2u mDim;
			std::string mText;
			std::string mPaperTrail;

			tResourceStorageLogData( ) : mSort( 0.f ) { }
			tResourceStorageLogData( 
				const tFilePathPtr& path,
				const Math::tVec2u& dim,
				f32 sort,
				const std::string & text,
				const std::string & trail ) 
				: mSort( sort )
				, mPath( path )
				, mDim( dim )
				, mText( text )
				, mPaperTrail( trail )
			{ }

			inline b32 operator<( const tResourceStorageLogData& other ) const 
			{ 
				if( mSort > other.mSort )
					return true;
				else if( mSort < other.mSort)
					return false;
				else
					return std::less<std::string>( )( mPath.fCStr( ), other.mPath.fCStr( ) );
			}
		};

		//------------------------------------------------------------------------------
		template <class tResourceType>
		static void fDumpResources( const tResourceDepotPtr& resDepot, const char* name )
		{
			u32 lodMem = 0;
			f32 tex_d = 0;
			f32 tex_s = 0;
			f32 tex_i = 0;
			f32 tex_g = 0;
			f32 tex_n = 0;
			f32 tex_r = 0;
			f32 tex_t = 0;
			f32 tex_o = 0;
			f32 tex_f = 0;
			f32 tex_terrain = 0;
			f32 tex_other = 0;

			// get all the resources from the depot
			tGrowableArray<tResourcePtr> resources;
			resDepot->fQueryByType( Rtti::fGetClassId<tResourceType>( ), resources );

			// convert to log data
			f32 totalMB = 0;
			tGrowableArray<tResourceStorageLogData> logData;
			for( u32 i = 0; i < resources.fCount( ); ++i )
			{
				const tResourcePtr & resPtr = resources[ i ];
				const tResourceType* resource = resPtr->fCast<tResourceType>( );
				if( !resource )
					continue;

				// Grab the size and the display string
				std::string display;
				const f32 mb = Memory::fToMB<f32>( resource->fComputeStorage( display ) );
				Math::tVec2u dim( 0, 0 );

				if( resPtr->fIsType<Gfx::tTextureFile>( ) )
				{
					dim.x = resPtr->fCast<Gfx::tTextureFile>( )->mWidth;
					dim.y = resPtr->fCast<Gfx::tTextureFile>( )->mHeight;

					//need to get char just before '.' in path
					const char* path = resPtr->fGetPath( ).fCStr( ) + resPtr->fGetPath( ).fLength( ) - 1;
					while( path[0] != '.' )
						--path;
					--path;

					if( path[0] == 'd' ) tex_d += mb;
					else if( path[0] == 's' ) tex_s += mb;
					else if( path[0] == 'i' ) tex_i += mb;
					else if( path[0] == 'g' ) tex_g += mb;
					else if( path[0] == 'n' ) tex_n += mb;
					else if( path[0] == 'r' ) tex_r += mb;
					else if( path[0] == 't' ) tex_t += mb;
					else if( path[0] == 'o' ) tex_o += mb;
					else if( path[0] == 'f' ) tex_f += mb;
					else if( strstr( resPtr->fGetPath( ).fCStr( ), "terrain" ) != NULL ) tex_terrain += mb;
					else tex_other += mb;
				}
				
				// Accumulate totale
				totalMB += mb;
				lodMem += resource->fLODMemoryInBytes( );

				std::string trail = "";

#ifdef sig_logging
				std::stringstream ss;
				if( Debug_Memory_DumpWithTrail )
				{
					if( resPtr->fPaperTrail( ).fCount( ) > 0 )
					{
						ss << "\tFollow the paper trail: ";
						for( u32 i = resPtr->fPaperTrail( ).fCount( ); i > 0; --i )
							ss << ( resPtr->fPaperTrail( )[ i - 1 ].fCStr( ) + ( i > 1 ? std::string(", ") : std::string("") ) );
					}
					else
						ss << "\tNo paper trail for this resource.";
				}
				trail = ss.str( );
#endif

				logData.fPushBack( tResourceStorageLogData( resPtr->fGetPath( ), dim, mb, display, trail ) );
			}

			std::sort( logData.fBegin( ), logData.fEnd( ) );

			// output log data
			log_line( 0, "---------> Dump Resources begin: " << name << ", " << totalMB << " MB <---------" );
			for( u32 i = 0; i < logData.fCount( ); ++i )
			{
				if( logData[ i ].mDim.x	 ) //texture dimensions!
					log_line( 0, logData[ i ].mPath << " " << logData[ i ].mDim << ": " << logData[ i ].mText );
				else
					log_line( 0, logData[ i ].mPath << ": " << logData[ i ].mText );

				if( Debug_Memory_DumpWithTrail )
					log_line( 0, logData[ i ].mPaperTrail );
			}

			if( lodMem )
				log_line( 0, "LOD Memory: " << lodMem / (1024.f * 1024.f) );

			if( tex_n > 0 ) log_line(0,"normal maps: "					<< std::fixed << std::setprecision(2) << tex_n << "MB" );
			if( tex_d > 0 ) log_line(0,"diffuse maps: "					<< std::fixed << std::setprecision(2) << tex_d << "MB" );
			if( tex_s > 0 ) log_line(0,"specular maps: "				<< std::fixed << std::setprecision(2) << tex_s << "MB" );
			if( tex_i > 0 ) log_line(0,"self illumination maps: "		<< std::fixed << std::setprecision(2) << tex_i << "MB" );
			if( tex_r > 0 ) log_line(0,"reflection(cube) maps: "		<< std::fixed << std::setprecision(2) << tex_r << "MB" );
			if( tex_t > 0 ) log_line(0,"lookup table/gradient maps: "	<< std::fixed << std::setprecision(2) << tex_t << "MB" );
			if( tex_o > 0 ) log_line(0,"opacity map: "					<< std::fixed << std::setprecision(2) << tex_o << "MB" );
			if( tex_f > 0 ) log_line(0,"font map: "						<< std::fixed << std::setprecision(2) << tex_f << "MB" );
			if( tex_g > 0 ) log_line(0,"gui textures: "					<< std::fixed << std::setprecision(2) << tex_g << "MB" );
			if( tex_terrain > 0 ) log_line(0,"all terrain textures: "	<< std::fixed << std::setprecision(2) << tex_terrain << "MB" );
			if( tex_other > 0 ) log_line(0,"unknown texture: "			<< std::fixed << std::setprecision(2) << tex_other << "MB" );

			log_line( 0, "--------->  Dump Resources end  <---------" );
		}
	}

#endif//sig_devmenu

	//------------------------------------------------------------------------------
#ifdef sig_devmenu
	void tGameAppBase::fPrintDebugStats( )
	{
		profile_pix( "fPrintDebugStats" );
		if( Debug_Memory_DumpMeshesAndTextures )
		{
			devvar_update_value( Debug_Memory_DumpMeshesAndTextures, 0.f );
			fDumpResources<Gfx::tTextureFile>( fResourceDepot( ), "Textures" );
			fDumpResources<Gfx::tGeometryFile>( fResourceDepot( ), "Meshes" );
		}

		if( Debug_Memory_DumpAnims )
		{
			devvar_update_value( Debug_Memory_DumpAnims, 0.f );
			fDumpResources<tAnimPackFile>( fResourceDepot( ), "Anim Packs" );
		}

		Gfx::tDebugFreeCamera::fGlobalAppTick( );

		if( !Debug_Stats_Render || mScreen->fCapturing( ) )
			return;

		fMakeDebugString( );

		const Math::tVec2u safeEdge = mScreen->fComputeGuiSafeEdge( );
		const f32 x = Debug_Stats_PositionX + safeEdge.x + ( fIsDevMenuActive( ) ? 300.f : 0.f );
		const Math::tVec4f debugTextRgba = Debug_Stats_TextColor;

		// Bake the text objects
		f32 fpsBoxHeight = mFpsText->fBuild( *mScreen );
		f32 statsBoxHeight = mStatsText->fBakeBox( 400, mStatsTextSS.str( ).c_str( ), 0, Gui::tText::cAlignLeft );

		// Compute yStart that will center the text vertically
		const f32 backBufferHeight = ( f32 )mScreen->fCreateOpts( ).mBackBufferHeight;
		f32 yStart = ( backBufferHeight - fpsBoxHeight - statsBoxHeight ) / 2.f;
		
		// Add draw calls for the text objects
		mFpsText->fSetRgbaTint( debugTextRgba );
		mFpsText->fAddDrawCall( *mScreen, x, yStart );
		mStatsText->fSetPosition( Math::tVec3f( x, yStart + fpsBoxHeight, 0.0f ) );
		mStatsText->fSetRgbaTint( debugTextRgba );
		mScreen->fAddScreenSpaceDrawCall( mStatsText->fDrawCall( ) );

		// Print the warning log
		std::stringstream warnSS;
		for( u32 w = 0; w < mWarningLog.fCount( ) && w < cMaxWarningMsgs; ++w )
		{
			warnSS << mWarningLog[ w ].mWarning;
			if( mWarningLog[ w ].mCount > 1 )
				warnSS << "(SPAM x" << mWarningLog[ w ].mCount << ")";
			warnSS << std::endl;
		}

		std::string warnText = warnSS.str( );
		const f32 warnHeight = mWarningText->fBakeBox( 850.0f, warnText.c_str( ), 0, Gui::tText::cAlignLeft );
		mWarningText->fSetPosition( Math::tVec3f( Debug_Warn_PositionX, Debug_Warn_PositionY - warnHeight, 0.f ) );
		mWarningText->fSetRgbaTint( Math::tVec4f( 0.f, 0.75f, 0.75f, 1.0f ) );
		mScreen->fAddScreenSpaceDrawCall( mWarningText->fDrawCall( ) );
	}
#endif//sig_devmenu

#ifdef sig_devmenu
	void tGameAppBase::fPrintRightSideDebugStats( )
	{
		if( !Debug_Watcher_Visible )
			return;

		profile_pix( "fPrintRightSideDebugStats" );

		mDebugTextRightSideSS.str( L"" );
		tDebugWatcherList::fDumpAllWatchers( &mDebugTextRightSideSS );
		const f32 boxWidth = 500.0f;
		const f32 statsBoxHeight = mDebugTextRightSide->fBakeBox( boxWidth, mDebugTextRightSideSS.str( ), Gui::tText::cAlignRight );
		const f32 x = mScreen->fCreateOpts( ).mBackBufferWidth - mScreen->fComputeGuiSafeEdge( ).x - boxWidth;
		const f32 y = (f32)mScreen->fComputeGuiSafeEdge( ).y;
		mDebugTextRightSide->fSetRgbaTint( Debug_Stats_TextColor );
		mDebugTextRightSide->fSetPosition( Math::tVec3f( x, y, 0.0f ) );
		mScreen->fAddScreenSpaceDrawCall( mDebugTextRightSide->fDrawCall( ) );
	}
#endif//sig_devmenu

#ifdef sig_devmenu
	void tGameAppBase::fMakeDebugString( )
	{
		mStatsTextSS.str( L"" );
		const s32 globalEntCount = tEntity::fGlobalEntityCount( );
		const s32 globalHighEntWaterCount = tEntity::fGlobalHighWaterEntityCount( );
		const s32 globalLogicCount = tLogic::fGlobalLogicCount( );
		const s32 globalHighLogicWaterCount = tLogic::fGlobalHighWaterLogicCount( );

		f32 memTracked = 0.f;
		f32 memUsed = 0.f;
		f32 memFree = 0.f;
		const f32 memMain =  Memory::fToMB<f32>( Memory::tMainMemoryProvider::fInstance( ).fNumBytesBeingManaged( ) );
		const f32 memRes =  Memory::fToMB<f32>( Memory::tResourceMemoryProvider::fInstance( ).fNumBytesBeingManaged( ) );

#ifdef sig_profile
		const f32 memAudio =  Memory::fToMB<f32>( tProfiler::fInstance( ).fQueryMemUsage( cProfileMemAudio ) );
		const f32 memAudioPhys = Memory::fToMB<f32>( tProfiler::fInstance( ).fQueryMemUsage( cProfileMemAudioPhys ) );
		const f32 memSqrat =   Memory::fToMB<f32>( tProfiler::fInstance( ).fQueryMemUsage( cProfileMemSqrat ) );
#else
		const f32 memAudio =  0.f;
		const f32 memAudioPhys = 0.f;
		const f32 memSqrat =   0.f;
#endif //sig_profile


		f32 memGC = 0;
		if( fSceneGraph( ) )
		{
			u32 memGCBytes = 0;
			const u32 cloudCount = fSceneGraph( )->fCloudList( ).fCount( );
			for( u32 c = 0; c < cloudCount; ++c )
			{
				if( Gfx::tGroundCoverCloud * gcCloud = fSceneGraph( )->fCloudList( )[ c ]->fDynamicCast<Gfx::tGroundCoverCloud>( ) )
					memGCBytes += gcCloud->fDef( )->fCalculateSize( );
			}

			memGC = Memory::fToMB<f32>( memGCBytes );
		}

		memTracked += memMain + memRes + memAudioPhys + memAudio  + memSqrat;

		const u32 memExtra = Memory::fDebugMemGetAdditionalTitleMemorySize( );

#ifdef platform_xbox360
		MEMORYSTATUS ms;
		GlobalMemoryStatus( &ms );
		memUsed = Memory::fToMB<f32>( ms.dwTotalPhys - ms.dwAvailPhys + Memory::fFromMB<u32>( memExtra ) ) - 32.f; // 32 is reserved for system
		memFree = Memory::fToMB<f32>( ms.dwAvailPhys );
		memTracked += Memory::fToMB<f32>( tGameAppBase::mMainVramHeap.fNumBytesBeingManaged( ) );
		memTracked += Memory::fToMB<f32>( Allocators::gFui->fTotalBytes() );
		memTracked += Memory::fToMB<f32>( Allocators::gScript->fTotalBytes() );
		memTracked += Memory::fToMB<f32>( Allocators::gMalloc->fTotalBytes() );
#else
		memUsed = memTracked;
#endif

		//mStatsTextSS << "corender = " << std::fixed << std::setprecision( 2 ) << fSceneGraph( )->fLastCoRenderTime( ) << " ms" << std::endl;
		//mStatsTextSS << "sg = " << ( tSceneGraph::fForceRunListsST( ) ? "ST" : "MT" ) << std::endl;
		mStatsTextSS << "logics = " << globalLogicCount << " / " << globalHighLogicWaterCount << std::endl;
		mStatsTextSS << "ents = " << globalEntCount << " / " << globalHighEntWaterCount << std::endl;
		mStatsTextSS << "goals = " << AI::tSigAIGoal::fGlobalCount( ) << std::endl;
		mStatsTextSS << "tris = " << mScreen->fGetWorldStats( ).fTotalTriCount( ) << std::endl;
		mStatsTextSS << "batches = " << mScreen->fGetWorldStats( ).mBatchSwitches << std::endl;
		mStatsTextSS << "draws = " << mScreen->fGetWorldStats( ).mNumDrawCalls << std::endl;
		mStatsTextSS << "memory = " << std::fixed << std::setprecision( 2 ) << memUsed << " (free: " << memFree << ")" << std::endl;
		mStatsTextSS << "   mem abyss = " << std::fixed << std::setprecision( 2 ) << memUsed - memTracked << std::endl;
#ifdef platform_xbox360
		mStatsTextSS << "   VRAM = " << tGameAppBase::mMainVramHeap << std::endl;
#endif//platform_xbox360
		mStatsTextSS << "   res = " << Memory::tResourceMemoryProvider::fInstance( ) << std::endl;
		mStatsTextSS << "   main = " << Memory::tMainMemoryProvider::fInstance( ) << std::endl;
		mStatsTextSS << "   audio = " << std::fixed << std::setprecision( 2 ) << memAudio << std::endl;
		mStatsTextSS << "   sqrat = " << std::fixed << std::setprecision( 2 ) << memSqrat << std::endl;
		mStatsTextSS << "   gc = " << std::fixed << std::setprecision( 2 ) << memGC << std::endl;
		Allocators::gFui->fLog(mStatsTextSS);
		Allocators::gScript->fLog(mStatsTextSS);
		Allocators::gMalloc->fLog(mStatsTextSS);

		const u32 totalSecondsRun = fRound<u32>( mSceneGraph->fElapsedTime( ) );
		const u32 totalGameSecondsRun = fRound<u32>( mSceneGraph->fPausableTime( ) );
		mStatsTextSS << "time = " << totalSecondsRun / 60 << ":" << std::setfill( L'0' ) << std::setw( 2 ) << totalSecondsRun % 60 << std::endl;
		mStatsTextSS << "unpaused time = " << totalGameSecondsRun / 60 << ":" << std::setfill( L'0' ) << std::setw( 2 ) << totalGameSecondsRun % 60 << std::endl;
		mStatsTextSS << "load = " << mLastLevelLoadTime << " s" << std::endl;

		if( memExtra )
		{
			mStatsTextSS << "ADD TITLE MEM: " << memExtra << "MB" << std::endl;

			if( memExtra > memFree )
				mStatsTextSS << "OVER MEM BY: " << (memExtra-memFree) << "MB" << std::endl;
		}

		if( Debug_Memory_DisplayMainMemPooled )
			Memory::tHeap::fInstance( ).fAddStats( mStatsTextSS );
		fAddToStatText( mStatsTextSS );

		fSceneGraph( )->fPhysics( )->fLogStats( mStatsTextSS );
	}
#endif//sig_devmenu

	void tGameAppBase::fAutoReloadResources( )
	{
		profile_pix("fAutoReloadResources");
#ifdef sig_devmenu // attempt to reload textures (must happen immediately after rendering)
		if( !Resources_AutoReload )
			return;

		static Time::tStopWatch lastChecked;
		const f32 cMinSecondsBetweenChecks = 5.0f;
		if( lastChecked.fGetElapsedS() < cMinSecondsBetweenChecks )
			return; // checking can be very slow, don't do it every frame.

		const u64 newBuildStamp = fComputeBuildStamp( );
		if( mBuildStamp == newBuildStamp )
			return;

		fSceneGraph( )->fWaitForLogicRunListsToComplete( );

		log_line( 0, "BuildStamp has changed, reloading resources..." );

		mBuildStamp = newBuildStamp;
		tGrowableArray<Rtti::tClassId> cids;
		cids.fPushBack( Rtti::fGetClassId<tLocalizationFile>( ) );
		cids.fPushBack( Rtti::fGetClassId<tDataTableFile>( ) );
		cids.fPushBack( Rtti::fGetClassId<tScriptFile>( ) );
		//cids.fPushBack( Rtti::fGetClassId<Gui::tFont>( ) );
		cids.fPushBack( Rtti::fGetClassId<Gfx::tTextureFile>( ) );
		cids.fPushBack( Rtti::fGetClassId<FX::tFxFile>( ) );
		cids.fPushBack( Rtti::fGetClassId<Gfx::tMaterialFile>( ) );
		fResourceDepot( )->fReloadResourcesByType( cids );
#endif//sig_devmenu
	}

	void tGameAppBase::fReadLicense( )
	{
		const b32 wasOldVersion = mIsFullVersion;

		// The help file says to not downgrade the game from full to trial if a user with full permissions signs out
		if( tUser::fCheckFullLicenseFlag( ) )
			mIsFullVersion = true;

		if( mIsFullVersion && !wasOldVersion )
			fOnFullVersionInstalled( );
	}

	void tGameAppBase::fDumpMemory( const char* context, b32 justLog, tDynamicArray<byte>* dumpOutput )
	{

		{
			Memory::tHeapStacker stacker( &Memory::tDebugMemoryHeap::fInstance( ) );

			Memory::tMemoryDumpOptions dump( justLog );
			if( !justLog )
				dump.mDump = NEW Memory::tMemoryDump( );

			Memory::tPool::fDumpGlobalPoolAllocations( dump );
			Memory::tMainMemoryProvider::fInstance( ).fDump( dump );
			Memory::tResourceMemoryProvider::fInstance( ).fDump( dump );
#ifdef platform_xbox360
			mMainVramHeap.fDump( dump );
			mNoCacheHeap.fDump( dump );
#endif

			if( dump.mDump )
			{
				if( dumpOutput )
				{
					// dump into provided output buffer
					tGameArchiveSave save;
					save.fSave( *dump.mDump, 0 );
					dumpOutput->fInitialize( save.fBuffer( ).fBegin( ), save.fBuffer( ).fCount( ) );
				}
				else
				{
					// save to file
#ifdef platform_xbox360
					tFilePathPtr dir( "game:\\memoryDump" );
#else
					tFilePathPtr dir( "c:\\memoryDump" );
#endif
					FileSystem::fCreateDirectory( dir );
					//FileSystem::fDeleteAllFilesInFolder( dir );

					tFilePathPtrList list;
					FileSystem::fGetFileNamesInFolder( list, dir );

					tFilePathPtr fileName( "1.Dmpml" );
					if( list.fCount( ) )
						fileName = tFilePathPtr( "2.Dmpml" );

					dump.mDump->fSave( tFilePathPtr::fConstructPath( dir, fileName ) );
				}
			}

			delete dump.mDump;
		}
	}

	void tGameAppBase::fConsolidateMemory( )
	{
#ifdef platform_xbox360
		mMainVramHeap.fConsolidate( );
		mNoCacheHeap.fConsolidate( );
#endif
		Memory::tResourceMemoryProvider::fInstance( ).fConsolidate( );
		Memory::tMainMemoryProvider::fInstance( ).fConsolidate( );
	}

	void tGameAppBase::fLogFreeMemory( )
	{
#ifdef platform_xbox360
		mMainVramHeap.fLogFreeMemory( );
		mNoCacheHeap.fLogFreeMemory( );
#endif
		Memory::tResourceMemoryProvider::fInstance( ).fLogFreeMemory( );
		Memory::tMainMemoryProvider::fInstance( ).fLogFreeMemory( );
	}

}

namespace Sig
{
	namespace
	{
		static void fExportGameMode( tScriptVm& vm )
		{
			Sqrat::Class<tGameMode, Sqrat::DefaultAllocator<tGameMode> > classDesc( vm.fSq( ) );
			classDesc
				.Prop("Invalid", &tGameMode::fInvalid)
				.Prop("IsFrontEnd", &tGameMode::fIsFrontEnd)
				.Prop("IsPurgatory", &tGameMode::fIsPurgatory)
				.Prop("IsSinglePlayer", &tGameMode::fIsSinglePlayer)
				.Prop("IsVersus", &tGameMode::fIsVersus)
				.Prop("IsCoOp", &tGameMode::fIsCoOp)
				.Prop("IsSinglePlayerOrCoop", &tGameMode::fIsSinglePlayerOrCoop)
				.Prop("IsMultiPlayer", &tGameMode::fIsMultiPlayer)
				.Prop("IsLocal", &tGameMode::fIsLocal)
				.Prop("IsNet", &tGameMode::fIsNet)
				.Prop("IsSplitScreen", &tGameMode::fIsSplitScreen)
				.Func("AddCoOpFlag", &tGameMode::fAddCoOpFlag)
				.Func("AddOnlineFlag", &tGameMode::fAddOnlineFlag)
				; 
			vm.fRootTable( ).Bind(_SC("tGameMode"), classDesc);
		}
		static void fExportGameAppBase( tScriptVm& vm )
		{
			Sqrat::DerivedClass<tGameAppBase, tApplication, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("HudRoot"),		&tGameAppBase::fRootHudCanvasFromScript)
				.Func(_SC("HudLayer"),		&tGameAppBase::fHudLayerFromScript)
				.Prop(_SC("LoadingCanvas"), &tGameAppBase::fLoadingCanvasFromScript)
				.Prop(_SC("WarningCanvas"),	&tGameAppBase::fWarningCanvasFromScript)
				.Prop(_SC("StatusCanvas"),	&tGameAppBase::fStatusCanvasFromScript)
				.Func(_SC("LocString"),		&tGameAppBase::fLocString)
				.Func(_SC("LocStringExists"), &tGameAppBase::fLocStringExists)
				.Prop(_SC("IsFullVersion"),		&tGameAppBase::fIsFullVersion)
				.Func(_SC("ComputeViewportRect"), &tGameAppBase::fViewportRect)
				.Func(_SC("ComputeViewportSafeRect"), &tGameAppBase::fViewportSafeRect)
				.Prop(_SC("DefaultLight"), &tGameAppBase::fDefaultLightForScript)
				.Prop(_SC("Audio"), &tGameAppBase::fAudioForScript)
				;
			vm.fRootTable( ).Bind(_SC("tGameAppBase"), classDesc );
		}
	}
	void tGameAppBase::fExportScriptInterface( tScriptVm& vm )
	{
		fExportGameMode( vm );
		fExportGameAppBase( vm );
	}
}
