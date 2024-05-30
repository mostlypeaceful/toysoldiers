#include "BasePch.hpp"
#if defined( platform_ios )
#include "tGeometryBufferVRam.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{

	void tGeometryBufferVRam::fAllocateInternal( const tDevicePtr& device, const Sig::byte* verts )
	{
		sigassert( !device.fNull( ) );
		
		// first create the vertex buffer
		
		const u32 bufferSize = fBufferSize( );
		
		GLuint glvb = 0;

		glGenBuffers( 1, &glvb );
		
		glBindBuffer( GL_ARRAY_BUFFER, glvb );
		
		//		d3ddev->CreateVertexBuffer(
		//								   bufferSize,
		//								   ( mAllocFlags & cAllocDynamic ) ? ( D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY ) : 0,
		//								   0,
		//								   ( mAllocFlags & cAllocDynamic ) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		//								   &dxvb,
		//								   0 );
		
		glBufferData( GL_ARRAY_BUFFER, bufferSize, 0/*vertices*/, GL_STATIC_DRAW );
		
		mPlatformHandle = ( tPlatformHandle )glvb;
	}

	void tGeometryBufferVRam::fDeallocateInternal( )
	{
		if( mPlatformHandle )
		{
			if( mPermaLockAddress )
				fDeepUnlock( );
			
			GLuint glvb = ( GLuint )mPlatformHandle;
			glDeleteBuffers( 1, &glvb );
		}
	}

	void tGeometryBufferVRam::fBufferData( const void* data, u32 numVerts, u32 vertOffset )
	{
		//IDirect3DVertexBuffer9* dxvb = ( IDirect3DVertexBuffer9* )mPlatformHandle;

		//Sig::byte* dxmem = 0;
		//dxvb->Lock( 0, 0, ( void** )&dxmem, 0 );

		//fMemCpyToGpu( dxmem + vertOffset * mFormat.fVertexSize( ), data, numVerts * mFormat.fVertexSize( ) );

		//dxvb->Unlock( );
	}

	Sig::byte* tGeometryBufferVRam::fDeepLock( )
	{
		if( mPlatformHandle && !mPermaLockAddress )
		{
			GLuint glvb = ( GLuint )mPlatformHandle;
			glBindBuffer( GL_ARRAY_BUFFER, glvb );
			GLvoid* o = glMapBufferOES( GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES );
			mPermaLockAddress = ( Sig::byte* )o;
		}
		
		return mPermaLockAddress;
	}

	Sig::byte* tGeometryBufferVRam::fQuickLock( u32 startVertex, u32 numVerts )
	{
		sigassert( mPermaLockAddress );
		return mPermaLockAddress + startVertex * mFormat.fVertexSize( );
	}

	void tGeometryBufferVRam::fDeepUnlock( )
	{
		if( mPlatformHandle && mPermaLockAddress )
		{
			GLuint glvb = ( GLuint )mPlatformHandle;
			glBindBuffer( GL_ARRAY_BUFFER, glvb );
			glUnmapBufferOES( GL_ARRAY_BUFFER );
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
		GLuint glvb = ( GLuint )mPlatformHandle;
		glBindBuffer( GL_ARRAY_BUFFER, glvb );
	}

}}
#endif//#if defined( platform_ios )
