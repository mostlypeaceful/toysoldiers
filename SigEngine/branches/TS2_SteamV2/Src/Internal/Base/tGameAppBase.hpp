#ifndef __tGameAppBase__
#define __tGameAppBase__
#include "tApplication.hpp"
#include "tFilePackage.hpp"
#include "Audio/tSystem.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tScreen.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tLightEntity.hpp"
#include "Gui/tFpsText.hpp"
#if defined( use_steam )
#include "tSteamManager.hpp"
#endif

namespace Sig
{
	class tGameMode
	{
	public:
		enum tState
		{
			cStateFrontEnd,
			cStateNormal,
			cStateCount,
		};
		enum tFlags
		{
			cFlagCoOp = ( 1 << 0 ),
			cFlagVersus = ( 1 << 1 ),
			cFlagOnline = ( 1 << 2 ),
			cMaskPlayWithOthers = cFlagCoOp | cFlagVersus | cFlagOnline,
		};
	public: // create
		tGameMode( ) : mState( cStateCount ), mFlags( 0 ), pad0( 0 ), pad1( 0 ) { }
		void fSetState( tState state, u32 flags = 0 ) { mState = state; mFlags = flags; }
		void fSetInvalid( ) { mState = cStateCount; mFlags = 0; }
		void fAddCoOpFlag( ) { mFlags |= cFlagCoOp; }
		void fAddVersusFlag( ) { mFlags |= cFlagVersus; }
		void fAddOnlineFlag( ) { mFlags |= cFlagOnline; }
		void fRemoveOnlineFlag( ) { mFlags &= ~cFlagOnline; }
	public: // compare
		inline b32 operator==( const tGameMode& other ) const { return mState == other.mState && mFlags == other.mFlags; }
	public: // query
		tState fState( ) const { return ( tState )mState; }
		u32 fFlags( ) const { return mFlags; }
		b32 fInvalid( ) const { return mState == cStateCount; }
		b32 fIsFrontEnd( ) const { return mState == cStateFrontEnd; }
		b32 fIsSinglePlayer( ) const { return !fIsMultiPlayer( ); }
		b32 fIsVersus( ) const { return fTestBits( mFlags, cFlagVersus ); }
		b32 fIsCoOp( ) const { return fTestBits( mFlags, cFlagCoOp ); }
		b32 fIsSinglePlayerOrCoop( ) const { return fIsSinglePlayer( ) || fIsCoOp( ); }
		b32 fIsMultiPlayer( ) const { return fTestBits( mFlags, cMaskPlayWithOthers ); }
		b32 fIsLocal( ) const { return !fIsNet( ); }
		b32 fIsNet( ) const { return fTestBits( mFlags, cFlagOnline ); }
		b32 fIsSplitScreen( ) const { return fIsMultiPlayer( ) && fIsLocal( ); }
#if defined( use_steam )
		b32 fIsSteamBigPictureMode( ) const { return tSteamManager::fInstance( ).fIsBigPictureMode( ); }
#else
		b32 fIsSteamBigPictureMode( ) const { return false; }
#endif
	public:
		template<class tArchive>
		void fSaveLoad( tArchive & archive )
		{
			archive.fSaveLoad( mState );
			archive.fSaveLoad( mFlags );
		}

	private:
		u8 mState;
		u8 mFlags;
		u8 pad0;
		u8 pad1;
	};


	///
	/// \brief Represents the game-specific implementation of the application, 
	/// including platform-independent game initialization, shutdown, and update logic.
	class tGameAppBase : public tApplication
	{
		define_dynamic_cast( tGameAppBase, tApplication );
	public:
		static inline tGameAppBase& fInstance( ) { sigassert( gGameAppBase ); return *gGameAppBase; }

	private:
		static tGameAppBase* gGameAppBase;
	protected:
		b32											mSimulate;
		b32											mInGameSimulationState;
		b32											mFixedTimeStep;

		u64											mBuildStamp;
		u32											mVRamUsed;
		f32											mLastLevelLoadTime;
		tGrowableArray< tFilePackagePtr >			mFilePackages;
		Audio::tSystemPtr							mSoundSystem;
		Gfx::tDevicePtr								mDevice;
		Gfx::tScreenPtr								mScreen;
		Gfx::tLightEntityPtr						mDefaultLight;
		Gui::tCanvasPtr								mRootHudCanvas;
		Gui::tCanvasPtr								mWarningCanvas;
		Gui::tCanvasPtr								mStatusCanvas;
		tHashTable< std::string, Gui::tCanvasPtr >	mHudLayers;
		tResourcePtr								mPostEffectsMaterialFile;
		tResourcePtr								mDefaultFont;
		tResourcePtr								mLocTextFile;
		tResourcePtr								mLocResFile;
		tDynamicArray<tResourcePtr>					mFonts;
		Gui::tFpsTextPtr							mFpsText;
		Gui::tTextPtr								mStatsText;

		b32											mIsFullVersion;

	public:

		tGameAppBase( );
		~tGameAppBase( );

		virtual void fTickApp( );
		virtual void fStartupApp( );
		virtual void fShutdownApp( );

		// these will not work correctly as bools! due to sqrat :(
		virtual void fPause( bool pause ) { }
		virtual bool fPaused( ) const { return false; }

		virtual void fSetSimulate( b32 sim ) { mSimulate = sim; }
		virtual b32 fSimulate( ) const { return mSimulate; }

		virtual void fSetIngameSimulationState( b32 sim ) { mInGameSimulationState = sim; }
		virtual b32 fIngameSimulationState( ) const { return mInGameSimulationState; }

		virtual void fEnableFixedTimeStep( b32 fixed );
		virtual b32 fFixedTimeStepEnabled( ) const { return mFixedTimeStep; }
		virtual void fSetFixedTimeStep( f32 fixedTimeStep );

		virtual b32 fUseLowDetailTextures( ) { return false; }
		virtual float fGetShadowRange() { return 600.0f;  }

		virtual void fAddToStatText( std::stringstream& currentStats ) { }

		virtual void fOnDeviceReset( ) { }

		void fChangeViewportCount( 
			u32 newCount, 
			u32 localUserCount, 
			b32 updateUsers,
			tUserArray * remoteUsers = NULL );
		void fResetViewportCameras( );
		virtual void fResetPlayerCameraAspectRatios() { }

		const Gfx::tScreenPtr& fScreen( ) const { return mScreen; }
		const Gfx::tDevicePtr& fDevice( ) const { return mDevice; }
		const Gfx::tLightEntityPtr& fDefaultLight( ) const { return mDefaultLight; }
		void fSetDefaultLight( Gfx::tLightEntity* light ) { mDefaultLight.fReset( light ); }
		const Audio::tSystemPtr& fSoundSystem( ) const { return mSoundSystem; }
		Gui::tCanvasPtr& fRootHudCanvas( ) { return mRootHudCanvas; }
		const Gui::tCanvasPtr& fRootHudCanvas( ) const { return mRootHudCanvas; }
		Sqrat::Object fRootHudCanvasFromScript( ) { return mRootHudCanvas.fScriptObject( ); }
		Gui::tCanvasPtr& fHudLayer( const std::string& layerName );
		Sqrat::Object fHudLayerFromScript( const std::string& layerName );
		
		Gui::tCanvasPtr& fWarningCanvas( ) { return mWarningCanvas; }
		const Gui::tCanvasPtr& fWarningCanvas( ) const { return mWarningCanvas; }
		Sqrat::Object fWarningCanvasFromScript( ) { return mWarningCanvas.fScriptObject( ); }

		Gui::tCanvasPtr& fStatusCanvas( ) { return mStatusCanvas; }
		const Gui::tCanvasPtr& fStatusCanvas( ) const { return mStatusCanvas; }
		Sqrat::Object fStatusCanvasFromScript( ) { return mStatusCanvas.fScriptObject( ); }


		Math::tRect fViewportRect( u32 ithViewport );
		Math::tRect fViewportSafeRect( u32 ithViewport );

		u32 fVRamUsed( ) const { return mVRamUsed; }
		void fSetVRamUsed( u32 numBytes ) { mVRamUsed = numBytes; }
		f32 fLastLevelLoadTime( ) const { return mLastLevelLoadTime; }
		void fSetLastLevelLoadTime( f32 time ) { mLastLevelLoadTime = time; }

		b32  fIsFullVersion( ) const { return mIsFullVersion; }
		virtual void fReadLicense( );
		virtual void fOnFullVersionInstalled( ) { }
		
		void fDumpMemory( const char* context, b32 justLog );
		virtual void fConsolidateMemory( );
		virtual void fLogFreeMemory( );

	public: // localization
		b32 fLocStringExists( const tStringPtr& stringId ) const;
		const tLocalizedString& fLocString( const tStringPtr& stringId ) const;
		const tResourcePtr& fLocFont( u32 fontId ) { return mFonts[ fontId ]; }
		static tResourcePtr fFontFromId( u32 fontId ) { return fInstance( ).fLocFont( fontId ); }
		virtual u8 fGetLocalizedThousandsSeparator( ) const { return ','; }
		virtual u8 fGetLocalizedDecimalSeparator( ) const { return '.'; }
	protected:

		struct tGameStartupParams
		{
			u32 mScreenWidth;
			u32 mScreenHeight;
			Math::tVec4f mRgbaClearColor;
			f32 mWorldHalfAxisLength;
			u32 mViewportCount;
			Audio::tAudioOptions mAudioOptions;
			Gfx::tDefaultAllocatorSettings mMemory;

			tGameStartupParams( )
				: mScreenWidth( 1280 )
				, mScreenHeight( 720 )
				, mRgbaClearColor( 164.f/255.f, 216.f/255.f, 255.f/255.f, 1.f )
				, mWorldHalfAxisLength( 1000.f )
				, mViewportCount( 1 )
			{
			}

		} mStartupParams; // if you want to change defaults, then change them in your app's constructor

		virtual void fPreLoadFilePackages( );
		virtual void fFillOutScreenCreateOpts( Gfx::tScreenCreationOptions& screenCreateOpts ) { }
		virtual void fStartupScript( );
		virtual void fStartupSceneGraph( );
		virtual void fStartupGfx( );
		virtual void fStartupProfiler( );
		virtual void fCreateDevice( Gfx::tScreenCreationOptions& screenCreateOptsOut ); // should allocate 'mDevice'
		virtual void fStartupSfx( ) { }
		virtual void fStartupUsers( );
		virtual void fExportGameScriptInterfaces( tScriptVm& vm ) { }
		virtual void fLoadPermaLoadedGameResources( ) { }
		virtual void fSetupPostEffects( ) { }
		virtual void fPreview( const tFilePathPtr& previewPath ) { }
		virtual void fReplay( const tFilePathPtr& replayPath ) { }
		virtual void fStartGame( ) { }
		virtual void fCreateGeometryAllocators( ); // you can override this to create more, just call down to this one as well
		virtual void fLoadMaterials( ); // you can override this to load more, just call down to this one as well
		virtual void fTickCameras( f32 dt ) { }
		virtual void fTickCanvas( f32 dt );

	protected:

		void fLoadLocResources( const tFilePathPtr& locFolder, const tDynamicArray<tStringPtr>& fontNames );
		void fTickRender( f32 dt );
		void fTickAudio( f32 dt );
		void fPrintDebugStats( );
		std::string fMakeDebugString( );
		void fAutoReloadResources( );
		u64  fComputeBuildStamp( ) const;
		b32  fShouldDoSingleStep( ) const;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

}

#endif//__tGameAppBase__
