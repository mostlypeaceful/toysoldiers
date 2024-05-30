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
	//------------------------------------------------------------------------------
	// tContentEnumerator
	//------------------------------------------------------------------------------
	b32 tContentEnumerator::fCreate( u32 userIndex, u32 deviceId, u32 contentType, b32 userSpecificOnly, u32 numItems )
	{
		sigassert( mState == cStateNull && "Can only create from Null state" );

		HANDLE enumHandle;
		DWORD bufferSize;
		DWORD result = XContentCreateEnumerator( 
			userIndex, 
			deviceId, 
			contentType, 
			userSpecificOnly ? XCONTENTFLAG_ENUM_EXCLUDECOMMON : 0, 
			numItems, 
			&bufferSize, 
			&enumHandle );


		if( result != ERROR_SUCCESS )
		{
			log_warning( "XContentCreateEnumerator failed with error: " << result );
			return false;
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
	void tContentEnumerator::fResult( u32 idx, tContentData & data, XCONTENT_DATA& dataOut )
	{
		sigassert( mState == cStateSuccess && "Results only available from StateSuccess" );
		sigassert( idx < fResultCount( ) );

		const XCONTENT_DATA * results = mEnumOp.fResults<XCONTENT_DATA>( );

		data.mDeviceId = results[ idx ].DeviceID;
		data.mContentType = results[ idx ].dwContentType;
		data.mDisplayName.fFromCStr( results[ idx ].szDisplayName );
		memcpy( data.mFileName, results[ idx ].szFileName, XCONTENT_MAX_FILENAME_LENGTH );
		
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
