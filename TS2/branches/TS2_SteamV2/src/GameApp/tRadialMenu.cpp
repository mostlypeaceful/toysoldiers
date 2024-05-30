#include "GameAppPch.hpp"
#include "tRadialMenu.hpp"
#include "tApplication.hpp"
#include "tGameController.hpp"
#include "tPlayer.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateRadialMenu( "CanvasCreateRadialMenu" ); }

	tRadialMenu::tRadialMenu( const tResourcePtr& scriptResource, const tUserPtr& user, const tGameControllerPtr& gc )
		: tScriptedControl( scriptResource )
		, mUser( user )
		, mGameController( gc )
		, mInputLevel( ~0 )
		, mActive( false )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateRadialMenu, this );
		sigassert( !mCanvas.fIsNull( ) );
	}
	tRadialMenu::~tRadialMenu( )
	{
		if( mInputLevel != ~0 )
			mUser->fDecInputFilterLevel( mInputLevel );
	}
	void tRadialMenu::fFadeIn( )
	{
		mActive = true;
		mInputLevel = mUser->fIncInputFilterLevel( );
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeIn" ).Execute( );
	}
	void tRadialMenu::fFadeOut( )
	{
		mActive = false;
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeOut" ).Execute( );
		sigassert( mInputLevel != ~0 );
		mUser->fDecInputFilterLevel( mInputLevel );
		mInputLevel = ~0;
	}
	b32 tRadialMenu::fTryHotKeys( const tGameControllerPtr& gc )
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "TryHotKeys" ).Evaluate<bool>( const_cast<tGameController*>( gc.fGetRawPtr( ) ) )!=0;
	}
	b32 tRadialMenu::fHighlightByAngle( f32 angle, f32 magnitude )
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "HighlightByAngle" ).Evaluate<bool>( angle, magnitude )!=0;
	}
	b32 tRadialMenu::fSelectActiveIcon( )
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "SelectActiveIcon" ).Evaluate<bool>( )!=0;
	}

	b32 tRadialMenu::fHighlightByPosition( const Math::tVec2f &pos, f32 minRadius )
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "HighlightByPosition" ).Evaluate<bool>( pos, minRadius )!=0;
	}

	Math::tVec2f tRadialMenu::fGetEntryPosition( u32 entryIdx )
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "GetEntryPosition" ).Evaluate<Math::tVec2f>( entryIdx );
	}


	const tGameControllerPtr& tRadialMenu::fGameController( ) const
	{
		return mGameController;
	}

}}


namespace Sig { namespace Gui
{
	void tRadialMenu::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tRadialMenu,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("GameController"), &tRadialMenu::fGameControllerFromScript)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("RadialMenu"), classDesc );
	}
}}

