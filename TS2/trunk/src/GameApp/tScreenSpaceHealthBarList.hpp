#ifndef __tScreenSpaceHealthBarList__
#define __tScreenSpaceHealthBarList__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"
#include "tUnitLogic.hpp"

namespace Sig { namespace Gui
{
	class tScreenSpaceHealthBarList : public tScriptedControl
	{
	public:
		explicit tScreenSpaceHealthBarList( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tScreenSpaceHealthBarList( );
		void fAddHealthBar( const tUnitLogic& unitLogic );

		void fSetColor( const tUnitLogic& unitLogic, const Math::tVec4f& color );
		void fSetFlashAndFill( const tUnitLogic& unitLogic, f32 flash, f32 fill );
		void fSetHealthPercent( const tUnitLogic& unitLogic, f32 percent );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		tUserPtr mUser;
		tGrowableArray< tEntityPtr > mUnits;
	};

	typedef tRefCounterPtr< tScreenSpaceHealthBarList > tScreenSpaceHealthBarListPtr;

}}

#endif//__tScreenSpaceHealthBarList__
