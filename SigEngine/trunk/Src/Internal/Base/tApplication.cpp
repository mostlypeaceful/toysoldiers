#include "BasePch.hpp"
#include "tApplication.hpp"
#include "FileSystem.hpp"
#include "tProfiler.hpp"

namespace Sig
{
	devvar( bool, Debug_Network_NoTimeout, false );

#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
	devvar( bool, Debug_Input_RequireWindowFocus, false );
#endif

	//------------------------------------------------------------------------------
	// tApplication
	//------------------------------------------------------------------------------
	tApplication* tApplication::gThis = 0;
	u32 tApplication::gTickCount = 0;

	//------------------------------------------------------------------------------
	tApplication::tApplication( )
		: mResourceDepot( NEW tResourceDepot )
#ifdef target_tools
		, mSceneGraph( NEW tSceneGraph( ) )
#endif
		, mKeepRunning( true )
		, mAppTime( 0.f )
		, mFrameDeltaTime( 0.f )
		, mSecondSignInWaitTime( -1.f )
		, mSystemUiShowing( false )
	{
		gThis = this;
	}

	//------------------------------------------------------------------------------
	tApplication::~tApplication( )
	{
		gThis = 0;
	}

	//------------------------------------------------------------------------------
	u32 tApplication::fFindActiveLocalUser( u32 buttonMask ) const
	{
		const u32 userCount = mLocalUsers.fCount( );
		for( u32 u = 0; u < userCount; ++u )
		{
			if( !mLocalUsers[ u ] )
				continue;

			if( mLocalUsers[ u ]->fRawGamepad( ).fButtonDown( buttonMask ) )
				return u;
		}

		return ~0;
	}

	//------------------------------------------------------------------------------
	s32 tApplication::fRun( const std::string& cmdLineBuffer )
	{
		fInitProfilingData( );

		sigassert( gTickCount == 0 );
		if( !fExternalInit( cmdLineBuffer ) )
			return -1;

		while( mKeepRunning )
		{
			if( fUpdateApplicationTime( ) )
				fOnTick( );
		}

		fExternalShutdown( );

		return 0;
	}

	//------------------------------------------------------------------------------
	b32 tApplication::fExternalInit( const std::string& cmdLineBuffer )
	{
		mCmdLine = cmdLineBuffer;
		
		fConfigureApp( mOptions, mPlatformOptions );
		
		if( !fPreRun( ) )
			return false;
		
		log_line( 0, "_________________________________________" );
		log_line( 0, "|                                       |" );
		log_line( 0, "|     Signal Studios SigEngine v2.0     |" );
		log_line( 0, "|        Application beginning...       |" );
		log_line( 0, "              " << mOptions.mGameName );
		log_line( 0, "*_______________________________________*" );

#if defined( build_debug )
		const char* build = "DEBUG";
#elif defined( build_internal )
		const char* build = "INTERNAL";
#elif defined( build_playtest )
		const char* build = "PLAYTEST";
#elif defined( build_profile )
		const char* build = "PROFILE";
#elif defined( build_release )
		const char* build = "RELEASE";
#else
		const char* build = "UNKNOWN BUILD CONFIG";
#endif
		log_line( 0, "BUILD: " << build );
		
		fSetGameRoot( );

		mFilePackageProvider = tFilePackageResourceProviderPtr( NEW tFilePackageResourceProvider( ) );
		mResourceDepot->fAddResourceProvider( tResourceProviderPtr( mFilePackageProvider.fGetRawPtr( ) ) );

		tResourceProviderPtr resProvider = tResourceProviderPtr( NEW tFileSystemResourceProvider( ) );
		resProvider->fSetRootPath( mGameRoot );
		mResourceDepot->fAddResourceProvider( resProvider );
		
#ifdef target_game
		fLoadDevMenuIniFiles( mGameRoot );
		mSceneGraph.fReset( NEW tSceneGraph( ) );
#endif//target_game

		if( Debug_Network_NoTimeout )
			fSetNetworkTimeouts( ~0, ~0 );
		
		fStartupApp( );
		fUnloadInitializationResources( );

		
		profile_reset( );
		
		return true;
	}
	
	//------------------------------------------------------------------------------
	void tApplication::fExternalShutdown( )
	{
		profile_shutdown( );
		
		fUnloadAlwaysLoadedResources( );
		
		fShutdownApp( );
		
		log_line( 0, "_________________________________________" );
		log_line( 0, "|                                       |" );
		log_line( 0, "|          ...Application ending        |" );
		log_line( 0, "*_______________________________________*" );
		
		fPostRun( );
	}

	//------------------------------------------------------------------------------
	void tApplication::fExternalOnTick( f32 dt )
	{
		if( fUpdateApplicationTime( dt ) )
			fOnTick( );
	}

	//------------------------------------------------------------------------------
	void tApplication::fOnTick( )
	{
		profile_pix("tApplication::fOnTick");
		mSceneGraph->fWaitForLogicRunListsToComplete( );

		fPreOnTick( );

		mResourceDepot->fUpdateLoadingResources( );

#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
		if( !Debug_Input_RequireWindowFocus || GetForegroundWindow( ) == fGetHwnd( ) )
#endif
			fPollInputDevices( );

		// step application state
		if( mNextAppState )
		{
			mAppState = mNextAppState;
			if( mAppState )
				mAppState->fOnBecomingCurrent( );
			mNextAppState.fRelease( );
		}
		if( mAppState )
		{
			mAppState->fOnTick( );
		}

		// update the application itself
		fTickApp( );

		fPostOnTick( );
	}

	//------------------------------------------------------------------------------
	void tApplication::fPollInputDevices( )
	{
		profile_pix( "tApplication::fPollInputDevices" );
		for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
		{
			if( mLocalUsers[ i ] )
			{
				sigassert( mLocalUsers[ i ]->fIsLocal( ) && "Only local users should be in the LOCAL users array" );
				mLocalUsers[ i ]->fPollInputDevices( fGetFrameDeltaTime( ) );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tApplication::fLoadDevMenuIniFiles( const tFilePathPtr& gameRoot )
	{
#ifdef sig_devmenu
		tFilePathPtrList files;

		const tFilePathPtr pathToRes( "..\\..\\res" );
		if( FileSystem::fFolderExists( pathToRes ) )
			FileSystem::fGetFileNamesInFolder( files, pathToRes, true, false );
		else
			FileSystem::fGetFileNamesInFolder( files, gameRoot, true, false );

		for( u32 i = 0; i < files.fCount( ); ++i )
		{
			if( !StringUtil::fCheckExtension( files[ i ].fCStr( ), ".ini" ) )
				continue;
			tDevMenuIniPtr iniFile( NEW tDevMenuIni );
			if( iniFile->fReadFile( files[ i ] ) )
			{
				mDevMenuInis.fPushBack( iniFile );

				{
					// set command line override
					const tDevMenuIni::tEntry* entry = iniFile->fFindEntry( tStringPtr( "@CmdLine" ) );
					if( entry )
						mCmdLine = entry->mStringValue.fCStr( );
				}

				{
					static const tStringPtr cDevStart( "DevStart" );
					const tDevMenuIni::tEntry* entry = iniFile->fFindEntry( cDevStart );
					if( entry )
						mDevMenuStart = entry->mStringValue.fCStr( );
				}

#ifdef sig_logging
				{
					// set log filters
					const tDevMenuIni::tEntry* entry = iniFile->fFindEntry( tStringPtr( "Log_Flags" ) );
					if( entry )
					{
						u32 logMask = Log::fGetLogFilterMask( );

						tGrowableArray< std::string > flags;
						StringUtil::fSplit( flags, entry->mStringValue.fCStr( ), "," );
						for( u32 i = 0; i < flags.fCount( ); ++i )
						{
							std::string cleaned = StringUtil::fEatWhiteSpace( flags[ i ] );
							b32 disable = cleaned.length( ) && cleaned[0] == '-';
							if( disable )
								cleaned.erase(0,1);
							for( u32 f = 0; f < Log::tFlagsType::fFlagsTypeCount( ); ++f )
							{
								const Log::tFlagsType& flag = Log::tFlagsType::fFlagsType( f );
								if( strcmp( flag.mShortName, cleaned.c_str( ) ) == 0 )
								{
									logMask |= flag.mFlag;
									if( disable )
										logMask &= ~flag.mFlag;
									break;
								}
							}
						}

						Log::fSetLogFilterMask( logMask );
					}

					entry = iniFile->fFindEntry( tStringPtr( "Log_NoSpam" ) );
					if( entry )
						Log::fSetLogNoSpam( entry->mValue.x != 0 );
				}
#endif// sig_logging
			}
		}
		//display all logging groups
		std::string loggingGroups;
		for(u32 i = 0; i < Log::tFlagsType::fFlagsTypeCount( ); ++i)
		{
			if( Log::fFlagEnabled( i ) )
			{
				loggingGroups += ' ';
				loggingGroups += Log::tFlagsType::fFlagsType( i ).mShortName;
			}
		}
		log_line( 0, "Logging groups enabled:" << loggingGroups );
#endif//sig_devmenu
	}

	//------------------------------------------------------------------------------
	void tApplication::fClearDevMenuIniFiles( )
	{
		mDevMenuInis.fDeleteArray( );
	}

	//------------------------------------------------------------------------------
	tResourcePtr tApplication::fAddInitializationResource( const tResourceId& rid, b32 blockUntilLoaded )
	{
		for( u32 i = 0; i < mInitializationResources.fCount( ); ++i )
		{
			if( mInitializationResources[ i ]->fGetResourceId( ) == rid )
				return mInitializationResources[ i ]; // already loaded
		}

		tResourcePtr res = mResourceDepot->fQuery( rid );
		mInitializationResources.fPushBack( res );
		res->fLoadDefault( this );
		if( blockUntilLoaded )
			res->fBlockUntilLoaded( );
		return res;
	}

	//------------------------------------------------------------------------------
	tResourcePtr tApplication::fAddAlwaysLoadedResource( const tResourceId& rid, b32 blockUntilLoaded )
	{
		for( u32 i = 0; i < mAlwaysLoadedResources.fCount( ); ++i )
		{
			if( mAlwaysLoadedResources[ i ]->fGetResourceId( ) == rid )
			{
				// already perma-loaded
				if( blockUntilLoaded && !mAlwaysLoadedResources[ i ]->fLoaded( ) )
					mAlwaysLoadedResources[ i ]->fBlockUntilLoaded( );
				
				return mAlwaysLoadedResources[ i ];
			}
		}

		tResourcePtr res = mResourceDepot->fQuery( rid );
		if( res )
		{
			mAlwaysLoadedResources.fPushBack( res );
			res->fLoadDefault( this );
			if( blockUntilLoaded )
				res->fBlockUntilLoaded( );
		}
		return res;
	}

	//------------------------------------------------------------------------------
	void tApplication::fInitProfilingData( )
	{
#if defined( profile_pix_textures )
		PIXEnableTextureTracking(0,0,0);
#endif
	}

	//------------------------------------------------------------------------------
	void tApplication::fUnloadAlwaysLoadedResources( )
	{
		mAlwaysLoadedResources.fSetCount( 0 );
	}

	//------------------------------------------------------------------------------
	void tApplication::fUnloadInitializationResources( )
	{
		mInitializationResources.fSetCount( 0 );
	}

	//------------------------------------------------------------------------------
	void tApplication::fNewApplicationState( const tApplicationStatePtr& appStatePtr )
	{
		mNextAppState = appStatePtr;
	}

	//------------------------------------------------------------------------------
	b32 tApplication::fUpdateApplicationTime( f32 dtOverride )
	{
		interlocked_inc( &gTickCount );

		f32 elapsed = 0.f;
		if( dtOverride > 0.f )
			elapsed = dtOverride;
		else
		{
			const f32 elapsedRaw	= mAppTimer.fGetElapsedS( );
			const f32 elapsedMax	= mOptions.mFrameTimeDeltaClamp > 0.f ? mOptions.mFrameTimeDeltaClamp : Math::cInfinity;
			elapsed					= fMin( elapsedRaw, elapsedMax );
		}

		if( elapsed < mOptions.mUpdateDelay )
			return false;

		mAppTimer.fResetElapsedS( 0.f );
		mAppTime += elapsed;
		mFrameDeltaTime = elapsed;
		return true;
	}

	//------------------------------------------------------------------------------
	b32	tApplication::fIsDevMenuActive( ) const
	{
		for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
		{
			if( mLocalUsers[ i ]->fDevMenu( ).fIsActive( ) )
				return true;
		}

		return false;
	}

#ifdef target_tools 
	//------------------------------------------------------------------------------
	const Sig::Memory::tMemoryOptions& Sig::tApplication::fMemoryOptions( )
	{
		static Sig::Memory::tMemoryOptions toolsOptions;
		return toolsOptions;
	}
#endif // target_tools

}

namespace Sig
{
	//------------------------------------------------------------------------------
	void tApplication::fExportScriptInterface( tScriptVm& vm )
	{
#define logic_event_script_export
#	include "ApplicationEvents.hpp"
#undef logic_event_script_export

		Sqrat::Class<tApplication, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			.Func(_SC("ExitGame"),					&tApplication::fQuitAsync)
			.Prop(_SC("SysUiShowing"),				&tApplication::fSystemUiShowing)
			.Func(_SC("FindActiveLocalUser"),		&tApplication::fFindActiveLocalUser)
			.Prop(_SC("SceneGraph"),				&tApplication::fSceneGraphScript)
			;

		vm.fRootTable( ).Bind(_SC("tApplication"), classDesc );

		vm.fRootTable( ).SetInstance(_SC("Application"), &fInstance( ));
	}

}
