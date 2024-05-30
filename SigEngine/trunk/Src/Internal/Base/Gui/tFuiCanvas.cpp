#include "BasePch.hpp"
#include "tFuiCanvas.hpp"
#include "Gfx/tScreen.hpp"
#include "Gfx/tRenderBatch.hpp"

namespace Sig
{
namespace Gui
{
	tFuiCanvas::tFuiCanvas( )
	{
		// Create a dummy RenderBatch to allow us to successfully submit draw calls despite having no geometry.
		Gfx::tRenderBatchData batchData;
		batchData.mRenderState = &Gfx::tRenderState::cDefaultColorOpaque;
		batchData.mBehaviorFlags |= Gfx::tRenderBatchData::cBehaviorDummy;
		fSetRenderBatch( Gfx::tRenderBatch::fCreate( batchData ) );
	}

	tFuiCanvas::~tFuiCanvas( )
	{
		fUnloadMovie( );
	}

	void tFuiCanvas::fOnTickCanvas( f32 dt )
	{
		tRenderableCanvas::fOnTickCanvas( dt );
	}

	void tFuiCanvas::fOnRenderCanvas( Gfx::tScreen& screen ) const
	{
		if( fDisabled( ) || fInvisible( ) || !mMovie )
			return;
		
		screen.fAddScreenSpaceDrawCall( fDrawCall( ) );
	}

	void tFuiCanvas::fLoadAndPlayMovie( const tFilePathPtr& filename )
	{
		fUnloadMovie( );
		mMovie = Fui::tFuiSystem::fInstance( ).fLoadAndPlay( filename );
	}

	void tFuiCanvas::fUnloadMovie( )
	{
		if( mMovie )
		{
			mMovie->fEnd( );
			mMovie.fRelease( );
		}
	}
}
}

namespace Sig
{
namespace Gui
{
	void tFuiCanvas::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tFuiCanvas, tRenderableCanvas, Sqrat::NoCopy<tFuiCanvas> > classDesc( vm.fSq( ) );
		classDesc
			.Func( _SC("LoadAndPlayMovie"), &tFuiCanvas::fLoadAndPlayMovie )
			.Func( _SC("UnloadMovie"),      &tFuiCanvas::fUnloadMovie )
			;
		vm.fNamespace( _SC("Gui") ).Bind( _SC("FuiCanvas"), classDesc );
	}
}
}
