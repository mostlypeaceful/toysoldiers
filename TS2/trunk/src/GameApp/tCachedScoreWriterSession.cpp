#include "GameAppPch.hpp"
#include "tCachedScoreWriterSession.hpp"

namespace Sig
{
	tCachedScoreWriterSession::tCachedScoreWriterSession( ) 
		: mStatsWritten( 0 )
	{
	}

	tCachedScoreWriterSession::~tCachedScoreWriterSession( )
	{
	}

	b32 tCachedScoreWriterSession::fCanWriteStats( )
	{
		return mSession.fNull( ) || mSession->fState( ) == tGameSession::cStateNull;
	}

	void tCachedScoreWriterSession::fReset( )
	{
		mQueuedWrites.fReset( );
	}

	void tCachedScoreWriterSession::fQueueStats( tPlatformUserId userId, u32 writeCount, const tGameSessionViewProperties writes[] )
	{
		if( !fCanWriteStats( ) ) // We've already kicked off a stat write
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
			mQueuedWrites.fPut( ptr );
		}
	}

	b32 tCachedScoreWriterSession::fStartSession( const tUserPtr& user )
	{
		if( !user )
			return false;

		mUser = user;

		if( mQueuedWrites.fNumItems( ) == 0 )
			return false;

		if( !fCanWriteStats( ) )
		{
			// We're already writing stats!
			return false;
		}

		mStatsWritten = 0;

		user->fSetContext( tUser::cUserContextGameType, tUser::cUserContextGameTypeStandard );

		u32 sessionFlags = tGameSession::cCreateUsesStats;
		mSession.fReset( NEW tGameSession( sessionFlags
			,0
			,1
			,make_delegate_memfn( 
			tGameSession::tStateChangedCallback, 
			tCachedScoreWriterSession, 
			fGameSessionCallback ) ) );

		b32 success = mSession->fCreate( user->fLocalHwIndex( ) );
		if( !success )
			mSession.fRelease( );

		return success;
	}

	void tCachedScoreWriterSession::fOnUserSignInChange(
		const tUserSigninInfo oldStates[], 
		const tUserSigninInfo newStates[] )
	{
		if( mUser )
		{
			u32 hwIdx = mUser->fLocalHwIndex( );
			if( newStates[ hwIdx ].mState != tUser::cSignInStateSignedInOnline )
			{
				mSession.fRelease( );
				mQueuedWrites.fReset( );
			}
		}
	}

	void tCachedScoreWriterSession::fTick( )
	{
		if( mSession.fNull( ) == false )
			mSession->fTick( 0.f );
	}

	void tCachedScoreWriterSession::fGameSessionCallback( tGameSession& gameSession, u32 oldState, b32 success )
	{
		if( mSession.fNull( ) )
			return;

		if( oldState == tGameSession::cStateCreating  &&  mSession->fState( ) == tGameSession::cStateCreated )
		{
			// Created!
			mSession->fJoinLocal( mUser->fLocalHwIndex( ), true );
		}
		else if( oldState == tGameSession::cStateJoiningLocalUsers && mSession->fState( ) == tGameSession::cStateCreated )
		{
			// Joined!
			mSession->fStart( );
		}
		else if( oldState == tGameSession::cStateStarting && mSession->fState( ) == tGameSession::cStateStarted ||
				 oldState == tGameSession::cStateFlushingStats && mSession->fState( ) == tGameSession::cStateStarted )
		{
			// Started! 
			// We can only write 5 things at a time
			u32 writeCount = 0;
			u32 maxWriteCount = fMin<u32>( 5, mQueuedWrites.fNumItems( ) );
			tPlatformUserId writeUserId = tUser::cInvalidUserId;

			tGameSessionViewProperties * properties = 
				(tGameSessionViewProperties *)alloca( sizeof(tGameSessionViewProperties) * maxWriteCount );

			while( writeCount < maxWriteCount )
			{
				if( mQueuedWrites.fNumItems( ) == 0 )
					break;

				const tStatsWritePtr & ptr = mQueuedWrites.fBack( );

				if( ptr.fNull( ) )
					break;

				if( writeUserId == tUser::cInvalidUserId )
					writeUserId = ptr->mUserId;

				// Can only write to one user
				if( ptr->mUserId != writeUserId )
					break;

				properties[ writeCount ].mViewId = ptr->mViewId;
				properties[ writeCount ].mNumProperties = ptr->mProperties.fCount( );
				properties[ writeCount ].mProperties = ptr->mProperties.fBegin( );

				++writeCount;

				mQueuedWrites.fGet( );
			}

			mSession->fWriteStats( mUser->fPlatformId( ), writeCount, properties ); 
		}
		else if( oldState == tGameSession::cStateWritingStats && mSession->fState( ) == tGameSession::cStateStarted )
		{
			if( mQueuedWrites.fNumItems( ) > 0 )
			{
				mSession->fFlushStats( );
				return;
			}
			// Done!
			mSession->fEnd( );
		}
		else if( oldState == tGameSession::cStateEnding && mSession->fState( ) == tGameSession::cStateCreated )
		{
			// Clear score writes after they've been written
			mQueuedWrites.fReset( );

			// Ended!
			mSession->fDelete( );
		}
	}
}