#ifndef __tTranslationGizmoGeometry__
#define __tTranslationGizmoGeometry__
#include "tGizmoGeometry.hpp"

namespace Sig
{
	class toolsgui_export tTranslationGizmoGeometry : public tGizmoGeometry
	{
	public:
		tTranslationGizmoGeometry( 
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
	private:
		void fGenerate( );
	};
}

#endif//__tTranslationGizmoGeometry__

