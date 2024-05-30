#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tIndexBufferVRam.hpp"
#include "tGeometryBufferVRam.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		D3DFORMAT fConvertIndexFormat( const tIndexFormat& format )
		{
			D3DFORMAT o = D3DFORMAT( 0 );
			switch( format.mStorageType )
			{
			case tIndexFormat::cStorageU16: o = D3DFMT_INDEX16; break;
			case tIndexFormat::cStorageU32: o = D3DFMT_INDEX32; break;
			default: sigassert( !"invalid index format" ); break;
			}
			return o;
		}

		D3DPRIMITIVETYPE fConvertPrimitiveType( tIndexFormat::tPrimitiveType primType )
		{
			// unsupported d3d types
			//D3DPT_POINTLIST;
			//D3DPT_TRIANGLEFAN;

			D3DPRIMITIVETYPE o;

			switch( primType )
			{
			case tIndexFormat::cPrimitiveLineList:			o = D3DPT_LINELIST; break;
			case tIndexFormat::cPrimitiveLineStrip:			o = D3DPT_LINESTRIP; break;
			case tIndexFormat::cPrimitiveTriangleList:		o = D3DPT_TRIANGLELIST; break;
			case tIndexFormat::cPrimitiveTriangleStrip:		o = D3DPT_TRIANGLESTRIP; break;
			default:										sigassert( !"invalid primitive type" ); break;
			}

			return o;
		}
	}

	void tIndexBufferVRam::fAllocateInternal( const tDevicePtr& device, const Sig::byte* ids )
	{
		sigassert( !device.fNull( ) );
		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		const u32 bufferSize = fBufferSize( );

		IDirect3DIndexBuffer9* dxib = 0;

		d3ddev->CreateIndexBuffer(
			bufferSize,
			( mAllocFlags & cAllocDynamic ) ? ( /*D3DUSAGE_DYNAMIC |*/ D3DUSAGE_WRITEONLY ) : 0,
			fConvertIndexFormat( mFormat ),
			( mAllocFlags & cAllocDynamic ) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
			&dxib,
			0 );

		mPlatformHandle = ( tPlatformHandle )dxib;

		if( ids )
			fBufferData( ids, mNumIndices );
	}

	void tIndexBufferVRam::fDeallocateInternal( )
	{
		IDirect3DIndexBuffer9* dxib = ( IDirect3DIndexBuffer9* )mPlatformHandle;
		if( dxib )
		{
			if( mPermaLockAddress )
				dxib->Unlock( );
			dxib->Release( );
		}
	}

	void tIndexBufferVRam::fBufferData( const void* data, u32 numIndices, u32 indexOffset )
	{
		IDirect3DIndexBuffer9* dxib = ( IDirect3DIndexBuffer9* )mPlatformHandle;

		Sig::byte* dxmem = 0;
		dxib->Lock( 0, 0, ( void** )&dxmem, 0 );

		fMemCpyToGpu( dxmem + indexOffset * mFormat.mSize, data, numIndices * mFormat.mSize );

		dxib->Unlock( );
	}

	Sig::byte* tIndexBufferVRam::fDeepLock( )
	{
		if( mPlatformHandle && !mPermaLockAddress )
		{
			IDirect3DIndexBuffer9* dxib = ( IDirect3DIndexBuffer9* )mPlatformHandle;
			void* dxmem = 0;
			dxib->Lock( 0, 0, &dxmem, 0 );
			mPermaLockAddress = ( Sig::byte* )dxmem;
		}

		return mPermaLockAddress;
	}

	Sig::byte* tIndexBufferVRam::fQuickLock( u32 startIndex, u32 numIndices )
	{
		sigassert( mPermaLockAddress );
		return mPermaLockAddress + startIndex * mFormat.mSize;
	}

	void tIndexBufferVRam::fDeepUnlock( )
	{
		if( mPlatformHandle && mPermaLockAddress )
		{
			IDirect3DIndexBuffer9* dxib = ( IDirect3DIndexBuffer9* )mPlatformHandle;
			dxib->Unlock( );
			mPermaLockAddress = 0;
		}
	}

	void tIndexBufferVRam::fQuickUnlock( Sig::byte* region )
	{
		// nothing to do
		sigassert( mPermaLockAddress );
	}

	void tIndexBufferVRam::fApply( const tDevicePtr& device ) const
	{
		IDirect3DIndexBuffer9* dxib = ( IDirect3DIndexBuffer9* )mPlatformHandle;
		sigassert( dxib );
		device->fGetDevice( )->SetIndices( dxib );
	}

	void tIndexBufferVRam::fRender( const tDevicePtr& device, u32 vertexCount, u32 baseVertexIndex, u32 primCount, u32 baseIndexIndex, tIndexFormat::tPrimitiveType primType ) const
	{
		if( primCount > 0 )
			device->fGetDevice( )->DrawIndexedPrimitive( 
				fConvertPrimitiveType( primType ),
				baseVertexIndex,
				0,
				vertexCount,
				baseIndexIndex,
				primCount );
	}

}}
#endif//#if defined( platform_xbox360 )


