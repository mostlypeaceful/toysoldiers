#ifndef __tScoreUI__
#define __tScoreUI__
#include "Gui/tScriptedControl.hpp"
#include "tPlayer.hpp"

namespace Sig { 

	class tBarrage;

namespace Gui
{
	class tScoreUI : public tScriptedControl
	{
	public:
		explicit tScoreUI( const tResourcePtr& scriptResource, const tPlayerPtr& player );
		~tScoreUI( );
		void fSetScore( s32 count, f32 percent );
		void fSetMoney( u32 money );

		void fShow( b32 show );
		void fShowMoney( b32 show );
		void fShowTickets( b32 show );

		void fProtectToyBox( );
		tUser* fUser( ) const { return mPlayer->fUser( ).fGetRawPtr( ); }
		tPlayer* fPlayer( ) const { return mPlayer.fGetRawPtr( ); }

		void Test( u32 action, u32 button, tPlayer* player, b32 held );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		tPlayerPtr mPlayer;
	};

	typedef tRefCounterPtr< tScoreUI > tScoreUIPtr;

}}

#endif//__tScoreUI__
