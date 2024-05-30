#include "BasePch.hpp"
#include "tStateableEntity.hpp"

namespace Sig
{

	tStateableEntity::tStateableEntity( u16 stateMask )
		: mStateMask( stateMask )
	{
	}

	void tStateableEntity::fStateMaskEnable( u32 index )
	{
		if( fLogic( ) )
			fLogic( )->fStateMaskEnable( index, fStateMask( ) );
	}

	namespace
	{
		static void fSetStateMaskRecursiveScript( tEntity* root, u32 mask )
		{
			sigassert( root );
			sigassert( mask < std::numeric_limits<u16>::max( ) );
			tStateableEntity::fSetStateMaskRecursive( *root, mask );
		}
		static void fStateMaskEnableRecursiveScript( tEntity* root, u32 index )
		{
			sigassert( root );
			sigassert( index < std::numeric_limits<u16>::max( ) );
			tStateableEntity::fStateMaskEnableRecursive( *root, index );
		}
	}

	void tStateableEntity::fExportScriptInterface( tScriptVm& vm )
	{
		// This does not derive from tEntity because it rendered fStateMaskEnableRecursiveScript inaccessible for some unknown reason. - matt
		Sqrat::Class<tStateableEntity, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

		classDesc
			.Func(_SC("StateMaskEnable"),					&tStateableEntity::fStateMaskEnable)
			.StaticFunc(_SC("SetStateMaskRecursive"),		&fSetStateMaskRecursiveScript)
			.StaticFunc(_SC("StateMaskEnableRecursive"),	&fStateMaskEnableRecursiveScript)
			;

		vm.fRootTable( ).Bind( _SC("StateableEntity"), classDesc );
	}
}

