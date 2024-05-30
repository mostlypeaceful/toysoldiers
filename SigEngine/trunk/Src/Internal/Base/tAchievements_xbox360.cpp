//------------------------------------------------------------------------------
// \file tAchievements_xbox360.cpp - 20 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined ( platform_xbox360 )
#include "tAchievements.hpp"

namespace Sig
{
	const u32 tAchievementsReader::cDetailsLabel = XACHIEVEMENT_DETAILS_LABEL;
	const u32 tAchievementsReader::cDetailsDescription = XACHIEVEMENT_DETAILS_DESCRIPTION;
	const u32 tAchievementsReader::cDetailsUnachieved = XACHIEVEMENT_DETAILS_UNACHIEVED;

	//------------------------------------------------------------------------------
	// tAchievementsWriter::tWriteData
	//------------------------------------------------------------------------------
	tAchievementsWriter::tWriteData::tWriteData( )
	{
		fZeroOut( mOverlapped );
	}

	//------------------------------------------------------------------------------
	tAchievementsWriter::tWriteData::~tWriteData( )
	{
		sigcheckfail_xoverlapped_done_else_wait_complete( &mOverlapped );
	}
	
	//------------------------------------------------------------------------------
	b32 tAchievementsWriter::tWriteData::fOverlapComplete( b32 & success )
	{
		if( !XHasOverlappedIoCompleted( &mOverlapped ) )
			return false;

		DWORD error = XGetOverlappedExtendedError( &mOverlapped );
		success = SUCCEEDED( error );
		fZeroOut( mOverlapped );
		return true;
	}
	
	//------------------------------------------------------------------------------
	// tAchievementsWriter
	//------------------------------------------------------------------------------
	tAchievementsWriter::tAchievementsWriter( 
		u32 localUserIndex, u32 count, const u32 achievements[] )
		: mState( cStateNull )
	{
		mToWrite.fNewArray( sizeof( XUSER_ACHIEVEMENT ) * count );
		
		XUSER_ACHIEVEMENT * ua = ( XUSER_ACHIEVEMENT * ) mToWrite.fBegin( );
		for( u32 a = 0; a < count; ++a )
		{
			ua[ a ].dwUserIndex = localUserIndex;
			ua[ a ].dwAchievementId = achievements[ a ];
		}
	}

	//------------------------------------------------------------------------------
	tAchievementsWriter::~tAchievementsWriter( )
	{
		fFinish( true );
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::fBegin( )
	{
		DWORD error = XUserWriteAchievements( 
			mToWrite.fCount( ) / sizeof( XUSER_ACHIEVEMENT ), 
			( XUSER_ACHIEVEMENT * )mToWrite.fBegin( ), 
			&mWriteData.mOverlapped );

		if( error != ERROR_IO_PENDING && error != ERROR_SUCCESS )
		{
			mState = cStateFail;
			log_warning( __FUNCTION__ << " failed with error: " << error );
			return;
		}

		mState = cStateWriting;
		tSaveInstance::fBegin( ); // call the base
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsWriter::fFinished( )
	{
		if( mState != cStateWriting )
			return true;

		b32 success;
		if( !mWriteData.fOverlapComplete( success ) )
			return false;
		
		mState = success ? cStateSuccess : cStateFail;
		return true;
	}

	//------------------------------------------------------------------------------
	void tAchievementsWriter::fFinish( b32 wait )
	{
		if( mState != cStateWriting )
			return;

		if( wait )
		{
			DWORD ignored;
			DWORD error = XGetOverlappedResult( &mWriteData.mOverlapped, &ignored, true );
			if( error != ERROR_SUCCESS )
			{
				mState = cStateFail;
				log_warning( __FUNCTION__ << " failed with GetOverlappedResult: " << error );
				return;
			}

			error = XGetOverlappedExtendedError( &mWriteData.mOverlapped );
			fZeroOut( mWriteData.mOverlapped );
			
			mState = SUCCEEDED( error ) ? cStateSuccess : cStateFail;
		}
		else
			fFinished( ); // This will update and handle the overlap and state
	}

	//------------------------------------------------------------------------------
	// tAchievementsReader::tReadData
	//------------------------------------------------------------------------------
	tAchievementsReader::tReadData::tReadData( )
		: mEnumerator( INVALID_HANDLE_VALUE )
	{
		fZeroOut( mOverlapped );
	}

	//------------------------------------------------------------------------------
	tAchievementsReader::tReadData::~tReadData( )
	{
		sigcheckfail_xoverlapped_done_else_wait_cancel( &mOverlapped );
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::tReadData::fCloseEnumerator( )
	{
		if( mEnumerator != INVALID_HANDLE_VALUE )
		{
			XCloseHandle( mEnumerator );
			mEnumerator = INVALID_HANDLE_VALUE;
		}
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsReader::tReadData::fOverlapComplete( b32 & success, u32 & count )
	{
		DWORD countInternal = 0;
		DWORD testResult = XGetOverlappedResult( &mOverlapped, &countInternal, FALSE );
		if( testResult == ERROR_IO_INCOMPLETE )
			return false;

		DWORD extendedError = XGetOverlappedExtendedError( &mOverlapped );

		success = ( testResult == ERROR_SUCCESS || HRESULT_CODE( extendedError ) == ERROR_NO_MORE_FILES );
		count = countInternal;

		fZeroOut( mOverlapped );
		return true;
	}


	//------------------------------------------------------------------------------
	// tAchievementsReader
	//------------------------------------------------------------------------------
	void tAchievementsReader::fRead( u32 startIdx, u32 toRead, u32 details, tPlatformUserId target )
	{
		sigassert( mState != cStateReading && "Cancel any pending reads before starting a new one" );
		sigassert( mUserIndex < tUser::cMaxLocalUsers );

		DWORD bufferSize;
		DWORD result = XUserCreateAchievementEnumerator( 
			0, // this title
			mUserIndex, 
			target, // invalid user queries achievements for userIndex
			XACHIEVEMENT_DETAILS_TFC | details, 
			startIdx, 
			toRead, 
			&bufferSize, 
			&mReadData.mEnumerator );

		// Did we succeed?
		if( result != ERROR_SUCCESS )
		{
			log_warning( "Error " << std::hex << result << " creating achievements enumerator" );
			mReadData.fCloseEnumerator( );
			return;
		}

		// Allocate our buffer
		if( mReadData.mBuffer.fCount( ) < bufferSize )
			mReadData.mBuffer.fNewArray( bufferSize );

		// Enumerate results
		result = XEnumerate( 
			mReadData.mEnumerator, 
			mReadData.mBuffer.fBegin( ), 
			mReadData.mBuffer.fCount( ), 
			NULL, 
			&mReadData.mOverlapped );

		// Success?
		if( result != ERROR_IO_PENDING && result != ERROR_SUCCESS )
		{
			log_warning( "Error " << std::hex << result << " enumerating achievements" );
			mReadData.fCloseEnumerator( );
			return;
		}

		mState = cStateReading;
	}

	//------------------------------------------------------------------------------
	void tAchievementsReader::fCancelRead( )
	{
		if( mState != cStateReading )
			return;

		XCancelOverlapped( &mReadData.mOverlapped );
		
		mReadData.fCloseEnumerator( );
		fZeroOut( mReadData.mOverlapped );
		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	b32	tAchievementsReader::fAdvanceRead( )
	{
		if( mState == cStateReading )
		{
			b32 success;
			if( !mReadData.fOverlapComplete( success, mReadCount ) )
				return false;

			mReadData.fCloseEnumerator( );
			mState = success ? cStateSuccess : cStateFailure;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	b32	tAchievementsReader::fIsAwarded( u32 id )
	{
		if( mState != cStateSuccess )
		{
			log_warning( __FUNCTION__ << " called while not in Success state" );
			return false;
		}

		const XACHIEVEMENT_DETAILS * deets = ( const XACHIEVEMENT_DETAILS * )mReadData.mBuffer.fBegin( );
		for( u32 u = 0; u < mReadCount; ++u )
		{
			if( deets[ u ].dwId != id )
				continue;

			return AchievementEarned( deets[ u ].dwFlags );
		}

		log_warning( __FUNCTION__ << ": Achievement (" << id << ") could not be found" );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tAchievementsReader::fGetData( u32 id, tAchievementData& out )
	{
		if( mState != cStateSuccess )
		{
			log_warning( __FUNCTION__ << " called while not in Success state" );
			return false;
		}

		const XACHIEVEMENT_DETAILS * deets = ( const XACHIEVEMENT_DETAILS * )mReadData.mBuffer.fBegin( );
		for( u32 u = 0; u < mReadCount; ++u )
		{
			if( deets[ u ].dwId != id )
				continue;

			tAchievementData data;
			out.mLabel.fFromCStr( deets[ u ].pwszLabel );
			out.mDescription.fFromCStr( deets[ u ].pwszDescription );
			out.mImageId = deets[ u ].dwImageId;
			return true;
		}

		log_warning( __FUNCTION__ << ": Achievement (" << id << ") could not be found" );
		return false;
	}
}
#endif//#if defined ( platform_xbox360 )

