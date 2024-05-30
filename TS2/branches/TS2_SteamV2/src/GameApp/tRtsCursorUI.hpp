#ifndef __tRtsCursorUI__
#define __tRtsCursorUI__
#include "Gui/tScriptedControl.hpp"
#include "tLocalizationFile.hpp"

namespace Sig 
{
class tUnitLogic;

namespace Gui
{
	class tRtsCursorUI : public tScriptedControl
	{
	public:
		explicit tRtsCursorUI( const tResourcePtr& scriptResource );
		~tRtsCursorUI( );

		// force show if you dont want a noMoney/noUpgrade = false to clear the no money icon until the timer expires
		void fSetNoMoney( b32 noMoney, b32 forceShow, u32 cost );
		void fSetNoUpgrade( b32 noUpgrade, b32 forceShow );
		void fSetNoRepair( b32 noRepair, b32 forceShow );
		void fSetGhostUnit( tUnitLogic* unit );
		void fSetText( const tLocalizedString& text );
		void fShowCapturePlatform( b32 show );

		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr< tRtsCursorUI > tRtsCursorUIPtr;

}}

#endif//__tRtsCursorUI__
