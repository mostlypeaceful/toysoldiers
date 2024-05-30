//------------------------------------------------------------------------------
// \file tGameArchiveSave.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGameArchiveSave__
#define __tGameArchiveSave__

namespace Sig
{
	/// \class tGameArchiveSave
	/// \brief A simple flat serializer for saving game objects to a memory blob.
	class base_export tGameArchiveSave : public tGameArchive
	{
	public:
		/// \brief Prepare to serialize to a buffer.
		tGameArchiveSave( );

		/// \brief Prepare to serialize to a buffer.
		/// Override tFilePathPtr deserialization with filePathArchiver.
		tGameArchiveSave( tFilePathArchiver* filePathArchiver );



		/// \brief Serialize a header and root object to the archive.
		template< class t > void fSave( t& object, u8 headerVersion = 0 );

		/// \brief Precondition: mSafeToSaveLoad.
		/// 
		/// Generally called by other fSaveLoad methods to implement the recursive serialization of the
		/// root object.  See fSave to start serializing the actual header and root object, which will
		/// 'prime' the archive for the duration of the save process.
		template< class t > void fSaveLoad( t& object );



		/// \brief BINARY SAVE SPECIFIC: Write raw bytes to the archive.
		void fPut( const void* data, u32 numBytes );

		/// \brief BINARY SAVE SPECIFIC: Get the written, possibly encrypted buffer.
		tGrowableArray<Sig::byte>& fBuffer( );

		/// \brief BINARY SAVE SPECIFIC: Get the written, possibly encrypted buffer.
		const tGrowableArray<Sig::byte>& fBuffer( ) const;



		/// \brief 	Disown / transfer ownership from the internal buffer to "buffer", or copy it if it cannot be disowned.
		/// \param	buffer	The new owner of the internal buffer.
		void fTransferBufferTo( tGrowableBuffer& buffer );

		/// \brief Lightly obfuscate the contents.
		///
		/// Note well that you should probably only call this once you're done writing to the archive, or
		/// you'll end up with a mix and match of different obfuscation levels.
		void fEncrypt( );

	private:
		tGrowableArray<Sig::byte> mBuffer;	///< Bytes accumulated so far, including the header, encrypted if fEncrypt called.
	};
}

#include "tGameArchiveSave.impl.hpp"

#endif //ndef __tGameArchiveSave__
