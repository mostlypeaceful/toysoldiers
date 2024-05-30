// Barrage UI
#ifndef __tBarrageUI__
#define __tBarrageUI__

#include "Gui/tScriptedControl.hpp"
#include "tBarrage.hpp"
#include "tUser.hpp"

namespace Sig { 

namespace Gui
{
	class tBarrageUI : public tScriptedControl
	{
	public:
		explicit tBarrageUI( const tUserPtr& user );
		~tBarrageUI( );

		void fSetUsable( b32 usable );
		void fStartSpinning( const tBarrage& barrage, f32 spinTime, b32 skipInto );
		void fSetAvailable( );
		void fBegin( );
		void fEnd( );
		void fUpdateTimer( f32 percent );
		void fShow( b32 show );

		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr mUser;
	};

	typedef tRefCounterPtr< tBarrageUI > tBarrageUIPtr;
} }

#endif //__tBarrageUI__