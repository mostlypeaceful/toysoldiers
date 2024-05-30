// World Space Flying Text
#include "GameAppPch.hpp"
#include "tWorldSpaceFlyingText.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateFlyingText( "CanvasCreateFlyingText" ); }

	tWorldSpaceFlyingText::tWorldSpaceFlyingText( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tScriptedControl( scriptResource )
		, mUser( user )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateFlyingText, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	void tWorldSpaceFlyingText::fSetTarget( const Math::tVec3f& target )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetTarget" ).Execute( target );
	}

	void tWorldSpaceFlyingText::fSetText( const Sqrat::Object& textObj )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetText" ).Execute( textObj );
	}

	void tWorldSpaceFlyingText::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tWorldSpaceFlyingText,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"), &fUser)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("WorldSpaceFlyingText"), classDesc );
	}

} }