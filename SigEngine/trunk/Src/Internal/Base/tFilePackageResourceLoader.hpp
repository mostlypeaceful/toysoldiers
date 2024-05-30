#ifndef __tFilePackageResourceLoader__
#define __tFilePackageResourceLoader__
#include "tStandardResourceLoader.hpp"

namespace Sig
{
	///
	/// \brief TODO document
	class /*base_export*/ tFilePackageResourceLoader : public tStandardResourceLoader
	{
		define_dynamic_cast( tFilePackageResourceLoader, tStandardResourceLoader );
		define_class_pool_new_delete( tFilePackageResourceLoader, 128 );
	private:
		u32 mFileOffset;
		u32 mFileSizeActual;
		u32 mFileSizeUncompressed;
		b32 mDecompress;

	public:
		tFilePackageResourceLoader(
			tResource* resource,
			const tAsyncFileReaderPtr& fileReader,
			u32 fileOffset, 
			u32 fileSizeActual,
			u32 fileSizeUncompressed, 
			b32 decompress );

		virtual void		fInitiate( );
		virtual void		fOnOpenSuccess( );
	};
}


#endif//__tFilePackageResourceLoader__

