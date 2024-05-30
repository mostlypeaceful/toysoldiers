//------------------------------------------------------------------------------
// \file tContent_xbox360.cpp - 24 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tContent.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tContentData
	//------------------------------------------------------------------------------

	const u32 tContentData::cContentTypeSavedGame	= XCONTENTTYPE_SAVEDGAME;
	const u32 tContentData::cContentTypeMarketPlace = XCONTENTTYPE_MARKETPLACE;
	const u32 tContentData::cContentTypePublisher	= XCONTENTTYPE_PUBLISHER;
	const u32 tContentData::cContentDeviceIdAny		= XCONTENTDEVICE_ANY;

	//------------------------------------------------------------------------------
	// tContent::tContentOp
	//------------------------------------------------------------------------------
	b32 tContent::tContentOp::fCreate( 
		u32 userIndex,
		const tStringPtr & rootName,
		const XCONTENT_DATA & contentData,
		u32 contentFlags )
	{
		if( !fPreExecute( ) )
			return false;

		//mContentData.DeviceID = contentData.mDeviceId;
		//mContentData.dwContentType = contentData.mContentType;
		//wcscpy_s( mContentData.szDisplayName, contentData.mDisplayName.c_str( ) );
		//memcpy( mContentData.szFileName, contentData.mFileName, XCONTENT_MAX_FILENAME_LENGTH );
		mContentData = contentData;

		DWORD result = XContentCreate( 
			userIndex, 
			rootName.fCStr( ), 
			&mContentData, 
			contentFlags, 
			NULL,					// Disposition, not filled in async op
			NULL,					// License mask, currently unused 
			fOverlapped( ) );

		if( result != ERROR_IO_PENDING )
		{
			log_warning( "XContentCreate failed with error: " << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tContent::tContentOp::fFlush( const tStringPtr & rootName )
	{
		if( !fPreExecute( ) )
			return false;

		DWORD result = XContentFlush( rootName.fCStr( ), fOverlapped( ) );

		if( result != ERROR_IO_PENDING )
		{
			log_warning( "XContentFlush failed with error: " << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32 tContent::tContentOp::fClose( const tStringPtr & rootName )
	{
		if( !fPreExecute( ) )
			return false;

		DWORD result = XContentClose( rootName.fCStr( ), fOverlapped( ) );

		if( result != ERROR_IO_PENDING )
		{
			log_warning( "XContentClose failed with error: " << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	// tContent
	//------------------------------------------------------------------------------

	const u32 tContent::cContentFlagCreateNew					= XCONTENTFLAG_CREATENEW;
	const u32 tContent::cContentFlagCreateAlways				= XCONTENTFLAG_CREATEALWAYS;
	const u32 tContent::cContentFlagOpenExisting				= XCONTENTFLAG_OPENEXISTING;
	const u32 tContent::cContentFlagOpenAlways					= XCONTENTFLAG_OPENALWAYS;
	const u32 tContent::cContentFlagTruncateExisting			= XCONTENTFLAG_TRUNCATEEXISTING;
	const u32 tContent::cContentFlagNoDeviceTransfer			= XCONTENTFLAG_NODEVICE_TRANSFER;
	const u32 tContent::cContentFlagNoProfileTransfer			= XCONTENTFLAG_NOPROFILE_TRANSFER;
	const u32 tContent::cContentFlagAllowProfileTransfer		= XCONTENTFLAG_ALLOWPROFILE_TRANSFER;
	const u32 tContent::cContentFlagStrongSigned				= XCONTENTFLAG_STRONG_SIGNED;
	const u32 tContent::cContentFlagMoveOnlyTransfer			= XCONTENTFLAG_MOVEONLY_TRANSFER;

	//------------------------------------------------------------------------------
	b32 tContent::fCreate( 
		u32 userIndex,
		const tStringPtr & rootName,
		const XCONTENT_DATA & contentData,
		u32 contentFlags )
	{
		sigassert( mState == cStateNull && "Content can only be created from Null state" );

		if( !mContentOp.fCreate( userIndex, rootName, contentData, contentFlags ) )
			return false;

		mRootName = rootName;
		mState = cStateCreating;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tContent::fFlush( )
	{
		sigassert( mState == cStateCreated && "Content can only be flushed from Created state" );

		if( !mContentOp.fFlush( mRootName ) )
			return false;

		mState = cStateFlushing;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tContent::fClose( )
	{
		switch( mState )
		{
		case cStateCreated:
			if( !mContentOp.fClose( mRootName ) )
				return false;
			mState = cStateClosing;
			return true;
		case cStateCorrupt:
		case cStateFailed:
			mState = cStateNull;
			return true;
		default:
			sigassert( "Content cannot be Closed from current state" );
		}

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tContent::fAdvance( )
	{
		// Check that we're in an operational stage
		if( mState != cStateCreating && mState != cStateFlushing && mState != cStateClosing )
			return true;

		// Query the status
		u32 result;
		// Result should be either XCONTENT_CREATED_NEW or XCONTENT_OPENED_EXISTING
		DWORD error = mContentOp.fGetResultWithError( result, false );
		if( error == ERROR_IO_INCOMPLETE )
			return false;

		// Creating
		if( mState == cStateCreating )
		{
			if( error == ERROR_SUCCESS )
			{
				mState = cStateCreated;
			}
			else if( error == ERROR_FILE_CORRUPT )
			{
				mState = cStateCorrupt;
			}
			else
			{
				log_warning( "Content creation failed with error: " << error );
				mState = cStateFailed;
			}
		}

		// Flushing
		else if( mState == cStateFlushing )
		{
			if( result != ERROR_SUCCESS )
				log_warning( "Content flush failed with error: " << result );

			mState = cStateCreated;
		}

		// Closing
		else if( mState == cStateClosing )
		{
			if( result != ERROR_SUCCESS )
				log_warning( "Content close failed with error: " << result );

			mState = cStateNull;
		}

		return true;
	}

}

#endif // defined( platform_xbox360 )
