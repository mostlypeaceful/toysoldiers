#include "BasePch.hpp"
#include "tGeometryBufferVRam.hpp"
#include "tDeviceResource.hpp"
#include "tDevice.hpp"
#include "tGloballyAccessible.hpp"
#include "tGeometryBufferVRamSlice.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \brief Facilitates automatic reset of resource on device loss/reset.
	class base_export tGeometryBufferVRamDeviceResource : public tDeviceResource
	{
		define_class_pool_new_delete( tGeometryBufferVRamDeviceResource, 128 );
	private:
		tGeometryBufferVRam* mBuffer;
	public:
		tGeometryBufferVRamDeviceResource( tDevice* device, tGeometryBufferVRam* buffer )
			: mBuffer( buffer )
		{
			fRegisterWithDevice( device );
		}
		virtual void fOnDeviceLost( tDevice* device )
		{
			mBuffer->fDeallocateInternal( Gfx::tGeometryBufferVRam::cDeallocDefault );
		}
		virtual void fOnDeviceReset( tDevice* device )
		{
			mBuffer->fAllocateInternal( tDevicePtr( device ) );
			mBuffer->fDeepLock( );
		}
	};

	base_export void fPreinitGeometryBufferVRamDeviceResourcePool( )
	{
		// Singleton whack-a-mole...
		tGeometryBufferVRamAllocator::fGlobalList( );
		tGeometryBufferVRamDeviceResource::fClassPool( );
	}



	tGeometryBufferVRam::tGeometryBufferVRam( u32 stream )
		: mStream( stream )
		, mNumVerts( 0 )
		, mNumInstances( 0 )
		, mAllocFlags( 0 )
		, mPlatformHandle( 0 )
		, mDeviceResource( 0 )
		, mPermaLockAddress( 0 )
	{
	}

	tGeometryBufferVRam::tGeometryBufferVRam( tNoOpTag )
		: mFormat( cNoOpTag )
	{
	}

	tGeometryBufferVRam::~tGeometryBufferVRam( )
	{
		fDeallocate( );
	}

	tGeometryBufferVRam::tGeometryBufferVRam( const tGeometryBufferVRam& other )
	{
		sigassert( !"not allowed" );
	}

	tGeometryBufferVRam& tGeometryBufferVRam::operator=( const tGeometryBufferVRam& other )
	{
		sigassert( !"not allowed" );
		return *this;
	}

	void tGeometryBufferVRam::fAllocate( const tDevicePtr& device, const tVertexFormat& format, u32 numVerts, u32 allocFlags )
	{
		if( allocFlags & cAllocDynamic )
			fCreateDeviceResource( device.fGetRawPtr( ) );

		fPseudoAllocate( format, numVerts, allocFlags );

		// perform platform-specific allocation
		mFormat.fAllocate( device, format );
		fAllocateInternal( device );
	}

	void tGeometryBufferVRam::fPseudoAllocate( const tVertexFormat& format, u32 numVerts, u32 allocFlags  )
	{
		fDeallocate( );

		mNumVerts = numVerts;
		mAllocFlags = allocFlags;
		mFormat.fPseudoAllocate( format );
	}

	void tGeometryBufferVRam::fAllocateInPlace( const tDevicePtr& device, const Sig::byte* verts )
	{
		sigassert( !( mAllocFlags & cAllocDynamic ) );

		// allocate vram vertex format
		mFormat.fAllocateInPlace( device );

		// perform platform-specific allocation
		fAllocateInternal( device, verts );
	}

	void tGeometryBufferVRam::fRelocateInPlace( ptrdiff_t delta )
	{
		mFormat.fRelocateInPlace( delta );
	}

	void tGeometryBufferVRam::fDeallocate( tDeallocFlags deallocFlags )
	{
		if( mAllocFlags & cAllocDynamic )
			fDestroyDeviceResource( );

		// perform platform-specific deallocation
		fDeallocateInternal( deallocFlags );

		// deallocate vram vertex format
		mFormat.fDeallocate( );

		mNumVerts = 0;
		mAllocFlags = 0;
		mPlatformHandle = 0;
		mPermaLockAddress = 0;
	}

	void tGeometryBufferVRam::fDeallocateInPlace( )
	{
		sigassert( !( mAllocFlags & cAllocDynamic ) );

		// perform platform-specific deallocation
		fDeallocateInternal( cDeallocDefault );

		// deallocate vram vertex format in place
		mFormat.fDeallocateInPlace( );

		// Only reset data that isn't LIP
		//mNumVerts = 0;
		//mAllocFlags = 0;
		mPlatformHandle = 0;
		mPermaLockAddress = 0;
	}

	void tGeometryBufferVRam::fCreateDeviceResource( tDevice* device )
	{
		if( device->fRequiresDeviceReset( ) )
		{
			sigassert( !mDeviceResource );
			mDeviceResource = ( tPlatformHandle )(NEW tGeometryBufferVRamDeviceResource( device, this ));
		}
	}

	void tGeometryBufferVRam::fDestroyDeviceResource( )
	{
		if( mDeviceResource )
		{
			delete ( tGeometryBufferVRamDeviceResource* )mDeviceResource;
			mDeviceResource = 0;
		}
	}

	u32 tGeometryBufferVRam::fStream( ) const
	{
		return mStream;
	}
}}

