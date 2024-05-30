#include "BasePch.hpp"
#include "tRenderTarget.hpp"


namespace Sig { namespace Gfx
{

	tRenderTargetPlatformBase::tRenderTargetPlatformBase( u32 width, u32 height, tFormat format, u32 multiSamplePower )
		: mFormat( format )
		, mWidth( width )
		, mHeight( height )
		, mMultiSamplePower( multiSamplePower )
	{
	}

}}



