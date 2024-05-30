//------------------------------------------------------------------------------
// \file tVoice_xbox360.cpp - 15 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tVoice.hpp"
#include <xaudio2.h>

namespace Sig
{
	namespace
	{
		static const XHV_PROCESSING_MODE gLocalModes[] = { XHV_VOICECHAT_MODE, XHV_LOOPBACK_MODE };
		static const XHV_PROCESSING_MODE gRemoteModes[] = { XHV_VOICECHAT_MODE };
	}

	const u32 tVoice::cPlaybackPriorityMax = XHV_PLAYBACK_PRIORITY_MAX;
	const u32 tVoice::cPlaybackPriorityMin = XHV_PLAYBACK_PRIORITY_MIN;
	const u32 tVoice::cPlaybackPriorityNever = XHV_PLAYBACK_PRIORITY_NEVER;
	const u32 tVoice::cMaxRemoteTalkers = XHV_MAX_REMOTE_TALKERS;

	//------------------------------------------------------------------------------
	b32 tVoice::fRegisterLocalTalker( u32 hwIndex )
	{
		HRESULT result = mSystem.mEngine->RegisterLocalTalker( hwIndex );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fUnregisterLocalTalker( u32 hwIndex )
	{
		HRESULT result = mSystem.mEngine->UnregisterLocalTalker( hwIndex );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fRegisterRemoteTalker( tPlatformUserId userId )
	{
		HRESULT result = mSystem.mEngine->RegisterRemoteTalker( userId, NULL, NULL, NULL );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fUnregisterRemoteTalker( tPlatformUserId userId )
	{
		HRESULT result = mSystem.mEngine->UnregisterRemoteTalker( userId );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStartProcessingModeLocal( u32 hwIndex, u32 mode )
	{
		HRESULT result = mSystem.mEngine->StartLocalProcessingModes( hwIndex, &gLocalModes[ mode ], 1 );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStopProcessingModeLocal( u32 hwIndex, u32 mode )
	{
		HRESULT result = mSystem.mEngine->StopLocalProcessingModes( hwIndex, &gLocalModes[ mode ], 1 );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStartProcessingModeRemote( tPlatformUserId userId, u32 mode )
	{
		HRESULT result = mSystem.mEngine->StartRemoteProcessingModes( userId, &gRemoteModes[ mode ], 1 );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStopProcessingModeRemote( tPlatformUserId userId, u32 mode )
	{
		HRESULT result = mSystem.mEngine->StopRemoteProcessingModes( userId, &gRemoteModes[ mode ], 1 );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tVoice::fSetPlaybackPriority( tPlatformUserId remoteId, u32 hwIndex, u32 priority )
	{
		HRESULT result = mSystem.mEngine->SetPlaybackPriority( remoteId, hwIndex, priority );
		if( FAILED( result ) )
			log_warning( __FUNCTION__ << " error: " << std::hex << result );
	}

	//------------------------------------------------------------------------------
	u32 tVoice::fGetRemoteTalkers( tPlatformUserId * remoteTalkers ) const
	{
		DWORD numTalkers = mSystem.mMaxRemoteTalkers;
		HRESULT result = mSystem.mEngine->GetRemoteTalkers( &numTalkers, remoteTalkers );
		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " error: " << std::hex << result );
			return 0;
		}

		return numTalkers;
	}

	//------------------------------------------------------------------------------
	u32 tVoice::fGetDataReadyFlag( ) const
	{
		return ( u32 )mSystem.mEngine->GetDataReadyFlags( );
	}

	//------------------------------------------------------------------------------
	void tVoice::fPullVoiceData( u32 hwIndex )
	{
		tUserVoice & voice = mVoices[ hwIndex ];

		DWORD unused;
		DWORD bytes = cChatBufferSize - voice.mBufferPos;
		HRESULT result = mSystem.mEngine->GetLocalChatData( 
			hwIndex, voice.mBuffer + voice.mBufferPos, &bytes, &unused );

		if( FAILED( result ) )
		{
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );
			return;
		}

		voice.mBufferPos += bytes;
	}

	//------------------------------------------------------------------------------
	u32 tVoice::fPushVoiceData( tPlatformUserId id, const byte * buffer, u32 bufferSize )
	{
		DWORD submitted = bufferSize;
		HRESULT result = mSystem.mEngine->SubmitIncomingChatData( id, buffer, &submitted );
		if( FAILED( result ) )
			log_warning( __FUNCTION__ << " failed with error: " << std::hex << result );

		return submitted;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fInitSystemData( u32 maxRemoteTalkers, tSystemData * data )
	{
		// Initialize XAudio2
		IXAudio2 * audio2 = NULL;
		HRESULT result = XAudio2Create( &audio2 /*, flags, hwThread*/ );

		if( FAILED( result ) || !audio2 )
			return false;

		// Create a mastering voice
		IXAudio2MasteringVoice * masteringVoice = NULL;
		result = audio2->CreateMasteringVoice( &masteringVoice );
		if( FAILED( result ) || !masteringVoice )
		{
			audio2->Release( );
			return false;
		}

		// Set up the XHV audio init params
		

		XHV_INIT_PARAMS initParams = {0};
		initParams.dwMaxRemoteTalkers            = maxRemoteTalkers;
		initParams.dwMaxLocalTalkers             = XHV_MAX_LOCAL_TALKERS;
		initParams.localTalkerEnabledModes       = gLocalModes;
		initParams.dwNumLocalTalkerEnabledModes  = array_length( gLocalModes );
		initParams.remoteTalkerEnabledModes      = gRemoteModes;
		initParams.dwNumRemoteTalkerEnabledModes = array_length( gRemoteModes );
		initParams.pXAudio2                      = audio2;

		// Create the XHV engine
		result = XHV2CreateEngine( &initParams, &data->mWorkerThread, &data->mEngine );

		if( FAILED( result ) || !data->mEngine )
		{
			masteringVoice->DestroyVoice( );
			audio2->Release( );
			return false;
		}

		data->mMaxRemoteTalkers = maxRemoteTalkers;

		// Success!
		return true;
	}

	//------------------------------------------------------------------------------
	void tVoice::fDestroySystemData( tSystemData * data )
	{
		// Release our thread handle
		if( data->mWorkerThread != INVALID_HANDLE_VALUE )
		{
			XCloseHandle( data->mWorkerThread );
			data->mWorkerThread = INVALID_HANDLE_VALUE;
		}

		// Release the engine
		if( data->mEngine )
		{
			data->mEngine->Release( );
			data->mEngine = NULL;
		}
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fIsLocalTalking( u32 localHwIndex )
	{
		if( mSystem.mEngine )
			return mSystem.mEngine->IsLocalTalking( localHwIndex );

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fIsRemoteTalking( tPlatformUserId userId )
	{
		if( mSystem.mEngine )
			return mSystem.mEngine->IsRemoteTalking( userId );

		return false;
	}
}
#endif//#if defined( platform_xbox360 )
