#include "BasePch.hpp"
#if defined( platform_msft )

namespace Sig
{
	void fSleep( u32 numMs )
	{
		Sleep( numMs );
	}
}

#endif
