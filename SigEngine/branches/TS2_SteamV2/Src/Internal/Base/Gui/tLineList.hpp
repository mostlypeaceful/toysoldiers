#ifndef __tLineList__
#define __tLineList__
#include "Gui/tRenderableCanvas.hpp"
#include "Gfx/tSolidColorGeometry.hpp"

namespace Sig { namespace Gfx
{
	struct tDefaultAllocators;
}} //Sig::Gfx

namespace Sig { namespace Gui
{
	class tLineList : public tRenderableCanvas
	{
		define_dynamic_cast( tLineList, tRenderableCanvas );
	public:
		void fSetGeometry( tGrowableArray<Gfx::tSolidColorRenderVertex>& solidColorVerts );
	private:
		void fSetGeometryInternal( const Gfx::tSolidColorRenderVertex* solidColorVerts, u32 numVerts );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		Gfx::tSolidColorGeometry mGeometry;
		tGrowableArray<Gfx::tSolidColorRenderVertex> mSysMemVerts;
	};

	typedef tRefCounterPtr<tLineList> tLineListPtr;

}}//Sig::Gui

#endif//__tLineList__
