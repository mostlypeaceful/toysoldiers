#ifndef __tSingleVectorGizmoGeometry__
#define __tSingleVectorGizmoGeometry__
#include "tGizmoGeometry.hpp"

namespace Sig
{
	class toolsgui_export tSingleVectorGizmoGeometry : public tGizmoGeometry
	{
	public:
		tSingleVectorGizmoGeometry( 
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
	private:
		void fGenerate( );
	};
}

#endif//__tSingleVectorGizmoGeometry__

