//------------------------------------------------------------------------------
// \file tVoice_pc.cpp - 15 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_ios )
#include "tVoice.hpp"

namespace Sig
{
	const u32 tVoice::cPlaybackPriorityMax = 0;
	const u32 tVoice::cPlaybackPriorityMin = 0xffff;
	const u32 tVoice::cPlaybackPriorityNever = 0xffffffff;
	const u32 tVoice::cMaxRemoteTalkers = 0;

	//------------------------------------------------------------------------------
	b32 tVoice::fRegisterLocalTalker( u32 hwIndex )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fUnregisterLocalTalker( u32 hwIndex )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fRegisterRemoteTalker( tPlatformUserId userId )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fUnregisterRemoteTalker( tPlatformUserId userId )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStartProcessingModeLocal( u32 hwIndex, u32 mode )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStopProcessingModeLocal( u32 hwIndex, u32 mode )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStartProcessingModeRemote( tPlatformUserId userId, u32 mode )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fStopProcessingModeRemote( tPlatformUserId userId, u32 mode )
	{
		log_warning_unimplemented( );
		return true;
	}

	//------------------------------------------------------------------------------
	void tVoice::fSetPlaybackPriority( tPlatformUserId remoteId, u32 hwIndex, u32 priority )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	u32 tVoice::fGetRemoteTalkers( tPlatformUserId * remoteTalkers ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tVoice::fGetDataReadyFlag( ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	void tVoice::fPullVoiceData( u32 hwIndex )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	u32 tVoice::fPushVoiceData( tPlatformUserId id, const byte * buffer, u32 bufferSize )
	{
		log_warning_unimplemented( );
		return bufferSize;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fInitSystemData( u32 maxRemoteTalkers, tSystemData * data )
	{
		log_warning_unimplemented( );
		data->mMaxRemoteTalkers = maxRemoteTalkers;
		return true;
	}

	//------------------------------------------------------------------------------
	void tVoice::fDestroySystemData( tSystemData * data )
	{
		log_warning_unimplemented( );
		data->mMaxRemoteTalkers = 0;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fIsLocalTalking( u32 localHwIndex )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tVoice::fIsRemoteTalking( tPlatformUserId userId )
	{
		log_warning_unimplemented( );
		return false;
	}
}
#endif//#if defined( platform_ios )
