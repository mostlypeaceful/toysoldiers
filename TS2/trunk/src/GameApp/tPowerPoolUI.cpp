#include "GameAppPch.hpp"
#include "tPowerPoolUI.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreatePowerPoolUI( "CanvasCreatePowerPoolUI" ); }

	tPowerPoolUI::tPowerPoolUI( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tScriptedControl( scriptResource )
		, mUser( user )
		, mIsShown( false )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreatePowerPoolUI, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	tPowerPoolUI::~tPowerPoolUI( )
	{
	}

	void tPowerPoolUI::fStep( f32 powerPoolPercent, f32 timerPercent, f32 dt )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Set" ).Execute( powerPoolPercent, timerPercent );
	}

	void tPowerPoolUI::fSetOverChargeActive( b32 active )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "OverChargeActive" ).Execute( active );
	}

	void tPowerPoolUI::fShow( b32 show )
	{
		mIsShown = show;
		Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( show );
	}

	void tPowerPoolUI::fShowBarrage( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "ShowBarrage" ).Execute( show );

	}

	void tPowerPoolUI::fSetCombo( u32 combo )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetCombo" ).Execute( combo );
	}

}}


namespace Sig { namespace Gui
{
	void tPowerPoolUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tPowerPoolUI,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"), &fUser)
		;
		vm.fNamespace(_SC("Gui")).Bind( _SC("PowerPoolUI"), classDesc );
	}
}}

