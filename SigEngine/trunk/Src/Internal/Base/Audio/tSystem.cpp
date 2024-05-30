#include "BasePch.hpp"
#include "tSystem.hpp"
#include "tSource.hpp" // only for intial integration testing
#include "tProfiler.hpp"

#ifdef sig_use_wwise

#	include <AK/SoundEngine/Common/AkMemoryMgr.h>	// Memory Manager
#	include <AK/SoundEngine/Common/AkModule.h>		// Default memory and stream managers
#	include <AK/SoundEngine/Common/IAkStreamMgr.h>	// Streaming Manager
#	include <AK/Tools/Common/AkPlatformFuncs.h>	// Thread defines
#	include <AK/Tools/Common/AkMonitorError.h>	// Thread defines
#	include <AK/SoundEngine/Common/AkSoundEngine.h> // Sound engine
#	include <AK/MusicEngine/Common/AkMusicEngine.h> // Music Engine

#	ifdef platform_xbox360
#		include <Audio/IO/XBox360/AkFilePackageLowLevelIOBlocking.h>	// Sample low-level I/O implementation
#elif	defined( platform_pcdx ) || defined( platform_metro )
#		include <Audio/IO/Win32/AkFilePackageLowLevelIOBlocking.h>	// Sample low-level I/O implementation
#endif

// Effect Plugins
#	include <AK/Plugin/AkRoomVerbFXFactory.h>
#	include <AK/Plugin/AkParametricEQFXFactory.h>
#	include <AK/Plugin/AkAudioInputSourceFactory.h>
#	include <AK/Plugin/AkPeakLimiterFXFactory.h>
#	include <AK/Plugin/AkSilenceSourceFactory.h>
#	include <AK/Plugin/AkVorbisFactory.h>
#	include <AK/Plugin/AkTimeStretchFXFactory.h>


#	ifndef AK_OPTIMIZED
#		include <AK/Comm/AkCommunication.h>
#	endif // AK_OPTIMIZED

#endif//sig_use_wwise

devvar( bool, Debug_Audio_Disable, false );

devvar( Sig::u32, Debug_Audio_MaxPools, 30 );
devvar( Sig::f32, Debug_Audio_DefaultMemory, 4.0f );
devvar( Sig::f32, Debug_Audio_LowerMemory, 4.0f );
devvar( Sig::f32, Debug_Audio_StreamMemory, 2.0f );
devvar( Sig::f32, Debug_Audio_VolumeChangeRate, 70.0f );
devvar( Sig::f32, Debug_Audio_CommandQueue, 256.0f );

devvar_clamp( Sig::u32, Debug_Audio_FlipAxis, 1, 0, 3, 0 );

// The float is how much is for the designer, independent of how much we need for the system. sort of.
devvar( Sig::f32, Debug_Audio_MaxPhysMemory, 25.0f + (Debug_Audio_DefaultMemory + Debug_Audio_LowerMemory + Debug_Audio_StreamMemory) );

namespace Sig { namespace Audio
{

	b32 tSystem::fGlobalDisable( ) 
	{ 
		return Debug_Audio_Disable;
	}

	namespace
	{
		const u32 cBufferSize = 512;
		u32 fConvertString( const wchar_t * input, char buffer[cBufferSize] )
		{
			u32 len = wcslen( input );
			len = fMin( len, cBufferSize-1 );

			for( u32 i = 0; i < len; ++i )
				buffer[ i ] = (char)input[ i ];

			buffer[ len ] = 0;
			return len;
		}

#ifdef sig_use_wwise
		void fAudioLogOutput(AK::Monitor::ErrorCode in_eErrorCode, const AkOSChar *in_pszError, AK::Monitor::ErrorLevel in_eErrorLevel, AkPlayingID in_playingID, AkGameObjectID in_gameObjID ) 
		{
			if( !Log::fFlagEnabled( Log::cFlagAudio ) )
				return;

			if( in_eErrorLevel == AK::Monitor::ErrorLevel_Error ) 
			{
				// ignore some errors. if someone needs to see these they can see them in the wwise profiler.
				switch( in_eErrorCode )
				{
				// Dont log
				case AK::Monitor::ErrorCode_NoValidSwitch: break; // A game object tried to set a switch that wasn't there. or the value doesnt exist.
				case AK::Monitor::ErrorCode_VoiceStarving: break; // Not enough sound playing to maximize system resources. Not exactly a bad thing.
				default: 
					{
#if defined (AK_XBOX360)
						log_warning_nospam( "WWISE: " << in_pszError );
#else
						// Log everything else
						char buffer[cBufferSize];
						fConvertString( in_pszError, buffer );
						log_warning_nospam( "WWISE: " << buffer );
#endif
					}
				}
			}
		}

		// We're using the default Low-Level I/O implementation that's part
		// of the SDK's sample code, with the file package extension
		CAkFilePackageLowLevelIOBlocking gLowLevelIO;
#endif//sig_use_wwise

		// originally from wwise: platform specific?
		const tStringPtr cVolumeSFX( "UI_VOLUME_CONTROL_SFX" );
		const tStringPtr cVolumeMusic( "UI_VOLUME_CONTROL_MUSIC" );
		const tStringPtr cVolumeHUD( "UI_VOLUME_CONTROL_HUD" );
		const f32 cVolumeMax = 100.f;
	}

	tAudioOptions::tAudioOptions( )
#if defined( platform_xbox360 ) && defined( sig_use_wwise )
		: mThreadIndex( AK_XBOX360_DEFAULT_PROCESSOR_ID )
#else
		: mThreadIndex( 1 )
#endif
	{ }

	tSystemPtr tSystem::gDefaultSystem;

	tSystem::tSystem( )
		: mUserMusicVolume( 1.f )
		, mUserSfxVolume( 1.f )
		, mUserHUDVolume( 1.f )
		, mListenerMode( ~0 ) //doesnt actually initialize the system to anything.
		, mListenerMask( 1 )
	{
		gDefaultSystem.fReset( this );
	}

	tSystem::~tSystem( )
	{
		fCleanup( );
	}

	b32 tSystem::fInitialize( const tAudioOptions& options )
	{
		mOptions = options;

		if( !fGlobalDisable( ) )
		{
#ifdef sig_use_wwise
			AK::Monitor::SetLocalOutput( AK::Monitor::ErrorLevel_All, &fAudioLogOutput);

			// --- Memory ---
			AkMemSettings memSettings;
			memSettings.uMaxNumPools = Debug_Audio_MaxPools;

			if( AK::MemoryMgr::Init( &memSettings ) != AK_Success )
			{
				sigassert( ! "Could not create the memory manager." );
				return false;
			}

			const u32 c1MB = 1024 * 1024;

			// --- Streaming ---
			AkStreamMgrSettings stmSettings;
			AK::StreamMgr::GetDefaultSettings( stmSettings );
			
			// Customize the Stream Manager settings here.

			if( !AK::StreamMgr::Create( stmSettings ) )
			{
				sigassert( ! "Could not create the Streaming Manager" );
				return false;
			}

			AkDeviceSettings deviceSettings;
			AK::StreamMgr::GetDefaultDeviceSettings( deviceSettings );
			deviceSettings.uIOMemorySize = u32(Debug_Audio_StreamMemory * c1MB);
#ifdef platform_xbox360 
			deviceSettings.threadProperties.dwProcessor = options.mThreadIndex;
#endif

			// Customize the streaming device settings here.

			// CAkFilePackageLowLevelIOBlocking::Init() creates a streaming device
			// in the Stream Manager, and registers itself as the File Location Resolver.
			if( gLowLevelIO.Init( deviceSettings ) != AK_Success )
			{
				sigassert( ! "Could not create the streaming device and Low-Level I/O system" );
				return false;
			}

			// --- Sound Engine ---
			AkInitSettings initSettings;
			AkPlatformInitSettings platformInitSettings;
			AK::SoundEngine::GetDefaultInitSettings( initSettings );
			AK::SoundEngine::GetDefaultPlatformInitSettings( platformInitSettings );

			initSettings.uCommandQueueSize					= u32(Debug_Audio_CommandQueue * 1024);
			initSettings.uDefaultPoolSize					= u32(Debug_Audio_DefaultMemory * c1MB);
			platformInitSettings.uLEngineDefaultPoolSize	= u32(Debug_Audio_LowerMemory * c1MB);
#ifdef platform_xbox360 
			platformInitSettings.threadBankManager.dwProcessor = options.mThreadIndex;
			platformInitSettings.threadLEngine.dwProcessor = options.mThreadIndex;
	#ifndef AK_OPTIMIZED
			platformInitSettings.threadMonitor.dwProcessor = options.mThreadIndex;
	#endif
#endif

			if( AK::SoundEngine::Init( &initSettings, &platformInitSettings ) != AK_Success )
			{
				sigassert( ! "Could not initialize the Sound Engine." );
				return false;
			}

			// --- Register Codecs ---
			if( AK::SoundEngine::RegisterCodec( AKCOMPANYID_AUDIOKINETIC, AKCODECID_VORBIS, CreateVorbisFilePlugin, CreateVorbisBankPlugin ) != AK_Success )
			{
				sigassert( ! "Could not initilize Vorbis Codec." );
				return false;
			}

			// --- Music Engine ---
			AkMusicSettings musicInit;
			AK::MusicEngine::GetDefaultInitSettings( musicInit );

			if( AK::MusicEngine::Init( &musicInit ) != AK_Success )
			{
				sigassert( ! "Could not initialize the Music Engine." );
				return false;
			}

			// --- Communications (Debugging) ---
	#if !defined( AK_OPTIMIZED ) && !defined( platform_metro )
			AkCommSettings commSettings;
			AK::Comm::GetDefaultInitSettings( commSettings );

			if( AK::Comm::Init( commSettings ) != AK_Success )
			{
				sigassert( ! "Could not initialize communication." );
				return false;
			}
	#endif // AK_OPTIMIZED

			// --- Register Plugins ---
			AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_ROOMVERB,			CreateRoomVerbFX, CreateRoomVerbFXParams );
			// TS2 decided not to use the EQ
			//AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_PARAMETRICEQ,		CreateParametricEQFX, CreateParametricEQFXParams );
			AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_PEAKLIMITER,		CreatePeakLimiterFX, CreatePeakLimiterFXParams );
			AK::SoundEngine::RegisterPlugin( AkPluginTypeSource, AKCOMPANYID_AUDIOKINETIC, AKSOURCEID_SILENCE,			CreateSilenceSource, CreateSilenceSourceParams );
			//AK::SoundEngine::RegisterPlugin( AkPluginTypeSource, AKCOMPANYID_AUDIOKINETIC, AKSOURCEID_AUDIOINPUT,		CreateAudioInputSource, CreateAudioInputSourceParams );
			AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC, AKEFFECTID_TIMESTRETCH,		CreateTimeStretchFX, CreateTimeStretchFXParams );
#endif // sig_use_wwise
		}

		mMasterSource.fReset( NEW tSource( "Master Source" ) );
		mGlobalValues.fReset( NEW tSource( true ) );

		mMasterSource->fLockToListener( 0 );

		if( !fGlobalDisable( ) )
			fLoadBanks( );

		// Yay!
		return true;
	}

	void tSystem::fCleanup( )
	{
		if( mMasterSource )
			mMasterSource.fRelease( );

		if( fGlobalDisable( ) ) return;
#ifdef sig_use_wwise
		AK::SoundEngine::UnregisterAllGameObj() ;

		// Reverse order as above

#if !defined( AK_OPTIMIZED ) && !defined( platform_metro )
		AK::Comm::Term();
#endif // AK_OPTIMIZED

		AK::MusicEngine::Term();
		AK::SoundEngine::Term();

		gLowLevelIO.Term();
		if ( AK::IAkStreamMgr::Get() )
			AK::IAkStreamMgr::Get()->Destroy();

		AK::MemoryMgr::Term();
#endif//sig_use_wwise
	}

	void tSystem::fUpdateAudio( f32 dt )
	{
		profile( cProfilePerfUpdateAudio );

		mMasterSource->fStepSmoothers( dt );
		mGlobalValues->fStepSmoothers( dt );

#ifdef sig_use_wwise
		AK::SoundEngine::RenderAudio();
#endif//sig_use_wwise
	}

#ifdef sig_use_wwise
	namespace
	{
		void fSetBaseResourcePath( const char* path )
		{
#ifdef platform_pcdx9
			if( gLowLevelIO.SetBasePath( StringUtil::fStringToWString( path ).c_str( ) ) != AK_Success )
				sigassert( ! "Could not setup paths." );
#else
			if( gLowLevelIO.SetBasePath( path ) != AK_Success )
				sigassert( ! "Could not setup paths." );
#endif
		}
	}
#endif

	void tSystem::fLoadBanks( )
	{
		if( !Debug_Audio_Disable )
		{
#ifdef sig_use_wwise

			for( u32 i = 0; i < mOptions.mFilePackages.fCount( ); ++i )
			{
				fSetBaseResourcePath( mOptions.mFilePackages[ i ].mDirectory.c_str( ) );

				AkBankID discard;
				if( gLowLevelIO.LoadFilePackage( (AkOSChar*)mOptions.mFilePackages[ i ].mFilename.c_str( ), discard ) != AK_Success )
					log_assert( 0, "Could not load file package: " << mOptions.mFilePackages[ i ].mDirectory << "/" << mOptions.mFilePackages[ i ].mFilename );
			}
			for( u32 i = 0; i < mOptions.mSoundBanks.fCount( ); ++i )
			{
				fSetBaseResourcePath( mOptions.mSoundBanks[ i ].mDirectory.c_str( ) );
				mPermaLoadedBanks.fPushBack( fLoadSoundBankByPath( tFilePathPtr( mOptions.mSoundBanks[ i ].mFilename.c_str( ) ) ) );
			}

			// Packs all use the absolute file path name
			//  Banks are all absolute also, unless they are in a pack, then it's just the name.

			// So load all the packs and banks and then set the base directory.

			// --- Setup Streaming Paths ---
			fSetBaseResourcePath( mOptions.mStreamingFilesBasePath.c_str( ) );

#endif//sig_use_wwise
		}
	}

	void tSystem::fSetMasterMask( u32 mask )
	{
		mMasterSource->fSetListenerMask( mask );
		mGlobalValues->fSetListenerMask( mask );
	}
	
	void tSystem::fSetListenerMask( u32 mask )
	{
		mListenerMask = mask;
	}

	void tSystem::fSetUserMusicVolume( f32 v ) 
	{ 
		mUserMusicVolume = v;
		fGlobalValues( )->fSetGameParamSmooth( cVolumeMusic, v * cVolumeMax, Debug_Audio_VolumeChangeRate );
	}

	void tSystem::fSetUserSfxVolume( f32 v ) 
	{ 
		mUserSfxVolume = v;
		fGlobalValues( )->fSetGameParamSmooth( cVolumeSFX, v * cVolumeMax, Debug_Audio_VolumeChangeRate );
	}

	void tSystem::fSetUserHUDVolume( f32 v ) 
	{ 
		mUserHUDVolume = v;
		fGlobalValues( )->fSetGameParamSmooth( cVolumeHUD, v * cVolumeMax, Debug_Audio_VolumeChangeRate );
	}

	namespace
	{
		static const tStringPtr cParamThreshold( "Limiter_Threshold" );
		static const tStringPtr cParamGain( "Limiter_Gain" );
		static const tStringPtr cParamRatio( "Limiter_Ratio" );
		static const tStringPtr cParamRelease( "Limiter_Release" );
	}
	
	void tSystem::fSetListenerMode( u32 mode )
	{
		mListenerMode = mode;
		const tSourcePtr& source = fGlobalValues( );
		switch( mode )
		{
		case cListeningModeSmall:
			source->fSetGameParam( cParamThreshold, 0 );
			source->fSetGameParam( cParamGain, 0 );
			source->fSetGameParam( cParamRatio, 0 );
			source->fSetGameParam( cParamRelease, 0 );
			break;
		case cListeningModeMedium:
			source->fSetGameParam( cParamThreshold, 0.5f );
			source->fSetGameParam( cParamGain, 0.5f );
			source->fSetGameParam( cParamRatio, 0.5f );
			source->fSetGameParam( cParamRelease, 0.5f );
			break;
		case cListeningModeLarge:
			source->fSetGameParam( cParamThreshold, 1.f );
			source->fSetGameParam( cParamGain, 1.f );
			source->fSetGameParam( cParamRatio, 1.f );
			source->fSetGameParam( cParamRelease, 1.f );
			break;
		default:
			sigassert( !"Invalid listener mode." );
		}
	}

	void tSystem::fSetGlobalSwitch( u32 group, u32 value )
	{
		fGlobalValues( )->fSetSwitch( group, value );
		fMasterSource( )->fSetSwitch( group, value );
	}
	void tSystem::fSetGlobalSwitch( const char* group, const char* value )
	{
		fGlobalValues( )->fSetSwitch( group, value );
		fMasterSource( )->fSetSwitch( group, value );
	}
	void tSystem::fSetGlobalSwitch( const tStringPtr& group, const tStringPtr& value )
	{
		fGlobalValues( )->fSetSwitch( group, value );
		fMasterSource( )->fSetSwitch( group, value );
	}
	void tSystem::fSetGlobalGameParam( u32 param, f32 value )
	{
		fGlobalValues( )->fSetGameParam( param, value );
		fMasterSource( )->fSetGameParam( param, value );
	}
	void tSystem::fSetGlobalGameParam( const char* param, f32 value )
	{
		fGlobalValues( )->fSetGameParam( param, value );
		fMasterSource( )->fSetGameParam( param, value );
	}
	void tSystem::fSetGlobalGameParam( const tStringPtr& param, f32 value )
	{
		fGlobalValues( )->fSetGameParam( param, value );
		fMasterSource( )->fSetGameParam( param, value );
	}

	void tSystem::fRegisterListener( tListener* l )
	{
		mListeners.fFindOrAdd( tListenerPtr( l ) );
	}

	void tSystem::fUnRegisterListener( tListener* l )
	{
		mListeners.fFindAndErase( l );
	}

	void tSystem::fSetState( u32 group, u32 value )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )

		log_line( Log::cFlagAudio, "GLOBAL SYSTEM 0x" << this << " " << group << " set to " << value );
		if_wwise( AK::SoundEngine::SetState( group, value ) );
	}

	void tSystem::fSetState( const tStringPtr& group, const tStringPtr& value )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )

		log_line( Log::cFlagAudio, "GLOBAL SYSTEM 0x" << this << " " << group << " set to " << value );
		if_wwise( AK::SoundEngine::SetState( group.fCStr( ), value.fCStr( ) ) );
	}

	u32 tSystem::fIdFromString( const tStringPtr& string )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return AK_INVALID_UNIQUE_ID; )

		if_wwise( return AK::SoundEngine::GetIDFromString( string.fCStr( ) ) );
	}

	void tSystem::fCheckMemoryConsumption( )
	{
		const f32 audioMemUsage = Memory::fToMB<f32>( profile_query_mem( cProfileMemAudioPhys ) );
		const f32 artistMax = Debug_Audio_MaxPhysMemory;
		if( audioMemUsage > artistMax )
		{
			log_warning( "Exceeded audio RAM allowance (current=" << audioMemUsage << ", max=" << artistMax << ")" );
		}

	}

	static const Math::tMat3f cMatXFlip
			( -Math::tVec3f::cXAxis
			, Math::tVec3f::cYAxis
			, Math::tVec3f::cZAxis
			, Math::tVec3f::cZeroVector );
	
	static const Math::tMat3f cMatYFlip
			( Math::tVec3f::cXAxis
			, -Math::tVec3f::cYAxis
			, Math::tVec3f::cZAxis
			, Math::tVec3f::cZeroVector );

	static const Math::tMat3f cMatZFlip
			( Math::tVec3f::cXAxis
			, Math::tVec3f::cYAxis
			, -Math::tVec3f::cZAxis
			, Math::tVec3f::cZeroVector );

	static Math::tVec3f fFlipVec( const Math::tVec3f& vec, b32 isPoint )
	{
		const Math::tMat3f * mat = NULL;
		switch( (u32)Debug_Audio_FlipAxis )
		{
		case 1: mat = &cMatXFlip; break;
		case 2: mat = &cMatYFlip; break;
		case 3: mat = &cMatZFlip; break;
		default: return vec;
		}

		if( isPoint )
			return mat->fXformPoint( vec );
		else
			return mat->fXformVector( vec );
	}

	AkSoundPosition fSigToAKPosition( const Math::tVec3f& posIn, const Math::tVec3f& zdirIn )
	{
#ifdef sig_use_wwise

		const Math::tVec3f pos = fFlipVec( posIn, true );
		const Math::tVec3f zdir = fFlipVec( zdirIn, false );

		AkSoundPosition result;
		result.Orientation.X = zdir.x;
		result.Orientation.Y = zdir.y;
		result.Orientation.Z = zdir.z;
		result.Position.X = pos.x;
		result.Position.Y = pos.y;
		result.Position.Z = pos.z;

		return result;
#else
		return 0;
#endif//sig_use_wwise
	}

	// Wise coordinate system is left handed Y up.
	// ie +Y up, +Z forward, +X right
	//  so X is negative than the sig coordinate system
	AkListenerPosition fSigToAKListenerPosition( const Math::tMat3f& xform )
	{
#ifdef sig_use_wwise
		AkListenerPosition result;

		const Math::tVec3f z = fFlipVec( xform.fZAxis( ), false );
		result.OrientationFront.X = z.x;
		result.OrientationFront.Y = z.y;
		result.OrientationFront.Z = z.z;

		const Math::tVec3f y = fFlipVec( xform.fYAxis( ), false );
		result.OrientationTop.X = y.x;
		result.OrientationTop.Y = y.y;
		result.OrientationTop.Z = y.z;

		const Math::tVec3f p = fFlipVec( xform.fGetTranslation( ), true );
		result.Position.X = p.x;
		result.Position.Y = p.y;
		result.Position.Z = p.z;

		return result;
#else
		return 0;
#endif//sig_use_wwise
	}

}}


#if 1 // def sig_use_wwise // harmless if not using wwise, causes link to fail

#include "Memory/tHeap.hpp"
using namespace Sig;

/// Audio memory hooks
namespace AK
{
	const Memory::tAllocStamp cAudioStamp( "Audio" );

#ifdef _XBOX
	void * AllocHook( size_t in_size )
	{
		profile_mem( cProfileMemAudioSys, in_size );
		return Memory::tHeap::fInstance( ).fAlloc( in_size, cAudioStamp );
	}
	void FreeHook( void * in_ptr )
	{
		profile_mem( cProfileMemAudioSys, -(s32)Memory::tHeap::fInstance( ).fSizeOfAlloc( in_ptr ) );
		Memory::tHeap::fInstance( ).fFree( in_ptr );
	}
	// Note: VirtualAllocHook() may be used by I/O pools of the default implementation
	// of the Stream Manager, to allow "true" unbuffered I/O (using FILE_FLAG_NO_BUFFERING
	// - refer to the Windows SDK documentation for more details). This is NOT mandatory;
	// you may implement it with a simple malloc().
	void * VirtualAllocHook(
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwAllocationType,
		DWORD in_dwProtect
		)
	{
		profile_mem( cProfileMemAudioSys, in_size );
		return Memory::tHeap::fInstance( ).fAlloc( in_size, cAudioStamp );
	}
	void VirtualFreeHook( 
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwFreeType
		)
	{
		profile_mem( cProfileMemAudioSys, -s32( Memory::tHeap::fInstance( ).fSizeOfAlloc( in_pMemAddress ) ) );
		Memory::tHeap::fInstance( ).fFree( in_pMemAddress );
	}
	// Note: PhysicalAllocHook() may be used by I/O pools of the default implementation
	// of the Stream Manager, and by the sound engine when it needs to create pools for 
	// soundbanks. THIS IS MANDATORY IN ORDER TO BE ABLE TO PLAY XMA FILES. Any pool that
	// contains XMA data needs to be allocated with XPhysicalAlloc() so that the hardware
	// XMA decoder can read it. Refer to the XDK documentation for more details.
	void * PhysicalAllocHook( 
		size_t in_size,                 
		ULONG_PTR in_ulPhysicalAddress, 
		ULONG_PTR in_ulAlignment,       
		DWORD in_dwProtect              
		)
	{
		profile_mem( cProfileMemAudioPhys, in_size );
		Audio::tSystem::fCheckMemoryConsumption( );
		return XPhysicalAlloc( in_size, in_ulPhysicalAddress, in_ulAlignment, in_dwProtect );
	}
	void PhysicalFreeHook( 
		void * in_pMemAddress   
		)
	{
		profile_mem( cProfileMemAudioPhys, -s32( XPhysicalSize( in_pMemAddress ) ) );
		XPhysicalFree( in_pMemAddress );
	}
#elif defined( platform_pcdx )
	void * AllocHook( size_t in_size )
	{
		return malloc( in_size );
	}
	void FreeHook( void * in_ptr )
	{
		free( in_ptr );
	}
	// Note: VirtualAllocHook() may be used by I/O pools of the default implementation
	// of the Stream Manager, to allow "true" unbuffered I/O (using FILE_FLAG_NO_BUFFERING
	// - refer to the Windows SDK documentation for more details). This is NOT mandatory;
	// you may implement it with a simple malloc().
	void * VirtualAllocHook(
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwAllocationType,
		DWORD in_dwProtect
		)
	{
		profile_mem( cProfileMemAudioSys, in_size );
		return VirtualAlloc( in_pMemAddress, in_size, in_dwAllocationType, in_dwProtect );
	}
	void VirtualFreeHook( 
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwFreeType
		)
	{
		profile_mem( cProfileMemAudioSys, -s32(in_size) );
		VirtualFree( in_pMemAddress, in_size, in_dwFreeType );
	}
#elif defined( platform_metro )
	void * AllocHook( size_t in_size )
	{
		return malloc( in_size );
	}
	void FreeHook( void * in_ptr )
	{
		free( in_ptr );
	}

	// Note: VirtualAllocHook() may be used by I/O pools of the default implementation
	// of the Stream Manager, to allow "true" unbuffered I/O (using FILE_FLAG_NO_BUFFERING
	// - refer to the Windows SDK documentation for more details). This is NOT mandatory;
	// you may implement it with a simple malloc().
	void * VirtualAllocHook(
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwAllocationType,
		DWORD in_dwProtect
		)
	{
		profile_mem( cProfileMemAudioSys, in_size );
		return malloc( in_size );
	}
	void VirtualFreeHook( 
		void * in_pMemAddress,
		size_t in_size,
		DWORD in_dwFreeType
		)
	{
		profile_mem( cProfileMemAudioSys, -s32(in_size) );
		free( in_pMemAddress );
	}
#endif
}

namespace Sig { namespace Audio {

	void tSystem::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tSystem, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Overload<void (Audio::tSystem::*)( const tStringPtr&, const tStringPtr& )>(_SC("SetState"), &tSystem::fSetState)
			.Overload<void (Audio::tSystem::*)( const tStringPtr&, const tStringPtr& )>(_SC("SetSwitch"), &tSystem::fSetGlobalSwitch)
			;
		vm.fRootTable( ).Bind(_SC("AudioSystem"), classDesc);
	}

}}//Sig::Audio

#endif//sig_use_wwise
