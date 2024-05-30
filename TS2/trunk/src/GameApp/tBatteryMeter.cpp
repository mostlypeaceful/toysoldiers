#include "GameAppPch.hpp"
#include "tBatteryMeter.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateBatteryMeter( "CanvasCreateBatteryMeter" ); }

	tBatteryMeter::tBatteryMeter( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tScriptedControl( scriptResource )
		, mUser( user )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateBatteryMeter, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	tBatteryMeter::~tBatteryMeter( )
	{
	}

	void tBatteryMeter::fSet( f32 percent )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Set" ).Execute( percent );
	}

	void tBatteryMeter::fFadeIn()
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeIn" ).Execute( );
	}

	void tBatteryMeter::fFadeOut()
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeOut" ).Execute( );
	}

}}

namespace Sig { namespace Gui
{
	void tBatteryMeter::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tBatteryMeter,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"),		&tBatteryMeter::fUser)
			.Prop(_SC("Vehicle"),	&tBatteryMeter::fVehicle)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("BatteryMeter"), classDesc );
	}
}}