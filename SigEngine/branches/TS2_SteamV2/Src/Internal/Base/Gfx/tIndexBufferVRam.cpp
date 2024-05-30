#include "BasePch.hpp"
#include "tIndexBufferVRam.hpp"
#include "tDevice.hpp"
#include "tDeviceResource.hpp"

namespace Sig { namespace Gfx
{

	///
	/// \brief Facilitates automatic reset of resource on device loss/reset.
	class base_export tIndexBufferVRamDeviceResource : public tDeviceResource
	{
		define_class_pool_new_delete( tIndexBufferVRamDeviceResource, 128 );
	private:
		tIndexBufferVRam* mBuffer;
	public:
		tIndexBufferVRamDeviceResource( tDevice* device, tIndexBufferVRam* buffer )
			: mBuffer( buffer )
		{
			fRegisterWithDevice( device );
		}
		virtual void fOnDeviceLost( tDevice* device )
		{
			mBuffer->fDeallocateInternal( );
		}
		virtual void fOnDeviceReset( tDevice* device )
		{
			mBuffer->fAllocateInternal( tDevicePtr( device ) );
		}
	};


	tIndexBufferVRam::tIndexBufferVRam( )
		: mNumIndices( 0 )
		, mNumPrimitives( 0 )
		, mAllocFlags( 0 )
		, mPlatformHandle( 0 )
		, mDeviceResource( 0 )
		, mPermaLockAddress( 0 )
	{
	}

	tIndexBufferVRam::tIndexBufferVRam( tNoOpTag )
		: mFormat( cNoOpTag )
	{
	}

	tIndexBufferVRam::~tIndexBufferVRam( )
	{
		fDeallocate( );
	}

	tIndexBufferVRam::tIndexBufferVRam( const tIndexBufferVRam& other )
	{
		sigassert( !"not allowed" );
	}

	tIndexBufferVRam& tIndexBufferVRam::operator=( const tIndexBufferVRam& other )
	{
		sigassert( !"not allowed" );
		return *this;
	}

	void tIndexBufferVRam::fAllocate( const tDevicePtr& device, const tIndexFormat& format, u32 numIndices, u32 numPrimitives, u32 allocFlags )
	{
		if( allocFlags & cAllocDynamic )
			fCreateDeviceResource( device.fGetRawPtr( ) );

		fPseudoAllocate( format, numIndices, numPrimitives, allocFlags );

		// perform platform-specific allocation
		fAllocateInternal( device );
	}

	void tIndexBufferVRam::fPseudoAllocate( const tIndexFormat& format, u32 numIndices, u32 numPrimitives, u32 allocFlags )
	{
		fDeallocate( );

		mNumIndices = numIndices;
		mNumPrimitives = numPrimitives;
		mAllocFlags = allocFlags;
		mFormat = format;
	}

	void tIndexBufferVRam::fAllocateInPlace( const tDevicePtr& device, const Sig::byte* verts )
	{
		sigassert( !( mAllocFlags & cAllocDynamic ) );

		// perform platform-specific allocation
		fAllocateInternal( device, verts );
	}

	void tIndexBufferVRam::fDeallocate( )
	{
		if( mAllocFlags & cAllocDynamic ) fDestroyDeviceResource( );

		// perform platform-specific deallocation
		fDeallocateInternal( );

		mFormat = tIndexFormat( );
		mNumIndices = 0;
		mNumPrimitives = 0;
		mAllocFlags = 0;
		mPlatformHandle = 0;
		mPermaLockAddress = 0;
	}

	void tIndexBufferVRam::fDeallocateInPlace( )
	{
		sigassert( !( mAllocFlags & cAllocDynamic ) );

		// perform platform-specific deallocation
		fDeallocateInternal( );

		mFormat = tIndexFormat( );
		mNumIndices = 0;
		mNumPrimitives = 0;
		mAllocFlags = 0;
		mPlatformHandle = 0;
		mPermaLockAddress = 0;
	}

	void tIndexBufferVRam::fCreateDeviceResource( tDevice* device )
	{
		if( device->fRequiresDeviceReset( ) )
		{
			sigassert( !mDeviceResource );
			mDeviceResource = ( tPlatformHandle )(new tIndexBufferVRamDeviceResource( device, this ));
		}
	}

	void tIndexBufferVRam::fDestroyDeviceResource( )
	{
		if( mDeviceResource )
		{
			delete ( tIndexBufferVRamDeviceResource* )mDeviceResource;
			mDeviceResource = 0;
		}
	}

}}
