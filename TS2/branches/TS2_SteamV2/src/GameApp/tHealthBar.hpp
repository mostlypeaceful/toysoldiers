#ifndef __tHealthBar__
#define __tHealthBar__

#include "Gui\tWorldSpaceScriptedControl.hpp"

namespace Sig { namespace Gui
{
	class tHealthBar : public tWorldSpaceScriptedControl
	{
	public:
		tHealthBar( const tUserArray& users );
		virtual ~tHealthBar( );
		void fSetSize( const Math::tVec2f& size );
		void fSetHealthBarPercent( f32 health );
		void fSetColor( const Math::tVec4f& color );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tHealthBar );
} }

#endif //__tHealthBar__