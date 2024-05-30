#include "BasePch.hpp"
#include "tGameSession.hpp"

namespace Sig
{
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
	{
		mViewsWrittenTo.fFill( ~0 );
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
	const tGameSessionDetails & tGameSession::fDetails( )
	{
		if( mDetailsOp.fValid( ) )
		{
			b32 success = mDetailsOp.fGetDetails( mDetails, true );
			sigassert( success && "Failed to get game session details" );
			mDetailsOp.fReset( );
		}

		return mDetails;
	}

	//------------------------------------------------------------------------------
	const tGameSessionInfo & tGameSession::fSessionInfo( )
	{
		return mData.mInfo;
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
	tGameSession::tState tGameSession::fSuccessNextState( ) const
	{
		switch( mState )
		{
		case cStateCreating:
			return cStateCreated;

		case cStateJoiningLocalUsers:
		case cStateJoiningRemoteUsers:
		case cStateRemovingLocalUsers:
		case cStateRemovingRemoteUsers:
		case cStateMigratingHost:
			return mPrevState;

		case cStateRegistering:
			return cStateCreated;

		case cStateStarting:
			return cStateStarted;

		case cStateWritingStats:
			return cStateStarted;

		case cStateFlushingStats:
			return cStateStarted;

		case cStateEnding:
			return cStateCreated;

		case cStateDeleting:
			return cStateNull;

		default:
			// assert?
			return mState;
		}
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fOperationPending( ) const
	{
		return mState != fSuccessNextState( );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fAdvanceState( b32 wait )
	{
		const tState successState = fSuccessNextState( );
		const b32 checkDetailStateOnFail = mState == cStateEnding || mState == cStateDeleting;

		if( !fOperationPending( ) )
			return;

		// Check if the operation is complete
		b32 success;
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
	b32 tGameSession::fMigrateHostLocal( u32 index )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Session can only be migrated from Created/Started state" );

		if( fMigrateHost( index, mData.mInfo ) )
		{
			mIsHost = true;
			fPushState( cStateMigratingHost );
			return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fMigrateHostRemote( tGameSessionInfo& sessionInfo )
	{
		sigassert( ( mState == cStateCreated || mState == cStateStarted ) && "Session can only be migrated from Created/Started state" );

		if( fMigrateHost( tUser::cUserIndexNone, sessionInfo ) )
		{
			fPushState( cStateMigratingHost );
			return true;
		}
		return false;
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
			log_warning( __FUNCTION__ << ": Attempt to push the current state again" );
			return;
		}

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
		default:
			// assert?
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
		log_warning( "GameSession state " << mState << " failed, moving to " << mPrevState );

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
		default:
			// assert?
			break;
		}

		if( !mStateChangedCallback.fNull( ) )
			mStateChangedCallback( *this, oldState, false );

		mStateTimer = 0.f;
	}

	//------------------------------------------------------------------------------
	void tGameSession::fOnLeaveCreating( b32 success )
	{
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
		malloca_array( b32, invitedArray, userCount );

		for( u32 u = 0; u < userCount; ++u )
			invitedArray[ u ] = invited;

		return fJoinLocal( userCount, indices, invitedArray.fBegin() );
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
		malloca_array( b32, invitedArray, userCount );
		for( u32 u = 0; u < userCount; ++u )
			invitedArray[ u ] = invited;

		return fJoinRemote( userCount, users, invitedArray.fBegin() );
	}

	//------------------------------------------------------------------------------
	b32 tGameSession::fLeaveRemote( const tPlatformUserId & user )
	{
		return fLeaveRemote( 1, &user );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fFlushQueuedStats( )
	{
		mQueuedWrites.fReserve( 1 );

		tStatsWritePtr forceFlush( NEW tStatsWrite( ) );
		forceFlush->mUserId = tUser::cInvalidUserId; // invalid indicates flush command
		mQueuedWrites.fPushLast( forceFlush );

		fFlushStats( );
	}

	//------------------------------------------------------------------------------
	void tGameSession::fProcessStatsWriteQueue( )
	{
		tGrowableArray< tStatsWritePtr > toWrite;
		tPlatformUserId writeUserId = tUser::cInvalidUserId;

		while( mQueuedWrites.fNumItems( ) )
		{
			tStatsWritePtr ptr = mQueuedWrites.fFirst( );

			// A user was removed
			if( ptr.fNull( ) )
			{
				mQueuedWrites.fTryPopFirst( );
				continue;
			}

			if( writeUserId == tUser::cInvalidUserId )
				writeUserId = ptr->mUserId;

			// We can only do writes for one user at a time
			if( writeUserId != ptr->mUserId )
				break;

			// Check if we need to do a flush before we can do the write
			b32 canWrite = false;

			// Special flush commands have invalid user id
			if( writeUserId != tUser::cInvalidUserId )
			{
				for( u32 v = 0; v < mViewsWrittenTo.fCount( ); ++v )
				{
					// Set the view if the array isn't full
					if( mViewsWrittenTo[ v ] == ~0 )
						mViewsWrittenTo[ v ] = ptr->mViewId;

					// The view is valid
					if( mViewsWrittenTo[ v ] == ptr->mViewId )
					{
						canWrite = true;
						break;
					}
				}
			}
			else if( !toWrite.fCount( ) )
			{
				// Remove the special flush command because we're going to flush below
				mQueuedWrites.fTryPopFirst( );
			}

			// If we can't write this one 
			if( !canWrite )
			{
				// and we didn't validate any before it, then we need to do a flush
				if( !toWrite.fCount( ) )
				{
					if( fFlushStats( ) )
						mViewsWrittenTo.fFill( ~0 );
					else
						log_warning( "Failed to flush stats" );
					return;
				}

				// otherwise we can just break out and write those we can
				else
					break;
			}

			// Remove it from the queue because the write will occur
			mQueuedWrites.fTryPopFirst( );
			toWrite.fPushBack( ptr );
		}

		const u32 toWriteCount = toWrite.fCount( );

		if( !toWriteCount )
			return;

		malloca_array( tGameSessionViewProperties, properties, toWriteCount );

		for( u32 w = 0; w < toWriteCount; ++w )
		{
			const tStatsWritePtr & ptr = toWrite[ w ];
			properties[ w ].mViewId = ptr->mViewId;
			properties[ w ].mNumProperties = ptr->mProperties.fCount( );
			properties[ w ].mProperties = ptr->mProperties.fBegin( );
		}

		if( !fWriteStats( writeUserId, toWriteCount, properties.fBegin() ) )
		{
			log_warning( "Stats write failed to start, requeueing" );
			for( u32 w = 0; w < toWriteCount; ++w )
				mQueuedWrites.fPushLast( toWrite[ w ] );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSession::fKillQueuedWritesForUser( tPlatformUserId userId )
	{
		for( u32 w = 0; w < mQueuedWrites.fNumItems( ); ++w )
		{
			if( mQueuedWrites[ w ]->mUserId == userId )
				mQueuedWrites[ w ].fRelease( );
		}
	}

	//------------------------------------------------------------------------------
	void tGameSession::fQueueWriteStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		if( mState < tGameSession::cStateStarted )
		{
			log_warning( "Attempt to write stats to session that is not started" );
			return;
		}

		if( userId == tUser::cInvalidUserId )
		{
			log_warning( "Attempt to write stats for invalid user" );
			return;
		}

		if( !writeCount )
			return;

		if( mQueuedWrites.fNumItems( ) + writeCount > mQueuedWrites.fCapacity( ) )
			mQueuedWrites.fResize( ( mQueuedWrites.fCapacity( ) + writeCount ) * 2  );

		// Store the writes to the queue
		for( u32 w = 0; w < writeCount; ++w )
		{
			const tGameSessionViewProperties & view = writes[ w ];

			tStatsWritePtr ptr( NEW tStatsWrite( ) );

			// Setup the write
			ptr->mUserId = userId;
			ptr->mViewId = view.mViewId;
			ptr->mProperties.fInitialize( view.mProperties, view.mNumProperties );

			// Push the write
			mQueuedWrites.fPushLast( ptr );
		}
	}

}
