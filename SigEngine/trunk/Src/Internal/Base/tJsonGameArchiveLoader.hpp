//------------------------------------------------------------------------------
// \file tJsonGameArchiveLoader.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonGameArchiveLoader__
#define __tJsonGameArchiveLoader__

#include "tJsonGameArchive.hpp"
#include "tJsonReader.hpp"

namespace Sig
{
	/// \class	tJsonGameArchiveLoader
	/// \brief	A JSON-decoding equivalent to tGameArchiveLoad, traverses objects with fSaveLoad defined and
	///			deserializes them naively from JSON.
	class base_export tJsonGameArchiveLoader : public tJsonGameArchive
	{
	public:
		/// \brief	Prepare to deserialize from a buffer.
		///			NOTE WELL: This class is NOT guaranteed to take a copy of this data!
		/// \param	data		The Low-ASCII/UTF8 (<0x80) encoded JSON to deserialize.
		/// \param	dataLength	The number of bytes & characters to be read from data
		tJsonGameArchiveLoader( const byte* data, u32 dataLength );

		/// \brief	Peek at the format version before deserializing the bulk of the object.
		/// \return	The version.
		u32 fPeekHeaderVersion( );

		/// \brief	Kick off deserialization of root object.
		template< class t > void fLoad( t& object );

		/// \brief Precondition: mSafeToSaveLoad.
		/// 
		/// Generally called by other fSaveLoad methods to implement the recursive deserialization of the
		/// root object.  See fLoad to start deserializing the actual root object, which will
		/// 'prime' the archive for the duration of the load process.
		template< class t > void fSaveLoad( t& object );

		/// \brief	JSON LOAD SPECIFIC: Get the underlying tJsonReader.
		tJsonReader& fReader( );

		/// \brief	JSON LOAD SPECIFIC: Get the underlying tJsonReader.
		const tJsonReader& fReader( ) const;

		/// \brief	Does this array alloc make sense / appear unmalicious?  If not, fFail( ).  Returns !fFailed( ).
		b32 fSanityCheckArrayAlloc( u32 typeSize, u32 arrayCount );

		/// \brief	Does this array alloc make sense / appear unmalicious?  If not, fFail( ).  Returns !fFailed( ).
		template< class t > b32 fSanityCheckArrayAlloc( u32 arrayCount );

		/// \brief	What platform saved this archive in the first place?  Shouldn't be too necessary for JSON...
		tPlatformId fSavedPlatform( ) const { return (tPlatformId)mHeader.mPlatform; }

	private:
		/// \brief	Read the preamble JSON describing version and platform into mHeader.
		void fReadHeader( tJsonReader& reader );

		/// \brief	Read the tail of the object containing mHeader and the primary data object.
		void fReadFooter( tJsonReader& reader );

		tJsonReader		mReader;
		u32				mTotalJsonSize;
	};
}

#include "tJsonGameArchiveLoader.impl.hpp"

#endif //ndef __tJsonGameArchiveLoader__
