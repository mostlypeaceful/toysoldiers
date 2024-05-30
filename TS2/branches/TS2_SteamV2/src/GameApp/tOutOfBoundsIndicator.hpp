// Out of bounds indicator for vehicles and characters
#ifndef __tOutOfBoundsIndicator__
#define __tOutOfBoundsIndicator__

#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{
	class tOutOfBoundsIndicator : public tScriptedControl
	{
	public:
		explicit tOutOfBoundsIndicator( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tOutOfBoundsIndicator( ) { }

		void fShow( b32 show );
		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr mUser;
		b32 mShown;
	};

	typedef tRefCounterPtr< tOutOfBoundsIndicator > tOutOfBoundsIndicatorPtr;
} }

#endif //__tOutOfBoundsIndicator__