#ifndef __tFocusCamera__
#define __tFocusCamera__
#include "tUseUnitCamera.hpp"
#include "Math/tDamped.hpp"

namespace Sig
{

	class base_export tFocusCamera : public tUseUnitCamera
	{
		define_dynamic_cast( tFocusCamera, tUseUnitCamera );
	public:
		explicit tFocusCamera( tPlayer& player, const tEntityPtr target, f32 duration, f32 blendIn );
		//virtual b32 fWantsAutoPop( ) const;

		u32& fWaitCount( ) { return mWaitCount; }
		b32  fHasChanged( ) const { return mHasChanged; }
		void fChangeTarget( tEntity* ent );

	protected:
		virtual void fOnActivate( b32 active );
		virtual void fUserTick( f32 dt, Gfx::tTripod& tripod );
		virtual void fUserBlendIn( f32 dt, Gfx::tTripod& tripod );

		tEntityPtr mTarget;
		Gfx::tTripod mOriginalTripod;

		f32 mTimer;
		b32 mHasChanged;
		u32 mWaitCount;
	};
}


#endif

