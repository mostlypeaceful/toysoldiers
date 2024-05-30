#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "XtlUtil.hpp"

namespace Sig { namespace XtlUtil
{
	devvar( b32, Debug_ContentEnumerator_SpamEnumeration, true );

	bool fCreateDirectory( const char* path )
	{
		return CreateDirectory( path, 0 )!=0;
	}

	HANDLE fCreateReadOnlyFileHandle( const char* path, b32 directory )
	{
		return CreateFile( 
			path, 
			0/*GENERIC_READ*/, 
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING | ( directory ? FILE_FLAG_BACKUP_SEMANTICS : 0 ),
			0 );
	}

	//------------------------------------------------------------------------------
	// tOverlappedOp
	//------------------------------------------------------------------------------
	tOverlappedOp::tOverlappedOp( )
	{
		fZeroOut( mOverlapped );
		mOverlapped.hEvent = INVALID_HANDLE_VALUE;
	}

	//------------------------------------------------------------------------------
	tOverlappedOp::~tOverlappedOp( )
	{
		// Wait on the operation if need be
		fWaitToComplete( );
		fReset( );
	}

	//------------------------------------------------------------------------------
	b32 tOverlappedOp::fIsComplete( ) const
	{
		return XHasOverlappedIoCompleted( &mOverlapped );
	}

	//------------------------------------------------------------------------------
	void tOverlappedOp::fWaitToComplete( )
	{
		if( !fIsComplete( ) )
		{
			u32 ignored;
			fGetResult( ignored, true );
		}
	}

	//------------------------------------------------------------------------------
	u32 tOverlappedOp::fGetResult( u32 & result, b32 wait )
	{
		DWORD resultInternal;
		DWORD error = XGetOverlappedResult( &mOverlapped, &resultInternal, wait ? TRUE : FALSE );
		if( error == ERROR_IO_INCOMPLETE )
			return ERROR_IO_INCOMPLETE;

		if( error != ERROR_SUCCESS )
		{
			error = HRESULT_CODE( XGetOverlappedExtendedError( &mOverlapped ) );
			log_warning( 0, "XGetOverlappedResult failed with error: " << error );
		}

		result = resultInternal;
		return error;
	}

	//------------------------------------------------------------------------------
	b32 tOverlappedOp::fGetResultNoMoreFilesOk( u32 & result, b32 wait )
	{
		DWORD resultInternal;
		DWORD error = XGetOverlappedResult( &mOverlapped, &resultInternal, wait ? TRUE : FALSE );
		if( error == ERROR_IO_INCOMPLETE )
			return false;

		if( error != ERROR_SUCCESS )
		{

			DWORD otherError = HRESULT_CODE( XGetOverlappedExtendedError( &mOverlapped ) );
			if( otherError != ERROR_NO_MORE_FILES )
			{
				log_warning( 0, "XGetOverlappedResult failed with error: " << otherError );
				return false;
			}
		}

		result = resultInternal;
		return true;
	}

	//------------------------------------------------------------------------------
	DWORD tOverlappedOp::fGetResultWithError( u32 & result, b32 wait )
	{
		DWORD resultInternal;
		DWORD error = XGetOverlappedResult( &mOverlapped, &resultInternal, wait ? TRUE : FALSE );
		result = resultInternal;
		return error;
	}

	//------------------------------------------------------------------------------
	b32 tOverlappedOp::fPreExecute( )
	{
		if( !fIsComplete( ) )
			return false;

		fReset( );

		mOverlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

		return true;
	}

	//------------------------------------------------------------------------------
	void tOverlappedOp::fCancel( )
	{
		DWORD error = XCancelOverlapped( &mOverlapped );
		if( error != ERROR_SUCCESS )
			log_warning( 0, "XCancelOverlapped failed with error: " << error );

		fReset( );
	}

	//------------------------------------------------------------------------------
	void tOverlappedOp::fReset( )
	{
		sigassert( fIsComplete( ) );
		
		if( mOverlapped.hEvent != INVALID_HANDLE_VALUE )
			XCloseHandle( mOverlapped.hEvent );

		fZeroOut( mOverlapped );
		mOverlapped.hEvent = INVALID_HANDLE_VALUE;
	}


	//------------------------------------------------------------------------------
	// tEnumerateOp
	//------------------------------------------------------------------------------
	tEnumerateOp::tEnumerateOp( )
		: mEnumeratorHandle( INVALID_HANDLE_VALUE )
		, mResultCount(0)
	{

	}

	//------------------------------------------------------------------------------
	tEnumerateOp::~tEnumerateOp( )
	{
		fWaitToComplete( );
		fReset( );
	}

	//------------------------------------------------------------------------------
	void tEnumerateOp::fInitialize( u32 bufferSize, HANDLE enumeratorHandle )
	{
		fReset( );
		
		mResultBuffer.fSetCount( bufferSize );
		mEnumeratorHandle = enumeratorHandle;
	}

	//------------------------------------------------------------------------------
	b32 tEnumerateOp::fBegin( )
	{
		if( !fPreExecute( ) )
			return false;

		if( mEnumeratorHandle == INVALID_HANDLE_VALUE )
			return false;

#if defined( XUTIL_ENUMERATE_BLOCKED )

		const u32 cMaxResultBufferCount = mResultBuffer.fCount() / sizeof(XCONTENT_CROSS_TITLE_DATA);
		log_line( 0, "mResultBuffer.fCount() == " << mResultBuffer.fCount() << " (" << cMaxResultBufferCount << " contents max)" );

		sigassert( mResultCount == 0 );

		for (;mResultCount<cMaxResultBufferCount;)
		{
			DWORD result = XEnumerateCrossTitle( 
				mEnumeratorHandle, 
				reinterpret_cast<XCONTENT_CROSS_TITLE_DATA*>(mResultBuffer.fBegin( )) + mResultCount,
				sizeof(XCONTENT_CROSS_TITLE_DATA), // maximum 1 result at a time, unlike XEnumerate.
				NULL,
				NULL );

			if ( Debug_ContentEnumerator_SpamEnumeration )
			{
				log_line( 0, "Called XEnumerateCrossTitle for index " << mResultCount << ", result == " << result );
			}

			switch ( result )
			{
			case ERROR_SUCCESS:
				++mResultCount;
				break;
			case ERROR_NO_MORE_FILES:
				return true;
			default:
				log_warning( 0, "XEnumerateCrossTitle called failed with error: " << result );
				return false;
			}
		}
		if ( mResultCount >= mResultBuffer.fCount( ) )
		{
			log_warning( 0, "mResultCount >= mResultBuffer.fCount( ), you may need to bump cMaxPackages" );
		}
		return true;
#else // !defined( XUTIL_ENUMERATE_BLOCKED )
#error This is just broken.  Why?  Because XEnumerateCrossTitle, unlike XEnumerate, will only return 1 result at a time.

		DWORD result = XEnumerateCrossTitle( 
			mEnumeratorHandle, 
			mResultBuffer.fBegin( ), 
			mResultBuffer.fCount( ), 
			NULL,						// Overlapped enumerate doesn't allow result count output
			fOverlapped( ) );

		return true;

		if( result != ERROR_IO_PENDING )
		{
			log_warning( 0, "XEnumerate called failed with error: " << result );
			return false;
		}

		return true;
#endif
	}

	//------------------------------------------------------------------------------
	void tEnumerateOp::fReset( )
	{
#if !defined( XUTIL_ENUMERATE_BLOCKED )
		tOverlappedOp::fReset( );
#endif

		if( mEnumeratorHandle != INVALID_HANDLE_VALUE )
		{
			XCloseHandle( mEnumeratorHandle );
			mEnumeratorHandle = INVALID_HANDLE_VALUE;
		}

		mResultCount = 0;
		mResultBuffer.fSetCount( 0 );
	}
}}
#endif//#if defined( platform_xbox360 )
