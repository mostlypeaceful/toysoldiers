//------------------------------------------------------------------------------
// \file tMarketplaceAsset_ios.cpp - 04 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_ios )
#include "tMarketplaceAsset.hpp"

namespace Sig
{
	b32 tMarketplaceAssetEnumerator::fCreate( u32 userIndex, u32 numItems )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tMarketplaceAssetEnumerator::fEnumerate( )
	{
		log_warning_unimplemented( );
		return false;
	}

	b32 tMarketplaceAssetEnumerator::fAdvance( )
	{
		log_warning_unimplemented( );
		return false;
	}

	u32 tMarketplaceAssetEnumerator::fResultCount( )
	{
		log_warning_unimplemented( );
		return 0;
	}

	void tMarketplaceAssetEnumerator::fGetResults( tMarketplaceAssetList& assetListOut )
	{
		log_warning_unimplemented( );
	}

	void tMarketplaceAssetEnumerator::fDestroy( )
	{
		log_warning_unimplemented( );
	}
}
#endif//#if defined( platform_ios )
