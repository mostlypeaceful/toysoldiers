#include "ToolsGuiPch.hpp"
#include "tMouseFollowGizmoGeometry.hpp"

namespace Sig
{
	tMouseFollowGizmoGeometry::tMouseFollowGizmoGeometry( 
		const Gfx::tDevicePtr& device,
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		fResetDeviceObjects( device, material, geometryAllocator, indexAllocator );
		fGenerate( );
	}

	void tMouseFollowGizmoGeometry::fOnDeviceReset( Gfx::tDevice* device )
	{
		fGenerate( );
	}

	void tMouseFollowGizmoGeometry::fGenerate( )
	{
		tGrowableArray< Gfx::tSolidColorRenderVertex > verts;
		tGrowableArray< u16 > ids;
		fBake( verts, ids );
	}

}

