//------------------------------------------------------------------------------
// \file tUserProfileServices_pc.cpp - 14 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tUserProfileServices.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tUserProfileServices
	//------------------------------------------------------------------------------
	const u32 tUserProfileServices::cProfilePictureKey = 1;
	const u32 tUserProfileServices::cProfileYAxisInversion = 2;
	const u32 tUserProfileServices::cProfileTitleSpecific1 = 3;
	const u32 tUserProfileServices::cProfileTitleSpecific2 = 4;
	const u32 tUserProfileServices::cProfileTitleSpecific3 = 5;

	const u32 tUserProfileServices::cYAxisInversionOff = 0;
	const u32 tUserProfileServices::cYAxisInversionOn = 1;

	//------------------------------------------------------------------------------
	void tUserProfileServices::fProcessLocalUser( u32 u )
	{
		log_warning_unimplemented( );
		mLocalUsers[ u ].mName[ 0 ] = 0;
		mLocalUsers[ u ].mState = cStateReady;
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fStartSettingsProcess( u32 localHwIndex, tRemoteUserQuery & query )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fStartNamesProcess( u32 localHwIndex, tRemoteUserQuery & query )
	{
		log_warning_unimplemented( );
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fUpdateProcess( tRemoteUserQuery & query, b32 & success )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fGetLocalSettingInternal( 
		u32 user, u32 setting, tUserData & data ) const
	{
		log_warning_unimplemented( );
		switch( setting )
		{
		case cProfileYAxisInversion:
			data.fSet( (s32)cYAxisInversionOff );
			return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fSaveLocalSettingsMT( 
		u32 localHwIndex, 
		const tUserProperty settings[], u32 settingCount, 
		const byte * titleBuffer, u32 titleBufferSize )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	const char * tUserProfileServices::fGetRemoteNameInternal( 
		tRemoteUserQuery & query, const tPlatformUserId & userId ) const
	{
		log_warning_unimplemented( );
		return NULL;
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fGetRemoteSettingInternal( 
		tRemoteUserQuery & query, const tPlatformUserId & userId, u32 setting, tUserData & data )  const
	{
		log_warning_unimplemented( );
		return false;
	}

} // ::Sig
