#ifndef __tMouseFollowGizmoGeometry__
#define __tMouseFollowGizmoGeometry__
#include "tGizmoGeometry.hpp"

namespace Sig
{
	class toolsgui_export tMouseFollowGizmoGeometry : public tGizmoGeometry
	{
	public:
		tMouseFollowGizmoGeometry( 
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
	private:
		void fGenerate( );
	};
}

#endif//__tMouseFollowGizmoGeometry__

