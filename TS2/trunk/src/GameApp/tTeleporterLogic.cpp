#include "GameAppPch.hpp"
#include "tTeleporterLogic.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tWaypointLogic.hpp"



namespace Sig
{	

	void tTeleporterLogic::fOnSpawn( )
	{
		tBreakableLogic::fOnSpawn( );
		tGameApp::fInstance( ).fCurrentLevel( )->fRegisterTeleporter( fOwnerEntity( ) );
	}

	void tTeleporterLogic::fOnStateChanged( )
	{
		// Change to >= in case we skip a state
		if( mCurrentState >= (s32) mBreakState )
		{
			if ( !mDestroyedPathNameDisable.fNull( ) )
			{
				//disable waypoints
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

				const tGrowableArray<tPathEntityPtr>& waypoints = level->fNamedWayPoints( );

				for( u32 i = 0; i < waypoints.fCount( ); ++i )
				{
					if( mDestroyedPathNameDisable == waypoints[ i ]->fName( ) )
					{
						tWaypointLogic* waypoint = waypoints[ i ]->fLogicDerived< tWaypointLogic >( );
						if( waypoint )
							waypoint->fSetAccessible( false );
					}
				}
			}

			if ( !mDestroyedPathNameEnable.fNull( ) )
			{
				//enable waypoints with same name
				tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );

				const tGrowableArray<tPathEntityPtr>& waypoints = level->fNamedWayPoints( );

				for( u32 i = 0; i < waypoints.fCount( ); ++i )
				{
					if( mDestroyedPathNameEnable == waypoints[ i ]->fName( ) )
					{
						tWaypointLogic* waypoint = waypoints[ i ]->fLogicDerived< tWaypointLogic >( );
						if( waypoint )
							waypoint->fSetAccessible( true );
					}
				}
			}
		}
	}

}


namespace Sig
{
	void tTeleporterLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tTeleporterLogic, tBreakableLogic, Sqrat::NoCopy<tTeleporterLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("ExitPathName"), &tTeleporterLogic::mExitPathName)
			.Var(_SC("DestroyedPathNameDisable"), &tTeleporterLogic::mDestroyedPathNameDisable)
			.Var(_SC("DestroyedPathNameEnable"), &tTeleporterLogic::mDestroyedPathNameEnable)
			;

		vm.fRootTable( ).Bind(_SC("TeleporterLogic"), classDesc);
	}
}

