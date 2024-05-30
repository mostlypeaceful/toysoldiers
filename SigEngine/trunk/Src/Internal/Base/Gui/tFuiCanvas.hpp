#ifndef __tFuiCanvas__
#define __tFuiCanvas__

#include "tRenderableCanvas.hpp"
#include "Fui.hpp"

namespace Sig
{
namespace Gui
{
	class base_export tFuiCanvas : public tRenderableCanvas
	{
		debug_watch( tFuiCanvas );
		define_dynamic_cast( tFuiCanvas, tRenderableCanvas );
	public:
		tFuiCanvas( );
		~tFuiCanvas( );

		virtual void fOnTickCanvas( f32 dt );
		virtual void fOnRenderCanvas( Gfx::tScreen& screen ) const;

		Fui::tFuiPtr fMovie( ) const { return mMovie; }
		void fLoadAndPlayMovie( const tFilePathPtr& filename );
		void fUnloadMovie( );
		
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		Fui::tFuiPtr mMovie;
	};
	typedef tRefCounterPtr<tFuiCanvas> tFuiCanvasPtr;
}
}

#endif//__tFuiCanvas__
