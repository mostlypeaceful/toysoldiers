#include "BasePch.hpp"
#if defined( platform_ios )
#include "tVertexFormatVRam.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	void tVertexFormatVRam::fAllocateInternal( const tDevicePtr& device )
	{
		mPlatformHandle = ( tPlatformHandle )0;
	}

	void tVertexFormatVRam::fDeallocateInternal( )
	{
		if( mPlatformHandle )
		{
//			fGlobalUnregister( this );
			mPlatformHandle = 0;
		}
	}

	void tVertexFormatVRam::fApply( const tDevicePtr& device ) const
	{
	}

}}
#endif//#if defined( platform_ios )

