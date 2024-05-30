//------------------------------------------------------------------------------
// \file tGameArchiveHeader.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2011-2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tGameArchiveHeader__
#define __tGameArchiveHeader__

namespace Sig
{
	struct base_export tGameArchiveHeader
	{
		u8 mPlatform;
		// ---------------------------------------------------------------------------------------------------
		// Everything above this line MUST be single bytes, endian swap info is not know on binary archivers
		// until mPlatform is read.
		u8 mVersion;
			
		tGameArchiveHeader( u8 platform, u8 version )
			: mPlatform( platform )
			, mVersion( version )
		{
		}

		template< class tArchive >
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mPlatform );
			if( mPlatform >= cPlatformLastPlusOne )
			{
				log_warning( "mPlatform ("<<mPlatform<<") >= cPlatformLastPlusOne ("<<cPlatformLastPlusOne<<") -- a corrupted/fuzzed profile or a new unknown platform" );
				archive.fFail();
				return;
			}
			archive.fSaveLoad( mVersion );
		}
	};
}

#endif //ndef __tGameArchiveHeader__
