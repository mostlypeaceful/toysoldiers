#ifndef __tDamageable__
#define __tDamageable__

namespace Sig { namespace Logic
{
	class tDamageable
	{
		define_dynamic_cast_base( tDamageable );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		virtual ~tDamageable( ) { }
	};
}}

namespace Sig
{
	template<class tDerived>
	inline tDerived* tLogic::fQueryDamageableDerived( )
	{
		Logic::tDamageable* comp = fQueryDamageable( );
		return comp ? comp->fDynamicCast< tDerived >( ) : 0;
	}
}

#endif//__tDamageable__
