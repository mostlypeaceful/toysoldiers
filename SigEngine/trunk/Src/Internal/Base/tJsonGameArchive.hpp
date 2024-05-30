//------------------------------------------------------------------------------
// \file tJsonGameArchive.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonGameArchive__
#define __tJsonGameArchive__

#include "tGameArchiveMode.hpp"
#include "tGameArchiveHeader.hpp"

namespace Sig
{
	/// \class	tJsonGameArchive
	///
	/// \brief	Base class of tJsonGameArchive{Save,Load}, this implements common functionality mimicing
	///			tGameArchive but working with plain JSON data rather than encrypted binary data.
	///
	class base_export tJsonGameArchive
	{
		declare_uncopyable( tJsonGameArchive );

	public:
		virtual ~tJsonGameArchive( );

		tGameArchiveMode fMode( ) const;				///< Is this archive used for reading or writing?
		b32 fFailed( ) const;							///< Should (de)serialization abort due to failed preconditions?
		const tGameArchiveHeader& fHeader( ) const;		///< Returns the (de)serialized header.

		/// \brief	Sets fFailed( ).  Use to indicate some precondition of (de)serialization failed and that
		///			operations should abort due to bad data etc. instead of potentially crashing the program.
		void fFail( );

	protected:
		/// \brief	Initialize base state from derived class.
		/// \param	mode	Indicates if this is a reader or a writer of JSON.
		explicit tJsonGameArchive( tGameArchiveMode mode );

	protected:
		tGameArchiveMode		mMode;				///< Indicates if this a reading archive or a writing archive.
		b32						mFailed;			///< Set when part of (de)serialization fails.  Never unset.
		b32						mSafeToSaveLoad;	///< Set during the execution of a derived class fSave or fLoad, indicating fSaveLoad is sane to call.
		tGameArchiveHeader		mHeader;			///< Stores the header to be serialized or the header that was unserialized, depending on context.
	};
}

#endif //ndef __tJsonGameArchive__
