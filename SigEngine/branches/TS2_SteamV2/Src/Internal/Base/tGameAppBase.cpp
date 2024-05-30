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
#include "tFileWriter.hpp"
#include "tProfiler.hpp"
#include "AI/tSigAIGoal.hpp"
#include "tSync.hpp"

namespace Sig
{
	devvar( bool, Resources_AutoReload, true );
	devvar( bool, Renderer_Settings_FullScreen, false );
	devvar( u32, Renderer_Settings_DefaultFullScreenResX, 1280 );
	devvar( u32, Renderer_Settings_DefaultFullScreenResY, 720 );
	devvar( bool, Debug_Memory_DumpMeshesAndTextures, false );
	devvar( bool, Debug_Memory_DumpWithTrail, false );
	devvar( bool, Debug_Stats_Render, false );
	devvar( bool, Game_HideHudRoot, false );
	devvar_clamp( f32, Debug_Stats_PositionX, 20.f, 0.f, 1280.f, 0 );
	devvar_clamp( f32, Debug_Stats_PositionY, 150.f, 0.f, 720.f, 0 );
	devrgba_clamp( Debug_Stats_TextColor, Math::tVec4f( 0.75f, 0.75f, 1.f, 0.75f ), 0.f, 1.f, 2 );

#ifdef sig_devmenu
	void fAppMemoryDump( tDevCallback::tArgs& args )
	{
		tGameAppBase::fInstance( ).fDumpMemory( "DevMenu", false );
	}
	void fAppMemoryDumpLog( tDevCallback::tArgs& args )
	{
		tGameAppBase::fInstance( ).fDumpMemory( "DevMenu", true );
	}
	void fAAACheatsReadLicense( tDevCallback::tArgs& args )
	{
		tGameAppBase::fInstance( ).fReadLicense( );
	}
#endif
	devcb( Debug_Memory_HeapDump, "0", make_delegate_cfn( tDevCallback::tFunction, fAppMemoryDump ) );
	devcb( Debug_Memory_HeapDumpJustLog, "0", make_delegate_cfn( tDevCallback::tFunction, fAppMemoryDumpLog ) );
	devcb( AAACheats_ReadLicense, "0", make_delegate_cfn( tDevCallback::tFunction, fAAACheatsReadLicense ) );

	tGameAppBase* tGameAppBase::gGameAppBase=0;
	tGameAppBase::tGameAppBase( )
		: mSimulate( true )
		, mInGameSimulationState( false )
// debug runs too slow to maintain a good frame rate, but at the same time 
// we want to be able to debug "real" situations (i.e. at target frame rate)
#ifdef build_debug
		, mFixedTimeStep( true )
#else
		, mFixedTimeStep( false )
#endif
		, mBuildStamp( 0 )
		, mVRamUsed( 0 )
		, mLastLevelLoadTime( 0.f )
		, mIsFullVersion( false )
	{
        profile_early_init( );
		gGameAppBase = this;
	}
	tGameAppBase::~tGameAppBase( )
	{
	}
	u64 tGameAppBase::fComputeBuildStamp( ) const
	{
#ifdef sig_devmenu
		if( mFilePackages.fCount( ) == 0 )
		{
			const tFilePathPtr buildStampPath = tFilePathPtr::fConstructPath( fResourceDepot( )->fRootPath( ), tFilePathPtr( "zzz.buildstamp" ) );
			return FileSystem::fGetLastModifiedTimeStamp( buildStampPath );
		}
#endif//sig_devmenu
		return 0;
	}
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
	void tGameAppBase::fTickApp( )
	{
		profile_pix("tGameAppBase::fTickApp");
		b32 fixedTimeStep = mFixedTimeStep;

#ifdef sig_devmenu
		const b32 singleStep = fShouldDoSingleStep( );
		if( singleStep || fScreen( )->fCapturing( ) )
			fixedTimeStep = true;
		if( singleStep )
			fSceneGraph( )->fPause( false );
#endif//sig_devmenu

		// run the simulation
		const f32 dt = fixedTimeStep ? fGetOptions( ).mFixedTimeStep : fGetFrameDeltaTime( );

		if( mSimulate )
		{
			sync_event_v( dt );
			sync_d_event_v( tRandom::fObjectiveRand( ).fState( ) );

			fSceneGraph( )->fAdvanceTime( dt );
			fTickCameras( dt );
		}

#if defined(platform_pcdx9)
		if ( tMoviePlayer::fInstance() )
		{
			fTickCanvas( dt );

			// clear geo. Normally done in fTickRender.
			Gfx::tGeometryBufferVRamAllocator::fGlobalPostRender( );
			Gfx::tIndexBufferVRamAllocator::fGlobalPostRender( );
		}
		else
		{
			fTickRender( dt );
		}
#else
		fTickRender( dt );
#endif

		fTickAudio( dt );
		fAutoReloadResources( );

		tScriptVm::fInstance( ).fLogStackTop( "tGameAppBase::fTickApp", 1 );

#ifdef sig_devmenu
		if( singleStep )
			fSceneGraph( )->fPause( true );
#endif//sig_devmenu

	}

	void tGameAppBase::fEnableFixedTimeStep( b32 fixed )
	{
		mFixedTimeStep = fixed;
#if defined( platform_pcdx ) && defined( target_game )
		if( mFixedTimeStep )
		{
			mOptions.mUpdateDelay = mOptions.mFixedTimeStep;
		}
		else
		{
			mOptions.mUpdateDelay = 0.0f;
		}
#endif //  defined( platform_pcdx ) && defined( target_game )
	}

	void tGameAppBase::fSetFixedTimeStep( f32 fixedTimeStep )
	{
		mOptions.mFixedTimeStep = fixedTimeStep;
		mOptions.mFrameTimeDeltaClamp = 4.0f * mOptions.mFixedTimeStep;
	}

	void tGameAppBase::fStartupApp( )
	{
		mBuildStamp = fComputeBuildStamp( );

		fPreLoadFilePackages( );
		fStartupScript( );
		fStartupSceneGraph( );
		fStartupGfx( );
		fStartupProfiler( );
		fStartupSfx( );
		fSetupPostEffects( ); //these will block
		fLoadPermaLoadedGameResources( );
		fStartupUsers( );

		// determine command line options
		const std::string& cmdLineBuffer = fGetCmdLine( );
		const tCmdLineOption previewMode( "preview", cmdLineBuffer );
		const tCmdLineOption replayMode( "replay", cmdLineBuffer );
#if defined( use_steam )
		const tCmdLineOption connect( "connect_lobby", cmdLineBuffer );
#endif

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
				FileSystem::fGetFileNamesInFolder( list, mResourceDepot->fRootPath( ), true, false, tFilePathPtr( ".rply" ) );

				if( list.fCount( ) == 1 )
					replayPath = list[ 0 ];
				else
					log_warning( 0, "No valid replay file could be found" );
			}

			log_line( 0, "Running REPLAY on file [" << replayPath << "]" );
			fReplay( replayPath );
		}
#if defined( use_steam )
		else if ( connect.fFound( ) )
		{
			tAddr lobby = connect.fGetTypedOption< tAddr >( );
			log_line( Log::cFlagSession, __FUNCTION__ " Received invite to lobby " << lobby );
			fReceivedGameInvite( lobby );
			// This is now called in fOnLevelLoadEnd
			//fOnGameInviteAccepted( 0 );
		}
#endif
		else
		{
			// run for rillz, with front end, logo screens, etc.
			fStartGame( );
		}
	}
	void tGameAppBase::fShutdownApp( )
	{
		mRootCanvas.fClearChildren( );
		mRootHudCanvas = Gui::tCanvasPtr( );
		mWarningCanvas = Gui::tCanvasPtr( );
		mStatusCanvas = Gui::tCanvasPtr( );
		mSceneGraph->fClear( );
		mSceneGraph.fRelease( );
		mAppState.fRelease( );

		for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
			mLocalUsers[ i ]->fSetViewport( Gfx::tScreenPtr( ), Gfx::tViewportPtr( ) );
		mLocalUsers.fSetCount( 0 );

		mFpsText.fRelease( );
		mStatsText.fRelease( );

		Gfx::tDefaultAllocators& defAllocators = Gfx::tDefaultAllocators::fInstance( );
		defAllocators.fDeallocate( );
		defAllocators.fUnloadMaterials( this );
		defAllocators = Gfx::tDefaultAllocators( );

		mScreen->fShutdown( );
		mScreen.fRelease( );

#if true //defined( build_release )
		// Disregard crashes, aquire the thin illusion of a proper shutdown

		// Skip ScriptVm reset
		mDevice.fRelease( );
		Gfx::tDevice::fSetDefaultDevice( Gfx::tDevicePtr( ) );
		// Skip audio system release

		_exit(0); // quit 'successfully' without cleaning up any globals or running any atexit handlers.
#endif
		tScriptVm::fInstance( ).fGarbageCollect( );
		tScriptVm::fInstance( ).fReset( );

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
	void tGameAppBase::fPreLoadFilePackages( )
	{
		// attempt to load package file for the entire game; note that blocking on load
		// will only block to read the TOC, not all the files contained there-in; also,
		// if the package is not present, no biggie, we load from unpackaged files
		const tFilePathPtr mainPackageName = tFilePathPtr::fConstructPath( fResourceDepot( )->fRootPath( ), tFilePathPtr( "res.sacb" ) );
		
		if( FileSystem::fFileExists( mainPackageName ) )
		{
			tFilePackagePtr mainPackage( NEW tFilePackage );
			mainPackage->fLoad( this, *fResourceDepot( ), mainPackageName, false );
			mainPackage->fBlockUntilLoaded( );
			if( mainPackage->fLoaded( ) )
			{
				mFilePackages.fPushBack( mainPackage );
				log_line( 0, "               (((((                               )))))" );
				log_line( 0, "***************(((((    running from '" << mainPackageName << "'    )))))***************" );
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

		fFillOutScreenCreateOpts( screenCreateOpts );
		sigassert( screenCreateOpts.mShadowMapLayerCount <= Gfx::tMaterial::cMaxShadowLayers );

		fCreateDevice( screenCreateOpts );

		sigassert( mDevice );
		Gfx::tDevice::fSetDefaultDevice( mDevice );
		mScreen.fReset( NEW Gfx::tScreen( mDevice, fSceneGraph( ), screenCreateOpts ) );
		mScreen->fSetRgbaClearColor( mStartupParams.mRgbaClearColor.x, mStartupParams.mRgbaClearColor.y, mStartupParams.mRgbaClearColor.z );

		// setup debug geometry
		fSceneGraph( )->fDebugGeometry( ).fResetDeviceObjects( *fResourceDepot( ), mDevice );

		fCreateGeometryAllocators( );

		fLoadMaterials( );

		// load default font (block until its loaded)
		mDefaultFont = fAddAlwaysLoadedResource( tResourceId::fMake<Gui::tFont>( Gui::tFont::fDevFontPath( ) ), true );

		mFpsText.fReset( NEW Gui::tFpsText( ) );
		mFpsText->fSetFont( mDefaultFont );
		mStatsText.fReset( NEW Gui::tText( ) );
		mStatsText->fSetFont( mDefaultFont );

		// show the window now that gfx is ready
		tApplication::fShowWindow( screenCreateOpts.mBackBufferWidth, screenCreateOpts.mBackBufferHeight, CW_USEDEFAULT, CW_USEDEFAULT, false, screenCreateOpts.mFullScreen );

		// create viewports
		fChangeViewportCount( mStartupParams.mViewportCount, mStartupParams.mViewportCount, false );

		// create/add HUD root
		mRootHudCanvas = Gui::tCanvasPtr( Sqrat::Object( NEW Gui::tCanvasFrame( ) ) );
		fRootCanvas( ).fAddChild( mRootHudCanvas );

		mWarningCanvas = Gui::tCanvasPtr( Sqrat::Object( NEW Gui::tCanvasFrame( ) ) );
		fRootCanvas( ).fAddChild( mWarningCanvas );

		mStatusCanvas = Gui::tCanvasPtr( Sqrat::Object( NEW Gui::tCanvasFrame( ) ) );
		fRootCanvas( ).fAddChild( mStatusCanvas );
	}
	void tGameAppBase::fStartupProfiler( )
	{
		profile_init( *mScreen );
	}
	void tGameAppBase::fCreateDevice( Gfx::tScreenCreationOptions& screenCreateOpts )
	{
#if defined( platform_xbox360 )
		screenCreateOpts.mWindowHandle = fGetWindowHandleGeneric( );
		screenCreateOpts.mVsync = 2; // 30 hz
		screenCreateOpts.mMultiSamplePower = Gfx::tScreen::fDefaultMultiSamplePower( );
		Gfx::tDisplayModeList displayModes;
		Gfx::tDevice::fEnumerateDisplayModes( displayModes );
		const u32 bestMatch = displayModes.fFindClosestMatch( Gfx::tDisplayMode( Renderer_Settings_DefaultFullScreenResX, Renderer_Settings_DefaultFullScreenResY ) );
		sigassert( bestMatch < displayModes.fCount( ) );

		screenCreateOpts.mFullScreen = true;
		screenCreateOpts.mBackBufferWidth = displayModes[ bestMatch ].mBackBufferWidth;
		screenCreateOpts.mBackBufferHeight = displayModes[ bestMatch ].mBackBufferHeight;
		mDevice.fReset( NEW Gfx::tDevice( screenCreateOpts ) );
#elif defined( platform_ios )
		screenCreateOpts.mWindowHandle = fGetWindowHandleGeneric( );
		screenCreateOpts.mVsync = 2; // 30 hz
		screenCreateOpts.mMultiSamplePower = Gfx::tScreen::fDefaultMultiSamplePower( );
		Gfx::tDisplayModeList displayModes;
		Gfx::tDevice::fEnumerateDisplayModes( displayModes );
		const u32 bestMatch = displayModes.fFindClosestMatch( Gfx::tDisplayMode( Renderer_Settings_DefaultFullScreenResX, Renderer_Settings_DefaultFullScreenResY ) );
		sigassert( bestMatch < displayModes.fCount( ) );
		
		screenCreateOpts.mFullScreen = true;
		screenCreateOpts.mBackBufferWidth = displayModes[ bestMatch ].mBackBufferWidth;
		screenCreateOpts.mBackBufferHeight = displayModes[ bestMatch ].mBackBufferHeight;
		mDevice.fReset( NEW Gfx::tDevice( screenCreateOpts ) );
#elif defined( platform_pcdx9 ) || defined( platform_pcdx10 )
		screenCreateOpts.mWindowHandle = fGetWindowHandleGeneric( );
		screenCreateOpts.mMultiSamplePower = Gfx::tScreen::fDefaultMultiSamplePower( );

		if (screenCreateOpts.mFullScreen )
		{
			Gfx::tDisplayModeList displayModes;
			Gfx::tDevice::fEnumerateDisplayModes( displayModes );

			// validate the screen rez...  they may have changed device since last saving the config
			const u32 bestMatch = displayModes.fFindClosestMatch( Gfx::tDisplayMode( screenCreateOpts.mBackBufferWidth, screenCreateOpts.mBackBufferHeight ) );
			sigassert( bestMatch < displayModes.fCount( ) );
			screenCreateOpts.mBackBufferWidth = displayModes[ bestMatch ].mBackBufferWidth;
			screenCreateOpts.mBackBufferHeight = displayModes[ bestMatch ].mBackBufferHeight;
		}
		mDevice.fReset( NEW Gfx::tDevice( screenCreateOpts ) );

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

			mLocalUsers[ i ].fReset( user  );
		}

	}

	void tGameAppBase::fCreateGeometryAllocators( )
	{
		Gfx::tDefaultAllocators& defAllocators = Gfx::tDefaultAllocators::fInstance( );

		defAllocators.fCreateAllocatorsForGameApp( mDevice, mStartupParams.mMemory );
	}
	void tGameAppBase::fLoadMaterials( )
	{
		Gfx::tDefaultAllocators& defAllocators = Gfx::tDefaultAllocators::fInstance( );

		defAllocators.fLoadMaterials( mResourceDepot, this );

		mPostEffectsMaterialFile = fAddAlwaysLoadedResource( tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tPostEffectsMaterial::fMaterialFilePath( ) ), true );
	}
	Math::tRect tGameAppBase::fViewportRect( u32 ithViewport )
	{
		if( mScreen && ithViewport < mScreen->fGetViewportCount( ) )
		{
			const Gfx::tViewportPtr& vp = mScreen->fViewport( ithViewport );
			return vp->fComputeRect( mScreen );
		}
		
		return Math::tRect( );
	}
	Math::tRect tGameAppBase::fViewportSafeRect( u32 ithViewport )
	{
		if( mScreen && ithViewport < mScreen->fGetViewportCount( ) )
		{
			const Gfx::tViewportPtr& vp = mScreen->fViewport( ithViewport );
			return vp->fComputeSafeRect( mScreen );
		}

		return Math::tRect( );
	}

	b32 tGameAppBase::fLocStringExists( const tStringPtr& stringId ) const
	{
		const tLocalizationFile* locFile = mLocTextFile->fCast< tLocalizationFile >( );
		if( locFile->fStringIndexFromId( stringId ) >= 0 )
			return true;

		return false;
	}

	const tLocalizedString& tGameAppBase::fLocString( const tStringPtr& _stringId ) const
	{
		tStringPtr stringId = _stringId;

#if defined( platform_pcdx )
		std::stringstream ss;
		ss << stringId << "_PC";
		tStringPtr pcOverrideStringId( ss.str( ).c_str( ) );
		if( fLocStringExists( pcOverrideStringId ) )
			stringId = pcOverrideStringId;
#endif

		const tLocalizationFile* locFile = mLocTextFile->fCast< tLocalizationFile >( );
		sigassert( locFile );
		return locFile->fStringFromId( stringId );
	}

	void tGameAppBase::fLoadLocResources( const tFilePathPtr& locFolder, const tDynamicArray<tStringPtr>& fontNames )
	{
		mLocTextFile = fAddAlwaysLoadedResource( tResourceId::fMake< tLocalizationFile >( tFilePathPtr::fConstructPath( locFolder, tFilePathPtr( "text.locml" ) ) ), true );
		mLocResFile = fAddAlwaysLoadedResource( tResourceId::fMake< tLocalizationFile >( tFilePathPtr::fConstructPath( locFolder, tFilePathPtr( "res.locml" ) ) ), true );

		mFonts.fNewArray( fontNames.fCount( ) );

		// get fonts from loc file
		const tLocalizationFile* resLocFile = mLocResFile->fCast< tLocalizationFile >( );
		sigassert( resLocFile );
		for( u32 i = 0; i < resLocFile->fPathCount( ); ++i )
		{
			tFilePathPtr fontPath = resLocFile->fPathFromIndex( i );
			if( StringUtil::fCheckExtension( fontPath.fCStr( ), ".fntml" ) )
			{
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
					log_line( 0, "Loading font: index = " << fontType << ", path = " << fontPath );
					mFonts[ fontType ] = fAddAlwaysLoadedResource( tResourceId::fMake< Gui::tFont >( fontPath ), true );
				}
			}
		}

		// set font mapping callback to facilitate use of text objects from script
		Gfx::tDefaultAllocators& defGuiAllocators = Gfx::tDefaultAllocators::fInstance( );
		defGuiAllocators.mFontFromId = &fFontFromId;
	}
	void tGameAppBase::fTickRender( f32 dt )
	{
		profile_pix("fTickRender");
		fPrintDebugStats( );

		{
			profile_render( *mScreen );

			mScreen->fResetPreSwapTimer( );

			fTickCanvas( dt );		

			Gfx::tGeometryBufferVRamAllocator::fGlobalPreRender( );
			Gfx::tIndexBufferVRamAllocator::fGlobalPreRender( );

			if( mSimulate )
				mSceneGraph->fKickCoRenderMTRunList( );

			mScreen->fRender( &mRootCanvas );

			Gfx::tGeometryBufferVRamAllocator::fGlobalPostRender( );
			Gfx::tIndexBufferVRamAllocator::fGlobalPostRender( );
		}

		mFpsText->fPostRender( );
	}
	void tGameAppBase::fTickCanvas( f32 dt )
	{
		profile( cProfilePerfOnTickCanvas );
		if( mSimulate )
		{
			mRootCanvas.fOnTickCanvas( dt );
			mRootCanvas.fSetInvisible( Game_HideHudRoot );
		}

		// If we're not simulating we only tick the status canvas
		else
		{
			mStatusCanvas.fOnTickCanvas( dt );
		}
	}
	void tGameAppBase::fTickAudio( f32 dt )
	{
		if_devmenu( if( Audio::tSystem::fGlobalDisable( ) ) return; )

		profile_pix("fTickAudio");
		if( mSoundSystem )
			mSoundSystem->fUpdateAudio( dt );
	}
#ifdef sig_devmenu
	namespace
	{
		struct tResourceStorageLogData
		{
			tFilePathPtr mPath;
			tStringPtr mPaperTrail;
			f32 mStorage;
			tResourceStorageLogData( ) : mStorage( 0.f ) { }
			tResourceStorageLogData( const tFilePathPtr& path, f32 storage, const tStringPtr& trail ) : mPath( path ), mStorage( storage ), mPaperTrail( trail ) { }
			inline b32 operator<( const tResourceStorageLogData& other ) const 
			{ 
				if( mStorage > other.mStorage )
					return true;
				else if( mStorage < other.mStorage)
					return false;
				else
					return std::less<std::string>( )( mPath.fCStr( ), other.mPath.fCStr( ) );
			}
		};
		template <class tResourceType>
		static void fDumpResources( const tResourceDepotPtr& resDepot, const char* name )
		{
			// get all the textures from the resource depot
			tGrowableArray<tResourcePtr> textures;
			resDepot->fQueryByType( Rtti::fGetClassId<tResourceType>( ), textures );

			// convert to log data
			f32 totalMb = 0;
			tGrowableArray<tResourceStorageLogData> textureLogData;
			for( u32 i = 0; i < textures.fCount( ); ++i )
			{
				const tResourceType* tex = textures[ i ]->fCast<tResourceType>( );
				if( tex )
				{
					const tResourcePtr& res = textures[ i ];
					const f32 mb = tex->fComputeStorageInMB( );
					totalMb += mb;
					std::string trail = "";

#ifdef sig_logging
					if( Debug_Memory_DumpWithTrail )
					{
						if( res->fPaperTrail( ).fCount( ) > 0 )
						{
							trail += "\tFollow the paper trail: ";
							for( u32 i = res->fPaperTrail( ).fCount( ); i > 0; --i )
								trail += res->fPaperTrail( )[ i - 1 ].fCStr( ) + ( i > 1 ? std::string(", ") : std::string("") );
						}
						else
							trail += "\tNo paper trail for this resource.";
					}
#endif

					textureLogData.fPushBack( tResourceStorageLogData( textures[ i ]->fGetPath( ), mb, tStringPtr( trail ) ) );
				}
			}
			std::sort( textureLogData.fBegin( ), textureLogData.fEnd( ) );

			// output log data
			log_line( 0, "---------> Renderer_Memory_DumpMeshesAndTextures begin: " << name << ", " << totalMb << " MB <---------" );
			for( u32 i = 0; i < textureLogData.fCount( ); ++i )
			{
				log_line( 0, textureLogData[ i ].mPath << ": " << std::fixed << std::setprecision( 2 ) << textureLogData[ i ].mStorage << " MB" );
				if( Debug_Memory_DumpWithTrail )
					log_line( 0, textureLogData[ i ].mPaperTrail );
			}

			log_line( 0, "--------->  Renderer_Memory_DumpMeshesAndTextures end  <---------" );
		}

		template< typename t >
		std::string fDebugPrintHeap( t& heap, const std::string name )
		{
			const f32 c1MB = 1024.f * 1024.f;
			std::stringstream result;
			result << name << " = " << heap.fNumBytesBeingManaged( ) / c1MB << " - " << StringUtil::fToString( heap.fNumBytesAllocd( ) / c1MB );
			return result.str( );
		}
	}
#endif//sig_devmenu
	void tGameAppBase::fPrintDebugStats( )
	{
#ifdef sig_devmenu
		if( Debug_Memory_DumpMeshesAndTextures )
		{
			devvar_update_value( Debug_Memory_DumpMeshesAndTextures, 0.f );
			fDumpResources<Gfx::tTextureFile>( fResourceDepot( ), "Textures" );
			fDumpResources<Gfx::tGeometryFile>( fResourceDepot( ), "Meshes" );
		}

		Gfx::tDebugFreeCamera::fGlobalAppTick( );

		if( !Debug_Stats_Render || mScreen->fCapturing( ) || fIsDevMenuActive( ) )
			return;

		f32 x = Debug_Stats_PositionX;
#if defined( platform_xbox360 )
		const Math::tVec2u safeEdge = mScreen->fComputeGuiSafeEdge( );
		Debug_Stats_PositionX += safeEdge.x;
#endif
		const f32 yStart = Debug_Stats_PositionY;

		const Math::tVec4f debugTextRgba = Debug_Stats_TextColor;

		mFpsText->fSetRgbaTint( debugTextRgba );
		mFpsText->fPreRender( *mScreen, x, yStart );

		const Gui::tFont* defFont = mDefaultFont->fCast<Gui::tFont>( );
		sigassert( defFont );
		
		std::string statsText = fMakeDebugString( );

		mStatsText->fBakeBox( 400, statsText.c_str( ), 0, Gui::tText::cAlignLeft );
		mStatsText->fSetPosition( Math::tVec3f( x, yStart + 4.f * defFont->mDesc.mLineHeight, 0.0f ) );
		mStatsText->fSetRgbaTint( debugTextRgba );
		mScreen->fAddScreenSpaceDrawCall( mStatsText->fDrawCall( ) );
#endif//sig_devmenu
	}

	std::string tGameAppBase::fMakeDebugString( )
	{
		std::stringstream statsText;

#ifdef sig_devmenu
		const u32 globalEntCount = tEntity::fGlobalEntityCount( );
		const u32 globalHighEntWaterCount = tEntity::fGlobalHighWaterEntityCount( );

		f32 memTotal = 0.f;
		f32 memFree = 0.f;

		const f32 memResTemp = tProfiler::fInstance( ).fQueryMemUsage( cProfileMemResTemp ) / ( 1024.f * 1024.f );
		const f32 memXAlloc = tProfiler::fInstance( ).fQueryMemUsage( cProfileMemXAlloc ) / ( 1024.f * 1024.f );
		const f32 memPoolSpill = tProfiler::fInstance( ).fQueryMemUsage( cProfileMemPoolSpill ) / ( 1024.f * 1024.f );
		const f32 memMain = Memory::tResourceMemoryProvider::fInstance( ).fNumBytesBeingManaged( ) / ( 1024.f * 1024.f );
		const f32 memRes = Memory::tResourceMemoryProvider::fInstance( ).fNumBytesBeingManaged( ) / ( 1024.f * 1024.f );
		const f32 memAudio = tProfiler::fInstance( ).fQueryMemUsage( cProfileMemAudio ) / ( 1024.f * 1024.f );
		const f32 memGeom = tProfiler::fInstance( ).fQueryGeomAlloc( ) / ( 1024.f * 1024.f );
		const f32 memSqrat =  tProfiler::fInstance( ).fQueryMemUsage( cProfileMemSqrat ) / ( 1024.f * 1024.f );
		const f32 memScript =  tProfiler::fInstance( ).fQueryMemUsage( cProfileMemScript ) / ( 1024.f * 1024.f );

		memTotal += memMain + memRes + memResTemp + memXAlloc + memGeom;
		memTotal += tProfiler::fInstance( ).fQueryMemUsage( cProfileMemAudioPhys ) / ( 1024.f * 1024.f );

#ifdef platform_xbox360
		MEMORYSTATUS ms;
		GlobalMemoryStatus( &ms );
		memFree = ms.dwAvailPhys / ( 1024.f * 1024.f );

		memTotal += tGameAppBase::mVramHeap.fNumBytesBeingManaged( ) / ( 1024.f * 1024.f );
#endif

		//statsText << "corender = " << std::fixed << std::setprecision( 2 ) << fSceneGraph( )->fLastCoRenderTime( ) << " ms" << std::endl;
		statsText << "sg = " << ( tSceneGraph::fForceRunListsST( ) ? "ST" : "MT" ) << std::endl;
		statsText << "ents = " << globalEntCount << " / " << globalHighEntWaterCount << std::endl;
		statsText << "goals = " << AI::tSigAIGoal::fGlobalCount( ) << std::endl;
		statsText << "tris = " << mScreen->fGetWorldStats( ).fTotalTriCount( ) << std::endl;
		statsText << "batches = " << mScreen->fGetWorldStats( ).mBatchSwitches << std::endl;
		statsText << "draws = " << mScreen->fGetWorldStats( ).mNumDrawCalls << std::endl;
		statsText << "memory: = " << std::fixed << std::setprecision( 2 ) << memTotal << " (free: " << memFree << ")" << std::endl;
		statsText << fDebugPrintHeap( Memory::tResourceMemoryProvider::fInstance( ), "    res" ) << std::endl;
		statsText << "  temp = " << std::fixed << std::setprecision( 2 ) << memResTemp << std::endl;
		statsText << fDebugPrintHeap( Memory::tMainMemoryProvider::fInstance( ), "  main" ) << std::endl;
#ifdef platform_xbox360
		statsText << fDebugPrintHeap( tGameAppBase::mVramHeap, " VRAM" )  << std::endl;
#endif//platform_xbox360
		statsText << " p spill = " << std::fixed << std::setprecision( 2 ) << memPoolSpill << std::endl;
		statsText << "   sqrat = " << std::fixed << std::setprecision( 2 ) << memSqrat << std::endl;
		statsText << "  script = " << std::fixed << std::setprecision( 2 ) << memScript << std::endl;
		statsText << "   audio = " << std::fixed << std::setprecision( 2 ) << memAudio << std::endl;
		statsText << " xalloc = " << std::fixed << std::setprecision( 2 ) << memXAlloc << std::endl;
		statsText << "  geom = " << std::fixed << std::setprecision( 2 ) << memGeom << std::endl;

		const u32 totalSecondsRun = fRound<u32>( mSceneGraph->fElapsedTime( ) );
		statsText << "time = " << totalSecondsRun / 60 << ":" << std::setfill( '0' ) << std::setw( 2 ) << totalSecondsRun % 60 << std::endl;
		statsText << "load = " << mLastLevelLoadTime << " s" << std::endl;
		fAddToStatText( statsText );
#endif//sig_devmenu

		return statsText.str( );
	}

	void tGameAppBase::fAutoReloadResources( )
	{
		profile_pix("fAutoReloadResources");
#ifdef sig_devmenu // attempt to reload textures (must happen immediately after rendering)
		if( mFilePackages.fCount( ) > 0 || !Resources_AutoReload )
			return;

		const u64 newBuildStamp = fComputeBuildStamp( );
		if( mBuildStamp == newBuildStamp )
			return;

		fSceneGraph( )->fWaitForLogicRunListsToComplete( );

		log_line( Log::cFlagResource, "BuildStamp has changed, reloading resources..." );

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

	void tGameAppBase::fDumpMemory( const char* context, b32 justLog )
	{

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

		{
			Memory::tHeapStack stacker( &Memory::tMallocHeap::fInstance( ) );

			Memory::tMemoryDumpOptions dump( justLog );
			if( !justLog )
				dump.mDump = NEW Memory::tMemoryDump( );

			Memory::tPool::fDumpGlobalPoolAllocations( dump );
			Memory::tMainMemoryProvider::fInstance( ).fDump( dump );
			Memory::tResourceMemoryProvider::fInstance( ).fDump( dump );
#ifdef platform_xbox360
			mVramHeap.fDump( dump );
			mNoCacheHeap.fDump( dump );
#endif

			if( dump.mDump )
			{
				//dump.mDump->fSave( tFilePathPtr::fConstructPath( dir, fileName ) );
				dump.mDump->fSaveXml( tFilePathPtr::fConstructPath( dir, fileName ) );
			}

			delete dump.mDump;
		}
	}

	void tGameAppBase::fConsolidateMemory( )
	{
#ifdef platform_xbox360
		mVramHeap.fConsolidate( );
		mNoCacheHeap.fConsolidate( );
#endif
		Memory::tResourceMemoryProvider::fInstance( ).fConsolidate( );
		Memory::tMainMemoryProvider::fInstance( ).fConsolidate( );
	}

	void tGameAppBase::fLogFreeMemory( )
	{
#ifdef platform_xbox360
		mVramHeap.fLogFreeMemory( );
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
				.Prop("IsSinglePlayer", &tGameMode::fIsSinglePlayer)
				.Prop("IsVersus", &tGameMode::fIsVersus)
				.Prop("IsCoOp", &tGameMode::fIsCoOp)
				.Prop("IsSinglePlayerOrCoop", &tGameMode::fIsSinglePlayerOrCoop)
				.Prop("IsMultiPlayer", &tGameMode::fIsMultiPlayer)
				.Prop("IsLocal", &tGameMode::fIsLocal)
				.Prop("IsNet", &tGameMode::fIsNet)
				.Prop("IsSplitScreen", &tGameMode::fIsSplitScreen)
				.Prop("IsSteamBigPictureMode", &tGameMode::fIsSteamBigPictureMode)
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
				.Prop(_SC("WarningCanvas"),	&tGameAppBase::fWarningCanvasFromScript)
				.Prop(_SC("StatusCanvas"),	&tGameAppBase::fStatusCanvasFromScript)
				.Func(_SC("LocString"),		&tGameAppBase::fLocString)
				.Prop(_SC("IsFullVersion"),		&tGameAppBase::fIsFullVersion)
				.Func(_SC("ComputeViewportRect"), &tGameAppBase::fViewportRect)
				.Func(_SC("ComputeViewportSafeRect"), &tGameAppBase::fViewportSafeRect)
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
