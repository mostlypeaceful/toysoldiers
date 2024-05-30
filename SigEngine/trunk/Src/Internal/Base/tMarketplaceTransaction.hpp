//------------------------------------------------------------------------------
// \file tMarketplaceTransaction.hpp - 12 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMarketplaceTransaction__
#define __tMarketplaceTransaction__
#include "tMarketplaceOps.hpp"
#include "tUser.hpp"

namespace Sig
{
	typedef u64 tMarketplaceOfferId;

	///
	/// \class tMarketplaceTransaction
	/// \brief Represents a single marketplace transaction.
	///
	/// To ensure that a transaction can be resumed, client code should record
	/// the value of fNextState( ) before each call to fAdvanceState( ).
	class tMarketplaceTransaction
	{
	public:

		enum tState
		{
			cStateNull = 0,
			cStateCreateTransaction,
			cStatePurchaseAssets,
			cStateConsumeAssets,
			cStateGrantEntitlement,

			cNumStates
		};

		tMarketplaceTransaction( b32 skipConsumeOp );

		tState fState( ) const { return mState; }
		tState fNextState( ) const;
		u32 fStateAttempts( ) const { return mStateAttempts; }

		b32 fValid( ) const { return mState != cStateNull; }
		b32 fPending( ) const { return ( mState != cStateNull && mState != cStateGrantEntitlement ); }
		b32 fReadyToAdvance( ) const { return mSubState == cSubStateReadyToAdvance; }
		b32 fFailed( ) const { return mSubState == cSubStateFailed; }
		b32 fCancelled( ) const { return mSubState == cSubStateCancelled; }

		tPlatformUserId fUserId( ) const { return mUserId; }

		tMarketplaceOfferId fOfferId( u32 idx ) const { return mOfferIds[ idx ]; }
		u32 fOfferIdCount( ) const { return mOfferIds.fCount( ); }

		// Resets the transaction, starting at the specified state
		void fReset( const tUser& user, tMarketplaceOfferId offerId, tState state, b32 resumingExistingTransaction );

		// Updates the current state.
		// Has no effect if fReadyToAdvance( ) or fFailed( ) return true
		void fUpdate( );

		// Advances to the next state.
		// Asserts that fReadyToAdvance( ) returns true
		void fAdvanceState( );

		// Restarts the current state after a failure.
		// Asserts that fFailed( ) returns true
		void fRestartState();

	private:

		enum tSubState
		{
			cSubStateBegin = 0,
			cSubStateEnumerateAssets,
			cSubStateWaitForOp,
			cSubStateReadyToAdvance,
			cSubStateFailed,
			cSubStateCancelled
		};

		void fUpdateCreateTransaction( );
		void fUpdatePurchaseAssets( );
		void fUpdateConsumeAssets( );
		void fUpdateGrantEntitlement( );

		b32 fEnumerateAssets( );
		void fCreatePurchaseOp( );

	private:

		tPlatformUserId mUserId;
		tState mState;
		tGrowableArray< tMarketplaceOfferId > mOfferIds;

		u32 mStateAttempts;

		tSubState mSubState;
		u32 mUserLocalHwIndex;
		tMarketplacePurchaseOp mPurchaseOp;
		tMarketplaceConsumeOp mConsumeOp;
		tMarketplaceAssetEnumerator mAssetEnumerator;

		b32 mSkipConsumeOp;
		b32 mResumingExistingTransaction;
	};
}

#endif//__tMarketplaceTransaction__
