//------------------------------------------------------------------------------
// \file tMarketplaceOps_xbox360.cpp - 10 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tMarketplaceOps.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tMarketplacePurchaseOp
	//------------------------------------------------------------------------------
	const u32 tMarketplacePurchaseOp::cEntryPointFreeItems = XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_FREEITEMS;
	const u32 tMarketplacePurchaseOp::cEntryPointPaidItems = XSHOWMARKETPLACEDOWNLOADITEMS_ENTRYPOINT_PAIDITEMS;

	b32 tMarketplacePurchaseOp::fCreate( u32 userIndex, u32 entryPoint, const u64* offerIds, u32 offerIdCount )
	{
		if( !fPreExecute( ) )
			return false;

		DWORD result = XShowMarketplaceDownloadItemsUI( userIndex,
			entryPoint,
			offerIds,
			offerIdCount,
			&mResult,
			fOverlapped( ) );

		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( "XShowMarketplaceDownloadItemsUI failed with error: " << result );
			return false;
		}

		return true;
	}

	b32 tMarketplacePurchaseOp::fFailed( )
	{
		sigassert( fIsComplete( ) );
		return ( mResult == MPDI_E_OPERATION_FAILED || mResult == MPDI_E_INVALIDARG );
	}

	b32 tMarketplacePurchaseOp::fCancelled( )
	{
		sigassert( fIsComplete( ) );
		return ( mResult == MPDI_E_CANCELLED );
	}

	//------------------------------------------------------------------------------
	// tMarketplaceConsumeOp
	//------------------------------------------------------------------------------
	b32 tMarketplaceConsumeOp::fCreate( u32 userIndex, const tMarketplaceAsset* assets, u32 assetCount )
	{
		if( !fPreExecute( ) )
			return false;

		DWORD result = XMarketplaceConsumeAssets( userIndex,
			assetCount,
			assets,
			fOverlapped( ) );

		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( "XMarketplaceConsumeAssets failed with error: " << result );
			return false;
		}

		return true;
	}

	Sig::b32 tMarketplaceConsumeOp::fFailed()
	{
		sigassert( fIsComplete( ) );
		u32 result = 0;
		if( !fGetResult( result ) )
			return true;

		return ( result != ERROR_SUCCESS );
	}

	//------------------------------------------------------------------------------
	// tMarketplaceAcquireContentOp
	//------------------------------------------------------------------------------
	b32 tMarketplaceAcquireContentOp::fCreate( 
		u32 userIndex,
		const tMarketplaceContentOffer& offer,
		const GUID& offerId,
		const GUID& mediaId,
		const GUID& mediaInstanceId )
	{
		if( !fPreExecute( ) )
			return false;

		fMemCpy( mAcquireContentInfo.contentId, offer.contentId, sizeof( offer.contentId ) );
		const std::wstring displayName( offer.wszOfferName );
		fMemCpy( mAcquireContentInfo.displayName, displayName.c_str( ), ( displayName.length( ) + 1 ) * sizeof( displayName[ 0 ] ) );
		mAcquireContentInfo.installSize.QuadPart = offer.dwInstallSize;
		mAcquireContentInfo.legacyOfferId = offer.qwOfferID;
		mAcquireContentInfo.mediaId = mediaId;
		mAcquireContentInfo.mediaInstanceId = mediaInstanceId;
		mAcquireContentInfo.offerId = offerId;

		DWORD result = XMarketplaceAcquireFreeContent( userIndex,
			&mAcquireContentInfo,
			fOverlapped( ) );

		if( result != ERROR_SUCCESS && result != ERROR_IO_PENDING )
		{
			log_warning( "XMarketplaceAcquireContent failed with error: " << result );
			return false;
		}

		return true;
	}

	b32 tMarketplaceAcquireContentOp::fCancelled( )
	{
		sigassert( fIsComplete( ) );
		u32 result = 0;
		if( !fGetResult( result ) )
			return true;

		return ( result == ERROR_CANCELLED );
	}

	Sig::b32 tMarketplaceAcquireContentOp::fFailed( )
	{
		sigassert( fIsComplete( ) );
		u32 result = 0;
		if( !fGetResult( result ) )
			return true;

		return ( result != ERROR_SUCCESS && result != ERROR_CANCELLED );
	}

}
#endif//#if defined( platform_xbox360 )
