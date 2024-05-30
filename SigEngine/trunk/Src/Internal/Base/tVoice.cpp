//------------------------------------------------------------------------------
// \file tVoice.cpp - 15 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tVoice.hpp"
#include "tGameArchive.hpp"

namespace Sig
{
	namespace
	{
		static const f32 cMaxBufferTime = 0.2f;

		struct tVoiceHeader
		{
			tPlatformUserId mUserId;
			u32 mByteCount;
		};
	}

	// Ensure we can fit all the modes in the bits reserved for user voices
	static_assert( tVoice::cProcessingModeCount <= 32 );

	tRefCounterPtr< tVoice > tVoice::gInstancePtr;

	//------------------------------------------------------------------------------
	b32 tVoice::fInitialize( u32 maxRemoteTalkers )
	{
		// Already exists
		if( !gInstancePtr.fNull( ) )
			return true;

		tSystemData data;

		// Failed to initialize system resources
		if( !fInitSystemData( maxRemoteTalkers, &data ) )
			return false;

		// Success!
		gInstancePtr.fReset( NEW tVoice( data ) );
		return true;
	}

	//------------------------------------------------------------------------------
	void tVoice::fUpdateLocalUsers( )
	{
		malloca_array( tPlatformUserId, remoteTalkers, mSystem.mMaxRemoteTalkers );
		const u32 numRemoteTalkers = fGetRemoteTalkers( remoteTalkers.fBegin() );

		for( u32 hwIndex = 0; hwIndex < tUser::cMaxLocalUsers; ++hwIndex )
		{
			tUserVoice & voice = mVoices[ hwIndex ];

			// If the user is signed in
			if( tUser::fSignedIn( hwIndex ) )
			{
				// If it's not registered yet then register it
				if( !voice.mIsRegistered )
				{
					if( !fRegisterLocalTalker( hwIndex ) )
						continue;

					voice.mProcessingModes = 0;
					voice.mBufferPos = 0;
					voice.mIsRegistered = true;

					fStartProcessingMode( hwIndex, cProcessingModeVoiceChat );

					// Handle mutes for the new local user
					for( u32 r = 0; r < numRemoteTalkers; ++r )
					{
						const tPlatformUserId & remoteTalker = remoteTalkers[ r ];
						b32 isMuted = tUser::fIsMuted( hwIndex, remoteTalker );
						fSetPlaybackPriority( 
							remoteTalker, hwIndex, 
							isMuted ? cPlaybackPriorityNever : cPlaybackPriorityMax );
					}
				}
			}

			// The user is not signed in but is registered
			else if( voice.mIsRegistered )
			{
				fUnregisterLocalTalker( hwIndex );
				voice.mIsRegistered = false;
			}
		}
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fRegisterRemote( tPlatformUserId userId )
	{
		// Register the talker
		if( !fRegisterRemoteTalker( userId ) )
			return false;

		// Start the voice chat mode
		fStartProcessingModeRemote( userId, cProcessingModeVoiceChat );

		// Set the playback priorities for the remote user
		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
		{
			tUserVoice & voice = mVoices[ u ];
			if( !voice.mIsRegistered )
				continue;

			if( !tUser::fSignedIn( u ) )
				continue;

			const b32 muted = tUser::fIsMuted( u, userId );
			fSetPlaybackPriority( 
				userId, u, 
				muted ? cPlaybackPriorityNever : cPlaybackPriorityMax );
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fUnregisterRemote( tPlatformUserId userId )
	{
		return fUnregisterRemoteTalker( userId );
	}

	//------------------------------------------------------------------------------
	void tVoice::fUnregisterAllRemote( )
	{
		malloca_array( tPlatformUserId, remoteTalkers, mSystem.mMaxRemoteTalkers );
		const u32 remoteTalkerCount = fGetRemoteTalkers( remoteTalkers.fBegin() );

		for( u32 r = 0; r < remoteTalkerCount; ++r )
			fUnregisterRemoteTalker( remoteTalkers[ r ] );
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStartProcessingMode( u32 localHwIndex, u32 mode )
	{
		sigassert( localHwIndex < tUser::cMaxLocalUsers );
		sigassert( mode < cProcessingModeCount );

		tUserVoice & voice = mVoices[ localHwIndex ];

		// Is it registered?
		if( !voice.mIsRegistered )
		{
			log_warning( __FUNCTION__ << " error: User must be registered" );
			return false;
		}

		// If the mode is already started just return
		if( voice.mProcessingModes & ( 1 << mode ) )
			return true;

		// Log and return if it failed
		if( !fStartProcessingModeLocal( localHwIndex, mode ) )
			return false;

		// Flag the mode as started and return success
		voice.mProcessingModes |= ( 1 << mode );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStopProcessingMode( u32 localHwIndex, u32 mode )
	{
		sigassert( localHwIndex < tUser::cMaxLocalUsers );
		sigassert( mode < cProcessingModeCount );

		tUserVoice & voice = mVoices[ localHwIndex ];

		// Is it registered?
		if( !voice.mIsRegistered )
		{
			log_warning( __FUNCTION__ << " error: User must be registered" );
			return false;
		}

		// The mode is not currently enabled so just return success
		if( !( voice.mProcessingModes & ( 1 << mode ) ) )
			return true;

		// Stop the mode
		if( !fStopProcessingModeLocal( localHwIndex, mode ) )
			return false;

		// Unset the flag for the mode and return success
		voice.mProcessingModes &= ~( 1 << mode );
		return true;
	}

	//------------------------------------------------------------------------------S
	void tVoice::fTick( f32 dt )
	{
		fUpdateLocalUsers( );

		u32 bytesAvailable = 0;
		u32 dataReady = fGetDataReadyFlag( );
		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
		{
			// No data ready
			if( !( dataReady & ( 1 << u ) ) )
				continue;

			tUserVoice & voice = mVoices[ u ];

			// Not registered
			if( !voice.mIsRegistered )
				continue;

			if( cChatBufferSize - voice.mBufferPos < cChatPacketSize )
			{
				mNeedToSend = true;
				continue;
			}

			fPullVoiceData( u );

			if( voice.mBufferPos > ( ( cChatBufferSize * 7 ) / 10 ) )
				mNeedToSend = true;

			bytesAvailable += voice.mBufferPos;
		}

		mTimeSinceLastSend += dt;

		if( bytesAvailable && mTimeSinceLastSend > cMaxBufferTime )
			mNeedToSend = true;
	}

	//------------------------------------------------------------------------------
	u32 tVoice::fGetVoiceData( byte * buffer, u32 bufferSize )
	{
		b32 allSent = true;
		u32 bytesWritten = 0;

		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
		{
			tUserVoice & voice = mVoices[ u ];

			// Not registered
			if( !voice.mIsRegistered )
				continue;

			// No data
			if( !voice.mBufferPos )
				continue;

			// Not enough space for this buffer
			if( bufferSize < sizeof( tVoiceHeader ) + voice.mBufferPos )
			{
				allSent = false;
				continue;
			}

			tVoiceHeader * header = ( tVoiceHeader * )buffer;
			header->mByteCount = voice.mBufferPos;
			header->mUserId = tUser::fGetUserId( u );
			buffer += sizeof( *header );
			bufferSize -= sizeof( *header );
			bytesWritten += sizeof( *header );

			fMemCpy( buffer, voice.mBuffer, voice.mBufferPos );
			buffer += voice.mBufferPos;
			bufferSize -= voice.mBufferPos;
			bytesWritten += voice.mBufferPos;

			voice.mBufferPos = 0;
		}

		if( allSent )
		{
			mTimeSinceLastSend = 0;
			mNeedToSend = false;
		}

		return bytesWritten;
	}

	//------------------------------------------------------------------------------
	void tVoice::fSubmitVoiceData( const byte * buffer, u32 bufferSize )
	{
		while( bufferSize >= sizeof( tVoiceHeader ) )
		{
			const tVoiceHeader* header = ( const tVoiceHeader* )buffer;
			buffer += sizeof( *header );
			bufferSize -= sizeof( *header );

			u32 submitted = fPushVoiceData( header->mUserId, buffer, header->mByteCount );
			if( submitted != header->mByteCount )
			{
				// TODO: If necessary we may have to built a system for deferring these bytes till a later time
				log_warning( "Incoming voice buffer is full - lost " << header->mByteCount - submitted << " bytes of voice data" );
			}

			buffer += header->mByteCount;
			bufferSize -= header->mByteCount;
		}
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fBytesToSendAll( ) const
	{
		u32 bytes = 0;
		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
		{
			const tUserVoice & voice = mVoices[ u ];
			if( !voice.mIsRegistered )
				continue;

			if( !voice.mBufferPos )
				continue;

			bytes += sizeof( tVoiceHeader ) + voice.mBufferPos;
		}

		return bytes;
	}

	//------------------------------------------------------------------------------
	void tVoice::fUpdateSystemMutes( tGrowableBuffer & muteMsg )
	{
		malloca_array( tPlatformUserId, remoteTalkers, mSystem.mMaxRemoteTalkers );
		const u32 remoteTalkerCount = fGetRemoteTalkers( remoteTalkers.fBegin() );

		tMuteMsg msg;

		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
		{
			const tUserVoice & voice = mVoices[ u ];
			if( !voice.mIsRegistered )
				continue;

			msg.mUsers[ msg.mUserCount ] = tUser::fGetUserId( u );
			msg.mMutes[ msg.mUserCount ].fReserve( remoteTalkerCount );

			for( u32 r = 0; r < remoteTalkerCount; ++r )
			{
				// Query the system mute list
				b32 isMuted = tUser::fSignedIn( u ) && tUser::fIsMuted( u, remoteTalkers[ r ] );

				// If it's muted store this fact in the mute msg
				if( isMuted )
				{
					msg.mMutes[msg.mUserCount].fPushBack( remoteTalkers[ r ] );
				}

				// If it's not muted by the system it may be muted explicitly
				else
				{
					for( u32 m = 0; m < voice.mMuteCount; ++m )
					{
						if( voice.mMutes[ m ] == remoteTalkers[ r ] )
						{
							isMuted = true;
							break;
						}
					}
				}

				fSetPlaybackPriority( 
					remoteTalkers[ r ], u, 
					isMuted ? cPlaybackPriorityNever : cPlaybackPriorityMax );
			}

			++msg.mUserCount;
		}

		// Save the message
		tGameArchiveSave archive;
		archive.fSave( msg );

		// Push it to the buffer
		if( !muteMsg.fCount( ) )
			muteMsg.fSwap( archive.fBuffer( ) );
		else
			muteMsg.fInsert( muteMsg.fCount( ), archive.fBuffer( ).fBegin( ), archive.fBuffer( ).fCount( ) );
	}

	//------------------------------------------------------------------------------
	void tVoice::fHandleMuteMsg( const byte * msgBytes, u32 msgLength )
	{
		tGameArchiveLoad archive( msgBytes, msgLength );
		tMuteMsg msg; 
		archive.fLoad( msg );

		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
		{
			tUserVoice & voice = mVoices[ u ];

			tPlatformUserId userId = tUser::fSignedIn( u ) ? tUser::fGetUserId( u ) : tUser::cInvalidUserId;

			for( u32 r = 0; r < msg.mUserCount; ++r )
			{
				// Remove the mute for this user
				for( s32 m = voice.mMuteCount - 1; m >= 0; --m )
				{
					if( voice.mMutes[ m ] == msg.mUsers[ r ] )
					{
						voice.mMutes[ m ] = voice.mMutes[ --voice.mMuteCount ];
						break;
					}
				}

				b32 isMuted = false;

				// Add the mute if still muted
				const tGrowableArray<tPlatformUserId> & mutes = msg.mMutes[ r ];
				const u32 muteCount = mutes.fCount( );
				for( u32 m = 0; m < muteCount; ++m )
				{
					if( mutes[ m ] == userId )
					{
						voice.mMutes[ voice.mMuteCount++ ] = msg.mUsers[ r ];
						isMuted = true;
						break;
					}
				}

				// If not muted by the remote talker, see if muted locally
				if( !isMuted )
					isMuted = ( userId != tUser::cInvalidUserId ) ? tUser::fIsMuted( u, msg.mUsers[ r ] ) : false;

				fSetPlaybackPriority( 
					msg.mUsers[ r ], u, 
					isMuted ? cPlaybackPriorityNever : cPlaybackPriorityMax );
			}
		}
	}

	//------------------------------------------------------------------------------
	tVoice::tVoice( const tSystemData & data )
		: mSystem( data )
		, mNeedToSend( false )
		, mTimeSinceLastSend( 0.f )
	{
		mVoices.fNewArray( tUser::cMaxLocalUsers );
		fZeroOut( mVoices.fBegin( ), mVoices.fCount( ) );
	}

	//------------------------------------------------------------------------------
	tVoice::~tVoice( ) 
	{
		fDestroySystemData( &mSystem );
	}
}
