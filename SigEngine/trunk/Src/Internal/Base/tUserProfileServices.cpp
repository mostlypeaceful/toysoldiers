//------------------------------------------------------------------------------
// \file tUserProfileServices.cpp - 05 Dec 2011
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
	void tUserProfileServices::fInitialize( )
	{
		sigassert( !mLocalSettings.fCount( ) && !mRemoteSettings.fCount( ) );

		// NOTE: Potentially let additional information be set
		const u32 localSettings[] = 
		{
			cProfilePictureKey,
			cProfileYAxisInversion,
			cProfileTitleSpecific1,
			cProfileTitleSpecific2,
			cProfileTitleSpecific3
		};

		const u32 remoteSettings[] = 
		{
			cProfilePictureKey
		};

		mLocalSettings.fInitialize( localSettings, array_length( localSettings ) );
		mRemoteSettings.fInitialize( remoteSettings, array_length( remoteSettings ) );

		// Signal all signed in local users to want processing
		for( u32 i = 0; i < tUser::cMaxLocalUsers; ++i )
		{
			if( tUser::fSignedIn( i ) )
				mLocalUsers[ i ].mState = cStateWantsProcess;
		}

		fProcessLocalST( ); // Ensure local users are always maintained
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fShutdown( )
	{
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fProcessST( )
	{
		profile_pix( "tUserProfileServices::fProcessST" );
		fProcessLocalST( );
		fProcessRemoteST( );
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fOnUserSigninChange( 	
			const tUserSigninInfo oldStates[], 
			const tUserSigninInfo newStates[] )
	{
		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
		{
			// We've gone from not signed in to signed in
			if( oldStates[ u ].mState == tUser::cSignInStateNotSignedIn &&
				newStates[ u ].mState != tUser::cSignInStateNotSignedIn )
				fOnLocalUserSignedIn( u );

			// We've gone from signed in to not signed in
			else if( oldStates[ u ].mState != tUser::cSignInStateNotSignedIn &&
					 newStates[ u ].mState == tUser::cSignInStateNotSignedIn )
				fOnLocalUserSignedOut( u );

			// We've changed users on this hw index
			else if( newStates[ u ].mState != tUser::cSignInStateNotSignedIn &&
					 oldStates[ u ].mOfflineId != newStates[ u ].mOfflineId )
				fOnLocalUserSignedIn( u );
		}

		fProcessLocalST( ); // We always maintain local profiles
	}

	//------------------------------------------------------------------------------
	const char * tUserProfileServices::fGetLocalUserName( u32 localHwIndex ) const
	{
		if( mLocalUsers[ localHwIndex ].mState != cStateReady )
			return NULL;

		return mLocalUsers[ localHwIndex ].mName.fBegin( );
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileServices::fGetYAxisInverted( u32 localHwIndex ) const
	{
		tUserData data;
		fGetLocalSetting( localHwIndex, cProfileYAxisInversion, data );

		return data.fGetS32( ) == cYAxisInversionOn;
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fGetLocalTitleSpecificBuffers( 
		u32 localHwIndex, tGrowableBuffer & out ) const
	{
		sigassert( mLocalUsers[ localHwIndex ].mState == cStateReady );

		const u32 cSettingIds[] = { cProfileTitleSpecific1, cProfileTitleSpecific2, cProfileTitleSpecific3 };
		const u32 cSettingCount = array_length( cSettingIds );

		for( u32 i = 0; i < cSettingCount; ++i )
		{
			tUserData data;
			if( fGetLocalSettingInternal( localHwIndex, cSettingIds[ i ], data ) )
			{
				const void * bytes; u32 byteCount;
				data.fGetBinary( bytes, byteCount );
				out.fInsert( out.fCount( ), (u8*)bytes, byteCount );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fGetLocalSetting( u32 localHwIndex, u32 settingId, tUserData & data ) const
	{
		sigassert( mLocalUsers[ localHwIndex ].mState == cStateReady );
		const b32 found = fGetLocalSettingInternal( localHwIndex, settingId, data );
		sigassert( found && "Query for uncached setting!" );
	}

	//------------------------------------------------------------------------------
	tUserProfileServices::tRemoteUserQuery::tRemoteUserQuery( )
		: mState( cStateNull )
	{
#if defined( platform_xbox360 )
		fZeroOut( &mOverlapped );
#endif
	}

	//------------------------------------------------------------------------------
	tUserProfileServices::tRemoteUserQuery::~tRemoteUserQuery( )
	{
#if defined( platform_xbox360 )
		sigcheckfail_xoverlapped_done_else_wait_cancel( &mOverlapped );
#endif
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fProcessLocalST( )
	{
		for( u32 u = 0; u < mLocalUsers.cDimension; ++u )
		{
			if( mLocalUsers[ u ].mState == cStateWantsProcess )
				fProcessLocalUser( u );
		}
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fProcessRemoteST( )
	{
		for( u32 u = 0; u < mRemoteUserQueries.cDimension; ++u )
		{
			// Update remote users list to only contain users externally referenced
			for( s32 ru = mRemoteUsers[ u ].fCount( ) - 1; ru >= 0; --ru )
			{
				if( mRemoteUsers[ u ][ ru ]->fRefCount( ) == 1 )
					mRemoteUsers[ u ].fErase( ru );
			}

			// Process queries
			for( s32 q = mRemoteUserQueries[ u ].fCount( ) - 1; q >= 0; --q )
			{
				tRemoteUserQuery & query = *mRemoteUserQueries[ u ][ q ];

				switch( query.mState )
				{
				case cStateWantsProcess:
					{
						// Before we start the process on this query
						// check that we're not the only reference holder
						if( query.fRefCount( ) == 1 )
						{
							mRemoteUserQueries[ u ].fErase( q );
							break;
						}

						fStartSettingsProcess( u, query );

					} break;
				case cStateInSettingsProcess:
					{
						b32 success;
						if( fUpdateProcess( query, success ) )
						{
							if( success )
								fStartNamesProcess( u, query );
							else
							{
								log_warning( "Remote user settings query failed!" );
								query.mState = cStateError;
							}
						}

					} break;
				case cStateInNamesProcess:
					{
						b32 success;
						if( fUpdateProcess( query, success ) )
						{
							if( success )
								query.mState = cStateReady;
							else
							{
								log_warning( "Remote user name query failed!" );
								query.mState = cStateError;
							}
						}
					} break;
				case cStateNull: // Intentional fall-through
				case cStateReady:
					{
						// If we're the only reference holder of this query
						// then we know all relevant remote users have been
						// released and we should drop the query
						if( query.fRefCount( ) == 1 )
							mRemoteUserQueries[ u ].fErase( q );
					} break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fOnLocalUserSignedIn( u32 u )
	{
		mLocalUsers[ u ].mState = cStateWantsProcess;

		// Update all remote queries for this user for processing
		tGrowableArray<tRemoteUserQueryPtr> & queries = mRemoteUserQueries[ u ];
		const u32 queryCount = queries.fCount( );
		for( u32 q = 0; q < queryCount; ++q )
			queries[ q ]->mState = cStateWantsProcess;
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fOnLocalUserSignedOut( u32 u )
	{
		mLocalUsers[ u ].mState = cStateNull;

		// Report all queries as invalid
		tGrowableArray<tRemoteUserQueryPtr> & queries = mRemoteUserQueries[ u ];
		const u32 queryCount = queries.fCount( );
		for( u32 q = 0; q < queryCount; ++q )
			queries[ q ]->mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	void tUserProfileServices::fAcquireRemoteUser( 
		u32 localHwIndex, const tPlatformUserId & userId, tRemoteUserPtr & out )
	{
		sigassert( userId != tUser::cInvalidUserId );

		tGrowableArray<tRemoteUserPtr> & users = mRemoteUsers[ localHwIndex ];

		// Try to find the user
		const u32 remoteUserCount = users.fCount( );
		for( u32 u = 0; u < remoteUserCount; ++u )
		{
			if( users[ u ]->mUserId == userId )
			{
				out = users[ u ];
				return;
			}
		}

		// We didn't find the user so create it and register it for query
		out.fReset( NEW_TYPED( tRemoteUser ) ); 
		out->mUserId = userId;

		// Try to push the user onto the most recent query
		if( mRemoteUserQueries[ localHwIndex ].fCount( ) )
		{
			tRemoteUserQueryPtr ptr = mRemoteUserQueries[ localHwIndex ].fBack( );
			if( ptr->mState <= cStateWantsProcess && ptr->mUserIds.fCount( ) < 15 ) // Limit is from XDK docs
				out->mQuery = ptr;
		}

		// Create a new query cause there's wasn't one to use
		if( !out->mQuery )
		{
			out->mQuery.fReset( NEW_TYPED( tRemoteUserQuery ) );
			mRemoteUserQueries[ localHwIndex ].fPushBack( out->mQuery );
		}

		out->mQuery->mState = cStateWantsProcess;
		out->mQuery->mUserIds.fPushBack( userId );

		// Add the user to the array
		users.fPushBack( out );
		
	}

	//------------------------------------------------------------------------------
	// tUserProfileRef
	//------------------------------------------------------------------------------
	tUserProfileRef::tUserProfileRef( const tUserProfileRef & other )
		: mUser( other.mUser )
	{

	}

	//------------------------------------------------------------------------------
	tUserProfileRef::tUserProfileRef( u32 localRequester, const tPlatformUserId& id )
	{
		fReset( localRequester, id );
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileRef::fError( ) const
	{
		if( fNull( ) )
			return false;

		return mUser->mQuery->mState == tUserProfileServices::cStateError;
	}

	//------------------------------------------------------------------------------
	const tPlatformUserId & tUserProfileRef::fUserId( ) const
	{
		if( fNull( ) )
			return tUser::cInvalidUserId;

		return mUser->mUserId;
	}

	//------------------------------------------------------------------------------
	const char * tUserProfileRef::fGetName( ) const
	{
		if( fNull( ) )
			return NULL;

		return tUserProfileServices::fInstance( ).fGetRemoteNameInternal( *mUser->mQuery, mUser->mUserId );
	}

	//------------------------------------------------------------------------------
	b32 tUserProfileRef::fGetSetting( u32 setting, tUserData & userData ) const
	{
		if( fNull( ) ) 
			return false;

		return tUserProfileServices::fInstance( ).fGetRemoteSettingInternal(
			*mUser->mQuery, mUser->mUserId, setting, userData );
	}

	//------------------------------------------------------------------------------
	void tUserProfileRef::fReset( u32 localRequester, const tPlatformUserId & id )
	{
		sigassert( localRequester < tUser::cMaxLocalUsers );
		sigassert( id != tUser::cInvalidUserId );

		tUserProfileServices::fInstance( ).fAcquireRemoteUser( localRequester, id, mUser );
	}

	//------------------------------------------------------------------------------
	void tUserProfileRef::fRelease( )
	{
		mUser.fRelease( );
	}

	//------------------------------------------------------------------------------
	tUserProfileRef & tUserProfileRef::operator=( const tUserProfileRef & other )
	{
		mUser = other.mUser;
		return *this;
	}

} // ::Sig