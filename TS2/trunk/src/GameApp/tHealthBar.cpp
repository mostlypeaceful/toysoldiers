#include "GameAppPch.hpp"
#include "tHealthBar.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{
	tHealthBar::tHealthBar( const tUserArray& users ) 
	{
		fCreate( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptEnemyHealthBar ), users, tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ) );
	}

	tHealthBar::~tHealthBar( )
	{
	}

	void tHealthBar::fSetSize( const Math::tVec2f& size )
	{
		const tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetSize" ).Execute( size.x, size.y );
	}

	void tHealthBar::fSetHealthBarPercent( f32 health )
	{
		const f32 healthPercent = fClamp( health, 0.f, 1.f );
		const tControlArray& controls = fAccessControls( );

		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetHealthBarPercent" ).Execute( healthPercent );
	}
	void tHealthBar::fSetColor( const Math::tVec4f& color )
	{
		const tControlArray& controls = fAccessControls( );

		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetColor" ).Execute( color );
	}
}}
