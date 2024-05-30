#ifndef __tTurretUpgradeIndicator__
#define __tTurretUpgradeIndicator__

#include "Gui/tWorldSpaceScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{
	class tTurretRadialIndicator : public tWorldSpaceScriptedControl
	{
	public:
		tTurretRadialIndicator( const tResourcePtr& script, const tUserArray& users );
		virtual ~tTurretRadialIndicator( );
		void fShow( );
		void fShow( const tUserPtr& exceptThisGuy );
		void fHide( );
		void fSetPercent( f32 percent );
	};

	typedef tRefCounterPtr< tTurretRadialIndicator > tTurretRadialIndicatorPtr;
} }

#endif //__tTurretUpgradeIndicator__