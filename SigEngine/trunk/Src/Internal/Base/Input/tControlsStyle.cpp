#include "BasePch.hpp"
#include "Input/tControlsStyle.hpp"

namespace Sig { namespace Input
{
	void tControlsStyle_fExportScriptInterface( tScriptVm& vm )
	{
		vm.fConstTable( ).Const( "CONTROLS_STYLE_360_CONTROLLER", ( int )cControlsStyle360Controller );
		vm.fConstTable( ).Const( "CONTROLS_STYLE_KEYBOARD_MOUSE", ( int )cControlsStyleKeyboardMouse );
		vm.fConstTable( ).Const( "CONTROLS_STYLE_TOUCH", ( int )cControlsStyleTouch );

		vm.fConstTable( ).Const( "CONTROLS_STYLE_COUNT", ( int )cControlsStyleCount );
	}

	void tControlsType_fExportScriptInterface( tScriptVm& vm )
	{
		vm.fConstTable( ).Const( "CONTROL_TYPE_GAMEPAD",	( int )cCONTROL_TYPE_GAMEPAD );
		vm.fConstTable( ).Const( "CONTROL_TYPE_TOUCH",		( int )cCONTROL_TYPE_TOUCH );
		vm.fConstTable( ).Const( "CONTROL_TYPE_MOUSE",		( int )cCONTROL_TYPE_MOUSE );
		vm.fConstTable( ).Const( "CONTROL_TYPE_KEYBOARD",	( int )cCONTROL_TYPE_KEYBOARD);
		vm.fConstTable( ).Const( "CONTROL_TYPE_COUNT",		( int )cCONTROL_TYPE_COUNT );
	}
}}
