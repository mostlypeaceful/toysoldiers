#include "BasePch.hpp"
#include "tDevice.hpp"
#include "tDeviceResource.hpp"

namespace Sig { namespace Gfx
{
	u32 tDisplayModeList::fFindClosestMatch( const tDisplayMode& target ) const
	{
		u32 best = 0;
		u32 bestError = ~0;

		for( u32 i = 1; i < fCount( ); ++i )
		{
			const tDisplayMode& dm = fIndex( i );
			const u32 error = 
				fAbs( ( s32 )dm.mBackBufferWidth  - ( s32 )target.mBackBufferWidth  ) +
				fAbs( ( s32 )dm.mBackBufferHeight - ( s32 )target.mBackBufferHeight ) +
				fAbs( ( s32 )dm.mMultiSamplePower - ( s32 )target.mMultiSamplePower );
			if( error < bestError )
			{
				bestError = error;
				best = i;
			}
		}

		return best;
	}

	namespace
	{
		static tDevicePtr gDefaultDevice;
	}

	void tDevicePlatformBase::fSetDefaultDevice( const tDevicePtr& defaultDevice )
	{
		gDefaultDevice = defaultDevice;
	}

	const tDevicePtr& tDevicePlatformBase::fGetDefaultDevice( )
	{
		return gDefaultDevice;
	}

	tDevicePlatformBase::tDevicePlatformBase( )
		: mLastRenderStateInvalid( true )
		, mSingleScreenDevice( false )
		, mBorrowedDevice( false )
		, pad( false )
	{
	}

	tDevicePlatformBase::~tDevicePlatformBase( )
	{
		for( u32 i = 0; i < mDeviceResources.fCount( ); ++i )
			mDeviceResources[ i ]->mDevicePtr = 0;
	}

	void tDevicePlatformBase::fAddDeviceResource( tDeviceResource* dr )
	{
		if( static_cast<tDevice*>( this )->fRequiresDeviceReset( ) )
			mDeviceResources.fFindOrAdd( dr );
	}

	void tDevicePlatformBase::fRemoveDeviceResource( tDeviceResource* dr )
	{
		if( static_cast<tDevice*>( this )->fRequiresDeviceReset( ) )
			mDeviceResources.fFindAndEraseOrdered( dr );
	}

}}

