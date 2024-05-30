#ifndef __tGameAppBase__
#define __tGameAppBase__
#include "tApplication.hpp"
#include "tFilePackage.hpp"
#include "Audio/tSystem.hpp"
#include "tFuiMemoryOptions.hpp"
#include "tResourceLoadList2.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tScreen.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tLightEntity.hpp"
#include "Gui/tFpsText.hpp"

namespace Sig
{

	class tProjectFile;

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
		b32											mIsFullVersion;
		b32											mSimulate;
		b32											mInGameSimulationState;
		b32											mFixedTimeStep;

		u64											mBuildStamp;
		f32											mLastLevelLoadTime;
		tGrowableArray< tFilePackagePtr >			mPermaLoadedFilePackages;
		Audio::tSystemPtr							mSoundSystem;
		Gfx::tDevicePtr								mDevice;
		Gfx::tScreenPtr								mScreen;
		Gfx::tLightEntityPtr						mDefaultLight;

		Gui::tCanvasPtr								mWorldCanvas;
		Gui::tCanvasPtr								mRootHudCanvas;
		Gui::tCanvasPtr								mLoadingCanvas;
		Gui::tCanvasPtr								mWarningCanvas;
		Gui::tCanvasPtr								mStatusCanvas;

		tHashTable< std::string, Gui::tCanvasPtr >	mHudLayers;
		tResourcePtr								mProjectXml;
		tResourcePtr								mPostEffectsMaterialFile;
		tResourcePtr								mDefaultFont;
		tFilePathPtr								mLocFolder;
		tResourcePtr								mLocResFile;
		tDynamicArray<tResourcePtr>					mLocTextFileList;
		tDynamicArray<tResourcePtr>					mFonts;

#ifdef sig_devmenu
		Gui::tFpsTextPtr							mFpsText;
		Gui::tTextPtr								mStatsText;
		Gui::tTextPtr								mDebugTextRightSide;
		std::wstringstream							mStatsTextSS;
		std::wstringstream							mDebugTextRightSideSS;
	
		struct tWarningMessage
		{
			u32 mCount;
			f32 mLifeLeft;
			std::string mWarning;
		};
		Gui::tTextPtr								mWarningText;
		tGrowableArray<tWarningMessage>				mWarningLog;
		static const u32 cMaxWarningMsgs = 5;
#endif
		
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

		virtual void fSetFixedTimeStep( b32 fixed ) { mFixedTimeStep = fixed; }
		virtual b32 fFixedTimeStep( ) const { return mFixedTimeStep; }

		// Allow derived game applications to modify the dt for advancing the scene graph
		virtual f32 fModifySceneGraphDT( f32 ogDt ) const { return ogDt; }

		virtual void fAddToStatText( std::wstringstream& currentStats ) { }

		void fChangeViewportCount( 
			u32 newCount, 
			u32 localUserCount, 
			b32 updateUsers,
			tUserArray * remoteUsers = NULL );
		void fResetViewportCameras( );

		const Gfx::tScreenPtr& fScreen( ) const { return mScreen; }
		const Gfx::tDevicePtr& fDevice( ) const { return mDevice; }
		const Gfx::tLightEntityPtr& fDefaultLight( ) const { return mDefaultLight; }
		Gfx::tLightEntity* fDefaultLightForScript( ) const { return mDefaultLight.fGetRawPtr( ); }
		void fSetDefaultLight( Gfx::tLightEntity* light ) { mDefaultLight.fReset( light ); }
		const Audio::tSystemPtr& fSoundSystem( ) const { return mSoundSystem; }

		Gui::tCanvasFrame&			fRootCanvas( ) { return fScreen( )->fRootCanvas( ); }
		const Gui::tCanvasFrame&	fRootCanvas( ) const { return fScreen( )->fRootCanvas( ); }

		Gui::tCanvasPtr& fRootHudCanvas( ) { return mRootHudCanvas; }
		const Gui::tCanvasPtr& fRootHudCanvas( ) const { return mRootHudCanvas; }
		Sqrat::Object fRootHudCanvasFromScript( ) { return mRootHudCanvas.fScriptObject( ); }

		Gui::tCanvasPtr& fHudLayer( const std::string& layerName );
		Sqrat::Object fHudLayerFromScript( const std::string& layerName );

		Gui::tCanvasPtr& fLoadingCanvas( ) { return mLoadingCanvas; }
		const Gui::tCanvasPtr& fLoadingCanvas( ) const { return mLoadingCanvas; }
		Sqrat::Object fLoadingCanvasFromScript( ) { return mLoadingCanvas.fScriptObject( ); }

		Gui::tCanvasPtr& fWarningCanvas( ) { return mWarningCanvas; }
		const Gui::tCanvasPtr& fWarningCanvas( ) const { return mWarningCanvas; }
		Sqrat::Object fWarningCanvasFromScript( ) { return mWarningCanvas.fScriptObject( ); }

		Gui::tCanvasPtr& fStatusCanvas( ) { return mStatusCanvas; }
		const Gui::tCanvasPtr& fStatusCanvas( ) const { return mStatusCanvas; }
		Sqrat::Object fStatusCanvasFromScript( ) { return mStatusCanvas.fScriptObject( ); }
		
		Math::tRect fViewportRect( u32 ithViewport );
		Math::tRect fViewportSafeRect( u32 ithViewport );

		f32 fLastLevelLoadTime( ) const { return mLastLevelLoadTime; }
		void fSetLastLevelLoadTime( f32 time ) { mLastLevelLoadTime = time; }

		b32  fIsFullVersion( ) const { return mIsFullVersion; }
		virtual void fReadLicense( );
		virtual void fOnFullVersionInstalled( ) { }
		
		void fDumpMemory( const char* context, b32 justLog, tDynamicArray<byte>* dumpOutput );
		virtual void fConsolidateMemory( );
		virtual void fLogFreeMemory( );

		virtual void fDebugOutput( const std::string& text, 
								   const f32 screenX,
								   const f32 screenY,
								   const Math::tVec4f& color ) { }

		virtual Gfx::tTextureFile::tPlatformHandle fHandleFuiTextureLookup( const char* textureName, u32* outWidth, u32* outHeight ) { return NULL; }

	public: // localization
		b32 fLocStringExists( const tStringPtr& stringId ) const;
		const tLocalizedString& fLocString( const tStringPtr& stringId ) const;
		const tResourcePtr& fLocFont( u32 fontId ) { return mFonts[ fontId ]; }
		static tResourcePtr fFontFromId( u32 fontId ) { return fInstance( ).fLocFont( fontId ); }
		virtual u8 fGetLocalizedThousandsSeparator( ) const { return ','; }
		virtual u8 fGetLocalizedDecimalSeparator( ) const { return '.'; }

		void fCreateLocOverride( tResourceLoadList2* rll );
		void fClearLocOverride( );
		void fCreatePreloadLocTextResources( const tFilePathPtr& locFolder );
	protected:

		struct tGameStartupParams
		{
			u32 mScreenWidth;
			u32 mScreenHeight;
			Math::tVec4f mRgbaClearColor;
			f32 mWorldHalfAxisLength;
			u32 mViewportCount;
			Audio::tAudioOptions mAudioOptions;
			Fui::tFuiMemoryOptions mFuiOptions;
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

		void fStartupProjectSettings( );
		void fTryLoadPackage( const tFilePathPtr& root, const tFilePathPtr& name );
		virtual void fPreLoadFilePackages( ) = 0;
		virtual void fFillOutScreenCreateOpts( Gfx::tScreenCreationOptions& screenCreateOpts ) { }
		virtual void fStartupScript( );
		virtual void fStartupSceneGraph( );
		virtual void fStartupGfx( );
		virtual void fStartupProfiler( );
		virtual void fCreateDevice( Gfx::tScreenCreationOptions& screenCreateOptsOut ); // should allocate 'mDevice'
		virtual void fStartupSfx( ) { }
		virtual void fStartupUsers( );
		virtual void fExportGameScriptInterfaces( tScriptVm& vm ) { }
		virtual void fLoadInitalizationGameResources( ) { }
		virtual void fLoadPermaLoadedGameResources( ) { }
		virtual void fSetupPostEffects( ) { }
		virtual void fPreview( const tFilePathPtr& previewPath ) { }
		virtual void fReplay( const tFilePathPtr& replayPath ) { }
		virtual void fStartGame( ) { }
		virtual void fTickPlayers( f32 dt ) { }
		virtual void fTickCanvas( f32 dt );
        virtual b32 fShouldDoSingleStep( ) const;

	protected:

		void fLoadLocResources( const tFilePathPtr& locFolder, const tDynamicArray<tStringPtr>& fontNames );
		void fTickRender( f32 dt );
		void fTickAudio( f32 dt );
		if_devmenu( void fPrintDebugStats( ); )
		if_devmenu( void fPrintRightSideDebugStats( ); )
		if_devmenu( void fMakeDebugString( ); )
		void fAutoReloadResources( );
		u64  fComputeBuildStamp( ) const;
		static void fCaptureWarnings( const char * msg, u32 flag );
		
	public:
		Audio::tSystem* fAudioForScript( ) { return mSoundSystem.fGetRawPtr( ); }
		static void fExportScriptInterface( tScriptVm& vm );

		void fFadeToBlack( f32 seconds );
		void fFadeFromBlack( f32 seconds );
		b32 fBlackCanvasFullAlpha( );
		b32 fBlackCanvasZeroAlpha( );
	};

}

#endif//__tGameAppBase__
