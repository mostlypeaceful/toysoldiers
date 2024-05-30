// Ration Ticket UI for Script
#include "GameAppPch.hpp"
#include "tRationTicketUI.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{

	namespace { static const tStringPtr cCanvasCreateRationTicketUI( "CanvasCreateRationTicketUI" ); }
	
	tRationTicketUI::tRationTicketUI( )
		: tScriptedControl( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptRationTicketUI ) )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateRationTicketUI, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	void tRationTicketUI::fRationTicketProgress( u32 index, f32 progress, f32 max, tUser* user )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "RationTicketProgress" ).Execute( index, progress, max, user );
	}

	void tRationTicketUI::fAwardRationTicket( u32 index, tUser* user )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "AwardRationTicket" ).Execute( index, user );
	}

	void tRationTicketUI::fFailRationTicket( u32 index, tUser* user )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FailRationTicket" ).Execute( index, user );
	}

	void tRationTicketUI::fAwardNewRank( u32 rankIndex, tUser* user )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "AwardNewRank" ).Execute( rankIndex, user );
	}

} }

namespace Sig { namespace Gui
{

	void tRationTicketUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tRationTicketUI,Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("RationTicketUI"), classDesc );
	}

} }