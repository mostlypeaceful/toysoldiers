// Achievement Buy Notification
#ifndef __tAchievementBuyNotification__
#define __tAchievementBuyNotification__
#include "Gui/tScriptedControl.hpp"
#include "tPlayer.hpp"

namespace Sig { namespace Gui
{
	class tAchievementBuyNotification : public tScriptedControl
	{
	public:
		explicit tAchievementBuyNotification( const tResourcePtr& scriptResource, const tPlayerPtr& player, u32 index );
		~tAchievementBuyNotification( ) { }

		void fShow( b32 show );
		tPlayer* fPlayer( ) const { return mPlayer.fGetRawPtr( ); }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		tPlayerPtr mPlayer;
		u32 mIndex;
	};
} }

#endif //__tAchievementBuyNotification__