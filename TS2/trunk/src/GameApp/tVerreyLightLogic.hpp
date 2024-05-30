#ifndef __tVerreyLightLogic__
#define __tVerreyLightLogic__
#include "tShellLogic.hpp"

namespace Sig
{
	class tVerreyLightLogic : public tLogic
	{
		define_dynamic_cast( tVerreyLightLogic, tLogic );
	public:
		tVerreyLightLogic( );
		virtual ~tVerreyLightLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );

		virtual void fActST( f32 dt );
		virtual void fMoveST( f32 dt );

	public:
		static void fSpawn( const Math::tVec3f& origin );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		tShellPhysics mPhysics;
		f32 mDeathHeight;
		Audio::tSourcePtr mAudioSource;

	};

}

#endif//__tVerreyLightLogic__