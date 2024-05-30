//------------------------------------------------------------------------------
// \file tMarketplaceContentOffer_xbox360.cpp - 07 Aug 2013
// \author colins
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tMarketplaceContentOffer.hpp"

namespace Sig
{
	b32 tMarketplaceContentOfferEnumerator::fCreate( u32 userIndex, const u64* offerIds, u32 numOfferIds, u32 numItems )
	{
		sigassert( mState == cStateNull && "Can only create from Null state" );

		HANDLE enumHandle = INVALID_HANDLE_VALUE;
		DWORD requiredBufferSize = 0;
		DWORD result = XMarketplaceCreateCurrencyOfferEnumeratorByOffering
			( userIndex
			, numItems
			, offerIds
			, numOfferIds
			, &requiredBufferSize
			, &enumHandle );

		if( result != ERROR_SUCCESS )
		{
			log_warning( "XMarketplaceCreateCurrencyOfferEnumeratorByOffering failed with error: " << result );
			return false;
		}

		mEnumOp.fInitialize( requiredBufferSize, enumHandle );
		mState = cStateCreated;
		return true;
	}

	b32 tMarketplaceContentOfferEnumerator::fCreate( u32 userIndex, u32 offerType, u32 contentCategories, u32 numItems )
	{
		sigassert( mState == cStateNull && "Can only create from Null state" );

		HANDLE enumHandle = NULL;
		DWORD requiredBufferSize = 0;
		DWORD result = XMarketplaceCreateCurrencyOfferEnumerator
			( userIndex
			, offerType
			, contentCategories
			, numItems
			, &requiredBufferSize
			, &enumHandle );

		if( result != ERROR_SUCCESS )
		{
			log_warning( "XMarketplaceCreateCurrencyOfferEnumerator failed with error: " << result );
			return false;
		}

		mEnumOp.fInitialize( requiredBufferSize, enumHandle );
		mState = cStateCreated;
		return true;
	}

	b32 tMarketplaceContentOfferEnumerator::fEnumerate( )
	{
		sigassert( mState == cStateCreated && "Can only begin enumeration from created state" );

		if( !mEnumOp.fBegin( ) )
			return false;

		mState = cStateEnumerating;
		return true;
	}

	b32 tMarketplaceContentOfferEnumerator::fAdvance( )
	{
		sigassert( mState != cStateNull && "Cannot advance Null MarketplaceContentOfferEnumerator" );

		if( mState == cStateEnumerating )
		{
			if( !mEnumOp.fIsComplete( ) )
				return false;

			// The operation is complete
			u32 result; // Result is the number of items returned
			if( mEnumOp.fGetResultNoMoreFilesOk( result, true ) )
				mState = cStateSuccess;
			else
				mState = cStateFail;
		}

		return true;
	}

	u32 tMarketplaceContentOfferEnumerator::fResultCount( )
	{
		sigassert( mState == cStateSuccess && "Result count only available from StateSuccess" );

		return mEnumOp.fResultCount( );
	}

	void tMarketplaceContentOfferEnumerator::fGetResults( tMarketplaceContentOfferList& contentOfferListOut )
	{
		sigassert( mState == cStateSuccess && "Results only available from StateSuccess" );
		
		const tMarketplaceContentOffer* results = mEnumOp.fResults<tMarketplaceContentOffer>( );
		for( u32 resultIdx = 0; resultIdx < fResultCount( ); ++resultIdx )
			contentOfferListOut.fPushBack( results[ resultIdx ] );
	}

	void tMarketplaceContentOfferEnumerator::fDestroy( )
	{
		if( !mEnumOp.fIsComplete( ) )
			mEnumOp.fCancel( );
		mEnumOp.fReset( );
		mState = cStateNull;
	}
}
#endif//#if defined( platform_xbox360 )
