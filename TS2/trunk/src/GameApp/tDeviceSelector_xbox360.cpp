#include "GameAppPch.hpp"
#if defined( platform_xbox360 )
#include "tDeviceSelector.hpp"
#include "tUser.hpp"

namespace Sig{

	//------------------------------------------------------------------------------
	tDeviceSelector::tDeviceSelector( u64 minSize ) 
	: mLaunchedDeviceSelectionUI( false )
	, mSaveDeviceId( 0 )
	, mMinFileSize( minSize )
	, mTryingToShowUi( false )
	, mForceShowUi( false )
	, mSelectionJustCompleted( false )
	, mPrevStatusResult( 0 )
	, mUser( NULL )
	{
	}

	void tDeviceSelector::fSetSaveDeviceId( u8 deviceId )
	{
		// Only set valid devices
		if( deviceId != 0 && XContentGetDeviceState( deviceId, NULL ) == ERROR_SUCCESS )
			mSaveDeviceId = deviceId;
	}

	void tDeviceSelector::fTick( )
	{
		if( mTryingToShowUi )
		{
			if( mUser )
				fChooseSaveDeviceId( *mUser, mForceShowUi );
		}
		else if( mLaunchedDeviceSelectionUI )
		{
			DWORD curResult = XGetOverlappedExtendedError( &mDeviceIdXov );
			if( mPrevStatusResult == ERROR_IO_PENDING && curResult == ERROR_SUCCESS )
				mSelectionJustCompleted = true;
			else if( mPrevStatusResult == ERROR_IO_INCOMPLETE && curResult != ERROR_IO_INCOMPLETE )
				mSelectionJustCompleted = true;
			else
				mSelectionJustCompleted = false;
			mPrevStatusResult = curResult;
		}
	}

	b32 tDeviceSelector::fDeviceSelected( )
	{
		if( mLaunchedDeviceSelectionUI )
			return XGetOverlappedExtendedError( &mDeviceIdXov ) == ERROR_SUCCESS;
		else
			return fSaveDeviceId( ) != 0 ;
	}

	u32 tDeviceSelector::fSaveDeviceId( )
	{
		return mSaveDeviceId;
	}

	void tDeviceSelector::fReset( )
	{
		mSaveDeviceId = 0;
		mLaunchedDeviceSelectionUI = false;
	}

	//------------------------------------------------------------------------------
	void tDeviceSelector::fChooseSaveDeviceId( tUser& user, b32 forceShow )
	{
		if( !user.fSignedIn( ) || !user.fIsLocal( ) )
			return;

		mForceShowUi = forceShow;
		mUser = &user;

		mSaveDeviceId = 0;
		ULONGLONG saveSize = XContentCalculateSize( mMinFileSize, 1 );
		ULARGE_INTEGER ulSize = {0};
		ulSize.HighPart = (u32)(saveSize >> 32);
		ulSize.LowPart = (u32)(saveSize);

		DWORD flag = forceShow ? XCONTENTFLAG_FORCE_SHOW_UI : XCONTENTFLAG_NONE;

		mPrevStatusResult = XShowDeviceSelectorUI(
			user.fLocalHwIndex( ),
			XCONTENTTYPE_SAVEDGAME,
			flag,
			ulSize,
			&mSaveDeviceId,
			&mDeviceIdXov
			);

		if( mPrevStatusResult != ERROR_IO_PENDING	)
		{
			const DWORD lastError = GetLastError( );
			log_warning( 0, "XShowDeviceSelectorUI Failed with error code: " << mPrevStatusResult << ", GetLastError(): " << lastError );
		}
		sigassert( mPrevStatusResult == ERROR_IO_PENDING || mPrevStatusResult == ERROR_ACCESS_DENIED );

		if( mPrevStatusResult == ERROR_ACCESS_DENIED )
			mTryingToShowUi = true;
		else
		{
			mTryingToShowUi = false;
			mLaunchedDeviceSelectionUI = true;
		}
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fNeedsToChooseSaveDevice( u8 curDeviceID )
	{
		XDEVICE_DATA deviceData;
		// Check the user's current setting for their save device ID
		// if it exists then we don't need to ask for a new one
		if( curDeviceID != 0 && XContentGetDeviceData( curDeviceID , &deviceData ) == ERROR_SUCCESS )
		{
			// It exists, so check available space
			ULONGLONG saveSize = XContentCalculateSize( mMinFileSize, 1 );
			XContentGetDeviceData( curDeviceID, &deviceData );
			if( deviceData.ulDeviceFreeBytes < saveSize )
			{
				// Not enough space! Choose a new device!
				return true;
			}

			// Use the deviceId passed in ( loaded from user's profile )
			mSaveDeviceId = curDeviceID;
			//mLaunchedDeviceSelectionUI = true; 
			return false;
		}

		return !mLaunchedDeviceSelectionUI && !fDeviceSelected( );

		XCONTENTDEVICEID testDeviceId = curDeviceID;
	}

	b32 tDeviceSelector::fDeviceHasEnoughSpace( ULONGLONG size )
	{
		// There is no device 0
		if( mSaveDeviceId == 0 )
			return false;

		XDEVICE_DATA deviceData;

		ULONGLONG saveSize = XContentCalculateSize( size, 1 );

		if( XContentGetDeviceData( mSaveDeviceId , &deviceData ) == ERROR_SUCCESS )
			if( deviceData.ulDeviceFreeBytes > saveSize )
				return true;
		return false;
	}

	b32 tDeviceSelector::fDeviceIsAvailable( )
	{
		// There is no device 0
		if( mSaveDeviceId == 0 )
			return false;

		return XContentGetDeviceState( mSaveDeviceId, NULL ) == ERROR_SUCCESS;
	}
}

#endif//#if defined( platform_xbox360 )