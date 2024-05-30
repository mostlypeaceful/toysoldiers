#include "GameAppPch.hpp"
#include "tSinglePlayerWaveList.hpp"
//#include "tApplication.hpp"
#include "tGameApp.hpp"
#include "tWaveList.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateWaveList( "CanvasCreateWaveList" ); }

	tSinglePlayerWaveList::tSinglePlayerWaveList( const tResourcePtr& scriptResource, const tUserPtr& user, const tStringPtr& layer )
		: tScriptedControl( scriptResource )
		, mUser( user )
		, mIsVisible( false )
		, mLayer( layer )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateWaveList, this );
		sigassert( !mCanvas.fIsNull( ) );
	}
	tSinglePlayerWaveList::~tSinglePlayerWaveList( )
	{
	}

	void tSinglePlayerWaveList::fSetup( tWaveList* wavelist )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Setup" ).Execute( wavelist );
	}

	void tSinglePlayerWaveList::fAddWaveIcon( tWaveDesc& wave )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "AddWaveIcon" ).Execute( &wave );
	}

	void tSinglePlayerWaveList::fAddWaveIcon( tOffensiveWaveDesc& wave, u32 unitID )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "AddWaveIcon" ).Execute( &wave, unitID );
	}

	void tSinglePlayerWaveList::fNextWave( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "NextWave" ).Execute( );
	}

	void tSinglePlayerWaveList::fReadying( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Readying" ).Execute( );
	}

	void tSinglePlayerWaveList::fCountdownTimer( f32 time )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "CountdownTimer" ).Execute( time );
	}

	void tSinglePlayerWaveList::fSurvivalTimer( f32 time )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "UpdateSurvivalTimer" ).Execute( time );
	}

	void tSinglePlayerWaveList::fSurvivalRound( u32 round )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "UpdateSurvivalText" ).Execute( round );
	}

	void tSinglePlayerWaveList::fLaunchStart( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "LaunchStart" ).Execute( );
	}

	void tSinglePlayerWaveList::fLaunching( f32 dt )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Launching" ).Execute( dt );
	}

	void tSinglePlayerWaveList::fShow( b32 show )
	{
		if( mIsVisible != show )
		{
			mIsVisible = show;
			Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( show && !tGameApp::fExtraDemoMode( ) );
		}
	}
	void tSinglePlayerWaveList::fLooping( b32 looping )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Looping" ).Execute( looping );
	}

	void tSinglePlayerWaveList::fClear( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Clear" ).Execute( );
	}

	void tSinglePlayerWaveList::fFinalEnemyWave()
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FinalEnemyWave" ).Execute( );
	}

}}


namespace Sig { namespace Gui
{
	void tSinglePlayerWaveList::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tSinglePlayerWaveList,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop( _SC( "User" ), &tSinglePlayerWaveList::fUser ) 
			.Var( _SC( "Layer" ), &tSinglePlayerWaveList::mLayer ) 
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("SinglePlayerWaveList"), classDesc );
	}
} }

