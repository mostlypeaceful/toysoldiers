#include "GameAppPch.hpp"
#include "tVersusWaveList.hpp"

namespace Sig { namespace Gui
{

	tVersusWaveList::tVersusWaveList( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tSinglePlayerWaveList( scriptResource, user, tStringPtr::cNullPtr )
	{
	}

	tVersusWaveList::~tVersusWaveList( )
	{
	}

	void tVersusWaveList::fFinalEnemyWave( ) 
	{ 
		fClear( ); 
	}

} }

namespace Sig { namespace Gui
{
	void tVersusWaveList::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tVersusWaveList,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		//classDesc
		//	;
		vm.fNamespace(_SC("Gui")).Bind( _SC("VersusWaveList"), classDesc );
	}
} }