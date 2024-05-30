#ifndef __tScaleGizmoGeometry__
#define __tScaleGizmoGeometry__
#include "tGizmoGeometry.hpp"

namespace Sig
{

	class toolsgui_export tScaleGizmoGeometry : public tGizmoGeometry
	{
	public:
		tScaleGizmoGeometry( 
			const Gfx::tDevicePtr& device,
			const Gfx::tMaterialPtr& material, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );
		virtual void fOnDeviceReset( Gfx::tDevice* device );
	private:
		void fGenerate( );
		void fAddBoxFace( 
			const Math::tVec3f& v0, const Math::tVec3f& v1, const Math::tVec3f& v2, const Math::tVec3f& v3,
			tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
			tGrowableArray< u16 >& ids,
			u32 iaxis );
		void fAddBoxFace( 
			const Math::tVec3f& v0, const Math::tVec3f& v1, const Math::tVec3f& v2, const Math::tVec3f& v3,
			tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
			tGrowableArray< u16 >& ids,
			const u32 vtxColor,
			tTriangleArray& triArray );
	};

}

#endif//__tScaleGizmoGeometry__
