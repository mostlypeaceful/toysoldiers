#include "GameAppPch.hpp"
#include "tBulletLogic.hpp"
#include "tUnitLogic.hpp"

namespace Sig
{
	tBulletLogic::tBulletLogic( )
	{ 
		mIsBullet = true;
		mFirstTickOffset = (u8)sync_rand( fIntInRange( 1, 255 ) );
	}

	void tBulletLogic::fComputeNewPosition( f32 dt )
	{
		mNextPos = fOwnerEntity( )->fObjectToWorld( );
		mNextPos.fTranslateGlobal( mLaunchVector * dt );
		sync_event_v_c( mNextPos, tSync::cSCProjectile );

		fCheckLevelBounds( );
	}
}

namespace Sig
{
	void tBulletLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tBulletLogic, tProjectileLogic, Sqrat::NoCopy<tBulletLogic> > classDesc( vm.fSq( ) );
		//classDesc
		//	;
		vm.fRootTable( ).Bind(_SC("BulletLogic"), classDesc);
	}
}