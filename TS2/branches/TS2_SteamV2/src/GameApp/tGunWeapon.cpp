#include "GameAppPch.hpp"
#include "tGunWeapon.hpp"

using namespace Sig::Math;

namespace Sig
{

	tGunWeapon::tGunWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst )
		: tWeapon( desc, inst )
	{
	}
	void tGunWeapon::fProcessST( f32 dt )
	{
		if( mPlayer )
		{
			mAITarget.fRelease( );

			if( mDesc.mRaycastAdjustTargets )
				fRayCastAndAdjustTarget( );

			fUpdateTargetRelatedData( dt );
		}

		tWeapon::fProcessST( dt );

		fDebugRenderShootDirect( );
	}
	void tGunWeapon::fProcessMT( f32 dt )
	{
		tWeapon::fProcessMT( dt );
	}
}

