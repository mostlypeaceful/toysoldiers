#include "GameAppPch.hpp"
#include "tTimedSpawnLogic.hpp"
#include "tGameEffects.hpp"


namespace Sig
{	
	namespace 
	{ 
	}

	tTimedSpawnLogic::tTimedSpawnLogic( )
		: mSpawned( false )
		, mTimeRemaining( 0.1f )
		, mRepeatTime( -1.f )
	{
	}
	void tTimedSpawnLogic::fOnSpawn( )
	{
		tLogic::fOnSpawn( );
		fOnPause( false );

		if( fOwnerEntity( )->fName( ).fExists( ) )
		{
			mTimeRemaining = (f32)atof( fOwnerEntity( )->fNameCStr( ) );
			mTimeRemaining = fClamp( mTimeRemaining, 0.f, 50000.0f );
		}
	}
	void tTimedSpawnLogic::fOnPause( b32 paused )
	{
		if( paused )
			fRunListRemove( cRunListThinkST );
		else
			fRunListInsert( cRunListThinkST );
			
	}
	void tTimedSpawnLogic::fThinkST( f32 dt )
	{
		if( !mSpawned )
		{
			mTimeRemaining -= dt;
			if( mTimeRemaining <= 0.f )
				fSpawnIt( );
		}
	}
	void tTimedSpawnLogic::fSpawnIt( )
	{
		if( mRepeatTime > 0.f )
			mTimeRemaining = mRepeatTime;
		else
		{
			fOwnerEntity( )->fDelete( );
			mSpawned = true;
		}

		if( mFilepath.fExists( ) )
			fOwnerEntity( )->fSpawnChild( mFilepath );

		if( mEffectID.fExists( ) )
			tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), mEffectID );
	}
}


namespace Sig
{
	void tTimedSpawnLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tTimedSpawnLogic, tLogic, Sqrat::NoCopy<tTimedSpawnLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Var(_SC("Filepath"), &tTimedSpawnLogic::mFilepath)
			.Var(_SC("EffectID"), &tTimedSpawnLogic::mEffectID)
			.Var(_SC("Time"), &tTimedSpawnLogic::mTimeRemaining)
			.Var(_SC("RepeatTime"), &tTimedSpawnLogic::mRepeatTime)
			;

		vm.fRootTable( ).Bind(_SC("TimedSpawnLogic"), classDesc);
	}
}

