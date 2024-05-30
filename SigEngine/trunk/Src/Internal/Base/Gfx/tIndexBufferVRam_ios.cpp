#include "BasePch.hpp"
#if defined( platform_ios )
#include "tIndexBufferVRam.hpp"
#include "tGeometryBufferVRam.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	void tIndexBufferVRam::fAllocateInternal( const tDevicePtr& device, const Sig::byte* ids )
	{
		mPlatformHandle = ( tPlatformHandle )0;
	}

	void tIndexBufferVRam::fDeallocateInternal( )
	{
	}

	void tIndexBufferVRam::fBufferData( const void* data, u32 numIndices, u32 indexOffset )
	{
		//IDirect3DIndexBuffer9* dxib = ( IDirect3DIndexBuffer9* )mPlatformHandle;

		//Sig::byte* dxmem = 0;
		//dxib->Lock( 0, 0, ( void** )&dxmem, 0 );

		//fMemCpyToGpu( dxmem + indexOffset * mFormat.mSize, data, numIndices * mFormat.mSize );

		//dxib->Unlock( );
	}

	Sig::byte* tIndexBufferVRam::fDeepLock( )
	{
		return 0;
	}

	Sig::byte* tIndexBufferVRam::fQuickLock( u32 startIndex, u32 numIndices )
	{
		return 0;
	}

	void tIndexBufferVRam::fDeepUnlock( )
	{
	}

	void tIndexBufferVRam::fQuickUnlock( Sig::byte* region )
	{
	}

	void tIndexBufferVRam::fApply( const tDevicePtr& device ) const
	{
	}

	void tIndexBufferVRam::fRender( const tDevicePtr& device, const tRenderBatchData& data ) const
	{
	}

	void tIndexBufferVRam::fRenderInstanced( const tDevicePtr& device, const tRenderBatchData& data ) const
	{
		log_warning_unimplemented( );
	}

}}
#endif//#if defined( platform_ios )


