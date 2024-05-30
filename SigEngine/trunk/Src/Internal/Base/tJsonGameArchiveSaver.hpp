//------------------------------------------------------------------------------
// \file tJsonGameArchiveSaver.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonGameArchiveSaver__
#define __tJsonGameArchiveSaver__

#include "tJsonGameArchive.hpp"
#include "tJsonWriter.hpp"

namespace Sig
{
	/// \class	tJsonGameArchiveSaver
	/// \brief	A JSON-encoding equivalent to tGameArchiveSave, traverses objects with fSaveLoad defined and
	///			serializes them naively as JSON.
	class base_export tJsonGameArchiveSaver : public tJsonGameArchive
	{
	public:
		/// \brief	Prepare to serialize as JSON to a buffer.
		tJsonGameArchiveSaver( );

		/// \brief	Kick off serialization of root object.
		template< class t > void fSave( t& object, u8 version = 0 );

		/// \brief Precondition: mSafeToSaveLoad.
		/// 
		/// Generally called by other fSaveLoad methods to implement the recursive serialization of the
		/// root object.  See fSave to start serializing the actual header and root object, which will
		/// 'prime' the archive for the duration of the save process.
		template< class t > void fSaveLoad( t& object );



		/// \brief	JSON SAVE SPECIFIC: Get the underlying tJsonWriter
		tJsonWriter& fWriter( );

		/// \brief	JSON SAVE SPECIFIC: Get the underlying tJsonWriter
		const tJsonWriter& fWriter( ) const;



		/// \brief 	Disown / transfer ownership from the internal buffer to "buffer", or copy it if it cannot
		///			be disowned.  Note that this buffer does NOT include the '\0' terminal!
		///
		/// \param	buffer	The new owner of the internal buffer.
		void fTransferBufferTo( tGrowableBuffer& target );

	private:
		tJsonWriter	mWriter;
	};
}

#include "tJsonGameArchiveSaver.impl.hpp"

#endif //ndef __tJsonGameArchiveSaver__
