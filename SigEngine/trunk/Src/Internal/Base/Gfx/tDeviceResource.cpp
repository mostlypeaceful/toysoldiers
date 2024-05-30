#include "BasePch.hpp"
#include "tDeviceResource.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	tGrowableArray< std::string > tDeviceResource::gLogs;

	void tDeviceResource::fLogAlloc( std::string tag, void* addr )
	{
		std::stringstream str;
		str << tag << " " << addr;
		Gfx::tDeviceResource::gLogs.fPushBack( str.str() );
	}

	void tDeviceResource::fRemAlloc( std::string tag, void* addr )
	{
		std::stringstream str;
		str << tag << " " << addr;
		if( !Gfx::tDeviceResource::gLogs.fFindAndErase( str.str() ) )
		{
			log_warning( "didn't find this one " << str.str() );
		}
	}

	void tDeviceResource::fDumpLogs( )
	{
		for( u32 i = 0; i < gLogs.fCount(); ++i )
			log_warning( gLogs[i] );
	}

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

