#include "GameAppPch.hpp"
#include "tOutOfBoundsIndicator.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateOutOfBoundsIndicator( "CanvasCreateOutOfBoundsIndicator" ); }

	tOutOfBoundsIndicator::tOutOfBoundsIndicator( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tScriptedControl( scriptResource )
		, mUser( user )
		, mShown( false )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateOutOfBoundsIndicator, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	void tOutOfBoundsIndicator::fShow( b32 show )
	{
		if( show != mShown )
		{
			mShown = show;
			Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( show );
		}
	}

	void tOutOfBoundsIndicator::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tOutOfBoundsIndicator,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"),		&tOutOfBoundsIndicator::fUser)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("OutOfBoundsIndicator"), classDesc );
	}

} }