#include "BasePch.hpp"
#include "tDeviceResource.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	tDeviceResource::tDeviceResource( )
		: mDevicePtr( 0 )
	{
	}

	tDeviceResource::~tDeviceResource( )
	{
		fUnregisterWithDevice( );
	}

	void tDeviceResource::fRegisterWithDevice( tDevice* device )
	{
		fUnregisterWithDevice( );
		mDevicePtr = device;
		device->fAddDeviceResource( this );
	}

	void tDeviceResource::fUnregisterWithDevice( )
	{
		if( mDevicePtr )
		{
			mDevicePtr->fRemoveDeviceResource( this );
			mDevicePtr = 0;
		}
	}

}}

