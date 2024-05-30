#include "BasePch.hpp"
#if defined( platform_msft )

namespace Sig
{
	void fSleep( u32 numMs )
	{
#if !defined( platform_metro )
		Sleep( numMs );
#endif
	}
}

#endif // defined( platform_msft )
