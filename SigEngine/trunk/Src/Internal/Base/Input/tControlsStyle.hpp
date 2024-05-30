#ifndef __tControlsStyle__
#define __tControlsStyle__

namespace Sig { namespace Input
{
	enum tControlsStyle
	{
		cControlsStyle360Controller,
		cControlsStyleKeyboardMouse,
		cControlsStyleTouch,

		cControlsStyleCount,
	};

	base_export void tControlsStyle_fExportScriptInterface( tScriptVm& vm );

	enum tCONTROL_TYPE
	{
		cCONTROL_TYPE_GAMEPAD = 0u,
		cCONTROL_TYPE_TOUCH = 1u,
		cCONTROL_TYPE_MOUSE = 2u,
		cCONTROL_TYPE_KEYBOARD = 3u,
		cCONTROL_TYPE_COUNT = 4,
	};

	base_export void tControlsType_fExportScriptInterface( tScriptVm& vm );
}}


#endif//__tControlsStyle__
