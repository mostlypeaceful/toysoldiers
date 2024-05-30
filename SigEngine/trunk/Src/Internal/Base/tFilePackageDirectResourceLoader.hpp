//------------------------------------------------------------------------------
// \file tFilePackageDirectResourceLoader.hpp - 04 Oct 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tFilePackageDirectResourceLoader__
#define __tFilePackageDirectResourceLoader__
#include "tDirectResourceLoader.hpp"

namespace Sig
{
	class tFilePackageDirectResourceLoader : public tDirectResourceLoader
	{
		define_dynamic_cast( tFilePackageDirectResourceLoader, tDirectResourceLoader );

	private:
		u32 mFileOffset;
		u32 mFileSizeActual;
		u32 mFileSizeUncompressed;
		b32 mDecompress;

	public:
		tFilePackageDirectResourceLoader(
			tResource* resource,
			const tAsyncFileReaderPtr& fileReader,
			u32 headerSizeOffset,
			u32 fileOffset, 
			u32 fileSizeActual,
			u32 fileSizeUncompressed, 
			b32 decompress );

		//------------------------------------------------------------------------------
		// tResourceLoader Overrides
		//------------------------------------------------------------------------------
		virtual void		fInitiate( );

	protected:

		virtual void		fOnOpenSuccess( );
		virtual void		fOnReadHeaderSizeSuccess( );

		virtual tAsyncFileReaderPtr fCreateChildReaderInternal( );
	};
}

#endif//__tFilePackageDirectResourceLoader__
