#ifndef __tCannonWeapon__
#define __tCannonWeapon__
#include "tWeapon.hpp"
#include "tWorldSpaceArc.hpp"
#include "tSgFileRefEntity.hpp"

namespace Sig
{

	class tCannonWeapon : public tWeapon
	{
	public:
		explicit tCannonWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst );

		virtual void fOnDelete( );
		virtual void fProcessST( f32 dt );
		virtual void fProcessMT( f32 dt );
		virtual void fBeginRendering( tPlayer* player );
		virtual void fEndRendering( );

	private:
		void fUpdateUI( f32 dt );
		void fReallyEndRendering( );

	protected:
		virtual void fComputeIdealAngle( ) { fComputeIdealAngleArc( ); }
		virtual f32 fEstimateTimeToImpact( const Math::tVec3f& pos ) const { return fEstimateTimeToImpactArc( pos ); }

	private:
		tWorldSpaceArcPtr mTargetingLines;
		tSgFileRefEntityPtr mTargetVisual;

		f32 mTargetingAlpha;
		f32 mLastUIArcTime;
		b8 mRenderUI;
		b8 mRenderFadeOut;
		b8 pad1;
		b8 pad2;

		f32 mBlinkTimer;
		f32 mBlinkIntensity;
	};

}

#endif//__tCannonWeapon__
