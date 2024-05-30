#include "BasePch.hpp"
#include "tSolidColorGeometry.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	tSolidColorGeometry::tSolidColorGeometry( )
		: tDynamicGeometry( tRenderBatchData::cBehaviorIgnoreStats )
	{
	}

	void tSolidColorGeometry::fResetDeviceObjects( 
		const tDevicePtr& device,
		const tMaterialPtr& material, 
		const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		mMaterial = material;
		tDynamicGeometry::fResetDeviceObjects( geometryAllocator, indexAllocator );
		fRegisterWithDevice( device.fGetRawPtr( ) );
	}

	void tSolidColorGeometry::fBake( 
		tGrowableArray< Gfx::tSolidColorRenderVertex >& verts,
		tGrowableArray< u16 >& ids, u32 numPrims )
	{
		if( !fAllocateGeometry( *mMaterial, verts.fCount( ), ids.fCount( ), numPrims ) )
			return; // couldn't get geometry

		// copy vert data to gpu
		fCopyVertsToGpu( verts.fBegin( ), verts.fCount( ) );

		// generate indices
		fCopyIndicesToGpu( ids.fBegin( ), ids.fCount( ) );
	}


}}

