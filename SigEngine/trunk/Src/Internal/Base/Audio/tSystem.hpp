#ifndef __tSystem__
#define __tSystem__

#include "tSource.hpp"
#include "tListener.hpp"
#include "tSoundBank.hpp"

#ifdef sig_use_wwise
	struct AkSoundPosition;
	struct AkListenerPosition;
#else
	typedef int AkSoundPosition;
	typedef int AkListenerPosition;

	// HACKY HACKY HACK: Supress AkTypes.h #include from Wwise_IDs.h
	typedef unsigned int AkUniqueID; // But provide AkUniqueID so it compiles
	#define _AK_DATA_TYPES_H_
#endif

namespace Sig { namespace Audio
{
	///
	/// \brief Store absolute path to a file, separating directory and filename.
	struct tAbsPath
	{
		std::string mDirectory;
		std::string mFilename;

		tAbsPath( ) { }

		tAbsPath( const std::string& dir, const std::string& file )
			: mDirectory( dir )
			, mFilename( file )
		{ }
	};

	///
	/// \brief Startup options for the audio system.
	struct tAudioOptions
	{
		tAudioOptions( );

		u32				mThreadIndex;

		// These are not tFilePathPtrs because we dont want to impart our formating out to the audio subsystem.
		std::string		mStreamingFilesBasePath;
		tGrowableArray<tAbsPath> mFilePackages;
		tGrowableArray<tAbsPath> mSoundBanks;
	};

	class tSystem;
	define_smart_ptr( base_export, tRefCounterPtr, tSystem ); 

	///
	/// \brief Represents an entire (usually global) sound system; all audio functionality begins here.
	/// Kind of like a graphics device, but for audio.
	class base_export tSystem : public tUncopyable, public tRefCounter
	{
		//underlying sound system is globally accessed
		//declare_singleton( tSystem );

	private:
		static tSystemPtr gDefaultSystem;

	public:
		static void					fSetDefaultSystem( const tSystemPtr& defaultSystem ) { gDefaultSystem = defaultSystem; }
		static const tSystemPtr&	fGetDefaultSystem( ) { return gDefaultSystem; }

		static b32 fGlobalDisable( );

		static void fCheckMemoryConsumption( );
	public:
		tSystem( );
		~tSystem( );

		b32 fInitialize( const tAudioOptions& options = tAudioOptions( ) );
		void fCleanup( );

		void fUpdateAudio( f32 dt );

		const tAudioOptions& fOptions( ) const { return mOptions; }

		void fSetState( u32 group, u32 state );
		void fSetState( const tStringPtr& group, const tStringPtr& state );
		
		u32 fIdFromString( const tStringPtr& string );

		f32 fUserMusicVolume( ) const { return mUserMusicVolume; }
		void fSetUserMusicVolume( f32 v );

		f32 fUserSfxVolume( ) const { return mUserSfxVolume; }
		void fSetUserSfxVolume( f32 v );

		f32 fUserHUDVolume( ) const { return mUserHUDVolume; }
		void fSetUserHUDVolume( f32 v );

		// Setting this to zero will effectively disable the audio.
		//  Passing true to keepMasterEnabled will allow the master source to keep playing sound.
		void fSetMasterMask( u32 mask );
		void fSetListenerMask( u32 mask );
		u32  fListenerMask( ) const { return mListenerMask; }

		// Main source for playing menu and 2d sounds
		const tSourcePtr& fMasterSource( ) { return mMasterSource; }

		// Sound source representing the global audio bus. Do not play sounds on this source. only set states and rtpcs.
		const tSourcePtr& fGlobalValues( ) { return mGlobalValues; }

		void fRegisterListener( tListener* l );
		void fUnRegisterListener( tListener* l );

		tGrowableArray<tListenerPtr>& fListeners( ) { return mListeners; }
		const tGrowableArray<tListenerPtr>& fListeners( ) const { return mListeners; }

		enum tListeningMode
		{
			cListeningModeSmall,
			cListeningModeMedium,
			cListeningModeLarge,
			cListeningModeCount
		};

		void fSetListenerMode( u32 mode );
		u32 fListenerMode( ) const { return mListenerMode; }

		void fSetGlobalSwitch( u32 group, u32 value );
		void fSetGlobalSwitch( const char* group, const char* value );
		void fSetGlobalSwitch( const tStringPtr& group, const tStringPtr& value );
		void fSetGlobalGameParam( u32 param, f32 value );
		void fSetGlobalGameParam( const char* param, f32 value );
		void fSetGlobalGameParam( const tStringPtr& param, f32 value );

	private:
		tAudioOptions mOptions;
		f32 mUserMusicVolume;
		f32 mUserSfxVolume;
		f32 mUserHUDVolume;
		u32 mListenerMode;
		u32 mListenerMask;

		tSourcePtr mMasterSource;
		tSourcePtr mGlobalValues;

		tGrowableArray< tListenerPtr > mListeners;
		tGrowableArray< tSoundBankPtr > mPermaLoadedBanks;

		void fLoadBanks( );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	// Utility stuff
	AkSoundPosition fSigToAKPosition( const Math::tVec3f& pos, const Math::tVec3f& zdir );
	AkListenerPosition fSigToAKListenerPosition( const Math::tMat3f& xform );

}}


#endif//__tSystem__

