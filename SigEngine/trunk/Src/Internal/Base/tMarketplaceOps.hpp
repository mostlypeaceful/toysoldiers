//------------------------------------------------------------------------------
// \file tMarketplaceOps.hpp - 10 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMarketplaceOps__
#define __tMarketplaceOps__
#include "tMarketplaceAsset.hpp"
#include "tMarketplaceContentOffer.hpp"

namespace Sig
{
#if defined( platform_xbox360 )

	class tMarketplacePurchaseOp : public XtlUtil::tOverlappedOp
	{
	public:

		static const u32 cEntryPointFreeItems;	// Offers only free and purchased items
		static const u32 cEntryPointPaidItems;	// Offers free and paid items

	public:

		tMarketplacePurchaseOp( ) : mResult( 0 ) { }

		b32 fFailed( );
		b32 fCancelled( );

		b32 fCreate( 
			u32 userIndex,
			u32 entryPoint,
			const u64* offerIds,
			u32 offerIdCount );

	private:
		HRESULT mResult;
	};

	class tMarketplaceConsumeOp : public XtlUtil::tOverlappedOp
	{	
	public:

		b32 fFailed( );

		b32 fCreate( 
			u32 userIndex,
			const tMarketplaceAsset* assets,
			u32 assetCount );
	};

	class tMarketplaceAcquireContentOp : public XtlUtil::tOverlappedOp
	{
	public:

		tMarketplaceAcquireContentOp( ) : mResult( 0 ) { }

		b32 fFailed( );
		b32 fCancelled( );

		b32 fCreate( 
			u32 userIndex,
			const tMarketplaceContentOffer& offer,
			const GUID& offerId,
			const GUID& mediaId,
			const GUID& mediaInstanceId );

	private:
		HRESULT mResult;
		XMARKETPLACE_ACQUIRE_CONTENT_INFO mAcquireContentInfo;
	};

#else

	class tMarketplacePurchaseOp
	{
	public:
		static const u32 cEntryPointFreeItems = 0;
		static const u32 cEntryPointPaidItems = 0;

		b32 fFailed( ) { return true; }
		b32 fCancelled( ) { return false; }
		b32 fIsComplete( ) { return true; }

		b32 fCreate( 
			u32 userIndex,
			u32 entryPoint,
			const u64* offerIds,
			u32 offerIdCount ) { return false; }
	};

	class tMarketplaceConsumeOp
	{
	public:
		b32 fFailed( ) { return true; }
		b32 fIsComplete( ) { return true; }

		b32 fCreate( 
			u32 userIndex,
			const tMarketplaceAsset* assets,
			u32 assetCount ) { return false; }
	};

#endif//#if defined( platform_xbox360 )
}

#endif//__tMarketplaceOps__
