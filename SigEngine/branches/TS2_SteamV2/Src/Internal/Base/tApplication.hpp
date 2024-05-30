#ifndef __tApplication__
#define __tApplication__
#include "tUser.hpp"
#include "tCmdLineOption.hpp"
#include "tResourceDepot.hpp"
#include "tSceneGraph.hpp"
#include "Gui/tCanvas.hpp"

namespace Sig { namespace ApplicationEvent
{
#include "ApplicationEvents.hpp"
}}

#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#	include "tApplication_pc.hpp"
#elif defined( platform_xbox360 )
#	include "tApplication_xbox360.hpp"
#elif defined( platform_ios )
#	include "tApplication_ios.hpp"
#else
#	error Invalid platform for tApplication defined!
#endif

namespace Sig
{
	class tApplication;
	class tApplicationState;
	typedef tRefCounterPtr<tApplicationState> tApplicationStatePtr;


	///
	/// \brief Provides base functionality for the derived, game-specific
	/// game application class. An application's "main" should be implemented
	/// using the macro "implement_application( yourDerivedGameClassType )",
	/// and nothing more. You can see the platform-specific application class
	/// files to see how the macro is implemented if you are curious.
	class base_export tApplication : public tApplicationPlatformBase
	{
		friend class tApplicationState;
		define_dynamic_cast_base( tApplication );
	private:
		static tApplication* gThis;
	public:
		static inline tApplication& fInstance( ) { sigassert( gThis ); return *gThis; }
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		///
		/// \brief Startup options config. Can use defaults, or override as desired at the game level.
		struct base_export tStartupOptions
		{
			///
			/// \brief Game name is used for display in the windows start bar,
			/// title bar, logging, etc. Not too important.
			std::string		mGameName;

			///
			/// \brief The update delay will affect the core task kernel, meaning
			/// NOTHING will update more frequently than this value. Generally you
			/// will want to do your frame-rate regulation on a finer level, but this
			/// provides a quick-n-easy way of regulating/syncing all tasks to be
			/// frame-rate-independent.
			f32				mUpdateDelay;

			///
			/// \brief In an ideal world, the game would run one frame per this many seconds.
			/// \note This value is not referenced/used anywhere in the engine, but it serves
			/// as a common point of access for this value in game code (if relevant).
			f32				mFixedTimeStep;

			///
			/// \brief The frame delta time will be clamped to this value, if greater than zero.
			f32				mFrameTimeDeltaClamp;

			///
			/// \brief This value will be used as the maximum amount of time to perform
			/// resource updating per frame (in milliseconds).
			f32				mResourceLoadingTimeoutMS;

			///
			/// \brief Instructs the app to automatically configure file system with
			/// "res" directory as the root (should be used for game, probably not by tools).
			b32				mAutoDetectGameRoot;

			///
			/// \brief Game root override
			tFilePathPtr	mGameRootOverride;

			tStartupOptions( )
				: mGameName( "SigEngineGame" )
				, mUpdateDelay( 0.f )
				, mFixedTimeStep( 1.f / 30.f )
				, mFrameTimeDeltaClamp( 0.f )
				, mResourceLoadingTimeoutMS( 1.f )
				, mAutoDetectGameRoot( true )
			{
			}
		};

	protected:

		typedef tGrowableArray<tDevMenuIniPtr> tDevMenuIniArray;
		typedef tGrowableArray<tResourcePtr> tAlwaysLoadedResourceList;

		tApplicationStatePtr		mAppState;
		tApplicationStatePtr		mNextAppState;
		tUserArray					mLocalUsers;
		tStartupOptions				mOptions;
		tPlatformStartupOptions		mPlatformOptions;
		std::string					mCmdLine;
		tResourceDepotPtr			mResourceDepot;
		tSceneGraphPtr				mSceneGraph;
		Gui::tCanvasFrame			mRootCanvas;
		tAlwaysLoadedResourceList	mAlwaysLoadedResources;
		tDevMenuIniArray			mDevMenuInis;
		mutable Time::tStopWatch	mAppTimer;
		b32							mKeepRunning;
		f32							mAppTime;
		f32							mFrameDeltaTime;
		f32							mSecondSignInWaitTime; //for xbox FAQ #17
		b32							mSystemUiShowing;
#if defined( platform_pcdx ) && defined( target_game )
		STEAM_CALLBACK( tApplication, fOnSteamOverlayToggled, GameOverlayActivated_t, mCallbackSteamOverlayToggled );
#endif // defined( platform_pcdx ) && defined( target_game )

	public:

		///
		/// \brief Post a request to quit the application.
		/// If reboot is true, starts it again as fast as possible. in a platform dependent way.
		void fQuitAsync( b32 reboot = false ); // defined in platform-specific files

		///
		/// \brief Query for whether the app is scheduled to quit.
		b32 fKeepRunning( ) const { return mKeepRunning; }

		///
		/// \brief Access the startup options.
		const tStartupOptions& fGetOptions( ) const { return mOptions; }

		///
		/// \brief Access command line.
		const std::string& fGetCmdLine( ) const { return mCmdLine; }

		///
		/// \brief Access resource depot.
		const tResourceDepotPtr& fResourceDepot( ) const { return mResourceDepot; }

		///
		/// \brief Access scene graph.
		const tSceneGraphPtr& fSceneGraph( ) const { return mSceneGraph; }

		///
		/// \brief Access canvas objects
		Gui::tCanvasFrame& fRootCanvas( ) { return mRootCanvas; }
		const Gui::tCanvasFrame& fRootCanvas( ) const { return mRootCanvas; }

		///
		/// \brief Access the users array.
		const tUserArray& fLocalUsers( ) const { return mLocalUsers; }

		///
		/// \brief Returns the index of the first user with buttonMask held or ~0
		u32 fFindActiveLocalUser( u32 buttonMask = ~0 ) const;

		/////
		///// \brief Increments all local user input levels
		//void fIncLocalUserInputLevels( );

		/////
		///// \brief Decrements all local user input levels
		//void fDecLocalUserInputLevels( );

		///
		/// \brief Query for the absolute application time from the start of the application.
		f32 fGetApplicationTime( ) const { return mAppTime; }

		///
		/// \brief Query for the time delta between the current frame and the last frame.
		f32 fGetFrameDeltaTime( ) const { return mFrameDeltaTime; }

		///
		/// \brief Query for whether the "system UI" (platform-specific) is visible.
		b32 fSystemUiShowing( ) const { return mSystemUiShowing; }

		///
		/// \brief Runs the entire game, by running the game kernel. This method
		/// returns after the app runs out of tasks, or else after a quit message 
		/// has been posted by calling fQuitAsync.
		s32 fRun( const std::string& cmdLineBuffer );
		
		///
		/// \brief For systems/platforms that can't just control the entire application update loop.
		b32  fExternalInit( const std::string& cmdLineBuffer );
		void fExternalShutdown( );
		void fExternalOnTick( f32 dt );

	protected:

		tApplication( );
		virtual ~tApplication( );

		virtual void fConfigureApp( tStartupOptions& opts, tPlatformStartupOptions& platformOpts ) = 0;
		virtual void fTickApp( ) = 0;
		virtual void fStartupApp( ) = 0;
		virtual void fShutdownApp( ) = 0;

	public:
		virtual void fOnUserSignInChange( u32 userMask ) { }
		virtual void fOnSystemUiChange( b32 sysUiVisible ) { }
		virtual void fOnGameInviteAccepted( u32 localHwIndex ) { }
		virtual void fOnMuteListChanged( ) { }
		virtual void fOnStorageDeviceChanged( ) { }
		virtual void fOnPartyMembersChanged( ) { }
		virtual void fOnInputDevicesChanged( u32 connected ) { }
		virtual	void fOnDlcInstalled( ) { }
		virtual	void fReadLicense( ) { }

		virtual void fConsolidateMemory( ) { }
		virtual void fLogFreeMemory( ) { }

		static const Memory::tMemoryOptions& fMemoryOptions( );

	protected:

		f32 fElapsedFromTopOfFrameMs( ) const { return mAppTimer.fGetElapsedMs( ); }

		///
		/// \brief Polls input devices for all local users. Game apps may
		/// want to override to perform different functionality (for example buffered
		/// input for lockstep networking, etc).
		virtual void fPollInputDevices( );


		///
		/// \brief Load and apply user devmenu ini files.
		///
		/// In a nutshell, we look for all the *.ini files in
		/// the default devmenu ini file directory, and load and
		/// apply each one in turn; this means that if two .ini
		/// files have the same variable in it, the one that gets
		/// applied later will override the earlier one, meaning
		/// lexicographically larger file names win.
		/// regarding the default devmenu ini file directory:
		/// first we back up and look directly inside the res folder;
		/// if that folder doesn't exist, then we're probably running
		/// from a console hard-drive or something, which means we
		/// need to look in the root directory.
		void fLoadDevMenuIniFiles( const tFilePathPtr& gameRoot );

		///
		/// \brief Clear the list of devmenu ini files. This will generally have
		/// little effect, but any devmenu variables that are added after this
		/// call will use default values.
		void fClearDevMenuIniFiles( );

	public:
		///
		/// \brief Adds (and blocks untils loaded) a resource that the application wishes to
		/// remain loaded for the duration of the app's existence.
		/// \return The resource ptr requested (will either be loaded or will have failed to load).
		/// \note The "always loaded rzesources" will be unloaded prior to calling the derived game app's
		/// fShutdownApp( ) method; this means that if you need access to the resource within that method,
		/// you should add a load reference of your own to the returned resource.
		tResourcePtr fAddAlwaysLoadedResource( const tResourceId& rid, b32 blockUntilLoaded = false );
		const tGrowableArray<tResourcePtr>& fAlwaysLoadedResources( ) const { return mAlwaysLoadedResources; }

		///
		/// \brief A resource has failed to load.
		virtual void fResourceLoadFailure( tResource* resource ) { }

	private:

		///
		/// \brief Unloads all perma-loaded resources (this happens at shutdown time)
		void fUnloadAlwaysLoadedResources( );

	public:
		///
		/// \brief Transition to a new application state.
		void fNewApplicationState( const tApplicationStatePtr& appStatePtr );

		///
		/// \brief See if any dev menus are active
		b32	fIsDevMenuActive( ) const;

	protected:
		b32 fCacheCurrentGameInvite( u32 localHwIndex );

	private:

		void fSetGameRoot( );

		b32  fPreRun( );
		void fPostRun( );

		void fPreOnTick( );
		void fOnTick( );
		void fPostOnTick( );

		void fSetNetworkTimeouts( u32 peerMin, u32 peerMax );

		///
		/// \brief Internal method for updating application time.
		/// \return true if it's time for another update tick.
		b32 fUpdateApplicationTime( f32 dtOverride = -1.f );
	};


	///
	/// \brief Base class for application state object.
	class base_export tApplicationState : public tRefCounter
	{
	protected:
		tApplication&		mApp;
	public:
		tApplicationState( tApplication& app ) : mApp( app ) { }
		virtual ~tApplicationState( ) { }
		virtual void fOnBecomingCurrent( ) { }
		virtual void fOnTick( ) { }
	protected:
		void fNewApplicationState( const tApplicationStatePtr& appStatePtr ) { mApp.fNewApplicationState( appStatePtr ); }
	};

}


#endif//__tApplication__
