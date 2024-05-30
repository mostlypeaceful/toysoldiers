#include "GameAppPch.hpp"
#include "tLevelEventHandler.hpp"
#include "tUnitLogic.hpp"

namespace Sig
{
	void tLevelEventHandler::tLevelEvent::fFire( tUnitLogic* unitLogic )
	{
		for( u32 i = 0; i < fCount( ); ++i )
			fIndex( i ).Execute( unitLogic->fOwnerEntity( )->fScriptLogicObject( ) );
	}

	tLevelEventHandler::tLevelEventHandler( )
	{
	}

	void tLevelEventHandler::fAddObserver( GameFlags::tLEVEL_EVENT type, Sqrat::Function func )
	{
		mEvents[ type ].fPushBack( func );
	}

	void tLevelEventHandler::fFire( GameFlags::tLEVEL_EVENT type, tUnitLogic* unitLogic )
	{
		mEvents[ type ].fFire( unitLogic );
	}
}
