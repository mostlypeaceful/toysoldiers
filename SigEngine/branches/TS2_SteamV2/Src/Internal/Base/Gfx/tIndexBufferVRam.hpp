#ifndef __tIndexBufferVRam__
#define __tIndexBufferVRam__
#include "tIndexFormat.hpp"

namespace Sig { namespace Gfx
{
	class tDevice;
	class tDevicePtr;
	class tGeometryBufferVRam;
	class tIndexBufferVRamDeviceResource;

	///
	/// \brief Encapsulates the vram-side of an index buffer used
	/// for rendering primitives from a geometry buffer.
	class base_export tIndexBufferVRam
	{
		declare_reflector( );
		friend class tIndexBufferVRamDeviceResource;
	public:
		enum tAllocFlags
		{
			cAllocDynamic = (1<<0), ///< Hint that the buffer will be locked/unlocked frequently for writes
		};
	public:
		typedef Sig::byte* tPlatformHandle;
	protected:
		tIndexFormat			mFormat;
		u32						mNumIndices;
		u32						mNumPrimitives;
		u32						mAllocFlags;
		tPlatformHandle			mPlatformHandle;
		tPlatformHandle			mDeviceResource;
		Sig::byte*				mPermaLockAddress;

	public:
		tIndexBufferVRam( );
		tIndexBufferVRam( tNoOpTag );
		~tIndexBufferVRam( );
		tIndexBufferVRam( const tIndexBufferVRam& other );
		tIndexBufferVRam& operator=( const tIndexBufferVRam& other );

		///
		/// \brief Allocate the vram index buffer.
		void fAllocate( const tDevicePtr& device, const tIndexFormat& format, u32 numIndices, u32 numPrimitives, u32 allocFlags );

		///
		/// \brief "Fake" allocate so that we can save to binary load-in-place stream.
		void fPseudoAllocate( const tIndexFormat& format, u32 numIndices, u32 numPrimitives, u32 allocFlags );

		///
		/// \brief Allocate the vram index buffer following a load-in-place operation. If you don't know what
		/// this means, then you shouldn't be using it.
		void fAllocateInPlace( const tDevicePtr& device, const Sig::byte* verts );

		///
		/// \brief Deallocate the vram index buffer.
		void fDeallocate( );

		///
		/// \brief Deallocate the vram index buffer prior to an "unload-in-place". If you don't know what
		/// this means, then you shouldn't be using it.
		void fDeallocateInPlace( );

		///
		/// \brief Access the buffer's format object.
		inline const tIndexFormat& fIndexFormat( ) const { return mFormat; }

		///
		/// \brief Query for the number of indices.
		inline u32 fIndexCount( ) const { return mNumIndices; }

		///
		/// \brief Query for the number of primitives (i.e., triangles, lines, etc.)
		inline u32 fPrimitiveCount( ) const { return mNumPrimitives; }

		///
		/// \brief Query the underlying buffer size in bytes.
		inline u32 fBufferSize( ) const { return mNumIndices * mFormat.mSize; }

		///
		/// \brief Access the platform-specific geometry buffer handle
		inline tPlatformHandle fPlatformHandle( ) const { return mPlatformHandle; }

		///
		/// \brief Copy data to gpu
		void fBufferData( const void* data, u32 numIndices, u32 indexOffset = 0 );

		///
		/// \brief Lock a region of the index buffer. Defaults to locking the whole thing.
		/// \todo accept lock flags (discard, read-only, etc)
		Sig::byte* fDeepLock( );
		Sig::byte* fQuickLock( u32 startIndex = 0, u32 numIndices = ~0 );

		///
		/// \brief Unlock a previously locked region of the index buffer. You should pass
		/// the same pointer that was returned by a previous call to fLock.
		void fDeepUnlock( );
		void fQuickUnlock( Sig::byte* region );

		///
		/// \brief Applies the index buffer as the currently active rendering buffer.
		void fApply( const tDevicePtr& device ) const;

		///
		/// \brief Renders the primitives defined by this index buffer and contained in 
		/// the specified geometry buffer.
		void fRender( const tDevicePtr& device, u32 vertexCount, u32 baseVertexIndex, u32 primCount, u32 baseIndexIndex, tIndexFormat::tPrimitiveType primType ) const;

	private:
		
		void fCreateDeviceResource( tDevice* device );
		void fDestroyDeviceResource( );
		void fAllocateInternal( const tDevicePtr& device, const Sig::byte* ids = 0 );
		void fDeallocateInternal( );
	};

}}

#endif//__tIndexBufferVRam__
