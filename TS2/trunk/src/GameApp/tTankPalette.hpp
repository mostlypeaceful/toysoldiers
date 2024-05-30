#ifndef __tTankPalette__
#define __tTankPalette__
#include "tLogic.hpp"

namespace Sig
{

	class tTankPalette : public tLogic
	{
		define_dynamic_cast( tTankPalette, tLogic );
	public:
		tTankPalette( );
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		b32 fFinished( ) const { return mFired; }

		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tEntityPtr mProbe;
		f32 mInitialY;
		b32 mFired;
	};

}

#endif//__tTankPalette__
