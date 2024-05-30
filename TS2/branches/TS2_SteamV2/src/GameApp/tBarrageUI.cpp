#include "GameAppPch.hpp"
#include "tGameApp.hpp"
#include "tBarrageUI.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateBarrageIndicator( "CanvasCreateBarrageIndicator" ); }

	tBarrageUI::tBarrageUI( const tUserPtr& user )
		: tScriptedControl( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptBarrageIndicator ) )
		, mUser( user )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateBarrageIndicator, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	tBarrageUI::~tBarrageUI( )
	{
	}

	void tBarrageUI::fSetUsable( b32 usable )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetBarrageUsable" ).Execute( usable );
	}

	void tBarrageUI::fStartSpinning( const tBarrage& barrage, f32 spinTime, b32 skipInto )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "StartSpinning" ).Execute( barrage, spinTime, skipInto );
	}

	void tBarrageUI::fSetAvailable()
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetAvailable" ).Execute( );
	}

	void tBarrageUI::fBegin()
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "BarrageBegin" ).Execute( );
	}

	void tBarrageUI::fEnd()
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "BarrageEnd" ).Execute( );
	}

	void tBarrageUI::fUpdateTimer( f32 percent )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "UpdateTimer" ).Execute( percent );
	}

	void tBarrageUI::fShow( b32 show )
	{
		if( show )
			Sqrat::Function( mCanvas.fScriptObject( ), "FadeIn" ).Execute( );
		else
			Sqrat::Function( mCanvas.fScriptObject( ), "FadeOut" ).Execute( );
	}

	void tBarrageUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tBarrageUI, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"), &tBarrageUI::fUser)
			;
		vm.fNamespace( _SC( "Gui" ) ).Bind( _SC( "BarrageUI" ), classDesc );
	}

} }