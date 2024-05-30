#include "GameAppPch.hpp"
#include "tPersonalBestUI.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreatePersonalBestUI( "CanvasCreatePersonalBestUI" ); }

	tPersonalBestUI::tPersonalBestUI( const tPlayerPtr& player )
		: tScriptedControl( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptPersonalBestUI ) )
		, mPlayer( player )
		, mShown( false )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreatePersonalBestUI, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	void tPersonalBestUI::fNewPersonalBest( u32 statID, f32 newValue )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "NewPersonalBest" ).Execute( statID, newValue );
	}

	void tPersonalBestUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tPersonalBestUI,Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("Player"), &tPersonalBestUI::fPlayer)
			.Var(_SC("Shown"), &tPersonalBestUI::mShown)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("PersonalBestUI"), classDesc );
	}

} }