//------------------------------------------------------------------------------
// \file tContent_pcdx9.cpp - 28 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx ) || defined( platform_metro )
#include "tContent.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tContentData
	//------------------------------------------------------------------------------

	const u32 tContentData::cContentTypeSavedGame	= 0;
	const u32 tContentData::cContentTypeMarketPlace = 0;
	const u32 tContentData::cContentTypePublisher	= 0;
	const u32 tContentData::cContentDeviceIdAny		= 0;

	//------------------------------------------------------------------------------
	// tContent
	//------------------------------------------------------------------------------

	const u32 tContent::cContentFlagCreateNew					= 0;
	const u32 tContent::cContentFlagCreateAlways				= 0;
	const u32 tContent::cContentFlagOpenExisting				= 0;
	const u32 tContent::cContentFlagOpenAlways					= 0;
	const u32 tContent::cContentFlagTruncateExisting			= 0;
	const u32 tContent::cContentFlagNoDeviceTransfer			= 0;
	const u32 tContent::cContentFlagNoProfileTransfer			= 0;
	const u32 tContent::cContentFlagAllowProfileTransfer		= 0;
	const u32 tContent::cContentFlagStrongSigned				= 0;
	const u32 tContent::cContentFlagMoveOnlyTransfer			= 0;

	//------------------------------------------------------------------------------
	b32 tContent::fCreate( 
		u32 userIndex,
		const tStringPtr & rootName,
		const XCONTENT_DATA & contentData,
		u32 contentFlags )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tContent::fFlush( )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tContent::fClose( )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tContent::fAdvance( )
	{
		log_warning_unimplemented( );
		return true;
	}

}

#endif // defined( platform_pcdx ) || defined( platform_metro )
