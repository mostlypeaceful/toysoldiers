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

	void tSolidColorGeometry::fResetDeviceObjects( const tSolidColorGeometry& clone )
	{
		fResetDeviceObjects( tDevicePtr( clone.fDevice( ) ), clone.mMaterial, clone.fCurrentState( ).mGeometry.mBuffer, clone.fCurrentState( ).mIndices.mBuffer );
	}

}}

