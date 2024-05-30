#include "GameAppPch.hpp"
#include "tMobileTurretLogic.hpp"
#include "tVehicleLogic.hpp"


namespace Sig
{	

	tMobileTurretLogic::tMobileTurretLogic( )
		: mVehicleLogic( NULL )
	{ }

	void tMobileTurretLogic::fOnDelete( )
	{
		mVehicleLogic = NULL;
		mVehicle.fRelease( );
		tAnimatedBreakableLogic::fOnDelete( );
	}

	void tMobileTurretLogic::fOnSpawn( )
	{		
		mVehicleLogic = fOwnerEntity( )->fFirstAncestorWithLogicOfType<tVehicleLogic>( );
		sigassert( mVehicleLogic && "Mobile Turret needs vehicle logic parent" );

		mVehicle.fReset( mVehicleLogic->fOwnerEntity( ) );

		// I need to pass damage to my owner, but only direct. Explosion is passed when it hits the parent itself. ghetto?
		fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_LINKED_HITPOINTS, GameFlags::cLINKED_HITPOINTS_TRANSFER_ONLY_DIRECT ) );
		fSetHitpointLinkedUnitLogic( mVehicleLogic );

		// Also need parents country and unit id.
		u32 country = mVehicleLogic->fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_COUNTRY );
		u32 unitID = mVehicleLogic->fOwnerEntity( )->fQueryEnumValue( GameFlags::cENUM_COUNTRY );

		if( country != ~0 )
			fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_COUNTRY, country ) );
		else
			log_warning( 0, "No country found in tMobileTurretLogic parent" );

		if( unitID != ~0 )
			fOwnerEntity( )->fSetEnumValue( tEntityEnumProperty( GameFlags::cENUM_UNIT_ID, unitID ) );
		else
			log_warning( 0, "No unitID found in tMobileTurretLogic parent" );

		tAnimatedBreakableLogic::fOnSpawn( );
	}

}


namespace Sig
{
	void tMobileTurretLogic::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tMobileTurretLogic, tAnimatedBreakableLogic, Sqrat::NoCopy<tMobileTurretLogic> > classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("OwnerVehicle"),		&tMobileTurretLogic::fOwnerVehicle)
				;

			vm.fRootTable( ).Bind(_SC("MobileTurretLogic"), classDesc);
		}
	}
}

