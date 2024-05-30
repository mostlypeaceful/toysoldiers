//------------------------------------------------------------------------------
// \file tDeviceSelector_xbox360.cpp - 14 Nov 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tDeviceSelector.hpp"
#include "tUser.hpp"

namespace Sig{

	//------------------------------------------------------------------------------
	// tDeviceSelector
	//------------------------------------------------------------------------------
	void tDeviceSelector::fPlatformConstruct( )
	{
		fZeroOut( mOverlapped );
		mOverlapped.hEvent = INVALID_HANDLE_VALUE;
	}

	//------------------------------------------------------------------------------
	void tDeviceSelector::fPlatformDestruct( )
	{
		sigcheckfail_xoverlapped_done_else_wait_complete( &mOverlapped );
	}

	//------------------------------------------------------------------------------
	u64 tDeviceSelector::fPlatformDeviceFreeSpace( ) const
	{
		sigassert( fDeviceSelected( ) && "Sanity!" );

		XDEVICE_DATA deviceData;
		if( ERROR_SUCCESS == XContentGetDeviceData( mDeviceId, &deviceData ) )
			return deviceData.ulDeviceFreeBytes;

		return 0;
	}

	//------------------------------------------------------------------------------
	u64 tDeviceSelector::fPlatformFullSize( u64 size ) const
	{
		return XContentCalculateSize( size, 1 );
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fPlatformShowUI( b32 force )
	{
		fZeroOut( mOverlapped );
		mOverlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

		sigassert( mLocalHwIdx < tUser::cMaxLocalUsers && "Sanity!" );
		sigassert( mSelectionState != cSelectionState_Selecting && "Sanity!" );

		ULARGE_INTEGER ulSize = {0};
		ulSize.QuadPart = fPlatformFullSize( mMinFileSize );

		const DWORD flag = mForceSelection ? XCONTENTFLAG_FORCE_SHOW_UI : XCONTENTFLAG_NONE;
		const DWORD result = XShowDeviceSelectorUI( mLocalHwIdx, mContentType, flag, ulSize, &mDeviceId, &mOverlapped );

		if( result != ERROR_IO_PENDING	)
			log_warning( "XShowDeviceSelectorUI Failed with error code: " << result );
		sigassert( result == ERROR_IO_PENDING || result == ERROR_ACCESS_DENIED );

		return result == ERROR_IO_PENDING;
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fPlatformUpdateUI( b32& success, b32 wait )
	{
		DWORD error = XGetOverlappedResult( &mOverlapped, NULL, wait ? TRUE : FALSE );
		if( error == ERROR_IO_INCOMPLETE )
			return false;

		success = ( error == ERROR_SUCCESS );

#ifdef sig_logging
		if( !success )
		{
			DWORD extError = XGetOverlappedExtendedError( &mOverlapped );
			log_warning( "XShowDeviceSelectorUI overlapped failed with error: " << HRESULT_CODE( extError ) );	
		}
#endif
		return true;
	}
}
