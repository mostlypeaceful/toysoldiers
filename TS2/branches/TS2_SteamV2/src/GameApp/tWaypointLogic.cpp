#include "GameAppPch.hpp"
#include "tWaypointLogic.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"

namespace Sig
{
	tWaypointLogic::tWaypointLogic( ) : mAccessible( true ), mPathType( cPathTypeCount )
	{
	}

	tWaypointLogic::~tWaypointLogic( )
	{
	}
	void tWaypointLogic::fOnSpawn( )
	{
		fOnPause( false );

		tLogic::fOnSpawn( );

		if( fOwnerEntity( )->fName( ).fExists( ) )
			tGameApp::fInstance( ).fCurrentLevel( )->fRegisterNamedWaypoint( fOwnerEntity( ) );
	}
	void tWaypointLogic::fOnDelete( )
	{
		tLogic::fOnDelete( );
	}
	void tWaypointLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
		}
		else
		{
			fRunListInsert( cRunListActST );
		}
	}
	b32 tWaypointLogic::fHandleLogicEvent( const Logic::tEvent& e )
	{
		return tLogic::fHandleLogicEvent( e );
	}
	void tWaypointLogic::fActST( f32 dt )
	{
		tLogic::fActST( dt );
	}
	void tWaypointLogic::fSetAccessible( b32 accessible )
	{
		mAccessible = accessible;

		tPathEntity* path = fOwnerEntity( )->fDynamicCast< tPathEntity >( );
		if( path ) fPropagateAccessibility( path );
	}
	void tWaypointLogic::fPropagateAccessibility( tPathEntity* path )
	{
		sigassert( path );

		// Look at the previous points to see if they branch. Only propagate accessibility to non branching points
		for( u32 i = 0; i < path->fPrevPointCount( ); ++i )
		{
			// If I'm an only child, set my parents accessibility
			if( path->fPrevPoint( i )->fNextPointCount( ) == 1 )
			{
				tWaypointLogic* waypointLogic = path->fPrevPoint( i )->fLogicDerived<tWaypointLogic>( );
				sigassert( waypointLogic );
				waypointLogic->fSetAccessible( mAccessible );
			}
		}
	}
}

namespace Sig
{
	void tWaypointLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tWaypointLogic, tLogic, Sqrat::NoCopy<tWaypointLogic> > classDesc( vm.fSq( ) );

		classDesc
			.Prop( _SC("Accessible"), &tWaypointLogic::fIsAccessible, &tWaypointLogic::fSetAccessible )
			.Var( _SC("PathType"), &tWaypointLogic::mPathType )
			;

		vm.fRootTable( ).Bind(_SC("WaypointLogic"), classDesc);

		vm.fConstTable( ).Const(_SC("PATH_TYPE_GOAL"),			(int)cGoalPath);
		vm.fConstTable( ).Const(_SC("PATH_TYPE_TRENCH_PATH"),	(int)cTrenchPath);
		vm.fConstTable( ).Const(_SC("PATH_TYPE_EXIT_GENERATOR_PATH"), (int)cExitGeneratorPath);
		vm.fConstTable( ).Const(_SC("PATH_TYPE_FLYING_PATH"),	(int)cRandomFlyingPath);
		vm.fConstTable( ).Const(_SC("PATH_TYPE_CONTEXT_PATH"),	(int)cContextPath);
	}
}

