//------------------------------------------------------------------------------
// \file tVoice.hpp - 15 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tVoice__
#define __tVoice__

#include "tUser.hpp"

#ifdef platform_xbox360
#include <xhv2.h>
#else
#endif

namespace Sig
{

	///
	/// \class tVoice
	/// \brief 
	class tVoice : public tUncopyable, public tRefCounter
	{
	public:

		enum tProcessingMode
		{
			cProcessingModeVoiceChat = 0,
			cProcessingModeLoopback,

			cProcessingModeCount
		};

		static const u32 cPlaybackPriorityMax;
		static const u32 cPlaybackPriorityMin;
		static const u32 cPlaybackPriorityNever;

	public:

		static tVoice & fInstance( ) { return *gInstancePtr; }
		static b32 fInitialize( u32 maxRemoteTalkers );
		static b32 fInitialized( ) { return !gInstancePtr.fNull( ); }
		static void fDestroy( ) { gInstancePtr.fRelease( ); }

		static u32 fMaxVoiceDataSize( ) { return cChatBufferSize * tUser::cMaxLocalUsers; }

	public:

		inline b32 fNeedToSend( ) const { return mNeedToSend; }

		~tVoice( );

		void fUpdateLocalUsers( );

		b32 fRegisterRemote( tPlatformUserId userId );
		b32 fUnregisterRemote( tPlatformUserId userId );
		void fUnregisterAllRemote( );

		// All registered users start with VoiceChat enabled and Loopback disabled
		b32 fStartProcessingMode( u32 localHwIndex, u32 mode );
		b32 fStopProcessingMode( u32 localHwIndex, u32 mode );

		void fTick( f32 dt );

		u32 fGetVoiceData( byte * buffer, u32 bufferSize );
		void fSubmitVoiceData( const byte * buffer, u32 bufferSize );

		b32 fBytesToSendAll( ) const;

		void fUpdateSystemMutes( tGrowableBuffer & muteMsg );
		void fHandleMuteMsg( const byte * msgBytes, u32 msgLength );

	private:

#ifdef platform_xbox360

		struct tSystemData
		{
			tSystemData( ) 
				: mEngine( NULL )
				, mWorkerThread( INVALID_HANDLE_VALUE ) { }

			IXHV2Engine * mEngine;
			HANDLE mWorkerThread;
			u32 mMaxRemoteTalkers;
		};
		static const u32 cChatBufferSize = XHV_VOICECHAT_MODE_PACKET_SIZE * XHV_MAX_VOICECHAT_PACKETS;
		static const u32 cChatPacketSize = XHV_VOICECHAT_MODE_PACKET_SIZE;
		static const u32 cSystemMaxRemoteTalkers = XHV_MAX_REMOTE_TALKERS;
#else
		struct tSystemData
		{
			u32 mMaxRemoteTalkers;
		};
		static const u32 cChatBufferSize = 1;
		static const u32 cChatPacketSize = 0;
		static const u32 cSystemMaxRemoteTalkers = 1;
#endif

		struct tUserVoice
		{
			b32 mIsRegistered;
			u32 mProcessingModes;

			u32 mBufferPos;
			byte mBuffer[ cChatBufferSize ];

			u32 mMuteCount;
			tFixedArray<tPlatformUserId, cSystemMaxRemoteTalkers> mMutes;
		};

		struct tMuteMsg
		{
			tMuteMsg( ) : mUserCount( 0 ) { }

			u32 mUserCount;
			tFixedArray<tPlatformUserId, tUser::cMaxLocalUsers> mUsers;
			tFixedArray<tGrowableArray<tPlatformUserId>, tUser::cMaxLocalUsers> mMutes;

			template<class tArchive>
			void fSaveLoad( tArchive & archive )
			{
				archive.fSaveLoad( mUserCount );
				archive.fSaveLoad( mUsers );
				archive.fSaveLoad( mMutes );
			}
		};

	private:

		static b32 fInitSystemData( u32 maxRemoteTalkers, tSystemData * data );
		static void fDestroySystemData( tSystemData * data );

	private:

		tVoice( const tSystemData & data );

		b32 fRegisterLocalTalker( u32 hwIndex );
		b32 fUnregisterLocalTalker( u32 hwIndex );
		b32 fRegisterRemoteTalker( tPlatformUserId userId );
		b32 fUnregisterRemoteTalker( tPlatformUserId userId );

		b32 fStartProcessingModeLocal( u32 hwIndex, u32 mode );
		b32 fStopProcessingModeLocal( u32 hwIndex, u32 mode );
		b32 fStartProcessingModeRemote( tPlatformUserId userId, u32 mode );
		b32 fStopProcessingModeRemote( tPlatformUserId userId, u32 mode );

		void fSetPlaybackPriority( tPlatformUserId remoteId, u32 hwIndex, u32 priority );

		u32 fGetRemoteTalkers( tPlatformUserId * remoteTalkers ) const;
		u32 fGetDataReadyFlag( ) const;

		void fPullVoiceData( u32 hwIndex );
		u32 fPushVoiceData( tPlatformUserId id, const byte * buffer, u32 bufferSize );

	private:

		static tRefCounterPtr< tVoice > gInstancePtr;

	private:

		tSystemData mSystem;

		b32 mNeedToSend;
		f32 mTimeSinceLastSend;
		tDynamicArray< tUserVoice > mVoices;
	};
}

#endif//__tVoice__
