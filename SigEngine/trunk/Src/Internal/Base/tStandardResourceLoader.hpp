#ifndef __tStandardResourceLoader__
#define __tStandardResourceLoader__
#include "tResourceLoader.hpp"
#include "tAsyncFileReader.hpp"
#include "Memory/tPool.hpp"

namespace Sig
{

	///
	/// \brief TODO document
	class /*base_export */tStandardResourceLoader : public tResourceLoader
	{
		define_dynamic_cast( tStandardResourceLoader, tResourceLoader );
		define_class_pool_new_delete( tStandardResourceLoader, 128 );
	protected:
		tAsyncFileReaderPtr mFileReader;

	public:
		tStandardResourceLoader( tResource* res );

		// virtuals overriden from tResourceLoader
		virtual void		fInitiate( );
		virtual tLoadResult	fUpdate( );

		// newly introduced virtuals
		virtual void		fOnOpenSuccess( );
		virtual void		fOnOpenFailure( );
		virtual void		fOnReadSuccess( );
		virtual void		fOnReadFailure( );
		virtual void		fCleanupAfterSuccess( );
		virtual void		fCleanupAfterFailure( );
	};

}

#endif//__tStandardResourceLoader__
