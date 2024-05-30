#ifndef __tRotationGizmoGeometry__
#define __tRotationGizmoGeometry__
#include "tGizmoGeometry.hpp"

namespace Sig
{
	class toolsgui_export tRotationGizmoGeometry : public tGizmoGeometry
	{
	public:
		tRotationGizmoGeometry( 
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
	private:
		void fGenerate( );
	};
}

#endif//__tRotationGizmoGeometry__
