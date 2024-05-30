//------------------------------------------------------------------------------
// \file tMarketplaceAsset_xbox360.cpp - 04 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tMarketplaceAsset.hpp"

namespace Sig
{
	b32 tMarketplaceAssetEnumerator::fCreate( u32 userIndex, u32 numItems )
	{
		sigassert( mState == cStateNull && "Can only create from Null state" );

		HANDLE enumHandle = NULL;
		DWORD requiredBufferSize = 0;
		DWORD result = XMarketplaceCreateAssetEnumerator( userIndex,
			numItems,
			&requiredBufferSize,
			&enumHandle );

		if( result != ERROR_SUCCESS )
		{
			log_warning( "XMarketplaceCreateAssetEnumerator failed with error: " << result );
			return false;
		}

		mEnumOp.fInitialize( requiredBufferSize, enumHandle );
		mState = cStateCreated;
		return true;
	}

	// TODO: Revisit. This was copied from tContentEnumerator
	b32 tMarketplaceAssetEnumerator::fEnumerate( )
	{
		sigassert( mState == cStateCreated && "Can only begin enumeration from created state" );

		if( !mEnumOp.fBegin( ) )
			return false;

		mState = cStateEnumerating;
		return true;
	}

	// TODO: Revisit. This was copied from tContentEnumerator
	b32 tMarketplaceAssetEnumerator::fAdvance( )
	{
		sigassert( mState != cStateNull && "Cannot advance Null MarketplaceAssetEnumerator" );

		if( mState == cStateEnumerating )
		{
			if( !mEnumOp.fIsComplete( ) )
				return false;

			//TODO: Revisit. I'm not sure what this is supposed to do, but it doesn't look like it enumerates over
			//all results...it looks like it just gets the first set of results and then completes.
			//
			// The operation is complete
			u32 result; // Result is the number of items returned
			if( mEnumOp.fGetResultNoMoreFilesOk( result, true ) )
				mState = cStateSuccess;
			else
				mState = cStateFail;
		}

		return true;
	}

	u32 tMarketplaceAssetEnumerator::fResultCount( )
	{
		sigassert( mState == cStateSuccess && "Result count only available from StateSuccess" );

		return mEnumOp.fResultCount( );
	}

	void tMarketplaceAssetEnumerator::fGetResults( tMarketplaceAssetList& assetListOut )
	{
		sigassert( mState == cStateSuccess && "Results only available from StateSuccess" );
		
		const XMARKETPLACE_ASSET_ENUMERATE_REPLY* results = mEnumOp.fResults<XMARKETPLACE_ASSET_ENUMERATE_REPLY>( );
		for( u32 resultIdx = 0; resultIdx < fResultCount( ); ++resultIdx )
		{
			const XMARKETPLACE_ASSET_PACKAGE& assetPackage = results[ resultIdx ].assetPackage;
			for( u32 assetIdx = 0; assetIdx < assetPackage.cAssets; ++assetIdx )
			{
				const XMARKETPLACE_ASSET& asset = assetPackage.aAssets[ assetIdx ];

				tMarketplaceAsset& assetOut = assetListOut.fPushBack( );
				assetOut.dwAssetID = asset.dwAssetID;
				assetOut.dwQuantity = asset.dwQuantity;
			}
		}
	}

	void tMarketplaceAssetEnumerator::fDestroy( )
	{
		if( !mEnumOp.fIsComplete( ) )
			mEnumOp.fCancel( );
		mEnumOp.fReset( );
		mState = cStateNull;
	}
}
#endif//#if defined( platform_xbox360 )
