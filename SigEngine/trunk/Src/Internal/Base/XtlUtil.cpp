#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "XtlUtil.hpp"

namespace Sig { namespace XtlUtil
{
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
		if( !fIsComplete( ) )
		{
			// This is apparently pretty common / relied upon -_-;;
			log_warning( "tOverlappedOp destroyed while XOVERLAPPED IO still pending.  Blocking in lieu of corruption / crashing!" );
			fWaitToComplete( );
		}
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
	b32 tOverlappedOp::fGetResult( u32 & result, b32 wait )
	{
		DWORD resultInternal;
		DWORD error = XGetOverlappedResult( &mOverlapped, &resultInternal, wait ? TRUE : FALSE );
		if( error == ERROR_IO_INCOMPLETE )
			return false;

		if( error != ERROR_SUCCESS )
		{

			DWORD otherError = XGetOverlappedExtendedError( &mOverlapped );
			log_warning( "XGetOverlappedResult failed with error: " << HRESULT_CODE( otherError ) );
			return false;
		}

		result = resultInternal;
		return true;
	}
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
				log_warning( "XGetOverlappedResult failed with error: " << otherError );
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
			log_warning( "XCancelOverlapped failed with error: " << error );

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

		DWORD result = XEnumerate( 
			mEnumeratorHandle, 
			mResultBuffer.fBegin( ), 
			mResultBuffer.fCount( ), 
			NULL,						// Overlapped enumerate doesn't allow result count output
			fOverlapped( ) );

		return true;

		if( result != ERROR_IO_PENDING )
		{
			log_warning( "XEnumerate called failed with error: " << result );
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tEnumerateOp::fReset( )
	{
		tOverlappedOp::fReset( );

		if( mEnumeratorHandle != INVALID_HANDLE_VALUE )
		{
			XCloseHandle( mEnumeratorHandle );
			mEnumeratorHandle = INVALID_HANDLE_VALUE;
		}

		mResultBuffer.fSetCount( 0 );
	}

	//------------------------------------------------------------------------------
	// tContentDeleteOp
	//------------------------------------------------------------------------------
	tContentDeleteOp::tContentDeleteOp( )
		: mUserIdx( ~0 )
	{
		fMemSet( &mContentData, 0, sizeof( mContentData ) );
	}

	//------------------------------------------------------------------------------
	tContentDeleteOp::~tContentDeleteOp( )
	{
		fWaitToComplete( );
		fReset( );
	}

	//------------------------------------------------------------------------------
	void tContentDeleteOp::fInitialize( u32 userIdx, const XCONTENT_DATA& contentData )
	{
		fReset( );

		mUserIdx = userIdx;
		mContentData = contentData;
	}

	//------------------------------------------------------------------------------
	b32 tContentDeleteOp::fBegin( )
	{
		if( !fPreExecute( ) )
			return false;

		DWORD result = XContentDelete( 
			mUserIdx,
			&mContentData,
			fOverlapped( ) );

		if( result != ERROR_IO_PENDING )
		{
			log_warning( "XContentDelete failed with error: " << result );
			return false;
		}

		return true;
	}

	void fExportScriptInterface( tScriptVm& vm )
	{
		// Nothing to export currently
	}
}}
#endif//#if defined( platform_xbox360 )
