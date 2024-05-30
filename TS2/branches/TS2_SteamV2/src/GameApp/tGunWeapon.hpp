#ifndef __tGunWeapon__
#define __tGunWeapon__
#include "tWeapon.hpp"

namespace Sig
{

	class tGunWeapon : public tWeapon
	{
	public:
		explicit tGunWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst );

		virtual void fProcessST( f32 dt );
		virtual void fProcessMT( f32 dt );

	protected:
		virtual void fComputeIdealAngle( ) { fComputeIdealAngleDirect( ); }
		virtual f32 fEstimateTimeToImpact( const Math::tVec3f& pos ) const { return fEstimateTimeToImpactDirect( pos ); }

	};

}

#endif//__tGunWeapon__
