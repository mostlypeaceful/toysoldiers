#ifndef __tVertexFormatVRam__
#define __tVertexFormatVRam__
#include "tVertexFormat.hpp"


namespace Sig { namespace Gfx
{
	class tDevicePtr;

	///
	/// \brief Encapsulates a vram vertex format object.
	class base_export tVertexFormatVRam : public tVertexFormat
	{
		declare_reflector( );
	public:
		typedef Sig::byte* tPlatformHandle;
	private:
		tPlatformHandle	mPlatformHandle;
	public:
		tVertexFormatVRam( );
		tVertexFormatVRam( tNoOpTag );
		tVertexFormatVRam( const tDevicePtr& device, const tVertexFormat& vtxFormat );
		explicit tVertexFormatVRam( const tVertexFormat& vtxFormat );
		~tVertexFormatVRam( );

		tVertexFormatVRam( const tVertexFormatVRam& other );
		tVertexFormatVRam& operator=( const tVertexFormatVRam& other );

		///
		/// \brief Allocate the vram vertex format object.
		void fAllocate( const tDevicePtr& device, const tVertexFormat& vtxFormat );

		///
		/// \brief "Fake" allocate so that we can save to binary load-in-place stream.
		void fPseudoAllocate( const tVertexFormat& vtxFormat );

		///
		/// \brief Allocate the vram vertex format following a load-in-place operation. If you don't know what
		/// this means, then you shouldn't be using it.
		void fAllocateInPlace( const tDevicePtr& device );

		///
		/// \brief Relocate the vram vertex format following a load-in-place operation. If you don't know what
		/// this means, then you shouldn't be using it.
		void fRelocateInPlace( ptrdiff_t delta );

		///
		/// \brief Deallocate the vram vertex format object.
		void fDeallocate( );

		///
		/// \brief Deallocate the vram vertex format prior to an "unload-in-place". If you don't know what
		/// this means, then you shouldn't be using it.
		void fDeallocateInPlace( );

		///
		/// \brief Access the platform-specific geometry format handle
		inline tPlatformHandle fPlatformHandle( ) const { return mPlatformHandle; }

		///
		/// \brief See if it's already been allocated
		inline b32 fAllocated( ) const { return mPlatformHandle!=0; }

		///
		/// \brief Applies the vertex format as the currently active rendering format.
		void fApply( const tDevicePtr& device ) const;

	private:

		void fReset( const tVertexElement* elems, u32 numElems );

		void fAllocateInternal( const tDevicePtr& device );
		void fDeallocateInternal( );
	};

}}


#endif//__tVertexFormatVRam__

