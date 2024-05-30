#include "BasePch.hpp"
#include "tDamageable.hpp"

namespace Sig { namespace Logic
{
	void tDamageable::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tDamageable, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

		vm.fRootTable( ).Bind(_SC("Damageable"), classDesc);
	}
}}
