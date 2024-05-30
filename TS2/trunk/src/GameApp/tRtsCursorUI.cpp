#include "GameAppPch.hpp"
#include "tRtsCursorUI.hpp"
#include "tGameApp.hpp"
#include "tUnitLogic.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateWeaponUI( "CanvasCreateRtsCursorUI" ); }

	tRtsCursorUI::tRtsCursorUI( const tResourcePtr& scriptResource )
		: tScriptedControl( scriptResource )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateWeaponUI, this );
		log_assert( !mCanvas.fIsNull( ), "Canvas couldn't be created from script: " << scriptResource->fGetPath( ) );
	}
	tRtsCursorUI::~tRtsCursorUI( )
	{
	}

	void tRtsCursorUI::fSetNoMoney( b32 noMoney, b32 forceShow, u32 cost )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetNoMoney" ).Execute( noMoney, forceShow, cost );
	}
	void tRtsCursorUI::fSetNoUpgrade( b32 noUpgrade, b32 forceShow )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetNoUpgrade" ).Execute( noUpgrade, forceShow );
	}
	void tRtsCursorUI::fSetNoRepair( b32 noRepair, b32 forceShow )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetNoRepair" ).Execute( noRepair, forceShow );
	}
	void tRtsCursorUI::fSetGhostUnit( tUnitLogic* unit )
	{
		if( unit )
			Sqrat::Function( fCanvas( ).fScriptObject( ), "ShowUnitInfo" ).Execute( unit->fPurchaseText( ) );
		else
			Sqrat::Function( fCanvas( ).fScriptObject( ), "HideUnitInfo" ).Execute( );
	}

	void tRtsCursorUI::fSetText( const tLocalizedString& text )
	{
		Sqrat::Function( fCanvas( ).fScriptObject( ), "ShowUnitInfo" ).Execute( text );
	}
	void tRtsCursorUI::fShowCapturePlatform( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ShowCapturePlatform" ).Execute( show );
	}

}}


namespace Sig { namespace Gui
{
	void tRtsCursorUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tRtsCursorUI,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		//classDesc
		//	;
		vm.fNamespace(_SC("Gui")).Bind( _SC("RtsCursorUI"), classDesc );
	}
}}



