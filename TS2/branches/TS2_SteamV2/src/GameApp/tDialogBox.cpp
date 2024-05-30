#include "GameAppPch.hpp"
#include "tDialogBox.hpp"
#include "tApplication.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateDialogBox( "CanvasCreateDialogBox" ); }

	tDialogBox::tDialogBox( const tResourcePtr& scriptResource, const tUserPtr& user, b32 needsConfirmation )
		: tScriptedControl( scriptResource )
		, mUser( user )
		, mInputLevel( user->fInputFilterLevel( ) )
		, mNeedsConfirmation( needsConfirmation )
	{
		if( mNeedsConfirmation )
			mInputLevel =  mUser->fIncInputFilterLevel( );

		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateDialogBox, this );
		sigassert( !mCanvas.fIsNull( ) );
	}
	tDialogBox::~tDialogBox( )
	{
		if( mNeedsConfirmation )
			mUser->fDecInputFilterLevel( mInputLevel );
	}
	void tDialogBox::fFadeIn( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeIn" ).Execute( );
	}
	void tDialogBox::fFadeOut( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeOut" ).Execute( );
	}
	void tDialogBox::fSetText( const tStringPtr& text )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetText" ).Execute( text );
	}
}}


namespace Sig { namespace Gui
{
	void tDialogBox::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tDialogBox,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("Gamepad"), &tDialogBox::fGamepadFromScript)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("DialogBox"), classDesc );
	}
}}

