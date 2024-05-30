#include "BasePch.hpp"
#include "tGameSession.hpp"

namespace Sig
{
	static char* cStateNames[ ] =
	{
		"cStateNull",
		"cStateCreating",
		"cStateCreated",
		"cStateJoiningLocalUsers",
		"cStateRemovingLocalUsers",
		"cStateJoiningRemoteUsers",
		"cStateRemovingRemoteUsers",
		"cStateRegistering",
		"cStateStarting",
		"cStateStarted",
		"cStateWritingStats",
		"cStateFlushingStats",
		"cStateEnding",
		"cStateDeleting",
	};

	//------------------------------------------------------------------------------
	tGameSession::tGameSession( 
		u32 createFlags, 
		u32 maxPublicSlots, 
		u32 maxPrivateSlots,
		tStateChangedCallback callback )
		: mState( cStateNull )
		, mPrevState( cStateCount )
		, mIsHost( true )
		, mCreateFlags( createFlags )
		, mStateChangedCallback( callback )
		, mStateTimer( 0.f )
#if defined( use_steam )
		, mCallbackLobbyCreated( this, &tGameSession::fOnLobbyCreated )
		, mCallbackLobbyJoined( this, &tGameSession::fOnLobbyJoined )
		, mCallbackLobbyChatUpdate( this, &tGameSession::fOnLobbyChatUpdate )
		, mCallbackLobbyGameCreated( this, &tGameSession::fOnLobbyGameCreated )
		, mLobbyCreated( false )
#endif
	{
		fResetSlots( maxPublicSlots, maxPrivateSlots );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fSetNonce( tNonce nonce )
	{
		sigassert( ( !mIsHost && mState <= cStateStarting ) && "Setting session nonce only makes sense on non-started - non-host sessions" );
		mData.mNonce = nonce;
	}

	//------------------------------------------------------------------------------
	b32	tGameSession::fIsOnline( ) const
	{
		// Only one slot, no one can join but the local player
		if( fTotalSlots( ) == 1 )
			return false;

		// Uses matchmaking, so it can be found
		if( mCreateFlags & cCreateUsesMatchmaking )
			return true;

		// Invites and join by presence are disabled
		const u32 joinDisabled = cCreateInvitesDisabled |  cCreateJoinViaPresenceDisabled;
		if( mCreateFlags & joinDisabled )
			return false;

		// People could be invited or join via presence
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fTick( f32 dt )
	{
		mStateTimer += dt;
		fAdvanceState( false );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fWait( )
	{
		fAdvanceState( true );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fAdvanceState( b32 wait )
	{
		tState successState = mState;
		b32 checkDetailStateOnFail = false;
		
		switch( mState )
		{
		case cStateCreating:
			successState = cStateCreated;
			break;
		case cStateJoiningLocalUsers:
		case cStateJoiningRemoteUsers:
		case cStateRemovingLocalUsers:
		case cStateRemovingRemoteUsers:
			successState = mPrevState;
			break;
		case cStateRegistering:
			successState = cStateCreated;
			break;
		case cStateStarting:
			successState = cStateStarted;
			break;
		case cStateWritingStats:
			successState = cStateStarted;
			break;
		case cStateFlushingStats:
			successState = cStateStarted;
			break;
		case cStateEnding:
			checkDetailStateOnFail = true;
			successState = cStateCreated;
			break;
		case cStateDeleting:
			checkDetailStateOnFail = true;
			successState = cStateNull;
			break;
		}

		// No pending operation
		if( mState == successState )
			return;

		b32 success;

		// Check if the operation is complete
		if( !fOperationComplete( success, wait ) )
			return;

		if( !success && checkDetailStateOnFail )
			success = fCheckDetailState( successState );

		// If we succeeded and we have a new state then push it
		if( success )
			fPushState( successState );
		else
			fPopState( );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinZombie( u32 count )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Sessions can only be joined from Created/Started state" );

		if( fFreePublicSlots( ) + fFreePrivateSlots( ) < count )
			return false;

		// Consume private slots first
		const u32 privateToConsume = fMin( fFreePrivateSlots( ), count );
		mSlots[ cSlotFilledPrivate ] += privateToConsume;
		mSlots[ cSlotZombiePrivate ] += privateToConsume;
		count -= privateToConsume;

		// Then if need be consume public slots
		if( count )
		{
			sigassert( count <= fFreePublicSlots( ) && "Sanity!!" );

			mSlots[ cSlotFilledPublic ] += count;
			mSlots[ cSlotZombiePublic ] += count;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveZombie( u32 count )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Session can only be left from Created/Started state" );

		if( count > mSlots[ cSlotZombiePublic ] + mSlots[ cSlotZombiePrivate ] )
			return false;

		// Release public slots first
		const u32 publicToRestore = fMin( mSlots[ cSlotZombiePublic ], count );
		mSlots[ cSlotFilledPublic ] -= publicToRestore;
		mSlots[ cSlotZombiePublic ] -= publicToRestore;
		count -= publicToRestore;

		// Then if need be release private slots
		if( count )
		{
			sigassert( count <= mSlots[ cSlotZombiePrivate ] && "Sanity!!" );

			mSlots[ cSlotFilledPrivate ] -= count;
			mSlots[ cSlotZombiePrivate ] -= count;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fResetSlots( u32 maxPublic, u32 maxPrivate )
	{
		sigassert( mState == cStateNull && "Sanity: Can only reset slots in Null state" );

		mUsers.fSetCount( 0 );
		mUsers.fReserve( maxPublic + maxPrivate );

		mSlots.fFill( 0 );
		mSlots[ cSlotTotalPublic ] = maxPublic;
		mSlots[ cSlotTotalPrivate ] = maxPrivate;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fTryFillSlots( u32 & additionalPublic, u32 & additionalPrivate )
	{
		// Push excess private slot requests into public slot
		while( additionalPrivate + mSlots[ cSlotFilledPrivate ]  > mSlots[ cSlotTotalPrivate ] )
		{
			--additionalPrivate;
			++additionalPublic;
		}

		// Test if we're out of slots
		if( additionalPublic + mSlots[ cSlotFilledPublic ] > mSlots[ cSlotTotalPublic ] )
			return false;

		mSlots[ cSlotFilledPublic ] += additionalPublic;
		mSlots[ cSlotFilledPrivate ] += additionalPrivate;
		return true;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fPushState( tState newState )
	{
		if( newState == mState )
		{
			log_warning( 0, __FUNCTION__ << ": Attempt to push the current state again" );
			return;
		}

		log_line( Log::cFlagSession, __FUNCTION__ " moving from " << cStateNames[ mState ] << " to " << cStateNames[ newState ] );

		mPrevState = mState;
		mState = newState;

		switch( mPrevState )
		{
		case cStateCreating:
			fOnLeaveCreating( true );
			break;
		case cStateJoiningLocalUsers:
			fOnLeaveJoiningLocal( true );
			break;
		case cStateRemovingLocalUsers:
			fOnLeaveRemovingLocal( true );
			break;
		case cStateJoiningRemoteUsers:
			fOnLeaveJoiningRemote( true );
			break;
		case cStateRemovingRemoteUsers:
			fOnLeaveRemovingRemote( true );
			break;
		case cStateDeleting:
			fOnLeaveDeleting( true );
			break;
		}

		if( !mStateChangedCallback.fNull( ) )
			mStateChangedCallback( *this, mPrevState, true );

		mStateTimer = 0.f;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fPopState( )
	{
		sigassert( mPrevState != cStateCount && "Sanity: Mismatched state pops/pushes" );
		log_warning( 0, "GameSession state " << cStateNames[ mState ] << " failed, moving to " << cStateNames[ mPrevState ] );

		tState oldState = mState; // for callback

		mState = mPrevState;
		mPrevState = cStateCount;

		switch( oldState )
		{
		case cStateCreating:
			fOnLeaveCreating( false );
			break;
		case cStateJoiningLocalUsers:
			fOnLeaveJoiningLocal( false );
			break;
		case cStateRemovingLocalUsers:
			fOnLeaveRemovingLocal( false );
			break;
		case cStateJoiningRemoteUsers:
			fOnLeaveJoiningRemote( false );
			break;
		case cStateRemovingRemoteUsers:
			fOnLeaveRemovingRemote( false );
			break;
		}

		if( !mStateChangedCallback.fNull( ) )
			mStateChangedCallback( *this, oldState, false );

		mStateTimer = 0.f;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveCreating( b32 success )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		if( success )
		{
			sigassert( mState == cStateCreated ); // Sanity

			// If we just got created, sync query some game detail info
			fQueryDetails( );
		}
		else
		{
			// If we're popping from cStateCreating we have to close our session handle
			fCloseSessionData( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveDeleting( b32 success )
	{
		log_line( Log::cFlagSession, __FUNCTION__ );
		if( success )
		{
			sigassert( mState == cStateNull ); // Sanity

			// If we've advanced to Null, close our session data and reset the slots
			fCloseSessionData( );
			fResetSlots( mSlots[ cSlotTotalPublic ], mSlots[ cSlotTotalPrivate ] );
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinLocal( u32 index, b32 invited )
	{
		return fJoinLocal(1, &index, invited );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinLocal( u32 userCount, const u32 indices[], b32 invited )
	{
		b32 * invitedArray = (b32 * )alloca( sizeof( b32 ) * userCount );

		for( u32 u = 0; u < userCount; ++u )
			invitedArray[ u ] = invited;

		return fJoinLocal( userCount, indices, invitedArray );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveLocal( u32 index )
	{
		return fLeaveLocal( 1, &index );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeavelAllLocal( u32 & localUserCount )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Session can only be left from Created/Started state" );

		localUserCount = 0;
		tFixedArray< u32, tUser::cMaxLocalUsers > localUsers;

		for( u32 u = 0; u < mUsers.fCount( ); ++u )
		{
			if( mUsers[ u ].mIsLocal )
				localUsers[ localUserCount++ ] = mUsers[ u ].mLocalHwIndex;
		}

		if( localUserCount )
			return fLeaveLocal( localUserCount, localUsers.fBegin( ) );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinRemote( const tPlatformUserId & user, b32 invited )
	{
		return fJoinRemote( 1, &user, &invited );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fJoinRemote( u32 userCount, const tPlatformUserId users[], b32 invited )
	{
		b32 * invitedArray = (b32 *)alloca( sizeof(b32) * userCount );
		for( u32 u = 0; u < userCount; ++u )
			invitedArray[ u ] = invited;

		return fJoinRemote( userCount, users, invitedArray );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveRemote( const tPlatformUserId & user )
	{
		return fLeaveRemote( 1, &user );
	}

}
