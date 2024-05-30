#include "GameAppPch.hpp"
#include "tAchievementBuyNotification.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateAchievementBuyNotification( "CanvasCreateAchievementBuyNotification" ); }

	////////////////////////////////////////////////////////////////////////////////
	tAchievementBuyNotification::tAchievementBuyNotification( const tResourcePtr& scriptResource, const tPlayerPtr& player, u32 index )
		: tScriptedControl( scriptResource )
		, mPlayer( player )
		, mIndex( index )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateAchievementBuyNotification, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	void tAchievementBuyNotification::fShow( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( show );
	}

} }

namespace Sig { namespace Gui
{
	void tAchievementBuyNotification::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tAchievementBuyNotification, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("Player"), &tAchievementBuyNotification::fPlayer)
			.Var(_SC("Index"), &tAchievementBuyNotification::mIndex)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("AchievementBuyNotification"), classDesc );
	}
} }