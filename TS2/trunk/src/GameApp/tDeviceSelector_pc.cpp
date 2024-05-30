#include "GameAppPch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tDeviceSelector.hpp"

namespace Sig{

	//------------------------------------------------------------------------------
	tDeviceSelector::tDeviceSelector( u64 minSize ) :
	mLaunchedDeviceSelectionUI( false )
	, mMinFileSize( minSize )
	, mTryingToShowUi( false )
	, mForceShowUi( false )
	, mSelectionJustCompleted( false )
	, mPrevStatusResult( 0 )
	, mUser( NULL )
	{
	}

	void tDeviceSelector::fTick( )
	{
	}

	void tDeviceSelector::fSetSaveDeviceId( u8 deviceId )
	{
	}

	u32 tDeviceSelector::fSaveDeviceId( )
	{
		return 0;
	}

	b32 tDeviceSelector::fDeviceSelected( )
	{
		return false;
	}

	void tDeviceSelector::fReset( ) 
	{
	}

	//------------------------------------------------------------------------------
	void tDeviceSelector::fChooseSaveDeviceId( tUser& user, b32 forceShow )
	{
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fNeedsToChooseSaveDevice( u8 curDeviceID )
	{
		return false;
	}

	//------------------------------------------------------------------------------
	// Hacky way to determine if there is more than one device
	// Default DeviceID is 1, so try to get device data from IDs 2-20
	b32 tDeviceSelector::fMoreThanOneDevice( )
	{
		return false;
	}

	b32 tDeviceSelector::fDeviceHasEnoughSpace( u64 size )
	{
		return false;
	}

	b32 tDeviceSelector::fDeviceIsAvailable( )
	{
		return false;
	}
}

#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )