#ifndef __tHoverText__
#define __tHoverText__

#include "Gui\tWorldSpaceScriptedControl.hpp"
#include "tUnitLogic.hpp"

namespace Sig { namespace Gui
{
	class tHoverText : public tWorldSpaceScriptedControl
	{
	public:
		tHoverText( const tUserArray& user );
		tHoverText( const tUserPtr& user );
		virtual ~tHoverText( );
		
		void fSetVisibility( b32 visible );
		void fSetHoverUnit( tUnitLogic* unit );

	private:
		tUnitLogic* mHoverUnit;
		tUnitLogic* mPreviousUnit;
		f32 mPreviousHealth;
		b32 mPreviousUsable;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tHoverText );
} }

#endif //__tHoverText__