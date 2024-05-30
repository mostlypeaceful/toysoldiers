#include "GameAppPch.hpp"
#include "tScoreUI.hpp"
#include "tBarrage.hpp"
#include "tSync.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateScoreUI( "CanvasCreateScoreUI" ); }

	tScoreUI::tScoreUI( const tResourcePtr& scriptResource, const tPlayerPtr& player )
		: tScriptedControl( scriptResource )
		, mPlayer( player )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateScoreUI, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	tScoreUI::~tScoreUI( )
	{
	}

	void tScoreUI::fSetScore( s32 count, f32 percent )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetScore" ).Execute( count, percent );
	}

	void tScoreUI::fSetMoney( u32 money )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetMoney" ).Execute( money );
	}

	void tScoreUI::fShow( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( show );
	}

	void tScoreUI::fProtectToyBox( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ProtectToyBox" ).Execute( );
	}

	void tScoreUI::Test( u32 action, u32 button, tPlayer* player, b32 held )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Test" ).Execute( action, button, player, held );
	}

	void tScoreUI::fShowMoney( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ShowMoney" ).Execute( show );
	}

	void tScoreUI::fShowTickets( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ShowTickets" ).Execute( show );
	}
}}


namespace Sig { namespace Gui
{
	void tScoreUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tScoreUI,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"), &tScoreUI::fUser)
			.Prop(_SC("Player"), &tScoreUI::fPlayer)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("ScoreUI"), classDesc );
	}

}}

