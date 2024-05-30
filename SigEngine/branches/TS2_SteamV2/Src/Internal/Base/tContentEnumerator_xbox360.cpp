//------------------------------------------------------------------------------
// \file tContentEnumerator_xbox360.cpp - 23 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tContentEnumerator.hpp"

namespace Sig
{
	devvar( b32, Debug_ContentEnumerator_SpamResultData, true );

	//------------------------------------------------------------------------------
	// tContentEnumerator
	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fCreate( u32 userIndex, u32 deviceId, u32 contentType, b32 userSpecificOnly, u32 numItems )
	{
		sigassert( mState == cStateNull && "Can only create from Null state" );

		HANDLE enumHandle;
		DWORD bufferSize;
		DWORD result = XContentCreateCrossTitleEnumerator( 
			userIndex, 
			deviceId, 
			contentType, 
			userSpecificOnly ? XCONTENTFLAG_ENUM_EXCLUDECOMMON : 0, 
			1,  // "XEnumerateCrossTitle always returns exactly one item, so the value of this argument should always be "1"." (XDK Docs)
			NULL, // "Since XEnumerateCrossTitle always returns exactly one XCONTENT_CROSS_TITLE_DATA, it is not necessary to retrieve a size from this function. Simply pass NULL for this parameter." (XDK Docs)
			&enumHandle );

		bufferSize = numItems * sizeof(XCONTENT_CROSS_TITLE_DATA);


		if( result != ERROR_SUCCESS )
		{
			log_warning( 0, "XContentCreateCrossTitleEnumerator failed with error: " << result );
			return false;
		}
		else if ( Debug_ContentEnumerator_SpamResultData )
		{
			log_line( 0, "XContentCreateCrossTitleEnumerator succeeded with handle: " << enumHandle );
		}

		mEnumOp.fInitialize( bufferSize, enumHandle );
		mState = cStateCreated;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fEnumerate( )
	{
		sigassert( mState == cStateCreated && "Can only begin enumeration from created state" );

		if( !mEnumOp.fBegin( ) )
			return false;

		mState = cStateEnumerating;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fAdvance( )
	{
		sigassert( mState != cStateNull && "Cannot advance Null ContentEnumerator" );

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

	//------------------------------------------------------------------------------
	void tContentEnumerator::fWait( )
	{
		if( mState == cStateEnumerating )
		{
			mEnumOp.fWaitToComplete( );
			fAdvance( );
		}
	}

	//------------------------------------------------------------------------------
	u32 tContentEnumerator::fResultCount( )
	{
		sigassert( mState == cStateSuccess && "Result count only available from StateSuccess" );

		return mEnumOp.fResultCount( );
	}

	//------------------------------------------------------------------------------
	void tContentEnumerator::fResult( u32 idx, tContentData & data, XCONTENT_CROSS_TITLE_DATA& dataOut )
	{
		sigassert( mState == cStateSuccess && "Results only available from StateSuccess" );
		sigassert( idx < fResultCount( ) );

		const XCONTENT_CROSS_TITLE_DATA * results = mEnumOp.fResults<XCONTENT_CROSS_TITLE_DATA>( );

		data.mDeviceId = results[ idx ].DeviceID;
		data.mContentType = results[ idx ].dwContentType;
		data.mDisplayName.fFromCStr( results[ idx ].szDisplayName );
		memcpy( data.mFileName, results[ idx ].szFileName, XCONTENT_MAX_FILENAME_LENGTH );

		if ( Debug_ContentEnumerator_SpamResultData)
		{
			log_line( 0, "tContentEnumerator result @ "<<idx);
			log_line( 0, "  mDeviceId:    " << data.mDeviceId );
			log_line( 0, "  mDisplayName: " << data.mDisplayName );
			log_line( 0, "  mFileName:    " << data.mFileName );

			if ( data.mContentType == tContentData::cContentTypeMarketPlace )
			{
				log_line( 0, "  mContentType: Market Place" );
			}
			else if ( data.mContentType == tContentData::cContentTypePublisher )
			{
				log_line( 0, "  mContentType: Publisher" );
			}
			else if ( data.mContentType == tContentData::cContentTypeSavedGame )
			{
				log_line( 0, "  mContentType: Saved Game" );
			}
			else
			{
				log_line( 0, "  mContentType: ??? (" << data.mContentType << ")" );
			}
		}
		
		dataOut = results[ idx ];
	}

	//------------------------------------------------------------------------------
	void tContentEnumerator::fDestroy( )
	{
		mEnumOp.fCancel( );
		mEnumOp.fReset( );
		mState = cStateNull;
	}
}

#endif // defined( platform_xbox360 )
