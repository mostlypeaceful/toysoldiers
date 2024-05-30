//------------------------------------------------------------------------------
// \file tUserProfileServices.hpp - 05 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tUserProfileServices__
#define __tUserProfileServices__
#include "tUser.hpp"

namespace Sig
{
	///
	/// \class tUserProfileServices
	/// \brief Manages profile information for local and remote users
	class tUserProfileServices
	{
		friend class tUserProfileRef;
		declare_singleton( tUserProfileServices );
	public:

		static const u32 cProfilePictureKey;
		static const u32 cProfileYAxisInversion;
		static const u32 cProfileTitleSpecific1;
		static const u32 cProfileTitleSpecific2;
		static const u32 cProfileTitleSpecific3;

		static const u32 cYAxisInversionOff;
		static const u32 cYAxisInversionOn;
		


	public:

		// Initialize the service with what settings should be queried/maintained
		void fInitialize( );
		void fShutdown( );

		// Advance our settings queries
		void fProcessST( );

		// Inform the profile services of user sign in state changes
		void fOnUserSigninChange( 	
			const tUserSigninInfo oldStates[],
			const tUserSigninInfo newStates[] );

		// Access to local settings - always maintained
		const char * fGetLocalUserName( u32 localHwIndex ) const;
		b32 fGetYAxisInverted( u32 localHwIndex ) const;
		void fGetLocalTitleSpecificBuffers( u32 localHwIndex, tGrowableBuffer & out ) const;
		void fGetLocalSetting( u32 localHwIndex, u32 settingId, tUserData & data ) const;

		// This is a sync operation - call it from an MT thread. Note, you can break up the title
		// buffer yourself and pass it through as user data with the cProfileTitleSpecificX ids or 
		// pass the data through the title buffer args and the service will split it for you
		b32 fSaveLocalSettingsMT( 
			u32 localHwIndex, 
			const tUserProperty settings[], u32 settingCount, 
			const byte * titleBuffer = NULL, u32 titleBufferSize = 0 );

	private:

		enum tState
		{
			cStateNull = 0,
			cStateWantsProcess,
			cStateInSettingsProcess,
			cStateInNamesProcess,
			cStateReady,

			cStateError,
		};

		struct tLocalUser
		{
			tLocalUser( ) : mState( cStateNull ) { }

			u32 mState;
			tFixedArray<char, tUser::cUserNameSize> mName;
			tDynamicBuffer mSettings;
		};

		struct tRemoteUserQuery : public tRefCounter
		{
			tRemoteUserQuery( );
			~tRemoteUserQuery( );

			u32 mState;

#ifdef platform_xbox360
			XOVERLAPPED mOverlapped;
#endif // platform_xbox360

			tGrowableArray<tPlatformUserId> mUserIds;
			tDynamicBuffer mSettings;
			tDynamicBuffer mNames;
		};

		typedef tRefCounterPtr< tRemoteUserQuery > tRemoteUserQueryPtr;

		struct tRemoteUser : public tRefCounter
		{
			tPlatformUserId mUserId;
			tRemoteUserQueryPtr mQuery; 
		};

		typedef tRefCounterPtr< tRemoteUser > tRemoteUserPtr;

	private:

		void fProcessLocalST( );
		void fProcessRemoteST( );

		void fOnLocalUserSignedIn( u32 u );
		void fOnLocalUserSignedOut( u32 u );

		void fProcessLocalUser( u32 u );
		void fStartSettingsProcess( u32 localHwIndex, tRemoteUserQuery & query );
		void fStartNamesProcess( u32 localHwIndex, tRemoteUserQuery & query );
		b32 fUpdateProcess( tRemoteUserQuery & query, b32 & success );

		b32 fGetLocalSettingInternal( u32 user, u32 setting, tUserData & data ) const;

		// Access for tUserProfileRef
		const char * fGetRemoteNameInternal( 
			tRemoteUserQuery & query, const tPlatformUserId & userId ) const;
		b32 fGetRemoteSettingInternal( 
			tRemoteUserQuery & query, const tPlatformUserId & userId, u32 setting, tUserData & data )  const;
		void fAcquireRemoteUser( u32 localHwIndex, const tPlatformUserId & userId, tRemoteUserPtr & out );
	

	private:
		
		tFixedArray<tLocalUser, tUser::cMaxLocalUsers> mLocalUsers;
		tFixedArray<tGrowableArray<tRemoteUserPtr>, tUser::cMaxLocalUsers> mRemoteUsers;
		tFixedArray<tGrowableArray<tRemoteUserQueryPtr>, tUser::cMaxLocalUsers> mRemoteUserQueries;
		
		tDynamicArray<u32> mLocalSettings;
		tDynamicArray<u32> mRemoteSettings;
	};

	///
	/// \class tUserProfileRef
	/// \brief 
	class tUserProfileRef
	{
	public:

		tUserProfileRef( ) { }
		tUserProfileRef( const tUserProfileRef & other );
		tUserProfileRef( u32 localRequester, const tPlatformUserId& id );
		~tUserProfileRef( ) { fRelease( ); }

		b32 fNull( ) const { return mUser.fNull( ); }
		b32 fError( ) const;

		const tPlatformUserId & fUserId( ) const;

		const char * fGetName( ) const;
		b32 fGetSetting( u32 setting, tUserData & userData ) const;

		void fReset( u32 localRequester, const tPlatformUserId & id );
		void fRelease( );

		tUserProfileRef & operator=( const tUserProfileRef & other );

	private:

		tUserProfileServices::tRemoteUserPtr mUser;
	};
}

#endif//__tUserProfileServices__
