//------------------------------------------------------------------------------
// \file tMarketplaceContentOffer_ios.cpp - 07 Aug 2013
// \author colins
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_ios )
#include "tMarketplaceContentOffer.hpp"

namespace Sig
{
	b32 tMarketplaceContentOfferEnumerator::fCreate( u32 userIndex, const u64* offerIds, u32 numOfferIds, u32 numItems )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tMarketplaceContentOfferEnumerator::fCreate( u32 userIndex, u32 offerType, u32 contentCategories, u32 numItems )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tMarketplaceContentOfferEnumerator::fEnumerate( )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tMarketplaceContentOfferEnumerator::fAdvance( )
	{
		log_warning_unimplemented( );
		return false;
	}

	u32 tMarketplaceContentOfferEnumerator::fResultCount( )
	{
		log_warning_unimplemented( );
		return 0;
	}

	void tMarketplaceContentOfferEnumerator::fGetResults( tMarketplaceContentOfferList& contentOfferListOut )
	{
		log_warning_unimplemented( );
	}

	void tMarketplaceContentOfferEnumerator::fDestroy( )
	{
		log_warning_unimplemented( );
	}
}
#endif//#if defined( platform_ios )
