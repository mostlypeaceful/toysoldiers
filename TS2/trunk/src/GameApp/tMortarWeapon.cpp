#include "GameAppPch.hpp"
#include "tMortarWeapon.hpp"

namespace Sig
{
	tMortarWeapon::tMortarWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst )
		: tCannonWeapon( desc, inst )
	{
		mInvertPitch = true;
	}
}

