#include "BasePch.hpp"
#include "tRandom.hpp"
#include "Threads/tThread.hpp"

namespace Sig
{
	namespace
	{
		static u32 gAdvanceStateCount = 0;
		static u32 gObjectiveThread = Threads::tThread::fCurrentThreadId( );
		static b32 gLogOutput = false;
		inline void fAdvanceRandomState( u32& randState )
		{
			randState = randState * 1103515245 + 12345;
		}
	}

	tRandom tRandom::gSubjectiveRand( ( u32 )Time::fGetStamp( ) );
	tRandom tRandom::gObjectiveRand( 1 );

	void tRandom::fSetMainThread( u32 threadId )
	{
		gObjectiveThread = threadId;
	}

	tRandom& tRandom::fObjectiveRand( )
	{
		log_assert( gObjectiveThread == Threads::tThread::fCurrentThreadId( ), "Object rand only available on main thread (" << gObjectiveThread << "), currently on another (" << Threads::tThread::fCurrentThreadId() << ")" );
		return gObjectiveRand;
	}

	f32 tRandom::fRandStateToFloatMinusOneToOne( u32& randState )
	{
		fAdvanceRandomState( randState );
		
		// this is from http://rgba.scenesp.org/, from their article about fast floating point random number generation
		u32 a = (randState & 0x007fffff) | 0x40000000;
		return( *((f32*)&a) - 3.0f );
	}

	f32 tRandom::fRandStateToFloatZeroToOne( u32& randState )
	{
		fAdvanceRandomState( randState );

		// this is from http://rgba.scenesp.org/, from their article about fast floating point random number generation
		u32 a = (randState & 0x007fffff) | 0x3f800000;
		return( *((f32*)&a) - 1.f );
	}

	void tRandom::fRandStateAdvance( u32& randState )
	{
		fAdvanceRandomState( randState );
	}
	
	void tRandom::fSyncObjectiveSeed( u32 seed )
	{
		gAdvanceStateCount = 0;
		gLogOutput = true;

		gObjectiveRand = tRandom( seed );
		gObjectiveThread = Threads::tThread::fCurrentThreadId( );
	}
}



//--------------------------------------------------------------------------------------------------------------
//
//    Script-Specific Implementation
//
//--------------------------------------------------------------------------------------------------------------

namespace Sig
{
	namespace
	{
		b32 fScriptBool( tRandom* r )
		{
			b32 scriptRandBool = r->fBool( );

#ifdef sync_system_enabled
			if( r->fIsObjective( ) )
				sync_event_v_c( scriptRandBool, tSync::cSCRandom );
#endif

			return scriptRandBool;
		}
		s32 fScriptIntInRange( tRandom* r, s32 min, s32 max )
		{
			s32 scriptRandInt = r->fIntInRange( min, max );

#ifdef sync_system_enabled
			if( r->fIsObjective( ) )
				sync_event_v_c( scriptRandInt, tSync::cSCRandom );
#endif

			return scriptRandInt;
		}
		f32 fScriptFloatInRange( tRandom* r, f32 min, f32 max )
		{
			f32 scriptRandFloat = r->fFloatInRange( min, max );

#ifdef sync_system_enabled
			if( r->fIsObjective( ) )
				sync_event_v_c( scriptRandFloat, tSync::cSCRandom );
#endif

			return scriptRandFloat;
		}

		bool fScriptChance( tRandom * r, f32 chance )
		{
			b32 scriptRandChance = r->fChance( chance );

#ifdef sync_system_enabled
			if( r->fIsObjective( ) )
				sync_event_v_c( scriptRandChance, tSync::cSCRandom );
#endif
			return scriptRandChance ? true : false;
		}
	}

	void tRandom::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tRandom, Sqrat::DefaultAllocator<tRandom> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Bool"),	&fScriptBool)
			.GlobalFunc(_SC("Int"),		&fScriptIntInRange)
			.GlobalFunc(_SC("Float"),	&fScriptFloatInRange)
			.GlobalFunc(_SC("Chance"),	&fScriptChance)
			;

		vm.fRootTable( ).Bind( _SC("Random"), classDesc );

		vm.fRootTable( ).SetInstance(_SC("ObjectiveRand"), &gObjectiveRand);
		vm.fRootTable( ).SetInstance(_SC("SubjectiveRand"), &gSubjectiveRand);
	}

}

