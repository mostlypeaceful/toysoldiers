//------------------------------------------------------------------------------
// \file tIndexBufferSysRam.hpp - 06 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tIndexBufferSysRam__
#define __tIndexBufferSysRam__
#include "tIndexFormat.hpp"

namespace Sig { namespace Gfx
{

	///
	/// \brief Encapsulates a system ram copy of an index buffer. Allows you to manipulate
	/// the index buffer without worrying too much about the format (you only worry about
	/// the format when you allocate the buffer).
	class base_export tIndexBufferSysRam
	{
		tIndexFormat			mFormat;
		tDynamicBuffer			mBuffer;

	public:

		///
		/// \brief Allocate the index buffer from the specified format.
		void fAllocate( const tIndexFormat& format, u32 numIndices );

		///
		/// \brief Deallocate the index buffer.
		void fDeallocate( );

		///
		/// \brief Find out if this buffer has been allocated already.
		inline b32 fAllocated( ) const { return mFormat.fValid( ) > 0 && mBuffer.fCount( ) > 0; }

		///
		/// \brief Access the index format object.
		inline const tIndexFormat& fIndexFormat( ) const { return mFormat; }

		///
		/// \brief Access the index count.
		inline u32 fIndexCount( ) const { return ( mFormat.mSize == 0 ) ? 0 : ( mBuffer.fCount( ) / mFormat.mSize ); }

		///
		/// \brief Access the underlying dynamic buffer.
		inline const tDynamicBuffer& fGetBuffer( ) const { return mBuffer; }

		///
		/// \brief Endian-swap a buffer that was acquired via fGetBuffer( ). I.e., this buffer
		/// must be identical to the underlying buffer in 'this'.
		void fEndianSwapBuffer( tDynamicBuffer& buffer, tPlatformId targetPlatform ) const;

		///
		/// \brief Set all indices. This will automatically convert the u32 srcIndices
		/// to the proper format (either u16 or u32). This is a little more efficient
		/// than using fSetIndex individually for each index.
		void fSetIndices( u32 start, const u32* srcIndices, u32 numSrcIndices );
		void fSetIndices( u32 start, u32 srcStart, const tIndexBufferSysRam & src, u32 numSrcIndices );

		///
		/// \brief Copy the indices out to an array of u32. Auto-converts from internal format
		///	\assumes destIndices has at least numSrcIndices space
		void fGetIndices( u32 start, u32 * destIndices, u32 numSrcIndices ) const;

		///
		/// \brief Set the 'ith' index value.
		/// \param value Expected to be <= the maximum allowable index value for
		/// the current index format. asserts at run time if this is not so.
		inline void fSetIndex( u32 ithIndex, u32 value )
		{
			sigassert( value <= mFormat.mMaxValue );
			Sig::byte* pIdx = mBuffer.fBegin( ) + ithIndex * mFormat.mSize;

			if( mFormat.mStorageType == tIndexFormat::cStorageU16 )
				*( u16* )pIdx = ( u16 )value;
			else if( mFormat.mStorageType == tIndexFormat::cStorageU32 )
				*( u32* )pIdx = ( u32 )value;
			else
			{
				sigassert( !"invalid index format" );
			}
		}

		///
		/// \brief Get the 'ith' index value.
		inline u32 fGetIndex( u32 ithIndex ) const
		{
			const Sig::byte* pIdx = mBuffer.fBegin( ) + ithIndex * mFormat.mSize;

			if( mFormat.mStorageType == tIndexFormat::cStorageU16 )
				return ( u32 )*( u16* )pIdx;
			else if( mFormat.mStorageType == tIndexFormat::cStorageU32 )
				return ( u32 )*( u32* )pIdx;

			sigassert( !"invalid index format" );
			return ~0;
		}

	};

}}

#endif//__tIndexBufferSysRam__

