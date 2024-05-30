#ifndef __tGlobalLightDirGizmoGeometry__
#define __tGlobalLightDirGizmoGeometry__
#include "tGizmoGeometry.hpp"

namespace Sig
{
	class toolsgui_export tGlobalLightDirGizmoGeometry : public tGizmoGeometry
	{
	public:
		tGlobalLightDirGizmoGeometry( 
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
	private:
		void fGenerate( );
	};
}

#endif//__tGlobalLightDirGizmoGeometry__

