//------------------------------------------------------------------------------
// \file tGameArchiveLoad.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGameArchiveLoad__
#define __tGameArchiveLoad__

namespace Sig
{
	/// \class tGameArchiveLoad
	/// \brief A simple flat deserializer for loading game objects from a memory blob.
	class base_export tGameArchiveLoad : public tGameArchive
	{
	public:
		/// \brief Prepare to deserialize the buffer.
		/// N.B.: This object will NOT take ownership of this data!
		/// If decryption is necessary, it will copy the data.
		tGameArchiveLoad( const Sig::byte* data, u32 dataLen );

		/// \brief Prepare to deserialize the buffer.
		/// N.B.: This object will NOT take ownership of this data!
		/// If decryption is necessary, it will copy the data.
		/// Override tFilePathPtr deserialization with filePathArchiver.
		tGameArchiveLoad( const Sig::byte* data, u32 dataLen, tFilePathArchiver* filePathArchiver );



		/// \brief Look at the header version before the archive is fully read either for sanity checking or
		/// for simultaneously supporting multiple archive versions.
		u32 fPeekHeaderVersion( );

		/// \brief Deserialize a header and root object from the archive.
		template< class t > void fLoad( t& object );

		/// \brief Precondition: mSafeToSaveLoad.
		/// 
		/// Generally called by other fSaveLoad methods to implement the recursive deserialization of the
		/// root object.  See fLoad to start deserializing the actual root object, which will
		/// 'prime' the archive for the duration of the load process.
		template< class t > void fSaveLoad( t& object );



		/// Restart the archive read.
		void fResetReadPos( );

		/// Get raw binary data out of the archive.  Advances the read position, or sets fFailed( )
		void fGet( void* data, u32 numBytes );

		/// \brief Get the current read position within the buffer.
		const ::Sig::byte* fCurrentPos( ) const;

		/// \brief How many more bytes are available after the current position.
		u32 fBytesRemaining() const;

		/// \brief Advance the read position by numBytes, or if fewer bytes than that are available, fFail( ).
		void fAdvance( u32 numBytes );

		/// Does this array alloc make sense / appear unmalicious?  If not, fFail( ).  Returns !fFailed( ).
		b32 fSanityCheckArrayAlloc( u32 typeSize, u32 arrayCount );

		/// Does this array alloc make sense / appear unmalicious?  If not, fFail( ).  Returns !fFailed( ).
		template< class t > b32 fSanityCheckArrayAlloc( u32 arrayCount );



		/// \brief Deobfuscate the buffer.  Returns true if decryption was successful.
		b32 fDecrypt( );


		/// \brief What platform saved this archive in the first place?
		tPlatformId fSavedPlatform( ) const { return (tPlatformId)mHeader.mPlatform; }

	private:
		u32							mReadPos;			///< Index into mData of the current read position
		const byte*					mData;				///< Points at the original data passed in via ctor, or mDecryptedBuffer if fDecrypt( ) was called.
		u32							mDataLen;			///< Length in bytes of mData
		tGrowableArray<Sig::byte>	mDecryptedBuffer;	///< A decrypted version of 
	};
}

#include "tGameArchiveLoad.impl.hpp"

#endif //ndef __tGameArchiveLoad__
