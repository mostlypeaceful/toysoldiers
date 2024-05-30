#ifndef __tMortarWeapon__
#define __tMortarWeapon__
#include "tCannonWeapon.hpp"

namespace Sig
{

	class tMortarWeapon : public tCannonWeapon
	{
	public:
		explicit tMortarWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst );
	};

}

#endif//__tMortarWeapon__
