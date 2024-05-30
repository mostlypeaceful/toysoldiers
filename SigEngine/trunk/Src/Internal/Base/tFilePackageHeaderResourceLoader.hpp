#ifndef __tFilePackageHeaderResourceLoader__
#define __tFilePackageHeaderResourceLoader__
#include "tStandardResourceLoader.hpp"

namespace Sig
{

	///
	/// \brief Handles loading the initial header table part of a file package.
	class tFilePackageHeaderResourceLoader : public tStandardResourceLoader
	{
		define_class_pool_new_delete( tFilePackageHeaderResourceLoader, 128 );
	private:
		u32 mHeaderSize;

	public:
		tFilePackageHeaderResourceLoader( tResource* res, const tAsyncFileReaderPtr& fileReader );
		virtual void				fInitiate( );
		virtual void				fOnOpenSuccess( );
		virtual void				fOnReadSuccess( );
		virtual void				fCleanupAfterSuccess( );
	};
}


#endif//__tFilePackageHeaderResourceLoader__
