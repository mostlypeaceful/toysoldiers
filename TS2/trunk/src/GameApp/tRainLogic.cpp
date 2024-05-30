#include "GameAppPch.hpp"
#include "tRainLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "Gfx/tCamera.hpp"
using namespace Sig::Math;

namespace Sig
{
	tRainLogic::tRainLogic( )
	{
	}
	tRainLogic::~tRainLogic( )
	{
	}
	void tRainLogic::fOnSpawn( )
	{
		fOnPause( false );
		tLogic::fOnSpawn( );
	}
	void tRainLogic::fOnDelete( )
	{
		tLogic::fOnDelete( );
	}
	void tRainLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
		}
		else
		{
			// add self to run lists
			fRunListInsert( cRunListActST );
		}
	}
	
	void tRainLogic::fActST( f32 dt )
	{
		const Gfx::tCamera& cam = tApplication::fInstance( ).fLocalUsers( )[ 0 ]->fViewport( )->fRenderCamera( );
		fOwnerEntity( )->fMoveTo( cam.fLocalToWorld( ).fGetTranslation( ) );
		tLogic::fActST( dt );
	}
}


namespace Sig
{
	void tRainLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tRainLogic, tLogic, Sqrat::NoCopy<tRainLogic> > classDesc( vm.fSq( ) );
		classDesc
			;

		vm.fRootTable( ).Bind(_SC("RainLogic"), classDesc);
	}
}

