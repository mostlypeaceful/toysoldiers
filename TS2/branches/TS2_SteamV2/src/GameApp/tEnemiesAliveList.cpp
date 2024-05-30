#include "GameAppPch.hpp"
#include "tEnemiesAliveList.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateEnemiesAliveList( "CanvasCreateEnemiesAliveList" ); }

	tEnemiesAliveList::tEnemiesAliveList( const tResourcePtr& scriptResource, const tUserPtr& user, u32 enemyCountry )
		: tScriptedControl( scriptResource )
		, mUser( user )
		, mEnemyCountry( enemyCountry )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateEnemiesAliveList, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	tEnemiesAliveList::~tEnemiesAliveList( )
	{
	}

	void tEnemiesAliveList::fShow( b32 show )
	{
		if( show )
			Sqrat::Function( mCanvas.fScriptObject( ), "FadeIn" ).Execute( );
		else
			Sqrat::Function( mCanvas.fScriptObject( ), "FadeOut" ).Execute( );
	}

	void tEnemiesAliveList::fSetCount( u32 unitID, u32 country, u32 count )
	{
		if( country != mEnemyCountry )
			return;

		Sqrat::Function( mCanvas.fScriptObject( ), "SetCount" ).Execute( unitID, country, count );
	}

}}


namespace Sig { namespace Gui
{
	void tEnemiesAliveList::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tEnemiesAliveList,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop( _SC( "User" ), &tEnemiesAliveList::fUser )
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("EnemiesAliveList"), classDesc );
	}
}}

