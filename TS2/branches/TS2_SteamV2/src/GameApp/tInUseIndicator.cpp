#include "GameAppPch.hpp"
#include "tInUseIndicator.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{

	tInUseIndicator::tInUseIndicator( const tUserArray& users )
		: mShouldDelete( true )
	{
		fCreate( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptInUseIndicator ), users, tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ) );
	}

	tInUseIndicator::~tInUseIndicator( )
	{
	}

	void tInUseIndicator::fShow( )
	{
		const tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "Show" ).Execute( );
	}

	void tInUseIndicator::fShow( const tUserPtr& exceptThisGuy )
	{
		const tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
		{
			if( controls[ i ].mUser == exceptThisGuy )
				Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "Hide" ).Execute( );
			else
				Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "Show" ).Execute( );
		}
	}

	void tInUseIndicator::fHide( )
	{
		const tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "Hide" ).Execute( );
	}

	void tInUseIndicator::fSetIndicator( tPlayer* userInControl, u32 country )
	{
		if( userInControl )
			mShouldDelete = false;
		else
			mShouldDelete = true;

		const tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetIndicator" ).Execute( userInControl? userInControl : Sqrat::Object( ), country );
	}

} }