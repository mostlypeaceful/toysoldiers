//------------------------------------------------------------------------------
// \file tMarketplaceTransaction.cpp - 12 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tMarketplaceTransaction.hpp"

namespace Sig
{
	tMarketplaceTransaction::tMarketplaceTransaction( b32 skipConsumeOp )
		: mUserId( tUser::cInvalidUserId )
		, mState( cStateNull )
		, mStateAttempts( 0 )
		, mSubState( cSubStateBegin )
		, mUserLocalHwIndex( ~0 )
		, mSkipConsumeOp( skipConsumeOp )
		, mResumingExistingTransaction( false )
	{ }


	tMarketplaceTransaction::tState tMarketplaceTransaction::fNextState( ) const
	{
		tState nextState = ( tState )( ( mState + 1 ) % cNumStates );
		if( nextState == cStateConsumeAssets && mSkipConsumeOp )
			nextState = cStateGrantEntitlement;

		return nextState;
	}

	void tMarketplaceTransaction::fReset( const tUser& user, tMarketplaceOfferId offerId, tState state, b32 resumingExistingTransaction )
	{
		log_assert( state != cStateConsumeAssets || !mSkipConsumeOp, "State is not valid when mSkipConsumeOp is true" );

		mUserId = user.fPlatformId( );
		mUserLocalHwIndex = user.fLocalHwIndex( );
		mState = state;
		mStateAttempts = 1;
		mSubState = cSubStateBegin;
		mOfferIds.fSetCount( 0 );
		mOfferIds.fPushBack( offerId );
		mResumingExistingTransaction = resumingExistingTransaction;
	}

	void tMarketplaceTransaction::fUpdate( )
	{
		switch( mState )
		{
		case cStateCreateTransaction:
			fUpdateCreateTransaction( );
			break;
		case cStatePurchaseAssets:
			fUpdatePurchaseAssets( );
			break;
		case cStateConsumeAssets:
			fUpdateConsumeAssets( );
			break;
		case cStateGrantEntitlement:
			fUpdateGrantEntitlement( );
			break;
		}
	}

	void tMarketplaceTransaction::fAdvanceState( )
	{
		sigassert( mState != cStateGrantEntitlement && "Already at the last state. Call fReset() instead." );
		sigassert( mSubState == cSubStateReadyToAdvance && "State is not ready to advance." );

		mState = fNextState( );
		mSubState = cSubStateBegin;
		mStateAttempts = 1;
	}

	void tMarketplaceTransaction::fRestartState( )
	{
		sigassert( fFailed( ) && "Cannot restart a state that hasn't failed." );

		mResumingExistingTransaction = true;
		mSubState = cSubStateBegin;
		++mStateAttempts;
	}

	void tMarketplaceTransaction::fUpdateCreateTransaction( )
	{
		// Nothing to do, so allow the client to advance the state
		if( mSubState == cSubStateBegin )
			mSubState = cSubStateReadyToAdvance;
	}

	void tMarketplaceTransaction::fUpdatePurchaseAssets( )
	{
		if( mSubState == cSubStateBegin )
		{
			// If this is a new transaction, don't bother enumerating assets
			if( !mResumingExistingTransaction )
				fCreatePurchaseOp( );

			else if( !fEnumerateAssets( ) )
				mSubState = cSubStateFailed;
		}
		else if( mSubState == cSubStateEnumerateAssets )
		{
			if( mAssetEnumerator.fState( ) != tMarketplaceAssetEnumerator::cStateNull )
				mAssetEnumerator.fAdvance( );

			if( mAssetEnumerator.fSucceeded( ) )
			{
				// If assets are already purchased, we're ready for the next state.
				// If not, start a purchase operation
				if( mAssetEnumerator.fResultCount( ) > 0 )
				{
					log_line( Log::cFlagNetwork, "Found " << mAssetEnumerator.fResultCount( ) << " purchased assets." );
					mSubState = cSubStateReadyToAdvance;
				}
				else if( mResumingExistingTransaction )
				{
					// No assets were purchased, so we can cancel instead of bringing up the UI
					mSubState = cSubStateCancelled;
				}
				else
				{
					fCreatePurchaseOp( );
				}

				mAssetEnumerator.fDestroy( );
			}
			else if( mAssetEnumerator.fFailed( ) )
			{
				mAssetEnumerator.fDestroy( );
				mSubState = cSubStateFailed;
			}
		}
		else if( mSubState == cSubStateWaitForOp )
		{
			if( mPurchaseOp.fIsComplete( ) )
			{
				if( mPurchaseOp.fFailed( ) )
					mSubState = cSubStateFailed;
				else if( mPurchaseOp.fCancelled( ) )
					mSubState = cSubStateCancelled;
				else
					mSubState = cSubStateReadyToAdvance;
			}
		}
	}

	void tMarketplaceTransaction::fUpdateConsumeAssets( )
	{
		if( mSkipConsumeOp )
		{
			// Nothing to do, so allow the client to advance the state
			if( mSubState == cSubStateBegin )
				mSubState = cSubStateReadyToAdvance;

			return;
		}

		if( mSubState == cSubStateBegin )
		{
			if( !fEnumerateAssets( ) )
				mSubState = cSubStateFailed;;
		}
		else if( mSubState == cSubStateEnumerateAssets )
		{
			mAssetEnumerator.fAdvance( );

			if( mAssetEnumerator.fSucceeded( ) )
			{
				// If assets are already consumed, we're ready for the next state.
				// If not, start a consume operation
				if( mAssetEnumerator.fResultCount( ) == 0 )
				{
					log_line( Log::cFlagNetwork, "Assets are already consumed." );
					mSubState = cSubStateReadyToAdvance;
				}
				else
				{
					tMarketplaceAssetList assetList;
					mAssetEnumerator.fGetResults( assetList );

					log_line( Log::cFlagNetwork, "Found " << assetList.fCount( ) << " assets to consume." );

					b32 success = mConsumeOp.fCreate( mUserLocalHwIndex,
						assetList.fBegin( ),
						assetList.fCount( ) );

					if( success )
						mSubState = cSubStateWaitForOp;
					else
						mSubState = cSubStateFailed;
				}

				mAssetEnumerator.fDestroy( );
			}
			else if( mAssetEnumerator.fFailed( ) )
			{
				mAssetEnumerator.fDestroy( );
				mSubState = cSubStateFailed;
			}
		}
		else if( mSubState == cSubStateWaitForOp )
		{
			if( mConsumeOp.fIsComplete( ) )
			{
				if( mConsumeOp.fFailed( ) )
					mSubState = cSubStateFailed;
				else
					mSubState = cSubStateReadyToAdvance;
			}
		}
	}

	void tMarketplaceTransaction::fUpdateGrantEntitlement( )
	{
		// Nothing to do, so allow the client to advance the state
		if( mSubState == cSubStateBegin )
			mSubState = cSubStateReadyToAdvance;
	}

	b32 tMarketplaceTransaction::fEnumerateAssets( )
	{
		sigassert( mSubState != cSubStateEnumerateAssets && "Already enumerating assets." );

		// Start enumerating assets
		if( !mAssetEnumerator.fCreate( mUserLocalHwIndex, 10 ) )
			return false;
		if( !mAssetEnumerator.fEnumerate( ) )
			return false;

		mSubState = cSubStateEnumerateAssets;

		return true;
	}

	void tMarketplaceTransaction::fCreatePurchaseOp( )
	{
		sigassert( mOfferIds.fCount( ) > 0 && "Cannot start a purchase without offer Ids" );

		b32 success = mPurchaseOp.fCreate( mUserLocalHwIndex,
			tMarketplacePurchaseOp::cEntryPointPaidItems,
			mOfferIds.fBegin( ),
			mOfferIds.fCount( ) );

		if( success )
			mSubState = cSubStateWaitForOp;
		else
			mSubState = cSubStateCancelled; // Cancel instead of fail so we don't keep displaying the UI
	}
}
