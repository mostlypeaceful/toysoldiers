//------------------------------------------------------------------------------
// \file tGameArchiveMode.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGameArchiveMode__
#define __tGameArchiveMode__

namespace Sig
{
	/// \class	tGameArchiveMode
	/// \brief	Whether or not a given tGameArchive or equivalent loads from file or saves to file.
	enum tGameArchiveMode
	{
		cGameArchiveModeSave,	///< Saves to file or memory.
		cGameArchiveModeLoad,	///< Loads from file or memory.
	};
}

#endif //ndef __tGameArchiveMode__
