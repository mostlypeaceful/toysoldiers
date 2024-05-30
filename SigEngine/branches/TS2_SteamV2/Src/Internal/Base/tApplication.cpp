#include "BasePch.hpp"
#include "tApplication.hpp"
#include "FileSystem.hpp"
#include "tProfiler.hpp"

namespace Sig
{
	devvar( bool, Debug_Network_NoTimeout, false );
	devvar( bool, Debug_Memory_AllowNewPages, false );

	tApplication* tApplication::gThis=0;

	tApplication::tApplication( )
		: mResourceDepot( NEW tResourceDepot )
#ifdef target_tools
		, mSceneGraph( NEW tSceneGraph( ) )
#endif//target_tools
		, mKeepRunning( true )
		, mAppTime( 0.f )
		, mFrameDeltaTime( 0.f )
		, mSecondSignInWaitTime( -1.f )
		, mSystemUiShowing( false )
#if defined( platform_pcdx ) && defined( target_game )
		, mCallbackSteamOverlayToggled( this, &tApplication::fOnSteamOverlayToggled )
#endif // defined( platform_pcdx ) && defined( target_game )
	{
		gThis = this;
	}

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

	////------------------------------------------------------------------------------
	//void tApplication::fIncLocalUserInputLevels( )
	//{
	//	const u32 userCount = mLocalUsers.fCount( );
	//	for( u32 u = 0; u < userCount; ++u )
	//	{
	//		if( !mLocalUsers[ u ] )
	//			continue;

	//		mLocalUsers[ u ]->fIncInputFilterLevel( );
	//	}
	//}

	////------------------------------------------------------------------------------
	//void tApplication::fDecLocalUserInputLevels( )
	//{
	//	const u32 userCount = mLocalUsers.fCount( );
	//	for( u32 u = 0; u < userCount; ++u )
	//	{
	//		if( !mLocalUsers[ u ] )
	//			continue;

	//		mLocalUsers[ u ]->fDecInputFilterLevel( );
	//	}
	//}

	//------------------------------------------------------------------------------
	s32 tApplication::fRun( const std::string& cmdLineBuffer )
	{
		if( !fExternalInit( cmdLineBuffer ) )
			return -1;

		while( mKeepRunning )
		{
			profile_pix("gameloop");
			if( fUpdateApplicationTime( ) )
				fOnTick( );
		}

		fExternalShutdown( );

		return 0;
	}

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

		// Ensure tSync::fInstance is created before tSceneGraph starts spinning up threads to
		// avoid singleton construction race issue between scene graph syncs and thread registrations.
		tSync::fInstance( );
		
		fSetGameRoot( );
		
#ifdef target_game
		fLoadDevMenuIniFiles( fResourceDepot( )->fRootPath( ) );
		mSceneGraph.fReset( NEW tSceneGraph( ) );
#endif//target_game

		if( Debug_Network_NoTimeout )
			fSetNetworkTimeouts( ~0, ~0 );
		
		fStartupApp( );
		
		profile_reset( );
		
		return true;
	}
	
	void tApplication::fExternalShutdown( )
	{
		profile_shutdown( );
		
		fUnloadAlwaysLoadedResources( );
		
		fShutdownApp( );
		
		//sigassert( fResourceDepot( )->fResourcesWithLoadRefsCount( ) == 0 );
		
		log_line( 0, "_________________________________________" );
		log_line( 0, "|                                       |" );
		log_line( 0, "|          ...Application ending        |" );
		log_line( 0, "*_______________________________________*" );
		
		fPostRun( );
	}

	void tApplication::fExternalOnTick( f32 dt )
	{
		if( fUpdateApplicationTime( dt ) )
			fOnTick( );
	}

	void tApplication::fOnTick( )
	{
		profile_pix("fOnTick");

		mSceneGraph->fWaitForLogicRunListsToComplete( );

		fPreOnTick( );

#if defined(platform_pcdx9)
		if(!Gfx::tDevice::fGetDefaultDevice( )->fPrepareForGameTick())
		{
			// Audio needs to be updated while the app doesn't have focus so sounds can be stopped and volumes zeroed
			//fTickAudio( 0.0f );
			return;
		}
#endif

		// update any loading resources
		mResourceDepot->fUpdateLoadingResources( mOptions.mResourceLoadingTimeoutMS );

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
			mAppState->fOnTick( );

		// update the application itself
		fTickApp( );

		fPostOnTick( );
	}

	void tApplication::fPollInputDevices( )
	{
		profile_pix("fPollInputDevices");

		const f32 dt = fGetFrameDeltaTime( );

		Input::tKeyboard::fCaptureGlobalState( dt );

		for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
		{
			if( mLocalUsers[ i ] )
			{
				sigassert( mLocalUsers[ i ]->fIsLocal( ) && "Only local users should be in the LOCAL users array" );
				mLocalUsers[ i ]->fPollInputDevices( fGetFrameDeltaTime( ) );
			}
		}
	}

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

#ifdef sig_logging
				{
					// set log filters
					const tDevMenuIni::tEntry* entry = iniFile->fFindEntry( tStringPtr( "Log_Flags" ) );
					if( entry )
					{
						u32 logMask = Log::fGetLogFilterMask( );
						std::string loggingGroups = "Default";

						tGrowableArray< std::string > flags;
						StringUtil::fSplit( flags, entry->mStringValue.fCStr( ), "," );
						for( u32 i = 0; i < flags.fCount( ); ++i )
						{
							std::string cleaned = StringUtil::fEatWhiteSpace( flags[ i ] );
							for( u32 f = 0; f < Log::tFlagsType::fFlagsTypeCount( ); ++f )
							{
								const Log::tFlagsType& flag = Log::tFlagsType::fFlagsType( f );
								if( strcmp( flag.mShortName, cleaned.c_str( ) ) == 0 )
								{
									if( loggingGroups != "" ) loggingGroups += ", ";
									loggingGroups += flag.mShortName;

									logMask |= flag.mFlag;
									break;
								}
							}
						}

						log_line( 0, "Displaying log groups: " << loggingGroups );
						Log::fSetLogFilterMask( logMask );
					}
					else
					{
						log_line( 0, "Displaying default log groups" );
					}

					entry = iniFile->fFindEntry( tStringPtr( "Log_NoSpam" ) );
					if( entry )
						Log::fSetLogNoSpam( entry->mValue.x != 0 );
				}
#endif// sig_logging
			}
		}
#endif//sig_devmenu
	}

	void tApplication::fClearDevMenuIniFiles( )
	{
		mDevMenuInis.fDeleteArray( );
	}

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
		mAlwaysLoadedResources.fPushBack( res );
		res->fLoadDefault( this );
		if( blockUntilLoaded )
			res->fBlockUntilLoaded( );
		return res;
	}

	void tApplication::fUnloadAlwaysLoadedResources( )
	{
		for( u32 i = 0; i < mAlwaysLoadedResources.fCount( ); ++i )
			mAlwaysLoadedResources[ i ]->fUnload( this );
		mAlwaysLoadedResources.fSetCount( 0 );
	}

	void tApplication::fNewApplicationState( const tApplicationStatePtr& appStatePtr )
	{
		mNextAppState = appStatePtr;
	}

	b32 tApplication::fUpdateApplicationTime( f32 dtOverride )
	{
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

	b32	tApplication::fIsDevMenuActive( ) const
	{
		for( u32 i = 0; i < mLocalUsers.fCount( ); ++i )
		{
			if( mLocalUsers[ i ]->fDevMenu( ).fIsActive( ) )
				return true;
		}

		return false;
	}

	// Couldn't quite figure out how to make these project independent
	const Sig::Memory::tMemoryOptions& Sig::tApplication::fMemoryOptions( )
	{
		static Sig::Memory::tMemoryOptions ts2SpecifcOptions;
		ts2SpecifcOptions.mDebugAllowNewPages = Debug_Memory_AllowNewPages || tSync::fSyncEnabled( );
		return ts2SpecifcOptions;
	}

}

namespace Sig
{
	namespace 
	{
		void fExitGame( tApplication* app )
		{
			app->fQuitAsync( false );
		}
	}

	void tApplication::fExportScriptInterface( tScriptVm& vm )
	{
#define logic_event_script_export
#	include "ApplicationEvents.hpp"
#undef logic_event_script_export

		Sqrat::Class<tApplication, Sqrat::NoConstructor> classDesc( vm.fSq( ) );

		classDesc
			.GlobalFunc(_SC("ExitGame"),			&fExitGame)
			.Prop(_SC("SysUiShowing"),				&tApplication::fSystemUiShowing)
			.Func(_SC("FindActiveLocalUser"),		&tApplication::fFindActiveLocalUser)
			//.Func(_SC("IncLocalUserInputLevels"),	&tApplication::fIncLocalUserInputLevels)
			//.Func(_SC("DecLocalUserInputLevels"),	&tApplication::fDecLocalUserInputLevels)
			;

		vm.fRootTable( ).Bind(_SC("tApplication"), classDesc );

		vm.fRootTable( ).SetInstance(_SC("Application"), &fInstance( ));
	}

}
