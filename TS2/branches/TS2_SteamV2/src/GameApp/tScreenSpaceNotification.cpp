#include "GameAppPch.hpp"
#include "tScreenSpaceNotification.hpp"

namespace Sig { namespace Gui
{

	namespace { static const tStringPtr cCanvasCreateScreenSpaceNotification( "CanvasCreateScreenSpaceNotification" ); }

	tScreenSpaceNotification::tScreenSpaceNotification( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tScriptedControl( scriptResource )
		, mUser( user )
		, mEnabled( true )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateScreenSpaceNotification, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	tScreenSpaceNotification::~tScreenSpaceNotification( )
	{
	}

	void tScreenSpaceNotification::fSpawnText( const char* text, const Math::tVec4f& color )
	{
		fSpawnText( tLocalizedString::fFromCString( text ), color );
	}

	void tScreenSpaceNotification::fSpawnText( const tLocalizedString& text, const Math::tVec4f& color )
	{
		if( mEnabled ) // performance consideration
			Sqrat::Function( mCanvas.fScriptObject( ), "SpawnText" ).Execute( text, color );
	}

	void tScreenSpaceNotification::fEnable( b32 enable )
	{
		mEnabled = enable;
		mCanvas.fCanvas( )->fSetInvisible( !enable );

		if( !enable )
		{
			Sqrat::Function( mCanvas.fScriptObject( ), "Clear" ).Execute( );
		}
	}

}}

namespace Sig { namespace Gui
{

	void tScreenSpaceNotification::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tScreenSpaceNotification,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"), &fUser)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("ScreenSpaceNotification"), classDesc );
	}

}}