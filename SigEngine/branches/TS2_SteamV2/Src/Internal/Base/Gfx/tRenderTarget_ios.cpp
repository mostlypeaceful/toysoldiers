#include "BasePch.hpp"
#if defined( platform_ios )
#include "tRenderTarget.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	tRenderTarget::tRenderTarget( const tDevicePtr& device, u32 width, u32 height, tFormat format, u32 multiSamplePower, u32 edramSize )
		: tRenderTargetPlatformBase( width, height, format, multiSamplePower )
	{
	}

	tRenderTarget::~tRenderTarget( )
	{
		//fReleaseComPtr( mSurface );
	}

	b32 tRenderTargetPlatformBase::fFailed( ) const
	{
		return false;//( static_cast< const tRenderTarget* >( this )->mSurface == 0 );
	}

	void tRenderTarget::fApply( const tDevicePtr& device, u32 rtIndex ) const
	{
	}

}}
#endif//#if defined( platform_ios )

