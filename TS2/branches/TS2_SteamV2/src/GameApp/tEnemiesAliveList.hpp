#ifndef __tEnemiesAliveList__
#define __tEnemiesAliveList__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{
	class tEnemiesAliveList : public tScriptedControl
	{
	public:
		explicit tEnemiesAliveList( const tResourcePtr& scriptResource, const tUserPtr& user, u32 enemyCountry );
		~tEnemiesAliveList( );
		void fShow( b32 show );
		void fSetCount( u32 unitID, u32 country, u32 count );
		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		tUserPtr mUser;
		u32 mEnemyCountry;
	};

	typedef tRefCounterPtr< tEnemiesAliveList > tEnemiesAliveListPtr;

}}

#endif//__tEnemiesAliveList__
