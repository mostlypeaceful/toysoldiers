#include "BasePch.hpp"
#include "tWorldSpaceLines.hpp"
#include "tSolidColorMaterial.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	tWorldSpaceLines::tWorldSpaceLines( )
		: mStrip( false )
	{
	}


	void tWorldSpaceLines::fOnDeviceLost( tDevice* device )
	{
		// do nothing
	}

	void tWorldSpaceLines::fOnDeviceReset( tDevice* device )
	{
		fSetGeometryInternal( mSysMemVerts.fBegin( ), mSysMemVerts.fCount( ), mStrip );
	}

	void tWorldSpaceLines::fResetDeviceObjects( 
		const tDevicePtr& device,
		const tMaterialPtr& material, 
		const tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const tIndexBufferVRamAllocatorPtr& indexAllocator )
	{
		mGeometry.fResetDeviceObjects( device, material, geometryAllocator, indexAllocator );

		fSetRenderBatch( tRenderBatchPtr( ) );

		fRegisterWithDevice( device.fGetRawPtr( ) );
	}

	void tWorldSpaceLines::fSetGeometry( tGrowableArray<tSolidColorRenderVertex>& solidColorVerts, b32 strip )
	{
		mSysMemVerts.fSwap( solidColorVerts );
		fSetGeometryInternal( mSysMemVerts.fBegin( ), mSysMemVerts.fCount( ), strip );
	}

	void tWorldSpaceLines::fSetGeometryInternal( const tSolidColorRenderVertex* solidColorVerts, u32 numVerts, b32 strip )
	{
		mStrip = strip;

		Math::tAabbf box;
		box.fInvalidate( );
		for( u32 i = 0; i < numVerts; ++i )
			box |= solidColorVerts[ i ].mP;
		if( numVerts == 0 )
			box |= Math::tVec3f::cZeroVector;
		fSetObjectSpaceBox( box );

		mGeometry.fBake( ( Sig::byte* )solidColorVerts, numVerts, strip );
		fSetRenderBatch( mGeometry.fRenderBatch( ) );
	}

}}

