#ifndef __tRainLogic__
#define __tRainLogic__
#include "tPlayer.hpp"
#include "tFxFileRefEntity.hpp"


namespace Sig
{

	class tRainLogic : public tLogic
	{
		define_dynamic_cast( tRainLogic, tLogic );
	public:
		tRainLogic( );
		virtual ~tRainLogic( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fActST( f32 dt );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	};

}

#endif//__tRainLogic__