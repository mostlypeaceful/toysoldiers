#include "GameAppPch.hpp"
#include "tTurretUpgradeIndicator.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{

	tTurretRadialIndicator::tTurretRadialIndicator( const tResourcePtr& script, const tUserArray& users )
	{
		fCreate( script, users, tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ) );
	}

	tTurretRadialIndicator::~tTurretRadialIndicator( )
	{
	}

	void tTurretRadialIndicator::fShow( )
	{
		const tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "Show" ).Execute( );
	}

	void tTurretRadialIndicator::fShow( const tUserPtr& exceptThisGuy )
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

	void tTurretRadialIndicator::fHide( )
	{
		const tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "Hide" ).Execute( );
	}

	void tTurretRadialIndicator::fSetPercent( f32 percent )
	{
		const tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetPercent" ).Execute( percent );
	}

} }
