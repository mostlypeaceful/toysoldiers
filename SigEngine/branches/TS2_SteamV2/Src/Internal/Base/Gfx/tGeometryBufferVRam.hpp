#ifndef __tGeometryBufferVRam__
#define __tGeometryBufferVRam__
#include "tVertexFormatVRam.hpp"

namespace Sig { namespace Gfx
{
	class tDevice;
	class tGeometryBufferVRamDeviceResource;

	///
	/// \brief Encapsulates a vram vertex buffer object.
	class base_export tGeometryBufferVRam
	{
		declare_reflector( );
		friend class tGeometryBufferVRamDeviceResource;
	public:
		enum tAllocFlags
		{
			cAllocDynamic = (1<<0), ///< Hint that the buffer will be locked/unlocked frequently for writes
		};

	public:
		typedef Sig::byte* tPlatformHandle;
	protected:
		tVertexFormatVRam		mFormat;
		u32						mNumVerts;
		u32						mAllocFlags;
		tPlatformHandle			mPlatformHandle;
		tPlatformHandle			mDeviceResource;
		Sig::byte*				mPermaLockAddress;

	public:
		tGeometryBufferVRam( );
		tGeometryBufferVRam( tNoOpTag );
		~tGeometryBufferVRam( );
		tGeometryBufferVRam( const tGeometryBufferVRam& other );
		tGeometryBufferVRam& operator=( const tGeometryBufferVRam& other );

		///
		/// \brief Allocate the vram vertex buffer.
		void fAllocate( const tDevicePtr& device, const tVertexFormat& format, u32 numVerts, u32 allocFlags );

		///
		/// \brief "Fake" allocate so that we can save to binary load-in-place stream.
		void fPseudoAllocate( const tVertexFormat& format, u32 numVerts, u32 allocFlags  );

		///
		/// \brief Allocate the vram vertex buffer following a load-in-place operation. If you don't know what
		/// this means, then you shouldn't be using it.
		void fAllocateInPlace( const tDevicePtr& device, const Sig::byte* verts );

		///
		/// \brief Relocate the vram vertex buffer following a load-in-place operation. If you don't know what
		/// this means, then you shouldn't be using it.
		void fRelocateInPlace( ptrdiff_t delta );

		///
		/// \brief Deallocate the vram vertex buffer.
		void fDeallocate( );

		///
		/// \brief Deallocate the vram vertex buffer prior to an "unload-in-place". If you don't know what
		/// this means, then you shouldn't be using it.
		void fDeallocateInPlace( );

		///
		/// \brief Access the buffer's format object.
		inline const tVertexFormatVRam& fVertexFormat( ) const { return mFormat; }

		///
		/// \brief Query for the number of vertices.
		inline u32 fVertexCount( ) const { return mNumVerts; }

		///
		/// \brief Query the underlying buffer size in bytes.
		inline u32 fBufferSize( ) const { return mNumVerts * mFormat.fVertexSize( ); }

		///
		/// \brief Access the platform-specific geometry buffer handle
		inline tPlatformHandle fPlatformHandle( ) const { return mPlatformHandle; }

		///
		/// \brief Copy data to gpu
		void fBufferData( const void* data, u32 numVerts, u32 vertOffset = 0 );

		///
		/// \brief Lock a region of the vertex buffer. Defaults to locking the whole thing.
		/// \todo accept lock flags (discard, read-only, etc)
		Sig::byte* fDeepLock( );
		Sig::byte* fQuickLock( u32 startVertex = 0, u32 numVerts = ~0 );

		///
		/// \brief Unlock a previously locked region of the vertex buffer. You should pass
		/// the same pointer that was returned by a previous call to fLock.
		void fDeepUnlock( );
		void fQuickUnlock( Sig::byte* region );

		///
		/// \brief Applies the geometry buffer as the currently active rendering buffer.
		void fApply( const tDevicePtr& device ) const;

	private:
		
		void fCreateDeviceResource( tDevice* device );
		void fDestroyDeviceResource( );
		void fAllocateInternal( const tDevicePtr& device, const Sig::byte* verts = 0 );
		void fDeallocateInternal( );
	};

}}


#endif//__tGeometryBufferVRam__

