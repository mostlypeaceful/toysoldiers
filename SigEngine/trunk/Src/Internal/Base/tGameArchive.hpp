//------------------------------------------------------------------------------
// \file tGameArchive.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGameArchive__
#define __tGameArchive__

#include "tEncryption.hpp"
#include "tGameArchiveMode.hpp"
#include "tGameArchiveHeader.hpp"

namespace Sig
{
	class tFilePathArchiver;

	/// \class tGameArchive
	/// \brief The base class of tGameArchive{Load,Save}, this holds state of the simple flat (de)serializers.
	class base_export tGameArchive
	{
	public:
		/// \brief Initialize common game archive state to sane pre-load/save state.
		explicit tGameArchive( tGameArchiveMode mode );

		/// Query various current state
		tGameArchiveMode			fMode( ) const;
		const tGameArchiveHeader&	fHeader( ) const;
		tFilePathArchiver*			fFilePathArchiver( ) const;
		b32							fFailed( ) const;


		/// \brief Precondition: mSafeToSaveLoad.
		/// 
		/// Generally called by other fSaveLoad methods to implement the recursive (de)serialization of the
		/// root object.  See tGameArchiveLoad::fLoad or tGameArchiveSave::fSave to start (de)serializing the
		/// actual root object, which will actually 'prime' the archive for the duration of the save or load.
		template< class t > void fSaveLoad( t& object );


		/// \brief Start bailing out of (de)serialization due to errors instead of crashing.
		void fFail( );

	protected:
		tGameArchiveMode	mMode;				///< Are we a tGameArchiveSave or tGameArchiveLoad?
		b32					mFailed;			///< Did some portion of (de)serialization fail?
		b32					mSafeToSaveLoad;	///< Are we within a derived class's fLoad(root) or fSave(root)?
		tGameArchiveHeader	mHeader;			///< The header of the data we're reading or writing.
		tFilePathArchiver*	mFilePathArchiver;	///< Overrides tFilePathPtr (de)serialization implementation.
	};
}

#include "tGameArchive.impl.hpp"
#include "tEntitySaveData.hpp"
#include "tFilePathArchiver.hpp"

#endif//__tGameArchive__
