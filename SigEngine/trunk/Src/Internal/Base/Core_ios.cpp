#include "BasePch.hpp"
#if defined( platform_ios )

namespace Sig
{
	void fSleep( u32 numMs )
	{
		sleep( numMs );
	}
}

#endif//#if defined( platform_ios )
