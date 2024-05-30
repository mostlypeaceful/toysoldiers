//------------------------------------------------------------------------------
// \file tCloudStorageResourceLoader.hpp - 19 July 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tCloudStorageResourceLoader__
#define __tCloudStorageResourceLoader__
#include "tResourceLoader.hpp"
#include "tAsyncFileDownloader.hpp"
#include "Memory/tPool.hpp"

namespace Sig
{
	class iCloudStoragePtr;

	///
	/// \brief Loads resources from cloud storage
	class tCloudStorageResourceLoader : public tResourceLoader
	{
		define_dynamic_cast( tCloudStorageResourceLoader, tResourceLoader );
		define_class_pool_new_delete( tCloudStorageResourceLoader, 128 );
	protected:
		tAsyncFileDownloaderPtr mFileDownloader;

	public:
		tCloudStorageResourceLoader( tResource* res, const iCloudStoragePtr& cloudStorage );

		// virtuals overriden from tResourceLoader
		virtual void		fInitiate( );
		virtual tLoadResult	fUpdate( );

	protected:
		void fCleanupAfterSuccess( );
		void fCleanupAfterFailure( );
	};
}
#endif//__tCloudStorageResourceLoader__
