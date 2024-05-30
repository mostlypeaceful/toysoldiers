#include "GameAppPch.hpp"
#include "tBombDropOverlay.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateBombDropOverlay( "CanvasCreateBombDropOverlay" ); }

	tBombDropOverlay::tBombDropOverlay( const tUserPtr& user )
		: tScriptedControl( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptBombDropOverlay ) )
		, mUser( user )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateBombDropOverlay, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	tBombDropOverlay::~tBombDropOverlay( )
	{
	}

	void tBombDropOverlay::fShow( b32 show, tPlayer* player )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( show, player );

		if( show )
			tGameApp::fInstance( ).fPostEffectsManager( )->fPushEffectsData( mPostEffectData, mUser->fViewportIndex( ) );
		else
			tGameApp::fInstance( ).fPostEffectsManager( )->fPopEffectsData( mUser->fViewportIndex( ) );
	}

	void tBombDropOverlay::fSetAngle( f32 angle )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetAngle" ).Execute( -angle );
	}

	void tBombDropOverlay::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tBombDropOverlay, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"), &tBombDropOverlay::fUser)
			.Prop(_SC("PostEffectData"), &tBombDropOverlay::fPostEffectData)
			;
		vm.fNamespace( _SC( "Gui" ) ).Bind( _SC( "BombDropOverlay" ), classDesc );
	}

} }