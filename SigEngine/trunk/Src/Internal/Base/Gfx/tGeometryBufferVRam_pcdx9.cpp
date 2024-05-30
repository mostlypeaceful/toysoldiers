#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tGeometryBufferVRam.hpp"
#include "tDevice.hpp"
#ifdef target_game
#include "tGameAppBase.hpp"
#endif

namespace Sig { namespace Gfx
{

	void tGeometryBufferVRam::fAllocateInternal( const tDevicePtr& device, const Sig::byte* verts )
	{
		sigassert( !device.fNull( ) );

		IDirect3DDevice9* d3ddev = device->fGetDevice( );

		// first create the vertex buffer

		const u32 bufferSize = fBufferSize( );

		IDirect3DVertexBuffer9* dxvb = 0;

		d3ddev->CreateVertexBuffer(
			bufferSize,
			( mAllocFlags & cAllocDynamic ) ? ( D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY ) : 0,
			0,
			( mAllocFlags & cAllocDynamic ) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
			&dxvb,
			0 );

		sigassert( dxvb );

		mPlatformHandle = ( tPlatformHandle )dxvb;
		mPermaLockAddress = NULL;

		if( verts )
			fBufferData( verts, mNumVerts );
	}

	void tGeometryBufferVRam::fDeallocateInternal( tDeallocFlags deallocFlags )
	{
		const b32 async = deallocFlags & cDeallocAsync;
		if( async )
			log_warning_once( "tGeometryBufferVRam::fDeallocateInternal: cDeallocAsync not (yet) implemented for PC D3D9" );

		IDirect3DVertexBuffer9* dxvb = ( IDirect3DVertexBuffer9* )mPlatformHandle;
		if( dxvb )
		{
			if( mPermaLockAddress )
				dxvb->Unlock( );
			dxvb->Release( );
			mPlatformHandle = NULL;
		}
	}

	void tGeometryBufferVRam::fBufferData( const void* data, u32 numVerts, u32 vertOffset )
	{
		IDirect3DVertexBuffer9* dxvb = ( IDirect3DVertexBuffer9* )mPlatformHandle;

		Sig::byte* dxmem = 0;
		dxvb->Lock( 0, 0, ( void** )&dxmem, 0 );

		fMemCpyToGpu( dxmem + vertOffset * mFormat.fVertexSize( ), data, numVerts * mFormat.fVertexSize( ) );

		dxvb->Unlock( );
	}

	Sig::byte* tGeometryBufferVRam::fDeepLock( )
	{
		if( mPlatformHandle && !mPermaLockAddress )
		{
			IDirect3DVertexBuffer9* dxvb = ( IDirect3DVertexBuffer9* )mPlatformHandle;
			void* dxmem = 0;
			dxvb->Lock( 0, 0, &dxmem, 0 );
			mPermaLockAddress = ( Sig::byte* )dxmem;
		}

		return mPermaLockAddress;
	}

	Sig::byte* tGeometryBufferVRam::fQuickLock( u32 startVertex, u32 numVerts )
	{
#ifdef target_tools
		// The permalock address gets lost after the reset.
		if( mPlatformHandle && !mPermaLockAddress )
		{
			IDirect3DVertexBuffer9* dxvb = ( IDirect3DVertexBuffer9* )mPlatformHandle;
			void* dxmem = 0;
			dxvb->Lock( 0, 0, &dxmem, 0 );
			mPermaLockAddress = ( Sig::byte* )dxmem;
		}
#endif

		sigassert( mPermaLockAddress );
		return mPermaLockAddress + startVertex * mFormat.fVertexSize( );
	}

	void tGeometryBufferVRam::fDeepUnlock( )
	{
		if( mPlatformHandle && mPermaLockAddress )
		{
			IDirect3DVertexBuffer9* dxvb = ( IDirect3DVertexBuffer9* )mPlatformHandle;
			dxvb->Unlock( );
			mPermaLockAddress = 0;
		}
	}

	void tGeometryBufferVRam::fQuickUnlock( Sig::byte* region )
	{
		// nothing to do
		sigassert( mPermaLockAddress );
	}

	void tGeometryBufferVRam::fApply( const tDevicePtr& device ) const
	{
		IDirect3DVertexBuffer9* dxvb = ( IDirect3DVertexBuffer9* )mPlatformHandle;
		sigassert( dxvb );
		device->fGetDevice( )->SetStreamSource( mStream, dxvb, 0, mFormat.fVertexSize( ) );
	}

}}
#endif//#if defined( platform_pcdx9 )
