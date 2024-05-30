#include "GameAppPch.hpp"
#include "tAnimGoalLogic.hpp"

namespace Sig
{
	tAnimGoalLogic::tAnimGoalLogic( )
	{
		tAnimatable::fSetLogic( this );
	}
	tAnimGoalLogic::~tAnimGoalLogic( )
	{
	}
	void tAnimGoalLogic::fOnSpawn( )
	{
		tLogic::fOnSpawn( );
		tGoalDriven::fOnSpawn( this );
		tAnimatable::fOnSpawn( );
		fOnPause( false );
	}
	void tAnimGoalLogic::fOnDelete( )
	{
		tAnimatable::fOnDelete( );
		tGoalDriven::fOnDelete( this );
		tLogic::fOnDelete( );
	}
	void tAnimGoalLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListAnimateMT );
			fRunListRemove( cRunListMoveST );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListAnimateMT );
			fRunListInsert( cRunListMoveST );
		}
	}
	void tAnimGoalLogic::fActST( f32 dt )
	{
		Logic::tGoalDriven::fActST( this, dt );
	}
	void tAnimGoalLogic::fAnimateMT( f32 dt )
	{
		Logic::tAnimatable::fAnimateMT( dt );
	}
	void tAnimGoalLogic::fMoveST( f32 dt )
	{
		Logic::tAnimatable::fMoveST( dt );

		const b32 applyRefFrame = true; // TODO REFACTOR allow animator to disable, which means somewhere in MoMap probably
		if( applyRefFrame )
		{
			Math::tMat3f moveTo = fOwnerEntity( )->fObjectToWorld( );
			Logic::tAnimatable::fAnimatedSkeleton( )->fApplyRefFrameDelta( moveTo );
			fOwnerEntity( )->fMoveTo( moveTo );
		}
	}
}


namespace Sig
{
	void tAnimGoalLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tAnimGoalLogic, tLogic, Sqrat::NoCopy<tAnimGoalLogic> > classDesc( vm.fSq( ) );
		vm.fRootTable( ).Bind(_SC("AnimGoalLogic"), classDesc);
	}
}

