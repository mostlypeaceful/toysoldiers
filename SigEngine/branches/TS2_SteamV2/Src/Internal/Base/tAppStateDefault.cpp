#include "BasePch.hpp"
#include "tAppStateDefault.hpp"
#include "tApplication.hpp"

namespace Sig
{
	tAppStateDefault::tAppStateDefault( )
		: tApplicationState( tApplication::fInstance( ) )
	{
	}
}
