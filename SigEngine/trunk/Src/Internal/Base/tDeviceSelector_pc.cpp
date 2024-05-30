//------------------------------------------------------------------------------
// \file tDeviceSelector_pc.cpp - 15 Nov 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tDeviceSelector.hpp"

namespace Sig{

	//------------------------------------------------------------------------------
	// tDeviceSelector
	//------------------------------------------------------------------------------
	void tDeviceSelector::fPlatformConstruct( )
	{
		// Do any platform specific construction here
	}

	//------------------------------------------------------------------------------
	void tDeviceSelector::fPlatformDestruct( )
	{

	}

	//------------------------------------------------------------------------------
	u64 tDeviceSelector::fPlatformDeviceFreeSpace( ) const
	{
		log_warning_unimplemented( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u64 tDeviceSelector::fPlatformFullSize( u64 size ) const
	{
		log_warning_unimplemented( );
		return size;
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fPlatformShowUI( b32 force )
	{
		log_warning_unimplemented( );
		return false;
	}

	//------------------------------------------------------------------------------
	b32 tDeviceSelector::fPlatformUpdateUI( b32& success, b32 wait )
	{
		log_warning_unimplemented( );

		success = true;
		return true;
	}
}