#include "GameAppPch.hpp"
#include "tHoverText.hpp"
#include "tGameApp.hpp"
#include "tVehicleLogic.hpp"

namespace Sig { namespace Gui
{
	tHoverText::tHoverText( const tUserArray& users )
		: mHoverUnit( NULL )
		, mPreviousUnit( NULL )
		, mPreviousHealth( -1.0f )
		, mPreviousUsable( true )
	{
		fCreate( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptRtsHoverText ), users, tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ) );
	}

	tHoverText::tHoverText( const tUserPtr& user )
		: mHoverUnit( 0 )
	{
		fCreate( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptRtsHoverText ), *user, tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ) );
	}

	tHoverText::~tHoverText( )
	{
	}
	void tHoverText::fSetVisibility( b32 visible )
	{
		const Gui::tWorldSpaceScriptedControl::tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetVisibility" ).Execute( visible );
	}

	void tHoverText::fSetHoverUnit( tUnitLogic* unit )
	{
		mHoverUnit = unit;

		if( mHoverUnit )
		{
			fSetVisibility( true );
			
			sigassert( mHoverUnit->fOwnerEntity( ) );
			fMoveTo( mHoverUnit->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) );

			// still show the usable for turrets in general, so you can upgrade and repair and sell. 
			//  all other types are not usable in general.
			b32 usable = (tGameApp::fInstance( ).fDifficulty( ) != GameFlags::cDIFFICULTY_GENERAL || mHoverUnit->fLogicType( ) == GameFlags::cLOGIC_TYPE_TURRET);

			if( !unit->fCanBeUsed( ) )
				usable = false;

			tVehicleLogic* vehicleLogic = unit->fDynamicCast< tVehicleLogic >( );
			const Gui::tWorldSpaceScriptedControl::tControlArray& controls = fAccessControls( );
			for( u32 i = 0; i < controls.fCount( ); ++i )
			{
				if( (mPreviousUnit != mHoverUnit) || (usable != mPreviousUsable) )
					Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetHoverText" ).Execute( mHoverUnit->fHoverText( ), usable );

				if( mPreviousHealth != mHoverUnit->fHealthPercent( ) )
					Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetHealth" ).Execute( mHoverUnit->fHealthPercent( ) );

				if( vehicleLogic )
				{
					Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetPowerLevel" ).Execute( vehicleLogic->fPowerLevel( ) );
				}
			}

			mPreviousHealth = mHoverUnit->fHealthPercent( );
			mPreviousUsable = usable;
		}
		else
			fSetVisibility( false );

		mPreviousUnit = mHoverUnit;
	}
} }
