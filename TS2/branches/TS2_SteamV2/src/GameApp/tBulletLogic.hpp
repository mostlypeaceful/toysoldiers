#ifndef __tBulletLogic__
#define __tBulletLogic__

#include "tProjectileLogic.hpp"

namespace Sig
{

	class tBulletLogic : public tProjectileLogic
	{
		define_dynamic_cast( tBulletLogic, tProjectileLogic );
	public:
		tBulletLogic( );
	protected:
		virtual void fComputeNewPosition( f32 dt );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};

}

#endif//__tBulletLogic__
